#ifndef COMUNICACION_KERNEL_CPU_H_
#define COMUNICACION_KERNEL_CPU_H_

#include <utils/buffer.h>

typedef struct {
    uint32_t tid;
    uint32_t pid;
} t_hilo_a_cpu;

typedef struct {
    char* archivo_pseudocodigo;
    uint32_t prioridad;
} t_datos_crear_hilo;

t_buffer* serializar_hilo_a_cpu(t_hilo_a_cpu* hilo);
t_hilo_a_cpu* deserializar_hilo_a_cpu(t_buffer* buffer);
t_buffer* serializar_datos_crear_hilo(t_datos_crear_hilo* datos);
t_datos_crear_hilo* deserializar_datos_crear_hilo(t_buffer* buffer);
void destruir_datos_crear_hilo(t_datos_crear_hilo* datos);

#endif