#ifndef MENSAJES_H_
#define MENSAJES_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/config.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <utils/buffer.h>

typedef enum
{
	OPERACION_MENSAJE,
	OPERACION_DEVOLVER_CONTEXTO_EJECUCION,
	OPERACION_PAQUETE,
	OPERACION_EJECUTAR_HILO,
	OPERACION_DESALOJAR_HILO,
	OPERACION_CREAR_PROCESO,
	OPERACION_FINALIZAR_PROCESO,
	OPERACION_CREAR_HILO,
	OPERACION_FINALIZAR_HILO,
	OPERACION_ESPERAR_HILO,
	OPERACION_CANCELAR_HILO,
	OPERACION_CREAR_MUTEX,
	OPERACION_BLOQUEAR_MUTEX,
	OPERACION_DESBLOQUEAR_MUTEX,
	OPERACION_DUMP_MEMORY,
	OPERACION_IO,
	OPERACION_CONFIRMAR,
	OPERACION_NOTIFICAR_ERROR,
	OPERACION_DEVOLVER_INSTRUCCION,
	OPERACION_ACTUALIZAR_CONTEXTO,
	OPERACION_LEER_MEMORIA,
	OPERACION_ESCRIBIR_MEMORIA
} op_code;

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

t_buffer* serializar_paquete(t_paquete* paquete);
void eliminar_paquete(t_paquete* paquete);
void enviar_mensaje(char* mensaje, int socket_cliente);
int recibir_operacion(int socket_cliente);
t_buffer* recibir_buffer(uint32_t* size, int socket_cliente);
void recibir_mensaje(int socket_cliente, t_log* logger);
void atender_peticiones(t_log* logger, t_config* config, int socket_cliente);
int atender_peticion(t_log* logger, t_config* config, int socket_cliente);

#endif