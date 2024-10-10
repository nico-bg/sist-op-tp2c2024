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

typedef struct {
    char* archivo_pseudocodigo;
    uint32_t tamanio_proceso;
    uint32_t prioridad;
} t_datos_crear_proceso;

typedef struct {
    char* recurso;
} t_datos_operacion_mutex;

typedef struct {
    uint32_t tid;
} t_datos_operacion_hilo;

t_buffer* serializar_hilo_a_cpu(t_hilo_a_cpu* hilo);
t_hilo_a_cpu* deserializar_hilo_a_cpu(t_buffer* buffer);

t_buffer* serializar_datos_crear_hilo(t_datos_crear_hilo* datos);
t_datos_crear_hilo* deserializar_datos_crear_hilo(t_buffer* buffer);
void destruir_datos_crear_hilo(t_datos_crear_hilo* datos);

t_buffer* serializar_datos_crear_proceso(t_datos_crear_proceso* datos);
t_datos_crear_proceso* deserializar_datos_crear_proceso(t_buffer* buffer);
void destruir_datos_crear_proceso(t_datos_crear_proceso* datos);

t_buffer* serializar_datos_operacion_mutex(t_datos_operacion_mutex* datos);
t_datos_operacion_mutex* deserializar_datos_operacion_mutex(t_buffer* buffer);
void destruir_datos_operacion_mutex(t_datos_operacion_mutex* datos);

t_buffer* serializar_datos_operacion_hilo(uint32_t tid);
t_datos_operacion_hilo* deserializar_datos_operacion_hilo(t_buffer* buffer);
void destruir_datos_operacion_hilo(t_datos_operacion_hilo* datos);

#endif