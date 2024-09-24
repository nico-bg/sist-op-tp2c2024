#ifndef COMUNICACION_KERNEL_CPU_H_
#define COMUNICACION_KERNEL_CPU_H_

#include <utils/buffer.h>

typedef struct {
    uint32_t tid;
    uint32_t pid;
} t_hilo_a_cpu;

typedef enum {
    DEVOLUCION_DESALOJO_QUANTUM,
    DEVOLUCION_FINALIZACION,
    DEVOLUCION_BLOQUEO,
} t_motivo_devolucion;

t_buffer* serializar_hilo_a_cpu(t_hilo_a_cpu* hilo);
t_hilo_a_cpu* deserializar_hilo_a_cpu(t_buffer* buffer);

#endif