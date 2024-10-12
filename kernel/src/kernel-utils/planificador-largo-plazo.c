#include <kernel-utils/planificador-largo-plazo.h>
#include <utils/comunicacion_kernel_memoria.h>
#include <kernel-utils/conexion-a-memoria.h>

static void transicion_new_a_ready(t_pcb* proceso, t_tcb* hilo);
static void transicion_blocked_a_ready(t_tcb* hilo);
static op_code pedir_inicializacion_hilo_a_memoria(t_tcb* hilo);
static op_code pedir_inicializacion_proceso_a_memoria(t_pcb* proceso);
static t_tcb* crear_hilo_principal(t_pcb* proceso_padre);
static op_code pedir_finalizacion_proceso_a_memoria(t_pcb* proceso);
static op_code pedir_finalizacion_hilo_a_memoria(t_tcb* hilo);
static void finalizar_proceso(t_tcb* hilo_principal);
static void finalizar_hilo(t_tcb* hilo);
static t_pcb* buscar_proceso(uint32_t pid);
static t_tcb* buscar_hilo_en_proceso(t_pcb* proceso, uint32_t tid);
static bool existe_tid_en_lista(void* elemento);
static bool existe_tcb_en_lista(void* elemento);
static void remover_hilo_de_ready_o_blocked(t_tcb* hilo);
static bool esta_mutex_asignado_a_tid(void* elemento);

static uint32_t pid_auxiliar;
static uint32_t tid_auxiliar;

/**
 * @brief Se mantiene escuchando los cambios en los estados NEW y EXIT
 * para pasarlos a READY o liberarlos respectivamente
 */
void* planificador_largo_plazo()
{
    // Paralelizamos la liberación de hilos que estén en estado EXIT
    pthread_t hilo_liberacion_hilos_en_exit;
    pthread_create(&hilo_liberacion_hilos_en_exit, NULL, liberar_hilos_en_exit, NULL);
    pthread_detach(hilo_liberacion_hilos_en_exit);

    while(1) {
        // Esperamos hasta que se finalicen procesos para seguir intentando inicializar (solo si en algun momento no hubo memoria suficiente)
        sem_wait(&semaforo_memoria_suficiente);
        // Esperamos hasta que hayan procesos en NEW
        sem_wait(&semaforo_estado_new);

        // Obtenemos el siguiente proceso (cuyo hilo principal debería pasar a READY) de `estado_new` según FIFO
        pthread_mutex_lock(&mutex_estado_new);
        t_pcb* proceso_a_inicializar = list_get(estado_new, 0);
        pthread_mutex_unlock(&mutex_estado_new);

        int resultado_proceso = pedir_inicializacion_proceso_a_memoria(proceso_a_inicializar);

        // Si no hubo memoria suficiente pasamos a la siguiente iteración donde se va a bloquear por el `semaforo_memoria_suficiente`
        if(resultado_proceso == OPERACION_NOTIFICAR_ERROR) {
            continue;
        }

        // Si hubo memoria suficiente liberamos el semáforo de memoria para evitar bloquear el planificador en la siguiente iteración
        // ... y continuamos la ejecución
        sem_post(&semaforo_memoria_suficiente);

        // Inicializamos el hilo principal del proceso
        t_tcb* hilo_principal = crear_hilo_principal(proceso_a_inicializar);

        // Le enviamos el hilo a Memoria para que cree los contextos de ejecucion
        op_code resultado = pedir_inicializacion_hilo_a_memoria(hilo_principal);

        // Si la respuesta es exitosa, pasamos el tcb a `estado_ready` y sacamos el proceso de `estado_new`
        if(resultado == OPERACION_CONFIRMAR) {
            transicion_new_a_ready(proceso_a_inicializar, hilo_principal);
            log_info(logger, "## (%d:%d) Se crea el Hilo - Estado: READY", hilo_principal->pid_padre, hilo_principal->tid);
        }
    }
}

