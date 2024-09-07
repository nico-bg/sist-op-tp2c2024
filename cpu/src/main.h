#ifndef MAIN_H_
#define MAIN_H_

#include<stdio.h>
#include<stdlib.h>
#include <utils/configuracion.h>
#include <utils/conexiones.h>

t_config* iniciar_config_cpu();
t_log* iniciar_logger_cpu(t_config* config);
void terminar_programa(t_log* logger, t_config* config, int conexion);

#endif