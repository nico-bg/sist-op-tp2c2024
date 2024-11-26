#include <kernel-utils/syscalls.h>

static void transicion_exec_a_exit();
static void transicion_exec_a_blocked();
static void transicion_blocked_a_ready(t_tcb* hilo);
static void transicion_blocked_a_exit(t_tcb* hilo);
static void solicitar_inicializacion_hilo_a_memoria(t_tcb* hilo);
static t_pcb* buscar_proceso(uint32_t pid);
static bool encontrar_proceso_por_pid_auxiliar(void* elemento);
static bool existe_tid_en_lista(void* elemento);
static t_tcb* buscar_hilo_en_proceso(t_pcb* proceso, uint32_t tid);
static bool encontrar_mutex_proceso_en_ejecucion(void* elemento);
static void esperar_respuesta_dump_memory(void* args);
static bool esta_mutex_asignado_a_tid(void* elemento);
static op_code pedir_finalizacion_hilo_a_memoria(t_tcb* hilo);
static op_code pedir_finalizacion_proceso_a_memoria(t_pcb* proceso);
static void remover_hilo_de_ready_o_blocked(t_tcb* hilo);

uint32_t pid_auxiliar;
uint32_t tid_auxiliar;
char* recurso_buscado;

void syscall_finalizar_hilo()
{
    pthread_mutex_lock(&mutex_estado_exec);
    t_tcb* hilo = estado_exec;
    pthread_mutex_unlock(&mutex_estado_exec);

    transicion_exec_a_exit();

    op_code respuesta = pedir_finalizacion_hilo_a_memoria(hilo);
    t_pcb* proceso = buscar_proceso(hilo->pid_padre);

    if(respuesta != OPERACION_CONFIRMAR) {
        log_debug(logger_debug, "Respuesta desconocida al finalizar hilo. Cod: %d, PID: %d, TID: %d", respuesta, hilo->pid_padre, hilo->tid);
        abort();
    }

    // Desvinculo el hilo de la lista de tids del proceso
    tid_auxiliar = hilo->tid;
    list_remove_by_condition(proceso->tids, existe_tid_en_lista);

    // Busco los mutex que tiene asignados el hilo a finalizar
    tid_auxiliar = hilo->tid;
    t_list* mutex_asignados = list_filter(proceso->mutex, esta_mutex_asignado_a_tid);

    // Liberamos todos los mutex/recursos que tiene asignados el hilo a finalizar
    while(!list_is_empty(mutex_asignados)) {
        t_mutex* mutex_a_liberar = list_get(mutex_asignados, 0);
        list_remove_element(mutex_asignados, mutex_a_liberar);

        bool hay_hilos_bloqueados = !queue_is_empty(mutex_a_liberar->hilos_bloqueados);

        if(hay_hilos_bloqueados) {
            // Asigno el mutex al siguiente hilo bloqueado (FIFO)
            t_tcb* hilo_a_desbloquear = queue_pop(mutex_a_liberar->hilos_bloqueados);
            mutex_a_liberar->esta_libre = false;
            mutex_a_liberar->hilo_asignado = hilo_a_desbloquear;

            // Desbloqueo el hilo pasándolo a READY
            transicion_blocked_a_ready(hilo_a_desbloquear);
        } else {
            // Libero el mutex
            mutex_a_liberar->esta_libre = true;
            mutex_a_liberar->hilo_asignado = NULL;
        }
    }

    // Liberamos todos los hilos que estaban esperando a que este hilo finalice
    while(!list_is_empty(hilo->hilos_bloqueados)) {
        t_tcb* hilo_a_desbloquear = list_get(hilo->hilos_bloqueados, 0);
        list_remove_element(hilo->hilos_bloqueados, hilo_a_desbloquear);

        transicion_blocked_a_ready(hilo_a_desbloquear);
    }

    log_info(logger, "## (%d:%d) Finaliza el hilo", hilo->pid_padre, hilo->tid);
}

