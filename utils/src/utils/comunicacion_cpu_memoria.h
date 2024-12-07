#ifndef COMUNICACION_CPU_MEMORIA_H_
#define COMUNICACION_CPU_MEMORIA_H_

#include <utils/buffer.h>

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
    uint32_t Base;
    uint32_t Limite;
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

typedef struct {
    uint32_t pid;
    uint32_t tid;
    uint32_t dir_fisica;
    uint32_t tamanio;
} t_datos_leer_memoria;

typedef struct {
    uint32_t pid;
    uint32_t tid;
    uint32_t dir_fisica;
    uint32_t tamanio;
    uint32_t dato_a_escribir;
} t_datos_escribir_memoria;

t_buffer* seralizar_datos_solicitar_contexto(t_cpu_solicitar_contexto* datos);
t_cpu_solicitar_contexto* deserializar_datos_solicitar_contexto(t_buffer* buffer);
void destruir_datos_solicitar_contexto(t_cpu_solicitar_contexto* datos);

t_buffer* serializar_datos_contexto(t_contexto* datos);
t_contexto deserializar_datos_contexto(t_buffer* buffer);
t_contexto* deserializar_datos_contexto_memoria(t_buffer* buffer);
void destruir_datos_contexto(t_contexto* datos);

t_buffer* serializar_datos_solicitar_instruccion(t_datos_obtener_instruccion* datos);
t_datos_obtener_instruccion* deserializar_datos_solicitar_instruccion(t_buffer* buffer);
void destruir_datos_solicitar_instruccion(t_datos_obtener_instruccion* datos);

t_buffer* serializar_datos_devolver_instruccion(t_datos_devolver_instruccion* datos);
t_datos_devolver_instruccion* deserializar_datos_devolver_instruccion(t_buffer* buffer);
void destruir_datos_devolver_instruccion(t_datos_devolver_instruccion* datos);

t_buffer* serializar_datos_leer_memoria(t_datos_leer_memoria* datos);
t_datos_leer_memoria* deserializar_datos_leer_memoria(t_buffer* buffer);
void destruir_datos_leer_memoria(t_datos_leer_memoria* datos);

t_buffer* serializar_datos_escribir_memoria(t_datos_escribir_memoria* datos);
t_datos_escribir_memoria* deserializar_datos_escribir_memoria(t_buffer* buffer);
void destruir_datos_escribir_memoria(t_datos_escribir_memoria* datos);

#endif
