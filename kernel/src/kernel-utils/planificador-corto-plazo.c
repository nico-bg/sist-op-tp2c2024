#include <kernel-utils/planificador-corto-plazo.h>

// Necesaria para poder filtrar por prioridad en `obtener_lista_mayor_prioridad`
int mayor_prioridad_en_ready;

static void transicion_exec_a_ready(t_tcb* hilo);
static void transicion_ready_a_exec(t_tcb* hilo);
static t_tcb* obtener_siguiente_a_exec();
static t_tcb* obtener_siguiente_a_exec_fifo();
static void* comparar_mayor_prioridad(void* a, void* b);
static int obtener_mayor_prioridad_en_ready();
static bool filtrar_por_mayor_prioridad(void* elemento);
static t_list* obtener_lista_mayor_prioridad();
static t_tcb* obtener_siguiente_a_exec_prioridades();
static void solicitar_desalojo_hilo_a_cpu(t_tcb* hilo);
static void* temporizador_desalojo_por_quantum(void* arg);
static t_tcb* obtener_siguiente_a_exec_colas_multinivel();
static void enviar_hilo_a_cpu(t_tcb* hilo);
static void procesar_instrucciones_cpu(t_tcb* hilo);

void* planificador_corto_plazo()
{
    while(1) {
        // Si ya no hay hilos en READY, esperamos hasta que se agreguen (hacer un sem_post)
        // ...al crear un proceso, hilo, al desalojar un proceso por quantum, etc
        sem_wait(&semaforo_estado_ready);

        pthread_mutex_lock(&mutex_estado_ready);
        t_tcb* siguiente_a_exec = obtener_siguiente_a_exec();
        transicion_ready_a_exec(siguiente_a_exec);
        pthread_mutex_unlock(&mutex_estado_ready);

        procesar_instrucciones_cpu(siguiente_a_exec);
    }

    return NULL;
}

/**
 * @brief Envía el hilo a que sea procesado por la CPU y espera la llamada de alguna syscall u operación de desalojo
 * @brief Podemos utilizarla de manera recursiva para no replanificar si la operación no lo requiere
 */
