#include <utils/comunicacion_kernel_cpu.h>

t_buffer* serializar_hilo_a_cpu(t_hilo_a_cpu* hilo)
{
    t_buffer* buffer = buffer_create(2 * sizeof(uint32_t));

    buffer_add_uint32(buffer, hilo->pid);
    buffer_add_uint32(buffer, hilo->tid);

    return buffer;
}

t_hilo_a_cpu* deserializar_hilo_a_cpu(t_buffer* buffer)
{
    t_hilo_a_cpu* hilo_deserializado = malloc(2 * sizeof(uint32_t));

    hilo_deserializado->pid = buffer_read_uint32(buffer);
    hilo_deserializado->tid = buffer_read_uint32(buffer);

    return hilo_deserializado;
}

void destruir_hilo_a_cpu(t_hilo_a_cpu* hilo) {
    free(hilo);
}

t_buffer* serializar_datos_crear_hilo(t_datos_crear_hilo* datos)
{
    int archivo_pseudocodigo_length = strlen(datos->archivo_pseudocodigo) + 1;
    // Son 2 uint32_t, uno para la longitud del nombre del archivo y el otro para la prioridad
    int tamanio_buffer = 2 * sizeof(uint32_t) + archivo_pseudocodigo_length;
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
    int tamanio_buffer = 3 * sizeof(uint32_t) + archivo_pseudocodigo_length;
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

t_buffer* serializar_datos_operacion_mutex(t_datos_operacion_mutex* datos)
{
    int recurso_length = strlen(datos->recurso) + 1;
    t_buffer* buffer = buffer_create(recurso_length + sizeof(uint32_t));

    buffer_add_string(buffer, recurso_length, datos->recurso);

    return buffer;
}

t_datos_operacion_mutex* deserializar_datos_operacion_mutex(t_buffer* buffer)
{
    t_datos_operacion_mutex* datos = malloc(sizeof(t_datos_operacion_mutex));

    uint32_t length;
    datos->recurso = buffer_read_string(buffer, &length);

    return datos;
}

void destruir_datos_operacion_mutex(t_datos_operacion_mutex* datos)
{
    free(datos->recurso);
    free(datos);
}

t_buffer* serializar_datos_operacion_hilo(uint32_t tid)
{
    t_buffer* buffer = buffer_create(sizeof(uint32_t));

    buffer_add_uint32(buffer, tid);

    return buffer;
}

t_datos_operacion_hilo* deserializar_datos_operacion_hilo(t_buffer* buffer)
{
    t_datos_operacion_hilo* datos = malloc(sizeof(t_datos_operacion_hilo));

    datos->tid = buffer_read_uint32(buffer);

    return datos;
}

void destruir_datos_operacion_hilo(t_datos_operacion_hilo* datos)
{
    free(datos);
}

t_buffer* serializar_datos_operacion_io(t_datos_operacion_io* datos)
{
    t_buffer* buffer = buffer_create(sizeof(uint32_t));

    buffer_add_uint32(buffer, datos->tiempo);

    return buffer;
}

t_datos_operacion_io* deserializar_datos_operacion_io(t_buffer* buffer)
{
    t_datos_operacion_io* datos = malloc(sizeof(t_datos_operacion_io));

    datos->tiempo = buffer_read_uint32(buffer);

    return datos;
}

void destruir_datos_operacion_io(t_datos_operacion_io* datos)
{
    free(datos);
}