/**
 * @brief Se mantiene escuchando los cambios en el estado EXIT y libera la memoria de sus elementos
 * Si alguno tenía TID 0, buscar a su proceso padre y liberarlo de lista_procesos
 */
void* liberar_hilos_en_exit()
{
    while(1) {
        // Esperamos hasta que hayan TCBs en EXIT
        sem_wait(&semaforo_estado_exit);

        // Obtenemos el siguiente hilo a liberar
        pthread_mutex_lock(&mutex_estado_exit);
        t_tcb* hilo_a_liberar = list_get(estado_exit, 0);
        pthread_mutex_unlock(&mutex_estado_exit);

        // Si el TID corresponde al hilo principal de un proceso, finalizamos todos sus hilos
        // ...en caso contrario solo el hilo_a_liberar
        if(hilo_a_liberar->tid == 0) {
            finalizar_proceso(hilo_a_liberar);
        } else {
            finalizar_hilo(hilo_a_liberar);
        }

        pthread_mutex_lock(&mutex_estado_exit);
        list_remove_element(estado_exit, hilo_a_liberar);
        pthread_mutex_unlock(&mutex_estado_exit);

        destruir_tcb(hilo_a_liberar);
    }
}

static void finalizar_proceso(t_tcb* hilo_principal)
{
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
        list_remove_element(proceso->tids, tid_a_liberar);

        t_tcb* hilo_a_liberar = buscar_hilo_en_proceso(proceso, *tid_a_liberar);

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
    destruir_pcb(proceso);
}

static bool esta_mutex_asignado_a_tid(void* elemento)
{
    t_mutex* mutex = (t_mutex*) elemento;

    return mutex->hilo_asignado->tid == tid_auxiliar;
}

static void finalizar_hilo(t_tcb* hilo)
{
    op_code respuesta = pedir_finalizacion_hilo_a_memoria(hilo);
    t_pcb* proceso = buscar_proceso(hilo->pid_padre);

    if(respuesta != OPERACION_CONFIRMAR) {
        log_debug(logger_debug, "Respuesta desconocida al finalizar hilo. Cod: %d, PID: %d, TID: %d", respuesta, hilo->pid_padre, hilo->tid);
        abort();
    }

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
    destruir_tcb(hilo);

}

/**
 * @brief Crea una conexión con Memoria y envía el hilo serializado,
 * espera la respuesta y devuelve 0 si se pudo inicializar,
 * caso contrario (memoria insuficiente) devuelve -1
 */
static op_code pedir_inicializacion_hilo_a_memoria(t_tcb* hilo)
{
    // Establecer la conexión con Memoria
    int socket_memoria = crear_conexion_a_memoria();
    if (socket_memoria == -1) {
        log_error(logger, "Error al conectar con Memoria");
    }

    // Serializamos los datos a enviar en un buffer
    t_datos_inicializacion_hilo* datos_inicializacion = malloc(sizeof(t_datos_inicializacion_hilo));
    datos_inicializacion->pid = hilo->pid_padre;
    datos_inicializacion->tid = hilo->tid;
    datos_inicializacion->archivo_pseudocodigo = hilo->nombre_archivo;
    t_buffer* buffer_inicializacion = serializar_datos_inicializacion_hilo(datos_inicializacion);

    // Empaquetamos y serializamos los datos junto con el código de operación
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = OPERACION_CREAR_HILO;
    paquete->buffer = buffer_inicializacion;
    t_buffer* paquete_serializado = serializar_paquete(paquete);

    send(socket_memoria, paquete_serializado->stream, paquete_serializado->size, 0);

    buffer_destroy(paquete_serializado);
    eliminar_paquete(paquete);
    destruir_datos_inicializacion_hilo(datos_inicializacion);

    // Esperar respuesta de Memoria
    op_code respuesta = recibir_operacion(socket_memoria);

    // Cerrar la conexión con Memoria
    close(socket_memoria);

    // Retornar el resultado
    return respuesta;
}