static void procesar_instrucciones_cpu(t_tcb* hilo_en_ejecucion)
{
    enviar_hilo_a_cpu(hilo_en_ejecucion);

    // Esperamos la respuesta de la CPU para procesar una syscall, un desalojo, un bloqueo, etc
    op_code operacion = recibir_operacion(socket_cpu_dispatch);
    uint32_t size;
    t_buffer* buffer;

    switch (operacion)
    {
    case OPERACION_CREAR_PROCESO:
        log_info(logger, "## (%d:%d) - Solicitó syscall: PROCESS_CREATE", hilo_en_ejecucion->pid_padre, hilo_en_ejecucion->tid);

        // Recibimos y deserializamos los datos enviados por la CPU
        buffer = recibir_buffer(&size, socket_cpu_dispatch);
        t_datos_crear_proceso* datos_crear_proceso = deserializar_datos_crear_proceso(buffer);

        syscall_crear_proceso(datos_crear_proceso->archivo_pseudocodigo, datos_crear_proceso->tamanio_proceso, datos_crear_proceso->prioridad);
        destruir_datos_crear_proceso(datos_crear_proceso);

        // Continuamos ejecutando el hilo que solicitó la syscall
        procesar_instrucciones_cpu(hilo_en_ejecucion);
        break;
    case OPERACION_FINALIZAR_PROCESO:
        log_info(logger, "## (%d:%d) - Solicitó syscall: PROCESS_EXIT", hilo_en_ejecucion->pid_padre, hilo_en_ejecucion->tid);

        syscall_finalizar_proceso();
        break;
    case OPERACION_CREAR_HILO:
        log_info(logger, "## (%d:%d) - Solicitó syscall: THREAD_CREATE", hilo_en_ejecucion->pid_padre, hilo_en_ejecucion->tid);

        // Recibimos y deserializamos los datos enviados por la CPU
        buffer = recibir_buffer(&size, socket_cpu_dispatch);
        t_datos_crear_hilo* datos_crear_hilo = deserializar_datos_crear_hilo(buffer);

        syscall_crear_hilo(datos_crear_hilo->archivo_pseudocodigo, datos_crear_hilo->prioridad);
        destruir_datos_crear_hilo(datos_crear_hilo);

        // Continuamos ejecutando el hilo que solicitó la syscall
        procesar_instrucciones_cpu(hilo_en_ejecucion);
        break;
    case OPERACION_FINALIZAR_HILO:
        log_info(logger, "## (%d:%d) - Solicitó syscall: THREAD_EXIT", hilo_en_ejecucion->pid_padre, hilo_en_ejecucion->tid);
        syscall_finalizar_hilo();
        break;
    case OPERACION_ESPERAR_HILO:
        log_info(logger, "## (%d:%d) - Solicitó syscall: THREAD_JOIN", hilo_en_ejecucion->pid_padre, hilo_en_ejecucion->tid);

        // Recibimos y deserializamos los datos enviados por la CPU
        buffer = recibir_buffer(&size, socket_cpu_dispatch);
        t_datos_operacion_hilo* datos_esperar_hilo = deserializar_datos_operacion_hilo(buffer);

        bool hilo_bloqueado = syscall_esperar_hilo(datos_esperar_hilo->tid);
        destruir_datos_operacion_hilo(datos_esperar_hilo);

        // Continuamos ejecutando el hilo que solicitó la syscall solo si el hilo no fue bloqueado por la syscall
        if(!hilo_bloqueado) {
            procesar_instrucciones_cpu(hilo_en_ejecucion);
        }
        break;
    case OPERACION_CREAR_MUTEX:
        log_info(logger, "## (%d:%d) - Solicitó syscall: MUTEX_CREATE", hilo_en_ejecucion->pid_padre, hilo_en_ejecucion->tid);

        // Recibimos y deserializamos los datos enviados por la CPU
        buffer = recibir_buffer(&size, socket_cpu_dispatch);
        t_datos_operacion_mutex* datos_crear_mutex = deserializar_datos_operacion_mutex(buffer);

        syscall_crear_mutex(datos_crear_mutex->recurso);
        destruir_datos_operacion_mutex(datos_crear_mutex);

        // Continuamos ejecutando el hilo que solicitó la syscall
        procesar_instrucciones_cpu(hilo_en_ejecucion);
        break;
    case OPERACION_BLOQUEAR_MUTEX:
        log_info(logger, "## (%d:%d) - Solicitó syscall: MUTEX_LOCK", hilo_en_ejecucion->pid_padre, hilo_en_ejecucion->tid);

        // Recibimos y deserializamos los datos enviados por la CPU
        buffer = recibir_buffer(&size, socket_cpu_dispatch);
        t_datos_operacion_mutex* datos_bloquear_mutex = deserializar_datos_operacion_mutex(buffer);

        bool mutex_asignado = syscall_bloquear_mutex(datos_bloquear_mutex->recurso);
        destruir_datos_operacion_mutex(datos_bloquear_mutex);

        // Continuamos ejecutando el hilo que solicitó la syscall solo si el mutex fue asignado al hilo
        if(mutex_asignado) {
            procesar_instrucciones_cpu(hilo_en_ejecucion);
        }
        break;
    case OPERACION_DESBLOQUEAR_MUTEX:
        log_info(logger, "## (%d:%d) - Solicitó syscall: MUTEX_UNLOCK", hilo_en_ejecucion->pid_padre, hilo_en_ejecucion->tid);

        // Recibimos y deserializamos los datos enviados por la CPU
        buffer = recibir_buffer(&size, socket_cpu_dispatch);
        t_datos_operacion_mutex* datos_desbloquear_mutex = deserializar_datos_operacion_mutex(buffer);

        syscall_desbloquear_mutex(datos_desbloquear_mutex->recurso);
        destruir_datos_operacion_mutex(datos_desbloquear_mutex);

        // Continuamos ejecutando el hilo que solicitó la syscall
        procesar_instrucciones_cpu(hilo_en_ejecucion);
        break;
    case OPERACION_IO:
        log_info(logger, "## (%d:%d) - Solicitó syscall: IO", hilo_en_ejecucion->pid_padre, hilo_en_ejecucion->tid);

        // Recibimos y deserializamos los datos enviados por la CPU
        buffer = recibir_buffer(&size, socket_cpu_dispatch);
        t_datos_operacion_io* datos_io = deserializar_datos_operacion_io(buffer);

        syscall_io(datos_io->tiempo);
        destruir_datos_operacion_io(datos_io);
        break;
    case OPERACION_DUMP_MEMORY:
        log_info(logger, "## (%d:%d) - Solicitó syscall: DUMP_MEMORY", hilo_en_ejecucion->pid_padre, hilo_en_ejecucion->tid);

        syscall_dump_memory();
        break;
    case OPERACION_DESALOJAR_HILO:
        transicion_exec_a_ready(hilo_en_ejecucion);
        break;
    default:
        log_debug(logger_debug, "Motivo de devolución desconocido");
        break;
    }
}

