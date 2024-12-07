#include <utils/configuracion.h>
#include <utils/conexiones.h>
#include <utils/mensajes.h>
#include <utils/buffer.h>

#include <utils/comunicacion_cpu_memoria.h>
#include <utils/comunicacion_kernel_memoria.h>
#include <utils/comunicacion_memoria_filesystem.h>
#include <utils/timestamp.h>

#include "main.h"

void* leer_buffer_kernel(int cod_op, int socket_cliente);

void* leer_buffer_cpu(int cod_op, int socket_cliente);

void enviar_buffer(int cod_op, int socket_cliente, void* datos);

void confirmar_operacion(int socket_cliente);

void notificar_error(int socket_cliente);

op_code enviar_dump_memory(int socket_filesystem, t_datos_dump_memory* datos_kernel);