#ifndef PLANIFICADOR_CORTO_PLAZO_H_
#define PLANIFICADOR_CORTO_PLAZO_H_

#include <kernel-utils/globales.h>
#include <kernel-utils/estados.h>
#include <kernel-utils/estructuras.h>
#include <utils/mensajes.h>
#include <string.h>
#include <utils/comunicacion_kernel_cpu.h>
#include <utils/sleep.h>

typedef enum {
    DESALOJO_POR_QUANTUM,
    FINALIZACION,
    BLOQUEO,
} t_motivo_devolucion;

void planificador_corto_plazo();

t_tcb* obtener_siguiente_a_exec();
t_tcb* obtener_siguiente_a_exec_fifo();
t_tcb* obtener_siguiente_a_exec_prioridades();
t_tcb* obtener_siguiente_a_exec_colas_multinivel();
void enviar_hilo_a_cpu(t_tcb* hilo);
void transicion_hilo_a_exec(t_tcb* hilo);
t_motivo_devolucion esperar_devolucion_hilo();

#endif