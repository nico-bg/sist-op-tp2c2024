#include <utils/mensajes.h>

/* Serializa el paquete como stream del buffer a retornar */
t_buffer* serializar_paquete(t_paquete* paquete)
{
	// Se suma 2 veces el size de uint32 porque uno es para almacenar el codigo_operacion
	// ...y el otro es para almacenar el tamanio del buffer del paquete
	uint32_t tamanio_paquete = 2 * sizeof(uint32_t) + paquete->buffer->size;

	t_buffer* buffer_paquete = buffer_create(tamanio_paquete);

	buffer_add_uint32(buffer_paquete, (uint32_t) paquete->codigo_operacion);
	buffer_add_uint32(buffer_paquete, paquete->buffer->size);
	buffer_add(buffer_paquete, paquete->buffer->stream, paquete->buffer->size);

	return buffer_paquete;
}

void eliminar_paquete(t_paquete* paquete)
{
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

void enviar_mensaje(char* mensaje, int socket_cliente)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = OPERACION_MENSAJE;

	uint32_t mensaje_length = strlen(mensaje) + 1;
	// Sumamos uint32_t para tener en cuenta que se pueda serializar
	// ...el length del mensaje + el mensaje en sí mismo
	paquete->buffer = buffer_create(sizeof(uint32_t) + mensaje_length);
	buffer_add_string(paquete->buffer, mensaje_length, mensaje);

	t_buffer* a_enviar = serializar_paquete(paquete);

	send(socket_cliente, a_enviar->stream, a_enviar->size, 0);

	buffer_destroy(a_enviar);
	eliminar_paquete(paquete);
}


t_buffer* recibir_buffer(uint32_t* size, int socket_cliente)
{
	recv(socket_cliente, size, sizeof(uint32_t), MSG_WAITALL);

	t_buffer* buffer = buffer_create(*size);

	recv(socket_cliente, buffer->stream, *size, MSG_WAITALL);

	return buffer;
}

void recibir_mensaje(int socket_cliente, t_log* logger)
{
	uint32_t size;
	t_buffer* buffer = recibir_buffer(&size, socket_cliente);

	uint32_t mensaje_length;
	char* mensaje = buffer_read_string(buffer, &mensaje_length);

	log_info(logger, "Me llego el mensaje %s", mensaje);

	free(mensaje);
	buffer_destroy(buffer);
}

void atender_peticiones(t_log* logger, t_config* config, int socket_cliente)
{
    while(1) {
		int resultado = atender_peticion(logger, config, socket_cliente);

		/* El cliente se desconectó, por lo que cortamos el bucle */
		if(resultado == -1)
			break;
    }
}

int atender_peticion(t_log* logger, t_config* config, int socket_cliente)
{
	log_info(logger, "Esperando código de operación");
	int codigo_operacion = recibir_operacion(socket_cliente);

	switch(codigo_operacion) {
		case OPERACION_MENSAJE:
			recibir_mensaje(socket_cliente, logger);
			break;
		case -1:
			log_error(logger, "El cliente se desconectó");
			return -1;
		default:
			log_warning(logger, "Operación desconocida: %d", codigo_operacion);
			break;
	}

	return 0;
}

int recibir_operacion(int socket_cliente)
{
	int cod_op;
	if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0)
		return cod_op;
	else {
		close(socket_cliente);
		return -1;
	}
}