static void transicion_new_a_ready(t_pcb* proceso, t_tcb* hilo)
{
    // Sacamos el proceso del estado NEW
    pthread_mutex_lock(&mutex_estado_new);
    list_remove_element(estado_new, proceso);
    pthread_mutex_unlock(&mutex_estado_new);

    // Agregamos el hilo al estado READY
    pthread_mutex_lock(&mutex_estado_ready);
    list_add(estado_ready, hilo);
    pthread_mutex_unlock(&mutex_estado_ready);

    // Le notificamos al planificador de corto plazo que tiene mas hilos para planificar
    sem_post(&semaforo_estado_ready);
}

static t_tcb* crear_hilo_principal(t_pcb* proceso_padre)
{
    // Inicializamos el hilo principal
    t_tcb* hilo_principal = malloc(sizeof(t_tcb));
    hilo_principal->pid_padre = proceso_padre->pid;
    hilo_principal->nombre_archivo = string_duplicate(proceso_padre->nombre_archivo);
    hilo_principal->tid = 0;
    hilo_principal->prioridad = proceso_padre->prioridad;
    hilo_principal->hilos_bloqueados = list_create();

    return hilo_principal;
}

static op_code pedir_inicializacion_proceso_a_memoria(t_pcb* proceso)
{
    int socket_memoria = crear_conexion_a_memoria();

    if (socket_memoria == -1) {
        log_error(logger, "No se pudo establecer la coneccion con la memoria");
    }  

    t_datos_inicializacion_proceso* datos_inicializacion = malloc(sizeof(t_datos_inicializacion_proceso));
    datos_inicializacion->pid = proceso->pid;  // PID del proceso
    datos_inicializacion->tamanio = proceso->tamanio;  // Tamaño del proceso

    // Serializar los datos de inicialización del proceso
    t_buffer* buffer_inicializacion = serializar_datos_inicializacion_proceso(datos_inicializacion);

    // Empaquetamos y serializamos los datos junto con el código de operación
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = OPERACION_CREAR_PROCESO;
    paquete->buffer = buffer_inicializacion;
    t_buffer* paquete_serializado = serializar_paquete(paquete);

    send(socket_memoria, paquete_serializado->stream, paquete_serializado->size, 0);

    buffer_destroy(paquete_serializado);
    eliminar_paquete(paquete);
    destruir_datos_inicializacion_proceso(datos_inicializacion);

    // Esperar respuesta de Memoria
    op_code respuesta = recibir_operacion(socket_memoria);

    // Cerrar la conexión con Memoria
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

static bool existe_tcb_en_lista(void* elemento)
{
    t_tcb* hilo = (t_tcb*) elemento;

    return hilo->tid == tid_auxiliar;
}

static t_tcb* buscar_hilo_en_proceso(t_pcb* proceso, uint32_t tid)
{
    // TODO: REVISAR SI ES NECESARIO BUSCAR EN ESTADO EXEC
    tid_auxiliar = tid;
    bool tid_existente = list_any_satisfy(proceso->tids, existe_tid_en_lista);

    if(tid_existente) {
        t_tcb* hilo_encontrado;

        // Buscamos el TID en la lista de estado READY
        pthread_mutex_lock(&mutex_estado_ready);
        hilo_encontrado = list_find(estado_ready, existe_tcb_en_lista);
        pthread_mutex_unlock(&mutex_estado_ready);

        // Si no se encuentra el hilo en READY, lo buscamos en BLOCKED
        if(hilo_encontrado == NULL) {
            pthread_mutex_lock(&mutex_estado_blocked);
            hilo_encontrado = list_find(estado_blocked, existe_tcb_en_lista);
            pthread_mutex_unlock(&mutex_estado_blocked);
        }

        return hilo_encontrado;
    } else {
        return NULL;
    }
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