void syscall_crear_hilo(char* archivo_pseudocodigo, uint32_t prioridad)
{
    // Nos conectamos a la Memoria
    int fd_memoria = crear_conexion_a_memoria();

    // Guardamos en una variable auxiliar el hilo que invocó la syscall (Es el que está en estado EXEC)
    pthread_mutex_lock(&mutex_estado_exec);
    t_tcb* hilo_invocador = estado_exec;
    pthread_mutex_unlock(&mutex_estado_exec);

    // Buscamos el PCB correspondiente al hilo que invocó la syscall
    t_pcb* proceso_invocador = buscar_proceso(hilo_invocador->pid_padre);

    // Inicializamos un nuevo hilo con los datos que recibimos y con TID incremental
    t_tcb* nuevo_hilo = malloc(sizeof(t_tcb));
    nuevo_hilo->nombre_archivo = string_duplicate(archivo_pseudocodigo);
    nuevo_hilo->pid_padre = hilo_invocador->pid_padre;
    nuevo_hilo->prioridad = prioridad;
    nuevo_hilo->tid = proceso_invocador->ultimo_tid + 1;
    nuevo_hilo->hilos_bloqueados = list_create();

    // Le pedimos a la Memoria que inicialice los contextos de ejecución para el hilo creado
    solicitar_inicializacion_hilo_a_memoria(nuevo_hilo);

    // Actualizamos los datos del proceso asociado
    proceso_invocador->ultimo_tid += 1;

    uint32_t* tid_nuevo_hilo = malloc(sizeof(uint32_t)); // Creamos una copia dinámica para poder guardarlo en la lista
    *tid_nuevo_hilo = nuevo_hilo->tid;

    list_add(proceso_invocador->tids, tid_nuevo_hilo);

    // Agregamos el nuevo hilo al estado READY para que pueda ser planificado
    pthread_mutex_lock(&mutex_estado_ready);
    list_add(estado_ready, nuevo_hilo);
    pthread_mutex_unlock(&mutex_estado_ready);

    sem_post(&semaforo_estado_ready);

    log_info(logger, "## (%d:%d) Se crea el Hilo - Estado: READY", nuevo_hilo->pid_padre, nuevo_hilo->tid);

    close(fd_memoria);
}

/**
 * @brief Bloquea el hilo en ejecución hasta que el hilo del TID recibido finalice
 * @return Devuelve true solo si el hilo en ejecución fue correctamente bloqueado
 */
bool syscall_esperar_hilo(uint32_t tid)
{
    pthread_mutex_lock(&mutex_estado_exec);
    t_tcb* hilo_en_ejecucion = estado_exec;
    pthread_mutex_unlock(&mutex_estado_exec);

    t_pcb* proceso = buscar_proceso(hilo_en_ejecucion->pid_padre);

    t_tcb* hilo_a_esperar = buscar_hilo_en_proceso(proceso, tid);

    if(hilo_a_esperar != NULL) {
        // Agrego el hilo que invocó la syscall en la lista de bloqueados del hilo que se quiere esperar (asociado al TID recibido)
        list_add(hilo_a_esperar->hilos_bloqueados, hilo_en_ejecucion);
        transicion_exec_a_blocked();

        log_info(logger, "## (%d:%d) - Bloqueado por: PTHREAD_JOIN", hilo_en_ejecucion->pid_padre, hilo_en_ejecucion->tid);

        return true;
    }

    return false;
}

void syscall_crear_proceso(char* archivo_pseudocodigo, uint32_t tamanio_proceso, uint32_t prioridad)
{
    crear_proceso(archivo_pseudocodigo, tamanio_proceso, prioridad);
}

