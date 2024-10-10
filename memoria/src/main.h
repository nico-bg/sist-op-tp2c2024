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

static nodo_proceso* nodo_primer_proceso = NULL;

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
    char** archivo_pseudocodigo_th;
} estructura_hilo;

struct nodo_hilo {
    estructura_hilo hilo;
    nodo_hilo* siguiente_nodo_hilo;
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
    nodo_proceso* siguiente_nodo_proceso;
};

typedef struct {
    t_log* logger;
    int socket;
} parametros_hilo;

bool hay_espacio_en_memoria(int tamanio);

void terminar_programa(t_config* config, int conexion);

void hilo_kernel(void* socket);

void atender_kernel(void* socket_cliente);

void atender_peticion_kernel(int cod_op, int socket);

int atender_cpu(int socket_cliente);

void atender_peticion_cpu(int cod_op, int socket);



nodo_proceso* buscar_proceso_por_pid(int pid);

nodo_hilo* buscar_hilo_por_tid(int pid, int tid);

void iniciar_proceso(int pid, int tamanio, const char* archivo_pseudocodigo);

void finalizar_proceso(int pid);

void iniciar_hilo(int pid, int tid, const char* archivo_pseudocodigo);

void finalizar_hilo(int pid, int tid);


char** leer_archivo_pseudocodigo(const char* archivo_pseudocodigo);

void liberar_instrucciones(char** instrucciones);

int contar_lineas(const char* nombre_archivo);

nodo_proceso* buscar_ultimo_proceso(void);

nodo_hilo* buscar_ultimo_hilo(int pid);


estructura_hilo* devolver_contexto_ejecucion(int pid, int tid);

void actualizar_contexto_ejecucion(int pid, int tid, estructura_hilo* contexto_hilo);

char* devolver_instruccion(int pid, int tid, int PC);

#endif