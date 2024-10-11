#ifndef MAIN_H_
#define MAIN_H_

#include <utils/configuracion.h>
#include <utils/conexiones.h>
#include <utils/mensajes.h>
#include <utils/comunicacion_kernel_cpu.h>
#include <utils/comunicacion_cpu_memoria.h>
#include <semaphore.h>


typedef enum
{
    SET,
    READ_MEM,
    WRITE_MEM,
    SUM,
    SUB,
    JNZ,
    LOG
} instruccion;

typedef struct
{
    instruccion instruc;
    int algo1;
    int algo2;
} t_instruccion;

typedef struct
{
    int cliente_socket;
    int cliente_memoria;
    t_log *logger;
} t_thread_args;

t_hilo_a_cpu* pcb;

t_contexto contexto;

//sem_t sem_ciclo_de_instruccion;

void escuchar_dispatch();

void ciclo_de_instruccion();

void terminar_programa();


t_buffer* pedir_contexto(int servidor_memoria, t_buffer* buffer_pedido_contexto);

void setear_registro(char * registro, char * valor);

void sum_registro(char * registro1, char * registro2);

void sub_registro(char * registro1, char * registro2);

void read_mem(char * registro1, char * registro2);

void write_mem(char * registro1, char * registro2);

int mmu(int dir_fisica);

int mmu_dirLog_dirfis(int dir_logica);

void jnz_pc(char* registro, char* valor);


#endif