void syscall_finalizar_proceso()
{
    pthread_mutex_lock(&mutex_estado_exec);
    bool es_hilo_principal = estado_exec->tid == 0;
    t_tcb* hilo_principal = estado_exec;
    pthread_mutex_unlock(&mutex_estado_exec);

    if(!es_hilo_principal) {
        log_error(logger_debug, "Error al finalizar proceso, el hilo que invocó la syscall no es TID 0");
    }

    transicion_exec_a_exit();

    t_pcb* proceso = buscar_proceso(hilo_principal->pid_padre);
    op_code respuesta = pedir_finalizacion_proceso_a_memoria(proceso);

    if(respuesta != OPERACION_CONFIRMAR) {
        log_debug(logger_debug, "Respuesta desconocida al finalizar proceso. Cod: %d, PID: %d", respuesta, proceso->pid);
        abort();
    }

    // Saco el proceso de la lista de procesos
    pthread_mutex_lock(&mutex_lista_procesos);
    list_remove_element(lista_procesos, proceso);
    pthread_mutex_unlock(&mutex_lista_procesos);

    // Itero cada TID del proceso para finalizarlos, liberando los hilos bloqueados
    while(!list_is_empty(proceso->tids)) {
        int indice_ultimo_tid = list_size(proceso->tids) - 1;
        uint32_t* tid_a_liberar = (uint32_t*) list_get(proceso->tids, indice_ultimo_tid);

        t_tcb* hilo_a_liberar = buscar_hilo_en_proceso(proceso, *tid_a_liberar);

        list_remove_element(proceso->tids, tid_a_liberar);

        // Paso a READY todos los hilos que esperaban a que `hilo_a_liberar` finalice
        while(!list_is_empty(hilo_a_liberar->hilos_bloqueados)) {
            t_tcb* hilo_a_desbloquear = list_get(hilo_a_liberar->hilos_bloqueados, 0);
            list_remove_element(hilo_a_liberar->hilos_bloqueados, hilo_a_desbloquear);

            transicion_blocked_a_ready(hilo_a_desbloquear);
        }

        // Elimino al hilo del estado en que se encuentre y lo  destruyo
        remover_hilo_de_ready_o_blocked(hilo_a_liberar);
        destruir_tcb(hilo_a_liberar);
    }

    log_info(logger, "## Finaliza el proceso %d", proceso->pid);
    sem_post(&semaforo_memoria_suficiente);
    destruir_pcb(proceso);
}

void syscall_crear_mutex(char* recurso)
{
    pthread_mutex_lock(&mutex_estado_exec);
    uint32_t pid_en_ejecucion = estado_exec->pid_padre;
    pthread_mutex_unlock(&mutex_estado_exec);

    t_pcb* proceso = buscar_proceso(pid_en_ejecucion);

    t_mutex* nuevo_mutex = malloc(sizeof(t_mutex));

    nuevo_mutex->hilos_bloqueados = queue_create();
    nuevo_mutex->esta_libre = true;
    nuevo_mutex->recurso = string_duplicate(recurso);
    nuevo_mutex->hilo_asignado = NULL;

    list_add(proceso->mutex, nuevo_mutex);
}

/**
 * @brief Si el recurso está libre, se lo asigna al hilo en ejecución, en caso contrario lo bloquea
 * @return Devuelve true si el hilo pudo asignarse el mutex
 */
bool syscall_bloquear_mutex(char* recurso)
{
    recurso_buscado = recurso;

    pthread_mutex_lock(&mutex_estado_exec);
    t_tcb* hilo_en_ejecucion = estado_exec;
    pthread_mutex_unlock(&mutex_estado_exec);

    t_pcb* proceso = buscar_proceso(hilo_en_ejecucion->pid_padre);

    t_mutex* mutex = list_find(proceso->mutex, encontrar_mutex_proceso_en_ejecucion);

    if(mutex->esta_libre) {
        mutex->esta_libre = false;
        mutex->hilo_asignado = hilo_en_ejecucion;

        return true;
    } else {
        transicion_exec_a_blocked();
        queue_push(mutex->hilos_bloqueados, hilo_en_ejecucion);

        log_info(logger, "## (%d:%d) - Bloqueado por: MUTEX", hilo_en_ejecucion->pid_padre, hilo_en_ejecucion->tid);

        return false;
    }
}

