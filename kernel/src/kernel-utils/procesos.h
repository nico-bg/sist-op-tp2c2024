#ifndef PROCESOS_H_
#define PROCESOS_H_

#include <kernel-utils/estructuras.h>
#include <kernel-utils/globales.h>

t_pcb* crear_proceso(char* nombre_archivo, u_int32_t tamanio_proceso, u_int32_t prioridad);

#endif