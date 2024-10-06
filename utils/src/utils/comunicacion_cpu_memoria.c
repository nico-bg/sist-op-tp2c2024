#include <utils/comunicacion_cpu_memoria.h>


t_buffer* serializar_pedir_contexto(t_cpu_solicitar_contexto* hilo)
{
    t_buffer* buffer = buffer_create(sizeof(t_cpu_solicitar_contexto));

    buffer_add_uint32(buffer, hilo->tid);
    buffer_add_uint32(buffer, hilo->pid);

    return buffer;
}
/*
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
*/


t_buffer* serializar_datos_contexto(t_contexto* contexto)
{
    t_buffer* buffer = buffer_create(sizeof(t_contexto));

    buffer_add_uint32(buffer, contexto->PC);
    buffer_add_uint32(buffer, contexto->AX);
    buffer_add_uint32(buffer, contexto->BX);
    buffer_add_uint32(buffer, contexto->CX);
    buffer_add_uint32(buffer, contexto->DX);   
    buffer_add_uint32(buffer, contexto->EX);
    buffer_add_uint32(buffer, contexto->FX);
    buffer_add_uint32(buffer, contexto->GX);
    buffer_add_uint32(buffer, contexto->HX);
    buffer_add_uint32(buffer, contexto->Base);
    buffer_add_uint32(buffer, contexto->Limite);
    return buffer;
}

t_contexto deserializar_datos_contexto(t_buffer* buffer)
{
    t_contexto datos_contexto;// = malloc(sizeof(t_contexto));

    datos_contexto.PC = buffer_read_uint32(buffer);
    datos_contexto.AX = buffer_read_uint32(buffer);
    datos_contexto.BX = buffer_read_uint32(buffer);
    datos_contexto.CX = buffer_read_uint32(buffer);
    datos_contexto.DX = buffer_read_uint32(buffer);
    datos_contexto.EX = buffer_read_uint32(buffer);
    datos_contexto.FX = buffer_read_uint32(buffer);
    datos_contexto.GX = buffer_read_uint32(buffer);
    datos_contexto.HX = buffer_read_uint32(buffer);
    datos_contexto.Base = buffer_read_uint32(buffer);
    datos_contexto.Limite = buffer_read_uint32(buffer);
    return datos_contexto;
}

