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
