#ifndef MAIN_H_
#define MAIN_H_

#include <utils/configuracion.h>
#include <utils/conexiones.h>
#include <utils/mensajes.h>


/* Temporal hasta implementar */
#define DEVOLVER_CONTEXTO_EJECUCION 1
#define ACTUALIZAR_CONTEXTO_EJECUCION 2
#define DEVOLVER_INSTRUCCION 3
#define LEER_MEMORIA 4
#define ESCRIBIR_MEMORIA 5

#define CREAR_PROCESO 6
#define FINALIZAR_PROCESO 7
#define CREAR_HILO 8
#define FINALIZAR_HILO 9
#define MEMORY_DUMP 10


typedef struct nodo_hilo nodo_hilo;
typedef struct nodo_proceso nodo_proceso;
typedef struct {
    int tid;
    uint32_t PC;
    uint32_t AX;
    uint32_t BX;
    uint32_t CX;
    uint32_t DX;
    uint32_t EX;
    uint32_t FX;
    uint32_t GX;
    uint32_t HX;
} estructura_hilo;

struct nodo_hilo {
    estructura_hilo hilo;
    nodo_hilo* siguiente;
};

typedef struct {
    int pid;
    int tamanio;
    uint32_t base;
    uint32_t limite;
    char** archivo_pseudocodigo;
    nodo_hilo* lista_hilos;
} estructura_proceso;

struct nodo_proceso {
    estructura_proceso proceso;
    nodo_proceso* lista_proceso;
};

typedef struct {
    t_log* logger;
    int socket;
} parametros_hilo;


void terminar_programa(t_log* logger, t_config* config, int conexion);

void* atender_kernel(void* socket_cliente);

void* atender_peticion_kernel(t_log* logger, int cod_op);

int atender_cpu(t_log* logger, int socket_cliente);

void* atender_peticion_cpu(int cod_op);

void* hilo_kernel(void* args);


estructura_proceso* iniciar_proceso(int pid, int tamanio, uint32_t base, uint32_t limite, const char* archivo_pseudocodigo);

estructura_hilo* agregar_hilo(int pid, int tid);

void finalizar_proceso(int pid);

void finalizar_hilo(int pid, int tid);

estructura_proceso* buscar_proceso_por_pid(int pid);

estructura_hilo* buscar_hilo_por_tid(int pid, int tid);

void liberar_instrucciones(char** instrucciones);

char** leer_archivo_pseudocodigo(const char* nombre_archivo);

int contar_lineas(const char* nombre_archivo);

#endif