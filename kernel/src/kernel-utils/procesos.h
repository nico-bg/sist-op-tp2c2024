#ifndef PROCESOS_H_
#define PROCESOS_H_

#include <kernel-utils/estructuras.h>
#include <kernel-utils/globales.h>
#include <commons/string.h>
#include <utils/mensajes.h>
#include <utils/comunicacion_kernel_memoria.h>

void crear_proceso(char* nombre_archivo, uint32_t tamanio_proceso, uint32_t prioridad);
void destruir_pcb(t_pcb* pcb);
void destruir_tcb(t_tcb* tcb);

#endif