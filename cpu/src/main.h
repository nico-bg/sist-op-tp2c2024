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
    LOG, 
    DUMP_MEMORY,
    IO,
    PROCESS_CREATE,
    THREAD_CREATE,
    THREAD_JOIN,
    THREAD_CANCEL,
    MUTEX_CREATE,
    MUTEX_LOCK,
    MUTEX_UNLOCK,
    THREAD_EXIT,
    PROCESS_EXIT
}instruccion;


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

u_int32_t lectura_memoria(u_int32_t dir_fisica);

void escuchar_dispatch();

void ciclo_de_instruccion();

void terminar_programa();


t_buffer* pedir_contexto(int servidor_memoria, t_buffer* buffer_pedido_contexto);

char* pedir_proxima_instruccion(int servidor_memoria, t_buffer* buffer_pedido_devolver_instruccion);

void setear_registro(char *registro, uint32_t valor);

void sum_registro(char * registro1, char * registro2);

void sub_registro(char * registro1, char * registro2);

void read_mem(char * registro1, char * registro2);

void write_mem(char * registro1, char * registro2);

int mmu(int dir_fisica);

int mmu_dirLog_dirfis(int dir_logica);

void jnz_pc(char* registro, char* valor);

uint32_t obtener_registro(char* registro);

void escuchar_interrupciones();

void actualizar_contexto();

void ejecutar_instruccion_mutex(op_code operacion, char* recurso);

void enviar_operacion_a_kernel(op_code operacion);

void ejecutar_instruccion_hilo(op_code operacion, uint32_t tid);

#endif