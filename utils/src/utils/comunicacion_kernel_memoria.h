#ifndef COMUNICACION_KERNEL_MEMORIA_H_
#define COMUNICACION_KERNEL_MEMORIA_H_

#include <utils/buffer.h>

typedef struct {
    uint32_t pid;
    uint32_t tid;
    char* archivo_pseudocodigo;
} t_datos_inicializacion_hilo;

t_buffer* serializar_datos_inicializacion_hilo(t_datos_inicializacion_hilo* datos);
t_datos_inicializacion_hilo* deserializar_datos_inicializacion_hilo(t_buffer* buffer);
void destruir_datos_inicializacion_hilo(t_datos_inicializacion_hilo* datos);

#endif