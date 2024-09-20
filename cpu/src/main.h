#ifndef MAIN_H_
#define MAIN_H_

#include <utils/configuracion.h>
#include <utils/conexiones.h>
#include <utils/mensajes.h>


void ejecutar_pid();

void ejecutar_interrupcion();

void terminar_programa(t_log* logger, t_config* config, int conexion);

void iniciar();


#endif