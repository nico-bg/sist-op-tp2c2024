#ifndef PLANIFICADOR_CORTO_PLAZO_H_
#define PLANIFICADOR_CORTO_PLAZO_H_

#include <kernel-utils/globales.h>
#include <kernel-utils/estados.h>
#include <kernel-utils/estructuras.h>
#include <utils/mensajes.h>
#include <string.h>
#include <utils/comunicacion_kernel_cpu.h>

typedef enum {
    DESALOJO,
    FINALIZACION,
    BLOQUEO,
} t_motivo_devolucion;

void planificador_corto_plazo();
void planificador_corto_plazo_fifo();
void planificador_corto_plazo_prioridades();
void planificador_corto_plazo_colas_multinivel();

t_tcb* obtener_siguiente_a_exec_fifo();
void enviar_hilo_a_cpu(t_tcb* hilo);
void transicion_hilo_a_exec(t_tcb* hilo);
t_motivo_devolucion esperar_devolucion_hilo();

#endif