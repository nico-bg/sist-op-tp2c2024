#include "comunicaciones.h"


void* leer_buffer_kernel(int cod_op, int socket_cliente){

    t_buffer* buffer;
    uint32_t length;
    void* datos;

    switch(cod_op){

        case OPERACION_CREAR_PROCESO:
            buffer = recibir_buffer(&length, socket_cliente);
            datos = (t_datos_inicializacion_proceso*)deserializar_datos_inicializacion_proceso(buffer);
            break;

        case OPERACION_FINALIZAR_PROCESO:
            buffer = recibir_buffer(&length, socket_cliente);
            datos = (t_datos_finalizacion_proceso*)deserializar_datos_finalizacion_proceso(buffer);
            break;

        case OPERACION_CREAR_HILO:
            buffer = recibir_buffer(&length, socket_cliente);
            datos = (t_datos_inicializacion_hilo*)deserializar_datos_inicializacion_hilo(buffer);
            break;

        case OPERACION_FINALIZAR_HILO:
            buffer = recibir_buffer(&length, socket_cliente);
            datos = (t_datos_finalizacion_hilo*)deserializar_datos_finalizacion_hilo(buffer);
            break;

        case OPERACION_DUMP_MEMORY:
            buffer = recibir_buffer(&length, socket_cliente);
            datos = (t_datos_finalizacion_hilo*)deserializar_datos_finalizacion_hilo(buffer);
            break;

        default:
            //codigo
            break;
    }

    return datos;
}

void* leer_buffer_cpu(int cod_op, int socket_cliente){

    t_buffer* buffer;
    uint32_t length;
    void* datos;

    switch(cod_op){

        case OPERACION_DEVOLVER_CONTEXTO_EJECUCION:
            buffer = recibir_buffer(&length, socket_cliente);
            datos = (t_cpu_solicitar_contexto*)deserializar_datos_solicitar_contexto(buffer);
            break;

        case OPERACION_ACTUALIZAR_CONTEXTO:
            buffer = recibir_buffer(&length, socket_cliente);
            datos = (t_contexto*)deserializar_datos_contexto_memoria(buffer);
            break;

        case OPERACION_DEVOLVER_INSTRUCCION:
            buffer = recibir_buffer(&length, socket_cliente);
            datos = (t_datos_devolver_instruccion*)deserializar_datos_solicitar_instruccion(buffer);
            break;

        case OPERACION_LEER_MEMORIA:
            buffer = recibir_buffer(&length, socket_cliente);
            datos = (t_datos_leer_memoria*)deserializar_datos_leer_memoria(buffer);
            break;

        case OPERACION_ESCRIBIR_MEMORIA:
            buffer = recibir_buffer(&length, socket_cliente);
            datos = (t_datos_escribir_memoria*)deserializar_datos_escribir_memoria(buffer);
            break;

        default:
            log_error(logger, "CPU envió un código de operacion desconocido - %d", cod_op);
            break;
    }

    return datos;
}

void enviar_buffer(int cod_op, int socket_cliente, void* datos){

    t_buffer* buffer;
    t_buffer* paquete_serializado;
    t_paquete* paquete;

    switch(cod_op){

        case OPERACION_DEVOLVER_CONTEXTO_EJECUCION:

            t_contexto* contexto = malloc(sizeof(t_contexto));
            contexto = (t_contexto*)datos;

            buffer = serializar_datos_contexto(contexto);

            paquete = malloc(sizeof(t_paquete));
            paquete->codigo_operacion = OPERACION_DEVOLVER_CONTEXTO_EJECUCION;
            paquete->buffer = buffer;
            paquete_serializado = serializar_paquete(paquete);

            send(socket_cliente, paquete_serializado->stream, paquete_serializado->size, 0);

            buffer_destroy(paquete_serializado);
            eliminar_paquete(paquete);
            destruir_datos_contexto(contexto);            
            break;

        case OPERACION_DEVOLVER_INSTRUCCION:

            t_datos_devolver_instruccion* instruccion = malloc(sizeof(t_datos_devolver_instruccion));
            instruccion->instruccion=(char*)datos;

            buffer = serializar_datos_devolver_instruccion(instruccion);

            paquete = malloc(sizeof(t_paquete));
            paquete->codigo_operacion = OPERACION_DEVOLVER_INSTRUCCION;
            paquete->buffer = buffer;
            paquete_serializado = serializar_paquete(paquete);

            send(socket_cliente, paquete_serializado->stream, paquete_serializado->size, 0);

            buffer_destroy(paquete_serializado);
            eliminar_paquete(paquete);
            destruir_datos_devolver_instruccion(instruccion);
            break;
        default:
            log_error(logger, "Codigo de operacion desconocido");
            break;
    }
}

void confirmar_operacion (int socket_cliente){

    uint32_t op = OPERACION_CONFIRMAR;

    send(socket_cliente, &op, sizeof(uint32_t), 0);
}

void notificar_error(int socket_cliente){
    
    uint32_t op = OPERACION_NOTIFICAR_ERROR;

    send(socket_cliente, &op, sizeof(uint32_t), 0);
}

op_code enviar_dump_memory(int socket_filesystem, t_datos_dump_memory* datos_kernel)
{
    char* nombre_archivo = NULL;
    char* timestamp = obtener_timestamp();

    int tamanio = snprintf(NULL, 0, "%d-%d-%s.dmp", datos_kernel->pid, datos_kernel->tid, timestamp) + 1;
    nombre_archivo = malloc(tamanio);
    snprintf(nombre_archivo, tamanio, "%d-%d-%s.dmp", datos_kernel->pid, datos_kernel->tid, timestamp);

    nodo_proceso* nodo = buscar_proceso_por_pid(datos_kernel->pid);

    t_datos_dump_memory_fs* datos_a_enviar = malloc(sizeof(t_datos_dump_memory_fs));
    datos_a_enviar->nombre_archivo = nombre_archivo;
    datos_a_enviar->tamanio = nodo->proceso.tamanio;
    datos_a_enviar->contenido = malloc(datos_a_enviar->tamanio);
    memcpy(datos_a_enviar->contenido, memoria + nodo->proceso.base, datos_a_enviar->tamanio);

    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = OPERACION_DUMP_MEMORY;
    paquete->buffer = serializar_datos_dump_memory_fs(datos_a_enviar);

    t_buffer* paquete_serializado = serializar_paquete(paquete);

    send(socket_filesystem, paquete_serializado->stream, paquete_serializado->size, 0);

    buffer_destroy(paquete_serializado);
    eliminar_paquete(paquete);
    destruir_datos_dump_memory_fs(datos_a_enviar);

    op_code codigo_operacion = recibir_operacion(socket_filesystem);

    return codigo_operacion;
}