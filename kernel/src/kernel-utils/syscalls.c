#include <kernel-utils/syscalls.h>

static void transicion_exec_a_exit();
static void solicitar_finalizacion_hilo_a_memoria(uint32_t pid, uint32_t tid);
static void solicitar_inicializacion_hilo_a_memoria(t_tcb* hilo);
static t_pcb* buscar_proceso(uint32_t pid);
static bool encontrar_proceso_por_pid_auxiliar(void* elemento);

uint32_t pid_auxiliar;

void syscall_finalizar_hilo()
{
    // Guardamos en una variable auxiliar el hilo que invocó la syscall (Es el que está en estado EXEC)
    // pthread_mutex_lock(&mutex_estado_exec);
    // t_tcb* hilo_invocador = estado_exec;
    // pthread_mutex_unlock(&mutex_estado_exec);

    // TODO: Revisar si es necesario mover esta peticion al Planificador de largo plazo
    // solicitar_finalizacion_hilo_a_memoria(hilo_invocador->pid_padre ,hilo_invocador->tid);
    // El planificador de largo plazo se encarga de liberar los procesos
    transicion_exec_a_exit();
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
    nuevo_hilo->nombre_archivo = archivo_pseudocodigo;
    nuevo_hilo->pid_padre = hilo_invocador->pid_padre;
    nuevo_hilo->prioridad = prioridad;
    nuevo_hilo->tid = proceso_invocador->ultimo_tid + 1;
    nuevo_hilo->hilos_bloqueados = list_create();

    // Le pedimos a la Memoria que inicialice los contextos de ejecución para el hilo creado
    solicitar_inicializacion_hilo_a_memoria(nuevo_hilo);

    // Agregamos el nuevo hilo al estado READY para que pueda ser planificado
    pthread_mutex_lock(&mutex_estado_ready);
    list_add(estado_ready, nuevo_hilo);
    pthread_mutex_unlock(&mutex_estado_ready);

    log_info(logger, "## (%d:%d) Se crea el Hilo - Estado: READY", nuevo_hilo->pid_padre, nuevo_hilo->tid);

    close(fd_memoria);
}

void syscall_crear_proceso(char* archivo_pseudocodigo, uint32_t tamanio_proceso, uint32_t prioridad)
{
    crear_proceso(archivo_pseudocodigo, tamanio_proceso, prioridad);
}

void syscall_finalizar_proceso()
{
    pthread_mutex_lock(&mutex_estado_exec);
    bool es_hilo_principal = estado_exec->tid == 0;
    pthread_mutex_unlock(&mutex_estado_exec);

    if(!es_hilo_principal) {
        log_error(logger_debug, "Error al finalizar proceso, el hilo que invocó la syscall no es TID 0");
    }

    // Lo mandamos a EXIT para que el planificador de largo plazo se encargue de liberarlo
    // En caso de ser TID 0, el planificador liberará el hilo y el proceso asociado, caso contrario solo el hilo
    transicion_exec_a_exit();
}

void syscall_crear_mutex(char* recurso)
{
    pthread_mutex_lock(&mutex_estado_exec);
    uint32_t pid_en_ejecucion = estado_exec->pid_padre;
    pthread_mutex_unlock(&mutex_estado_exec);

    t_pcb* proceso = buscar_proceso(pid_en_ejecucion);

    t_mutex* nuevo_mutex = malloc(sizeof(t_mutex));

    // Inicializamos el mutex que va a contener nuestra estructura personalizada para los recursos
    pthread_mutex_t* mutex_interno = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(mutex_interno, NULL);

    nuevo_mutex->hilos_bloqueados = queue_create();
    nuevo_mutex->esta_libre = true;
    nuevo_mutex->recurso = string_duplicate(recurso);
    // nuevo_mutex->mutex = mutex_interno;

    list_add(proceso->mutex, nuevo_mutex);
}

char* recurso_buscado;

bool encontrar_mutex_proceso_en_ejecucion(void* elemento)
{
    return true;
    // t_mutex* mutex = (t_mutex*) elemento;

    // bool encontrado = strcmp(mutex->recurso, recurso_buscado) == 0;

    // return encontrado;
}

void syscall_bloquear_mutex(char* recurso)
{
    // recurso_buscado = recurso;
    // t_mutex* mutex = list_find(proceso->mutex, encontrar_mutex_proceso_en_ejecucion);

    // if(mutex->esta_libre) {
    //     mutex->esta_libre = false;
    // } else {
    //     list_add(mutex->hilos_bloqueados, estado_exec);
    // }
}

void syscall_desbloquear_mutex(char* recurso)
{

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
}

static void solicitar_finalizacion_hilo_a_memoria(uint32_t pid, uint32_t tid)
{
    int fd_conexion = crear_conexion_a_memoria();

    t_datos_finalizacion_hilo* datos_a_enviar = malloc(sizeof(t_datos_finalizacion_hilo));
    datos_a_enviar->pid = pid;
    datos_a_enviar->tid = tid;

    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = OPERACION_FINALIZAR_HILO;
    paquete->buffer = serializar_datos_finalizacion_hilo(datos_a_enviar);

    t_buffer* paquete_serializado = serializar_paquete(paquete);

    send(fd_conexion, paquete_serializado->stream, paquete_serializado->size, 0);

    buffer_destroy(paquete_serializado);
    eliminar_paquete(paquete);
    close(fd_conexion);
}

static void solicitar_inicializacion_hilo_a_memoria(t_tcb* hilo)
{
    // Nos conectamos a la Memoria
    int fd_memoria = crear_conexion_a_memoria();

    // Serializamos los datos a enviar en un buffer
    t_datos_inicializacion_hilo* datos_a_enviar = malloc(sizeof(t_datos_inicializacion_hilo));
    datos_a_enviar->pid = hilo->pid_padre;
    datos_a_enviar->tid = hilo->tid;
    datos_a_enviar->archivo_pseudocodigo = hilo->nombre_archivo;
    t_buffer* buffer_datos = serializar_datos_inicializacion_hilo(datos_a_enviar);

    // Empaquetamos y serializamos los datos junto con el código de operación
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = OPERACION_CREAR_HILO;
    paquete->buffer = buffer_datos;
    t_buffer* paquete_serializado = serializar_paquete(paquete);

    send(fd_memoria, paquete_serializado->stream, paquete_serializado->size, 0);

    buffer_destroy(paquete_serializado);
    eliminar_paquete(paquete);
    buffer_destroy(buffer_datos);
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
    pthread_mutex_lock(&mutex_lista_procesos);
    t_pcb* proceso_encontrado = list_find(lista_procesos, encontrar_proceso_por_pid_auxiliar);
    pthread_mutex_unlock(&mutex_lista_procesos);

    if(proceso_encontrado == NULL) {
        log_debug(logger_debug, "buscar_proceso: No se encontró el proceso con pid %d", pid);
        abort();
    }

    return proceso_encontrado;
}