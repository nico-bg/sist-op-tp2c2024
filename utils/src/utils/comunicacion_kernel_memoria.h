#ifndef COMUNICACION_KERNEL_MEMORIA_H_
#define COMUNICACION_KERNEL_MEMORIA_H_

#include <utils/buffer.h>

typedef struct {
    uint32_t pid;
    uint32_t tid;
    char* archivo_pseudocodigo;
} t_datos_inicializacion_hilo;

typedef struct {
    uint32_t pid;
    uint32_t tid;
} t_datos_finalizacion_hilo;

typedef struct {
    uint32_t pid;
    uint32_t tamanio;
    char* archivo_pseudocodigo;
} t_datos_inicializacion_proceso;

typedef struct {
    uint32_t pid;
} t_datos_finalizacion_proceso;


t_buffer* serializar_datos_inicializacion_hilo(t_datos_inicializacion_hilo* datos);
t_datos_inicializacion_hilo* deserializar_datos_inicializacion_hilo(t_buffer* buffer);
void destruir_datos_inicializacion_hilo(t_datos_inicializacion_hilo* datos);

t_buffer* serializar_datos_finalizacion_hilo(t_datos_finalizacion_hilo* datos);
t_datos_finalizacion_hilo* deserializar_datos_finalizacion_hilo(t_buffer* buffer);
void destruir_datos_finalizacion_hilo(t_datos_finalizacion_hilo* datos);

t_buffer* serializar_datos_inicializacion_proceso(t_datos_inicializacion_proceso* datos);
t_datos_inicializacion_proceso* deserializar_datos_inicializacion_proceso(t_buffer* buffer);
void destruir_datos_inicializacion_proceso(t_datos_inicializacion_proceso* datos);

t_buffer* serializar_datos_finalizacion_proceso(t_datos_finalizacion_proceso* datos);
t_datos_finalizacion_proceso* deserializar_datos_finalizacion_proceso(t_buffer* buffer);
void destruir_datos_finalizacion_proceso(t_datos_finalizacion_proceso* datos);

#endif