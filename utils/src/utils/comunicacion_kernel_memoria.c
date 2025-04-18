#include <utils/comunicacion_kernel_memoria.h>

t_buffer* serializar_datos_inicializacion_hilo(t_datos_inicializacion_hilo* datos)
{
    int archivo_pseudocodigo_length = strlen(datos->archivo_pseudocodigo) + 1;
    int tamanio_buffer = 3 * sizeof(uint32_t) + archivo_pseudocodigo_length;
    t_buffer* buffer = buffer_create(tamanio_buffer);

    buffer_add_uint32(buffer, datos->pid);
    buffer_add_uint32(buffer, datos->tid);
    buffer_add_string(buffer, archivo_pseudocodigo_length, datos->archivo_pseudocodigo);

    return buffer;
}

t_datos_inicializacion_hilo* deserializar_datos_inicializacion_hilo(t_buffer* buffer)
{
    t_datos_inicializacion_hilo* datos = malloc(sizeof(t_datos_inicializacion_hilo));

    datos->pid = buffer_read_uint32(buffer);
    datos->tid = buffer_read_uint32(buffer);

    uint32_t archivo_pseudocodigo_length;
    datos->archivo_pseudocodigo = buffer_read_string(buffer, &archivo_pseudocodigo_length);

    return datos;
}

void destruir_datos_inicializacion_hilo(t_datos_inicializacion_hilo* datos)
{
    free(datos->archivo_pseudocodigo);
    free(datos);
}


t_buffer* serializar_datos_finalizacion_hilo(t_datos_finalizacion_hilo* datos)
{
    t_buffer* buffer = buffer_create(2 * sizeof(uint32_t));
    buffer_add_uint32(buffer, datos->pid);
    buffer_add_uint32(buffer, datos->tid);

    return buffer;
}

t_datos_finalizacion_hilo* deserializar_datos_finalizacion_hilo(t_buffer* buffer)
{
    t_datos_finalizacion_hilo* datos = malloc(sizeof(t_datos_finalizacion_hilo));
    datos->pid = buffer_read_uint32(buffer);
    datos->tid = buffer_read_uint32(buffer);

    return datos;
}

void destruir_datos_finalizacion_hilo(t_datos_finalizacion_hilo* datos)
{
    free(datos);
}


t_buffer* serializar_datos_inicializacion_proceso(t_datos_inicializacion_proceso* datos)
{
    t_buffer* buffer = buffer_create(2 * sizeof(uint32_t));

    buffer_add_uint32(buffer, datos->pid);
    buffer_add_uint32(buffer, datos->tamanio);

    return buffer;
}

t_datos_inicializacion_proceso* deserializar_datos_inicializacion_proceso(t_buffer* buffer){

    t_datos_inicializacion_proceso* datos = malloc(sizeof(t_datos_inicializacion_proceso));

    datos->pid = buffer_read_uint32(buffer);
    datos->tamanio = buffer_read_uint32(buffer);

    return datos;
}

void destruir_datos_inicializacion_proceso(t_datos_inicializacion_proceso* datos)
{
    free(datos);
}


t_buffer* serializar_datos_finalizacion_proceso(t_datos_finalizacion_proceso* datos){

    t_buffer* buffer = buffer_create(sizeof(uint32_t));

    buffer_add_uint32(buffer, datos->pid);

    return buffer;
}

t_datos_finalizacion_proceso* deserializar_datos_finalizacion_proceso(t_buffer* buffer){

    t_datos_finalizacion_proceso* datos = malloc(sizeof(t_datos_finalizacion_proceso));

    datos->pid = buffer_read_uint32(buffer);

    return datos;
}

void destruir_datos_finalizacion_proceso(t_datos_finalizacion_proceso* datos){
    free(datos);
}

t_buffer* serializar_datos_dump_memory(t_datos_dump_memory* datos)
{
    t_buffer* buffer = buffer_create(2 * sizeof(uint32_t));

    buffer_add_uint32(buffer, datos->pid);
    buffer_add_uint32(buffer, datos->tid);

    return buffer;
}

t_datos_dump_memory* deserializar_datos_dump_memory(t_buffer* buffer)
{
    t_datos_dump_memory* datos = malloc(sizeof(t_datos_dump_memory));

    datos->pid = buffer_read_uint32(buffer);
    datos->tid = buffer_read_uint32(buffer);

    return datos;
}

void destruir_datos_dump_memory(t_datos_dump_memory* datos)
{
    free(datos);
}
