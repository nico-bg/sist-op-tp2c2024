#include <kernel-utils/planificador-corto-plazo.h>

void planificador_corto_plazo()
{
    while(1) {
        // Si ya no hay hilos en READY, esperamos hasta que se agreguen (hacer un sem_post)
        // ...al crear un proceso, hilo, al desalojar un proceso por quantum, etc
        sem_wait(&semaforo_estado_ready);

        pthread_mutex_lock(&mutex_estado_ready);
        t_tcb* siguiente_a_exec = obtener_siguiente_a_exec();
        transicion_hilo_a_exec(siguiente_a_exec);
        pthread_mutex_unlock(&mutex_estado_ready);

        // enviar_hilo_a_cpu(siguiente_a_exec);
        int motivo = esperar_devolucion_hilo();

        switch (motivo)
        {
        case FINALIZACION:
            // log_debug(logger_debug, "Motivo devolución: FINALIZACION");
            log_debug(logger_debug, "PID: %d, TID: %d, Prioridad: %d", siguiente_a_exec->pid_padre, siguiente_a_exec->tid, siguiente_a_exec->prioridad);
            break;
        case DESALOJO:
            log_debug(logger_debug, "Motivo de devolución: DESALOJO");
            break;
        case BLOQUEO:
            log_debug(logger_debug, "Motivo de devolución: BLOQUEO");
            break;            
        default:
            log_debug(logger_debug, "Motivo de devolución desconocido");
            break;
        }
    }
}

void planificador_corto_plazo_fifo()
{
    while(1) {
        // Si ya no hay hilos en READY, esperamos hasta que se agreguen (hacer un sem_post)
        // ...al crear un proceso, hilo, al desalojar un proceso por quantum, etc
        sem_wait(&semaforo_estado_ready);

        pthread_mutex_lock(&mutex_estado_ready);
        t_tcb* siguiente_a_exec = obtener_siguiente_a_exec_fifo();
        transicion_hilo_a_exec(siguiente_a_exec);
        pthread_mutex_unlock(&mutex_estado_ready);

        // enviar_hilo_a_cpu(siguiente_a_exec);
        int motivo = esperar_devolucion_hilo();

        switch (motivo)
        {
        case FINALIZACION:
            // log_debug(logger_debug, "Motivo devolución: FINALIZACION");
            log_debug(logger_debug, "Finalización <%d:%d>", siguiente_a_exec->pid_padre, siguiente_a_exec->tid);
            break;
        case DESALOJO:
            log_debug(logger_debug, "Motivo de devolución: DESALOJO");
            break;
        case BLOQUEO:
            log_debug(logger_debug, "Motivo de devolución: BLOQUEO");
            break;            
        default:
            log_debug(logger_debug, "Motivo de devolución desconocido");
            break;
        }
    }
}

t_tcb* obtener_siguiente_a_exec()
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

t_tcb* obtener_siguiente_a_exec_fifo()
{
    // Obtenemos el primero de la lista por ser el primero que ingresó
    // !WARNING: Tener en cuenta que si la lista está vacía va a lanzar un error
    t_tcb* siguiente_a_exec = (t_tcb*) list_get(estado_ready, 0);

    return siguiente_a_exec;
}

void* comparar_mayor_prioridad(void* a, void* b) {
    t_tcb* hilo_a = (t_tcb*) a;
    t_tcb* hilo_b = (t_tcb*) b;

    return hilo_a->prioridad < hilo_b->prioridad ? hilo_a : hilo_b;
};

// Necesaria para poder filtrar por prioridad en `obtener_lista_mayor_prioridad`
int mayor_prioridad_en_ready;

int obtener_mayor_prioridad_en_ready()
{
    t_tcb* hilo_mayor_prioridad = list_get_minimum(estado_ready, comparar_mayor_prioridad);

    return hilo_mayor_prioridad->prioridad;
}

bool filtrar_por_mayor_prioridad(void* elemento)
{
    t_tcb* hilo = (t_tcb*) elemento;

    return hilo->prioridad == mayor_prioridad_en_ready;
}

/**
 * @brief Retorna una lista con todos los t_tcb que tengan la misma prioridad, y esta sea la mayor
 * entre todos los hilos en READY
 */
t_list* obtener_lista_mayor_prioridad()
{
    mayor_prioridad_en_ready = obtener_mayor_prioridad_en_ready();

    t_list* lista_mayor_prioridad = list_filter(estado_ready, filtrar_por_mayor_prioridad);

    return lista_mayor_prioridad;
}

t_tcb* obtener_siguiente_a_exec_prioridades()
{
    t_tcb* siguiente_a_exec;

    t_list* lista_mayor_prioridad = obtener_lista_mayor_prioridad();

    siguiente_a_exec = list_get(lista_mayor_prioridad, 0);

    list_destroy(lista_mayor_prioridad);

    return siguiente_a_exec;
}

t_tcb* obtener_siguiente_a_exec_colas_multinivel()
{
    // TODO: Reemplazar esta implementación por la correspondiente a colas multinivel
    t_tcb* siguiente_a_exec = (t_tcb*) list_get(estado_ready, 0);

    return siguiente_a_exec;
}

/**
 * @brief Envía el `tid` del hilo y el `pid` de su proceso padre a la CPU para que el hilo entre en ejecución
 * !WARNING: Va a romper en el `send` si no logró conectar correctamente al socket en `main.c`
 */
void enviar_hilo_a_cpu(t_tcb* hilo)
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

/**
 * @brief Setea el hilo en `estado_exec` y lo saca de la lista de `estado_ready`
 */
void transicion_hilo_a_exec(t_tcb* hilo)
{
    pthread_mutex_lock(&mutex_estado_exec);
    estado_exec = hilo;
    pthread_mutex_unlock(&mutex_estado_exec);

    list_remove_element(estado_ready, hilo);
}

/**
 * @brief Se queda esperando una respuesta de la CPU que incluya el motivo por el cual devolvió el Hilo
 */
t_motivo_devolucion esperar_devolucion_hilo()
{
    return FINALIZACION;
}
