#include <kernel-utils/planificador-corto-plazo.h>

void planificador_corto_plazo()
{
    char* algoritmo_planificacion = config_get_string_value(config, "ALGORITMO_PLANIFICACION");

    if(strcmp(algoritmo_planificacion, "FIFO") == 0) {
        planificador_corto_plazo_fifo();
    }

    if(strcmp(algoritmo_planificacion, "PRIORIDADES") == 0) {
        planificador_corto_plazo_prioridades();
    }

    if(strcmp(algoritmo_planificacion, "CMN") == 0) {
        planificador_corto_plazo_colas_multinivel();
    }
}

/* PLANIFICADOR POR FIFO */

void planificador_corto_plazo_fifo()
{
    while(1) {
        if(hay_hilos_en_ready()) {
            t_tcb* siguiente_a_exec = obtener_siguiente_a_exec_fifo();
            enviar_hilo_a_cpu(siguiente_a_exec);
            transicion_hilo_a_exec(siguiente_a_exec);
            int motivo = esperar_devolucion_hilo();

            switch (motivo)
            {
            case FINALIZACION:
                log_debug(logger_debug, "Motivo devolución: FINALIZACION");
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
}

t_tcb* obtener_siguiente_a_exec_fifo()
{
    // Obtenemos el primero de la lista por ser el primero que ingresó
    // !WARNING: Tener en cuenta que si la lista está vacía va a lanzar un error
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
    estado_exec = hilo;
    list_remove_element(estado_ready, hilo);
}

t_motivo_devolucion esperar_devolucion_hilo()
{
    return FINALIZACION;
}

/* PLANIFICADOR POR PRIORIDADES */

void planificador_corto_plazo_prioridades() {

}

/* PLANIFICADOR POR COLAS MULTINIVEL */


void planificador_corto_plazo_colas_multinivel() {

}

bool hay_hilos_en_ready()
{
    return !list_is_empty(estado_ready);
}