#ifndef MAIN_H_
#define MAIN_H_

#include <utils/configuracion.h>
#include <utils/conexiones.h>
#include <utils/mensajes.h>
#include <utils/comunicacion_kernel_cpu.h>

typedef struct {
    int cliente_socket;
    int cliente_memoria;
    t_log *logger;
} t_thread_args;

void escuchar_dispatch();

void terminar_programa(t_log* logger, t_config* config, int conexion);


#endif