/* TRANSICIONES */

static void transicion_exec_a_ready(t_tcb* hilo)
{
    // Desalojo el hilo del estado EXEC (El mismo que recibimos como parámetro en esta función)
    pthread_mutex_lock(&mutex_estado_exec);
    estado_exec = NULL;
    pthread_mutex_unlock(&mutex_estado_exec);

    // Agregamos el hilo al estado READY
    pthread_mutex_lock(&mutex_estado_ready);
    list_add(estado_ready, hilo);
    pthread_mutex_unlock(&mutex_estado_ready);

    sem_post(&semaforo_estado_ready);
}

/**
 * @brief Setea el hilo en `estado_exec` y lo saca de la lista de `estado_ready`
 */
static void transicion_ready_a_exec(t_tcb* hilo)
{
    pthread_mutex_lock(&mutex_estado_exec);
    estado_exec = hilo;
    pthread_mutex_unlock(&mutex_estado_exec);

    // No agrego el mutex porque quien lo invoca (planificador a corto plazo) ya lo seteo
    // ...para asegurarse de que ningún otro hilo modifique el estado_ready mientras lo seguimos procesando
    list_remove_element(estado_ready, hilo);
}

/* ALGORITMOS PLANIFICACIÓN */

static t_tcb* obtener_siguiente_a_exec()
{
    char* algoritmo_planificacion = config_get_string_value(config, "ALGORITMO_PLANIFICACION");

    if(strcmp(algoritmo_planificacion, "FIFO") == 0) {
        return obtener_siguiente_a_exec_fifo();
    }

    if(strcmp(algoritmo_planificacion, "PRIORIDADES") == 0) {
        return obtener_siguiente_a_exec_prioridades();
    }

    if(strcmp(algoritmo_planificacion, "CMN") == 0) {
        return obtener_siguiente_a_exec_colas_multinivel();
    }

    log_debug(logger_debug, "Algoritmo de Planificación inválido: %s", algoritmo_planificacion);
    return NULL;
}

static t_tcb* obtener_siguiente_a_exec_fifo()
{
    // Obtenemos el primero de la lista por ser el primero que ingresó
    // !WARNING: Tener en cuenta que si la lista está vacía va a lanzar un error
    t_tcb* siguiente_a_exec = (t_tcb*) list_get(estado_ready, 0);

    return siguiente_a_exec;
}

static t_tcb* obtener_siguiente_a_exec_prioridades()
{
    t_tcb* siguiente_a_exec;

    t_list* lista_mayor_prioridad = obtener_lista_mayor_prioridad();

    siguiente_a_exec = list_get(lista_mayor_prioridad, 0);

    list_destroy(lista_mayor_prioridad);

    return siguiente_a_exec;
}

