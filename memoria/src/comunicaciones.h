#include <utils/configuracion.h>
#include <utils/conexiones.h>
#include <utils/mensajes.h>
#include <utils/buffer.h>

#include <utils/comunicacion_cpu_memoria.h>
#include <utils/comunicacion_kernel_memoria.h>

#include "main.h"

void* leer_buffer_kernel(int cod_op, int socket_cliente);

void* leer_buffer_cpu(int cod_op, int socket_cliente);

void enviar_buffer(int cod_op, int socket_cliente, void* datos);

void confirmar_operacion(int socket_cliente);

void notificar_error(int socket_cliente);