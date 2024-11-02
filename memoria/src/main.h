#ifndef MAIN_H_
#define MAIN_H_

#include <globales.h>
#include <particiones.h>

#include <utils/configuracion.h>
#include <utils/conexiones.h>
#include <utils/mensajes.h>

#include <utils/comunicacion_cpu_memoria.h>
#include <utils/comunicacion_kernel_memoria.h>

#include <commons/string.h>

/* Para el manejo de instrucciones y archivos */
#define MAX_LINE_LENGTH 255
#define NOMBRE 1
#define PATH 0

typedef struct nodo_hilo nodo_hilo;
typedef struct nodo_proceso nodo_proceso;

typedef struct {
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
    char* archivo_pseudocodigo;
    char** instrucciones;
} estructura_hilo;

struct nodo_hilo {
    estructura_hilo hilo;
    nodo_hilo* siguiente_nodo_hilo;
};

typedef struct {
    uint32_t pid;
    uint32_t tamanio;
    uint32_t base;
    uint32_t limite;
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

bool hay_espacio_en_memoria(uint32_t tamanio);

void terminar_programa(t_config* config, int conexion);

void hilo_kernel(void* socket);

void atender_kernel(void* socket_cliente);

void atender_peticion_kernel(int cod_op, int socket);

int atender_cpu(int socket_cliente);

void atender_peticion_cpu(int cod_op, int socket);



nodo_proceso* buscar_proceso_por_pid(uint32_t pid);

nodo_hilo* buscar_hilo_por_tid(uint32_t pid, uint32_t tid);

void iniciar_proceso(t_datos_inicializacion_proceso* datos, t_particion* particion);

uint32_t finalizar_proceso(t_datos_finalizacion_proceso* datos);

void iniciar_hilo(t_datos_inicializacion_hilo* datos);

void finalizar_hilo(t_datos_finalizacion_hilo* datos);


char** leer_archivo_pseudocodigo(const char* nombre_archivo);

void liberar_instrucciones(char** instrucciones);

int contar_lineas(const char* nombre_archivo);

char* obtener_path_completo(const char* nombre_archivo);

char* obtener_archivo_pseudocodigo(u_int32_t pid, uint32_t tid, int code);

nodo_proceso* buscar_ultimo_proceso(void);

nodo_hilo* buscar_ultimo_hilo(uint32_t pid);


t_contexto* devolver_contexto_ejecucion(t_cpu_solicitar_contexto* datos);

void actualizar_contexto_ejecucion(t_contexto* datos);

char* devolver_instruccion(t_datos_obtener_instruccion* datos);

estructura_hilo* convertir_struct(t_contexto* contexto);


uint32_t leer_memoria(t_datos_leer_memoria* datos);

void escribir_memoria(t_datos_escribir_memoria* datos);

#endif