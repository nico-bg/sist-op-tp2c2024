#ifndef ESTRUCTURAS_H_
#define ESTRUCTURAS_H_

#include <stdlib.h>
#include <stdint.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>

typedef struct {
    uint32_t tid;
    uint32_t prioridad;
    // Para poder buscar al proceso padre en `lista_procesos`
    uint32_t pid_padre;
    // Archivo de pseudocodigo a procesar
    char* nombre_archivo;
    // Lista de TCBs que se encuentren en estado BLOCKED esperando hasta que este hilo finalice
    // Nos va a servir para saber qué hilos mandar a READY cuando este hilo finalice
    t_list* hilos_bloqueados;
} t_tcb;

typedef struct {
    // Nombre del recurso creado con MUTEX_CREATE
    char* recurso;
    // Nos va a servir para validar si el recurso se encuentra asignado o no
    bool esta_libre;
    // Cola de hilos que están esperando el mutex
    t_queue* hilos_bloqueados;
    // Hilo al que se le asignó primero el mutex 
    t_tcb* hilo_asignado;
} t_mutex;

typedef struct {
    uint32_t pid;
    // Lista de thread ids del proceso. Esperamos que sea una lista de uint32_t
    t_list* tids;
    // Lista de t_mutex creados por los threads del proceso
    t_list* mutex;
    // Tamaño del proceso en Memoria
    uint32_t tamanio;
    // Entero incremental para reconocer el siguiente tid a crear
    uint32_t ultimo_tid;
    // Prioridad del hilo principal (TID 0)
    uint32_t prioridad;
    // Archivo de pseudocódigo del hilo principal (TID 0)
    char* nombre_archivo;
} t_pcb;

typedef enum {
    ESTADO_NEW,
    ESTADO_READY,
    ESTADO_EXEC,
    ESTADO_EXIT,
    ESTADO_BLOCKED
} t_estado;

#endif