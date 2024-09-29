#ifndef ESTRUCTURAS_H_
#define ESTRUCTURAS_H_

#include <stdlib.h>
#include <stdint.h>
#include <commons/collections/list.h>

typedef struct {
    uint32_t pid;
    // Lista de thread ids del proceso. Esperamos que sea una lista de uint32_t
    t_list* tids;
    // Lista de mutex creados por los threads del proceso
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

typedef struct {
    uint32_t tid;
    uint32_t prioridad;
    // Para poder buscar al proceso padre en `lista_procesos`
    uint32_t pid_padre;
    // Archivo de pseudocodigo a procesar
    char* nombre_archivo;
} t_tcb;

typedef enum {
    ESTADO_NEW,
    ESTADO_READY,
    ESTADO_EXEC,
    ESTADO_EXIT,
    ESTADO_BLOCKED
} t_estado;

#endif