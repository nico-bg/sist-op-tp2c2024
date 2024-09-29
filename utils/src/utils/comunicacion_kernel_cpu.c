#include <utils/comunicacion_kernel_cpu.h>

t_buffer* serializar_hilo_a_cpu(t_hilo_a_cpu* hilo)
{
    t_buffer* buffer = buffer_create(sizeof(t_hilo_a_cpu));

    buffer_add_uint32(buffer, hilo->tid);
    buffer_add_uint32(buffer, hilo->pid);

    return buffer;
}

t_hilo_a_cpu* deserializar_hilo_a_cpu(t_buffer* buffer)
{
    t_hilo_a_cpu* hilo_deserializado = malloc(sizeof(t_hilo_a_cpu));

    hilo_deserializado->tid = buffer_read_uint32(buffer);
    hilo_deserializado->pid = buffer_read_uint32(buffer);

    return hilo_deserializado;
}

t_buffer* serializar_datos_crear_hilo(t_datos_crear_hilo* datos)
{
    int archivo_pseudocodigo_length = strlen(datos->archivo_pseudocodigo) + 1;
    int tamanio_buffer = sizeof(uint32_t) + archivo_pseudocodigo_length;
    t_buffer* buffer = buffer_create(tamanio_buffer);

    buffer_add_string(buffer, archivo_pseudocodigo_length, datos->archivo_pseudocodigo);
    buffer_add_uint32(buffer, datos->prioridad);

    return buffer;
}

t_datos_crear_hilo* deserializar_datos_crear_hilo(t_buffer* buffer)
{
    t_datos_crear_hilo* datos = malloc(sizeof(t_datos_crear_hilo));

    uint32_t length;
    datos->archivo_pseudocodigo = buffer_read_string(buffer, &length);
    datos->prioridad = buffer_read_uint32(buffer);

    return datos;
}

void destruir_datos_crear_hilo(t_datos_crear_hilo* datos)
{
    free(datos->archivo_pseudocodigo);
    free(datos);
}

t_buffer* serializar_datos_crear_proceso(t_datos_crear_proceso* datos)
{
    int archivo_pseudocodigo_length = strlen(datos->archivo_pseudocodigo) + 1;
    int tamanio_buffer = 2 * sizeof(uint32_t) + archivo_pseudocodigo_length;
    t_buffer* buffer = buffer_create(tamanio_buffer);

    buffer_add_string(buffer, archivo_pseudocodigo_length, datos->archivo_pseudocodigo);
    buffer_add_uint32(buffer, datos->tamanio_proceso);
    buffer_add_uint32(buffer, datos->prioridad);

    return buffer;
}

t_datos_crear_proceso* deserializar_datos_crear_proceso(t_buffer* buffer)
{
    t_datos_crear_proceso* datos = malloc(sizeof(t_datos_crear_proceso));

    uint32_t length;
    datos->archivo_pseudocodigo = buffer_read_string(buffer, &length);
    datos->tamanio_proceso = buffer_read_uint32(buffer);
    datos->prioridad = buffer_read_uint32(buffer);

    return datos;
}

void destruir_datos_crear_proceso(t_datos_crear_proceso* datos)
{
    free(datos->archivo_pseudocodigo);
    free(datos);
}