void syscall_desbloquear_mutex(char* recurso)
{
    recurso_buscado = recurso;

    pthread_mutex_lock(&mutex_estado_exec);
    t_tcb* hilo_en_ejecucion = estado_exec;
    pthread_mutex_unlock(&mutex_estado_exec);

    t_pcb* proceso = buscar_proceso(hilo_en_ejecucion->pid_padre);

    t_mutex* mutex = list_find(proceso->mutex, encontrar_mutex_proceso_en_ejecucion);

    bool correctamente_asignado = hilo_en_ejecucion->tid == mutex->hilo_asignado->tid;
    bool hay_mas_hilos_bloqueados = !queue_is_empty(mutex->hilos_bloqueados);

    if(correctamente_asignado && hay_mas_hilos_bloqueados) {
        // TODO: Evaluar si es necesario bloquear con mutex
        // ...porque se puede producir una RC con la liberación de hilos en EXIT
        t_tcb* hilo_a_desbloquear = queue_pop(mutex->hilos_bloqueados);

        mutex->hilo_asignado = hilo_a_desbloquear;
        transicion_blocked_a_ready(hilo_a_desbloquear);
    }

    if(correctamente_asignado && !hay_mas_hilos_bloqueados) {
        mutex->hilo_asignado = NULL;
        mutex->esta_libre = true;
    }
}

void syscall_io(uint32_t tiempo)
{
    pthread_mutex_lock(&mutex_estado_exec);
    t_tcb* hilo_en_ejecucion = estado_exec;
    pthread_mutex_unlock(&mutex_estado_exec);

    transicion_exec_a_blocked();

    t_solicitud_io* solicitud_io = malloc(sizeof(t_solicitud_io));
    solicitud_io->hilo = hilo_en_ejecucion;
    solicitud_io->tiempo = tiempo;

    pthread_mutex_lock(&mutex_io);
    queue_push(cola_io, solicitud_io);
    pthread_mutex_unlock(&mutex_io);

    log_info(logger, "## (%d:%d) - Bloqueado por: IO", hilo_en_ejecucion->pid_padre, hilo_en_ejecucion->tid);

    sem_post(&semaforo_io);
}

void syscall_dump_memory()
{
    // Obtenemos el hilo que se encuentra en estado EXEC
    pthread_mutex_lock(&mutex_estado_exec);
    t_tcb* hilo_en_ejecucion = estado_exec;
    pthread_mutex_unlock(&mutex_estado_exec);

    int fd_conexion = crear_conexion_a_memoria();

    // Guardamos los datos a enviar en la estructura correspondiente
    t_datos_dump_memory* datos_a_enviar = malloc(sizeof(t_datos_dump_memory));
    datos_a_enviar->pid = hilo_en_ejecucion->pid_padre;
    datos_a_enviar->tid = hilo_en_ejecucion->tid;

    // Empaquetamos y serializamos los datos junto con el código de operación
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = OPERACION_DUMP_MEMORY;
    paquete->buffer = serializar_datos_dump_memory(datos_a_enviar);
    t_buffer* paquete_serializado = serializar_paquete(paquete);

    send(fd_conexion, paquete_serializado->stream, paquete_serializado->size, 0);

    // Pasamos el hilo a BLOCKED
    transicion_exec_a_blocked();

    log_info(logger, "## (%d:%d) - Bloqueado por: DUMP_MEMORY", hilo_en_ejecucion->pid_padre, hilo_en_ejecucion->tid);

    t_args_esperar_respuesta_dump_memory* args_hilo = malloc(sizeof(t_args_esperar_respuesta_dump_memory));
    args_hilo->fd_conexion = fd_conexion;
    args_hilo->hilo = hilo_en_ejecucion;

    // Creamos un hilo para paralelizar la espera de la respuesta del DUMP_MEMORY
    pthread_t hilo_dump_memory;
    pthread_create(&hilo_dump_memory, NULL, (void*) esperar_respuesta_dump_memory, args_hilo);
    pthread_detach(hilo_dump_memory);

    // Liberamos la memoria de las estructuras utilizadas
    buffer_destroy(paquete_serializado);
    eliminar_paquete(paquete);
    destruir_datos_dump_memory(datos_a_enviar);
}

/* UTILIDADES */

static void transicion_exec_a_exit()
{
    // Desalojo el hilo del estado EXEC guardándolo en otra variable auxiliar
    pthread_mutex_lock(&mutex_estado_exec);
    t_tcb* hilo_a_exit = estado_exec;
    estado_exec = NULL;
    pthread_mutex_unlock(&mutex_estado_exec);

    // Agregamos el hilo al estado EXIT
    pthread_mutex_lock(&mutex_estado_exit);
    list_add(estado_exit, hilo_a_exit);
    pthread_mutex_unlock(&mutex_estado_exit);

    sem_post(&semaforo_estado_exit);
}

