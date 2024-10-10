#ifndef PLANIFICADOR_LARGO_PLAZO_H_
#define PLANIFICADOR_LARGO_PLAZO_H_

#include <kernel-utils/globales.h>
#include <kernel-utils/estados.h>
#include <kernel-utils/procesos.h>
#include <commons/string.h>
#include <utils/comunicacion_kernel_memoria.h>
#include <utils/mensajes.h>


void* planificador_largo_plazo();
void* liberar_hilos_en_exit();

#endif