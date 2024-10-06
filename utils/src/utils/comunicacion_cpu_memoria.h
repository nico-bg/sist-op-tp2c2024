#ifndef COMUNICACION_CPU_MEMORIA_H_
#define COMUNICACION_CPU_MEMORIA_H_

#include <utils/buffer.h>

typedef struct
{
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
} t_contexto;


typedef struct {
    uint32_t tid;
    uint32_t pid;
} t_cpu_solicitar_contexto;

t_buffer* serializar_pedir_contexto(t_cpu_solicitar_contexto* hilo);

t_contexto deserializar_datos_contexto(t_buffer* buffer);

t_buffer* serializar_datos_contexto(t_contexto* contexto);

t_buffer* serializar_solicitar_datos_contexto();

#endif
