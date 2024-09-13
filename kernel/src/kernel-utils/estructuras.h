#ifndef ESTRUCTURAS_H_
#define ESTRUCTURAS_H_

#include <stdlib.h>
#include <commons/collections/list.h>

typedef struct {
    u_int32_t pid;
    // Lista de thread ids del proceso. Esperamos que sea una lista de u_int32_t
    t_list* tids;
    // Lista de mutex creados por los threads del proceso
    t_list* mutex;
} t_pcb;

typedef struct {
    u_int32_t tid;
    u_int32_t prioridad;
} t_tcb;

typedef enum {
    ESTADO_NEW,
    ESTADO_READY,
    ESTADO_EXEC,
    ESTADO_EXIT,
    ESTADO_BLOCKED
} t_nombre_estado;

typedef struct {
    t_nombre_estado nombre_estado;
    t_list* hilos; // Lista de tcb's
} t_estado;

/* Funciones para destruir la memoria de estas estructuras */
void destruir_pcb(t_pcb* pcb);
void destruir_tcb(t_tcb* tcb);
void destruir_estado(t_estado* estado);

#endif