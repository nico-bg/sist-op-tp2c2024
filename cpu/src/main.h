#ifndef MAIN_H_
#define MAIN_H_

#include <utils/configuracion.h>
#include <utils/conexiones.h>
#include <utils/mensajes.h>
#include <utils/comunicacion_kernel_cpu.h>
#include <semaphore.h>


typedef struct {
    int cliente_socket;
    int cliente_memoria;
    t_log *logger;
} t_thread_args;

typedef enum{
    SET,
    READ_MEM,
    WRITE_MEM,
    SUM,
    SUB,
    JNZ,
    LOG
}instruccion;

typedef struct{
    instruccion instruc;
    int algo1;
    int algo2;
}t_instruccion;

typedef struct{
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
}t_contexto;

t_hilo_a_cpu* pcb;

t_contexto contexto;

sem_t sem_ciclo_de_instruccion;

void escuchar_dispatch();

void ciclo_de_instruccion();

void terminar_programa(t_log* logger, t_config* config, int conexion);


#endif