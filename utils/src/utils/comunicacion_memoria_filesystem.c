#include <utils/comunicacion_memoria_filesystem.h>


t_buffer* serializar_datos_dump_memory_fs(t_datos_dump_memory_fs* datos)
{
    int tamanio_nombre_archivo =  strlen(datos->nombre_archivo) + 1;
    // Sumamos un uint32_t para guardar la longitud del nombre_archivo
    // El otro uint32_t es para guardar el atributo tamanio y sumamos datos->tamanio para guardar los datos de la memoria
    int tamanio = sizeof(uint32_t) + tamanio_nombre_archivo + sizeof(uint32_t) + datos->tamanio;
    t_buffer* buffer = buffer_create(tamanio);

    buffer_add_string(buffer, tamanio_nombre_archivo, datos->nombre_archivo);
    buffer_add_uint32(buffer, datos->tamanio);
    buffer_add(buffer, datos->contenido, datos->tamanio);

    return buffer;
}

t_datos_dump_memory_fs* deserializar_datos_dump_memory_fs(t_buffer* buffer)
{
    t_datos_dump_memory_fs* datos = malloc(sizeof(t_datos_dump_memory_fs));

    uint32_t length;
    datos->nombre_archivo = buffer_read_string(buffer, &length);
    datos->tamanio = buffer_read_uint32(buffer);
    buffer_read(buffer, datos->contenido, datos->tamanio);

    return datos;
}

void enviar_respuesta_operacion(int socket, bool resultado) {
    uint32_t respuesta = resultado ? 1 : 0;
    send(socket, &respuesta, sizeof(uint32_t), 0);
}

void destruir_datos_dump_memory_fs(t_datos_dump_memory_fs* datos)
{
    free(datos->nombre_archivo);
    // free(datos->contenido);
    free(datos);
}