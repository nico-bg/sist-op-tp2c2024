#ifndef MAIN_H_
#define MAIN_H_

#include <utils/configuracion.h>
#include <utils/conexiones.h>
#include <utils/mensajes.h>
#include <kernel-utils/globales.h>
#include <kernel-utils/inicializacion.h>
#include <kernel-utils/procesos.h>
#include <kernel-utils/estados.h>
#include <kernel-utils/planificador-largo-plazo.h>
#include <kernel-utils/syscalls.h>
#include <kernel-utils/planificador-corto-plazo.h>
#include <kernel-utils/dispositivo-io.h>

void terminar_programa();

#endif