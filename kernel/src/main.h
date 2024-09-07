#ifndef MAIN_H_
#define MAIN_H_

#include<stdio.h>
#include<stdlib.h>
#include <utils/configuracion.h>
#include <utils/conexiones.h>
#include <utils/mensajes.h>

void terminar_programa(t_log* logger, t_config* config, int conexion_cpu_dispatch, int conexion_cpu_interrupt, int conexion_memoria);

#endif