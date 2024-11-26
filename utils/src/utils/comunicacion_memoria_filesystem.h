#ifndef COMUNICACION_MEMORIA_FILESYSTEM_H_
#define COMUNICACION_MEMORIA_FILESYSTEM_H_

#include <utils/buffer.h>
#include <stdbool.h>
typedef struct {
    char* nombre_archivo;
    uint32_t tamanio;
    void* contenido;
} t_datos_dump_memory_fs;

void enviar_respuesta_operacion(int socket, bool resultado);
t_buffer* serializar_datos_dump_memory_fs(t_datos_dump_memory_fs* datos);
t_datos_dump_memory_fs* deserializar_datos_dump_memory_fs(t_buffer* buffer);
void destruir_datos_dump_memory_fs(t_datos_dump_memory_fs* datos);

#endif
