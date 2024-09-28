#ifndef MAIN_H_
#define MAIN_H_

#include <utils/configuracion.h>
#include <utils/conexiones.h>
#include <utils/mensajes.h>

/* Temporal hasta implementar */
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

typedef struct {
    t_log* logger;
    int socket;
} parametros_hilo;


void terminar_programa(t_log* logger, t_config* config, int conexion);

int atender_kernel(void* socket_cliente);

void* atender_peticion_kernel(t_log* logger, int cod_op);

int atender_cpu(t_log* logger, int socket_cliente);

void* atender_peticion_cpu(int cod_op);

void* hilo_kernel(void* args);

#endif