static void transicion_exec_a_blocked()
{
    // Desalojo el hilo del estado EXEC guardándolo en otra variable auxiliar
    pthread_mutex_lock(&mutex_estado_exec);
    t_tcb* hilo_a_blocked = estado_exec;
    estado_exec = NULL;
    pthread_mutex_unlock(&mutex_estado_exec);

    // Agregamos el hilo al estado BLOCKED
    pthread_mutex_lock(&mutex_estado_blocked);
    list_add(estado_blocked, hilo_a_blocked);
    pthread_mutex_unlock(&mutex_estado_blocked);
}

static void transicion_blocked_a_ready(t_tcb* hilo)
{
    // Saco el hilo del estado BLOCKED
    pthread_mutex_lock(&mutex_estado_blocked);
    list_remove_element(estado_blocked, hilo);
    pthread_mutex_unlock(&mutex_estado_blocked);

    // Agrego el hilo al estado READY
    pthread_mutex_lock(&mutex_estado_ready);
    list_add(estado_ready, hilo);
    pthread_mutex_unlock(&mutex_estado_ready);

    sem_post(&semaforo_estado_ready);
}

static void transicion_blocked_a_exit(t_tcb* hilo)
{
    pthread_mutex_lock(&mutex_estado_blocked);
    list_remove_element(estado_blocked, hilo);
    pthread_mutex_unlock(&mutex_estado_blocked);

    pthread_mutex_lock(&mutex_estado_exit);
    list_add(estado_exit, hilo);
    pthread_mutex_unlock(&mutex_estado_exit);
    sem_post(&semaforo_estado_exit);
}

static void solicitar_inicializacion_hilo_a_memoria(t_tcb* hilo)
{
    // Nos conectamos a la Memoria
    int fd_memoria = crear_conexion_a_memoria();

    // Serializamos los datos a enviar en un buffer
    t_datos_inicializacion_hilo* datos_a_enviar = malloc(sizeof(t_datos_inicializacion_hilo));
    datos_a_enviar->pid = hilo->pid_padre;
    datos_a_enviar->tid = hilo->tid;
    datos_a_enviar->archivo_pseudocodigo = string_duplicate(hilo->nombre_archivo);

    // Empaquetamos y serializamos los datos junto con el código de operación
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = OPERACION_CREAR_HILO;
    paquete->buffer = serializar_datos_inicializacion_hilo(datos_a_enviar);;
    t_buffer* paquete_serializado = serializar_paquete(paquete);

    send(fd_memoria, paquete_serializado->stream, paquete_serializado->size, 0);

    buffer_destroy(paquete_serializado);
    eliminar_paquete(paquete);
    destruir_datos_inicializacion_hilo(datos_a_enviar);
    close(fd_memoria);
}

static bool encontrar_proceso_por_pid_auxiliar(void* elemento)
{
    t_pcb* proceso = (t_pcb*) elemento;

    return proceso->pid == pid_auxiliar;
}

/**
 * @brief Devuelve la referencia al PCB de la `lista_procesos` cuyo `pid` coincida con el solicitado
 */
static t_pcb* buscar_proceso(uint32_t pid)
{
    pid_auxiliar = pid;
    pthread_mutex_lock(&mutex_lista_procesos);
    t_pcb* proceso_encontrado = list_find(lista_procesos, encontrar_proceso_por_pid_auxiliar);
    pthread_mutex_unlock(&mutex_lista_procesos);

    if(proceso_encontrado == NULL) {
        log_debug(logger_debug, "buscar_proceso: No se encontró el proceso con pid %d", pid);
        abort();
    }

    return proceso_encontrado;
}

static bool existe_tid_en_lista(void* elemento)
{
    uint32_t* tid = (uint32_t*) elemento;

    return *tid == tid_auxiliar;
}

