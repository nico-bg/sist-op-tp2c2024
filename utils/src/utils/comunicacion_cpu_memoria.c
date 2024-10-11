#include <utils/comunicacion_cpu_memoria.h>

t_buffer* seralizar_datos_solicitar_contexto(t_cpu_solicitar_contexto* datos){

    t_buffer* buffer = buffer_create(2 * sizeof(uint32_t));

    buffer_add_uint32(buffer, datos->pid);
    buffer_add_uint32(buffer, datos->tid);

    return buffer;
}

t_cpu_solicitar_contexto* deserializar_datos_solicitar_contexto(t_buffer* buffer){

    t_cpu_solicitar_contexto* datos = malloc(sizeof(t_cpu_solicitar_contexto));

    datos->pid = buffer_read_uint32(buffer);
    datos->tid = buffer_read_uint32(buffer);

    return datos;
}

void destruir_datos_solicitar_contexto(t_cpu_solicitar_contexto* datos){
    free(datos);
}


t_buffer* serializar_datos_contexto(t_contexto* datos){

    t_buffer* buffer = buffer_create(13 * sizeof(uint32_t));

    buffer_add_uint32(buffer, datos->pid);
    buffer_add_uint32(buffer, datos->tid);
    buffer_add_uint32(buffer, datos->PC);
    buffer_add_uint32(buffer, datos->AX);
    buffer_add_uint32(buffer, datos->BX);
    buffer_add_uint32(buffer, datos->CX);
    buffer_add_uint32(buffer, datos->DX);
    buffer_add_uint32(buffer, datos->EX);
    buffer_add_uint32(buffer, datos->FX);
    buffer_add_uint32(buffer, datos->GX);
    buffer_add_uint32(buffer, datos->HX);
    buffer_add_uint32(buffer, datos->base);
    buffer_add_uint32(buffer, datos->limite);

    return buffer;
}

t_contexto* deserializar_datos_contexto(t_buffer* buffer){

    t_contexto* datos = malloc(sizeof(t_contexto));
    
    datos->pid = buffer_read_uint32(buffer);
    datos->tid = buffer_read_uint32(buffer);
    datos->PC = buffer_read_uint32(buffer);
    datos->AX = buffer_read_uint32(buffer);
    datos->BX = buffer_read_uint32(buffer);
    datos->CX = buffer_read_uint32(buffer);
    datos->DX = buffer_read_uint32(buffer);
    datos->EX = buffer_read_uint32(buffer);
    datos->FX = buffer_read_uint32(buffer);
    datos->GX = buffer_read_uint32(buffer);
    datos->HX = buffer_read_uint32(buffer);
    datos->base = buffer_read_uint32(buffer);
    datos->limite = buffer_read_uint32(buffer);

    return datos;
}

void destruir_datos_contexto(t_contexto* datos){
    free(datos);
}


t_buffer* serializar_datos_solicitar_instruccion(t_datos_obtener_instruccion* datos){

    t_buffer* buffer = buffer_create(3 * sizeof(uint32_t));

    buffer_add_uint32(buffer, datos->pid);
    buffer_add_uint32(buffer, datos->tid);
    buffer_add_uint32(buffer, datos->PC);

    return buffer;
}

t_datos_obtener_instruccion* deserializar_datos_solicitar_instruccion(t_buffer* buffer){

    t_datos_obtener_instruccion* datos = malloc(sizeof(t_datos_obtener_instruccion));

    datos->pid = buffer_read_uint32(buffer);
    datos->tid = buffer_read_uint32(buffer);
    datos->PC = buffer_read_uint32(buffer);

    return datos;
}

void destruir_datos_solicitar_instruccion(t_datos_obtener_instruccion* datos){
    free(datos);
}


t_buffer* serializar_datos_devolver_instruccion(t_datos_devolver_instruccion* datos){

    t_buffer* buffer = buffer_create(sizeof(t_datos_devolver_instruccion));
    uint32_t length = strlen(datos->instruccion);

    buffer_add_string(buffer, length, datos->instruccion);

    return buffer;
}

t_datos_devolver_instruccion* deserializar_datos_devolver_instruccion(t_buffer* buffer){

    t_datos_devolver_instruccion* datos = malloc(sizeof(t_datos_devolver_instruccion));
    
    uint32_t length;

    datos->instruccion = buffer_read_string(buffer, &length);

    return datos;
}

void destruir_datos_devolver_instruccion(t_datos_devolver_instruccion* datos){
    free(datos);
}

