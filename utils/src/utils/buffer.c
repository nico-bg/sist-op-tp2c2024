#include <utils/buffer.h>

t_buffer* buffer_create(uint32_t size)
{
    t_buffer* buffer = malloc(sizeof(t_buffer));
    buffer->offset = 0;
    buffer->size = size;
    buffer->stream = malloc(buffer->size);

    return buffer;
}

void buffer_destroy(t_buffer *buffer)
{
    free(buffer->stream);
    free(buffer);
}

void buffer_add(t_buffer *buffer, void *data, uint32_t size)
{
    memcpy(buffer->stream + buffer->offset, data, size);
    buffer->offset += size;
}

void buffer_read(t_buffer *buffer, void *data, uint32_t size)
{
    memcpy(data, buffer->stream + buffer->offset, size);
    buffer->offset += size;
}

void buffer_add_uint32(t_buffer *buffer, uint32_t data) {
    buffer_add(buffer, &data, sizeof(uint32_t));
}

uint32_t buffer_read_uint32(t_buffer *buffer) {
    uint32_t data;
    buffer_read(buffer, &data, sizeof(uint32_t));
    return data;
}

void buffer_add_string(t_buffer *buffer, uint32_t length, char *string) {
    // Primero agregamos el length del string y luego el string en sí
    buffer_add_uint32(buffer, length);
    buffer_add(buffer, string, length);
}

char *buffer_read_string(t_buffer *buffer, uint32_t *length) {
    // Primero leemos la longitud del string, asignando el valor leido al puntero `length`
    *length = buffer_read_uint32(buffer);

    char *string = malloc(*length);
    buffer_read(buffer, string, *length);
    return string;
}

t_buffer *buffer_recibir(int socket) {
    // Leer el tamaño del buffer desde el socket
    uint32_t size;
    if (recv(socket, &size, sizeof(uint32_t), MSG_WAITALL) <= 0) {
        return NULL; // Error o desconexión
    }

    // Crear el buffer
    t_buffer *buffer = buffer_create(size);

    // Leer el contenido del buffer
    if (recv(socket, buffer->stream, size, MSG_WAITALL) <= 0) {
        buffer_destroy(buffer);
        return NULL; // Error o desconexión
    }

    return buffer;
}