static t_tcb* buscar_hilo_en_proceso(t_pcb* proceso, uint32_t tid)
{
    tid_auxiliar = tid;
    bool tid_existente = list_any_satisfy(proceso->tids, existe_tid_en_lista);

    if(tid_existente) {
        t_tcb* hilo_encontrado;

        // Buscamos el TID en la lista de estado READY
        pthread_mutex_lock(&mutex_estado_ready);
        hilo_encontrado = list_find(estado_ready, existe_tid_en_lista);
        pthread_mutex_unlock(&mutex_estado_ready);

        // Si no se encuentra el hilo en READY, lo buscamos en BLOCKED
        if(hilo_encontrado == NULL) {
            pthread_mutex_lock(&mutex_estado_blocked);
            hilo_encontrado = list_find(estado_blocked, existe_tid_en_lista);
            pthread_mutex_unlock(&mutex_estado_blocked);
        }

        return hilo_encontrado;
    } else {
        return NULL;
    }
}

static bool encontrar_mutex_proceso_en_ejecucion(void* elemento)
{
    t_mutex* mutex = (t_mutex*) elemento;

    bool encontrado = strcmp(mutex->recurso, recurso_buscado) == 0;

    return encontrado;
}

static void esperar_respuesta_dump_memory(void* args)
{
    t_args_esperar_respuesta_dump_memory* datos = (t_args_esperar_respuesta_dump_memory*) args;

    op_code respuesta = recibir_operacion(datos->fd_conexion);

    if(respuesta == OPERACION_CONFIRMAR) {
        transicion_blocked_a_ready(datos->hilo);
    } else {
        transicion_blocked_a_exit(datos->hilo);
    }

    close(datos->fd_conexion);
    free(args);
}

static bool esta_mutex_asignado_a_tid(void* elemento)
{
    t_mutex* mutex = (t_mutex*) elemento;

    return mutex->hilo_asignado->tid == tid_auxiliar;
}

static op_code pedir_finalizacion_hilo_a_memoria(t_tcb* hilo)
{
    int socket_memoria = crear_conexion_a_memoria();

    t_datos_finalizacion_hilo* datos = malloc(sizeof(t_datos_finalizacion_hilo));
    datos->pid = hilo->pid_padre;
    datos->tid = hilo->tid;

    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = OPERACION_FINALIZAR_HILO;
    paquete->buffer = serializar_datos_finalizacion_hilo(datos);
    t_buffer* paquete_serializado = serializar_paquete(paquete);

    send(socket_memoria, paquete_serializado->stream, paquete_serializado->size, 0);

    buffer_destroy(paquete_serializado);
    eliminar_paquete(paquete);
    destruir_datos_finalizacion_hilo(datos);

    op_code respuesta = recibir_operacion(socket_memoria);

    close(socket_memoria);

    return respuesta;
}

static op_code pedir_finalizacion_proceso_a_memoria(t_pcb* proceso)
{
    int socket_memoria = crear_conexion_a_memoria();

    t_datos_finalizacion_proceso* datos = malloc(sizeof(t_datos_finalizacion_proceso));
    datos->pid = proceso->pid;

    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = OPERACION_FINALIZAR_PROCESO;
    paquete->buffer = serializar_datos_finalizacion_proceso(datos);
    t_buffer* paquete_serializado = serializar_paquete(paquete);

    send(socket_memoria, paquete_serializado->stream, paquete_serializado->size, 0);

    buffer_destroy(paquete_serializado);
    eliminar_paquete(paquete);
    destruir_datos_finalizacion_proceso(datos);

    op_code respuesta = recibir_operacion(socket_memoria);

    close(socket_memoria);

    return respuesta;
}

static void remover_hilo_de_ready_o_blocked(t_tcb* hilo)
{
    // TODO: REVISAR SI ES NECESARIO BUSCAR EN ESTADO EXEC
    // Remuevo el hilo de READY si es que se encuentra en este estado
    pthread_mutex_lock(&mutex_estado_ready);
    bool estaba_en_ready = list_remove_element(estado_ready, hilo);
    pthread_mutex_lock(&mutex_estado_ready);

    // Si estaba en READY, reduzco el contador porque eliminamos un elemento
    if(estaba_en_ready) {
        sem_wait(&semaforo_estado_ready);
    }

    // Remuevo el hilo de BLOCKED si es que se encuentra en este estado
    pthread_mutex_lock(&mutex_estado_blocked);
    list_remove_element(estado_blocked, hilo);
    pthread_mutex_lock(&mutex_estado_blocked);
}