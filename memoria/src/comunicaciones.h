#include <utils/configuracion.h>
#include <utils/conexiones.h>
#include <utils/mensajes.h>
#include <utils/buffer.h>

#include <utils/comunicacion_cpu_memoria.h>
#include <utils/comunicacion_kernel_memoria.h>

#include "main.h"

typedef struct {
    int pid;
    int tamanio;
    char* archivo_pseudocodigo;
} datos_proceso;

typedef struct {
    int pid;
    int tid;
    uint32_t PC;
    char* archivo_pseudocodigo;
} datos_hilo;

typedef struct {
    uint32_t PC;
    uint32_t AX;
    uint32_t BX;
    uint32_t CX;
    uint32_t DX;
    uint32_t EX;
    uint32_t FX;
    uint32_t GX;
    uint32_t HX;
    uint32_t Base;
    uint32_t Limite;
} datos_contexto_hilo;


void* leer_buffer_kernel(int cod_op);

void* leer_buffer_cpu(int cod_op);

void enviar_buffer(int cod_op, void* datos);

estructura_hilo* convertir_struct(datos_contexto_hilo* datos_contexto_hilo);