#include "comunicaciones.h"


#define DEVOLVER_CONTEXTO_EJECUCION 1
#define ACTUALIZAR_CONTEXTO_EJECUCION 2
#define DEVOLVER_INSTRUCCION 3
#define LEER_MEMORIA 4
#define ESCRIBIR_MEMORIA 5

#define CREAR_PROCESO 6
#define FINALIZAR_PROCESO 7
#define CREAR_HILO 8
#define FINALIZAR_HILO 9
#define MEMORY_DUMP 10


void* recibir_mensaje_kernel(int socket){

	void* msg = NULL;

	if(recv(socket, msg, sizeof(t_paquete), MSG_WAITALL) > 0)
		return msg;
	else {
		close(socket);
		return msg;
	}
}


void* leer_buffer_kernel(int cod_op, int socket_cliente){

    t_buffer* buffer;
    uint32_t length;
    void* datos;

    switch(cod_op){

        case CREAR_PROCESO:
            buffer = recibir_buffer(&length, socket_cliente);
            datos = (t_datos_inicializacion_proceso*)deserializar_datos_inicializacion_proceso(buffer);
            break;

        case FINALIZAR_PROCESO:
            buffer = recibir_buffer(&length, socket_cliente);
            datos = (t_datos_finalizacion_proceso*)deserializar_datos_finalizacion_proceso(buffer);
            break;

        case CREAR_HILO:
            buffer = recibir_buffer(&length, socket_cliente);
            datos = (t_datos_inicializacion_hilo*)deserializar_datos_inicializacion_hilo(buffer);
            break;

        case FINALIZAR_HILO:
            buffer = recibir_buffer(&length, socket_cliente);
            datos = (t_datos_finalizacion_hilo*)deserializar_datos_finalizacion_hilo(buffer);
            break;

        case MEMORY_DUMP:
            //log_info(logger, "Función aún no implementada!")
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

        case DEVOLVER_CONTEXTO_EJECUCION:
            buffer = recibir_buffer(&length, socket_cliente);
            datos = (t_cpu_solicitar_contexto*)deserializar_datos_solicitar_contexto(buffer);
            break;

        case ACTUALIZAR_CONTEXTO_EJECUCION:
            buffer = recibir_buffer(&length, socket_cliente);
            datos = (t_contexto*)deserializar_datos_contexto(buffer);
            break;

        case DEVOLVER_INSTRUCCION:
            buffer = recibir_buffer(&length, socket_cliente);
            datos = (t_datos_devolver_instruccion*)deserializar_datos_solicitar_instruccion(buffer);
            break;

        case LEER_MEMORIA:
            //log_info(logger, "Función aún no implementada!")
            break;

        case ESCRIBIR_MEMORIA:
            //log_info(logger, "Función aún no implementada!")
            break;

        default:
            //log_error(logger, "CPU envió un código de operacion desconocido - %d", cod_op);
            break;
    }

    return datos;
}

void enviar_buffer(int cod_op, int socket_cliente, void* datos){
    switch(cod_op){
        case DEVOLVER_CONTEXTO_EJECUCION:
        //codigo
        break;

        case DEVOLVER_INSTRUCCION:
        //codigo
        break;

        default:
        //codigo
        break;
    }
}