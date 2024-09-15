#ifndef PLANIFICADOR_LARGO_PLAZO_H_
#define PLANIFICADOR_LARGO_PLAZO_H_

#include <kernel-utils/globales.h>
#include <kernel-utils/estados.h>

void planificador_largo_plazo();
void liberar_hilos_en_exit();
int pedir_inicializacion_hilo_a_memoria(t_tcb* hilo);

#endif