static t_tcb* obtener_siguiente_a_exec_colas_multinivel()
{
    t_tcb* siguiente_a_exec = obtener_siguiente_a_exec_prioridades();

    pthread_t hilo_desalojo_por_quantum;
    pthread_create(&hilo_desalojo_por_quantum, NULL, temporizador_desalojo_por_quantum, siguiente_a_exec);
    pthread_detach(hilo_desalojo_por_quantum);

    return siguiente_a_exec;
}

/* UTILS PARA ALGORITMOS DE PLANIFICACIÓN */

static void* comparar_mayor_prioridad(void* a, void* b) {
    t_tcb* hilo_a = (t_tcb*) a;
    t_tcb* hilo_b = (t_tcb*) b;

    return hilo_a->prioridad < hilo_b->prioridad ? hilo_a : hilo_b;
};

static int obtener_mayor_prioridad_en_ready()
{
    t_tcb* hilo_mayor_prioridad = list_get_minimum(estado_ready, comparar_mayor_prioridad);

    return hilo_mayor_prioridad->prioridad;
}

static bool filtrar_por_mayor_prioridad(void* elemento)
{
    t_tcb* hilo = (t_tcb*) elemento;

    return hilo->prioridad == mayor_prioridad_en_ready;
}

/**
 * @brief Retorna una lista con todos los t_tcb que tengan la misma prioridad, y esta sea la mayor
 * entre todos los hilos en READY
 */
static t_list* obtener_lista_mayor_prioridad()
{
    mayor_prioridad_en_ready = obtener_mayor_prioridad_en_ready();

    t_list* lista_mayor_prioridad = list_filter(estado_ready, filtrar_por_mayor_prioridad);

    return lista_mayor_prioridad;
}

static void* temporizador_desalojo_por_quantum(void* arg)
{
    t_tcb* hilo_escuchado = (t_tcb*) arg;

    int quantum = config_get_int_value(config, "QUANTUM");

    esperar_ms(quantum);
    solicitar_desalojo_hilo_a_cpu(hilo_escuchado);

    return NULL;
}

/* COMUNICACIÓN CON LA CPU */

static void solicitar_desalojo_hilo_a_cpu(t_tcb* hilo)
{
    pthread_mutex_lock(&mutex_estado_exec);
    bool sigue_en_exec = estado_exec == hilo;
    pthread_mutex_unlock(&mutex_estado_exec);

    if(sigue_en_exec) {
        uint32_t operacion = OPERACION_DESALOJAR_HILO;
        send(socket_cpu_interrupt, &operacion, sizeof(uint32_t), 0);
    }
}

/**
 * @brief Envía el `tid` del hilo y el `pid` de su proceso padre a la CPU para que el hilo entre en ejecución
 * !WARNING: Va a romper en el `send` si no logró conectar correctamente al socket en `main.c`
 */
static void enviar_hilo_a_cpu(t_tcb* hilo)
{
    /* Armamos el paquete con los datos a enviar serializados */
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = OPERACION_EJECUTAR_HILO;

    t_hilo_a_cpu* datos_a_enviar = malloc(sizeof(t_hilo_a_cpu));
    datos_a_enviar->tid = hilo->tid;
    datos_a_enviar->pid = hilo->pid_padre;

    paquete->buffer = serializar_hilo_a_cpu(datos_a_enviar);

    /* Serializamos el paquete y enviamos los datos a la CPU */
    t_buffer* paquete_serializado = serializar_paquete(paquete);
    send(socket_cpu_dispatch, paquete_serializado->stream, paquete_serializado->size, 0);

    /* Liberamos la memoria correspondiente */
    free(datos_a_enviar);
    buffer_destroy(paquete_serializado);
    eliminar_paquete(paquete);
}
