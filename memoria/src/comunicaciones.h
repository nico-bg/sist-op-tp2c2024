#include <utils/configuracion.h>
#include <utils/conexiones.h>
#include <utils/mensajes.h>
#include <utils/buffer.h>

#include <utils/comunicacion_cpu_memoria.h>
#include <utils/comunicacion_kernel_memoria.h>

#include "main.h"

/* CPU
typedef struct
{
    uint32_t pid;
    uint32_t tid;
    uint32_t PC;
    uint32_t AX;
    uint32_t BX;
    uint32_t CX;
    uint32_t DX;
    uint32_t EX;
    uint32_t FX;
    uint32_t GX;
    uint32_t HX;
    uint32_t base;
    uint32_t limite;
} t_contexto;

typedef struct {
    uint32_t tid;
    uint32_t pid;
} t_cpu_solicitar_contexto;

typedef struct {
    uint32_t pid;
    uint32_t tid;
    uint32_t PC;
} t_datos_obtener_instruccion;

typedef struct {
    char* instruccion;
} t_datos_devolver_instruccion;
*/


/* KERNEL
typedef struct {
    uint32_t pid;
    uint32_t tid;
    char* archivo_pseudocodigo;
} t_datos_inicializacion_hilo;

typedef struct {
    uint32_t pid;
    uint32_t tid;
} t_datos_finalizacion_hilo;

typedef struct {
    uint32_t pid;
    uint32_t tamanio;
    char* archivo_pseudocodigo;
} t_datos_inicializacion_proceso;

typedef struct {
    uint32_t pid;
} t_datos_finalizacion_proceso;
*/

void* leer_buffer_kernel(int cod_op, int socket_cliente);

void* leer_buffer_cpu(int cod_op, int socket_cliente);

void enviar_buffer(int cod_op, int socket_cliente, void* datos);