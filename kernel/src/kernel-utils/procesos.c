#include <kernel-utils/procesos.h>

/**
 * @brief Representa a la syscall de PROCESS_CREATE.
 * 
 * @brief Crea un proceso y su hilo principal, los agrega a sus respectivas estructuras globales.
 * @brief El pcb se agrega a `lista_procesos`, y el tcb principal se agrega a `estado_new`
 * @param nombre_archivo Nombre del archivo de pseudocódigo que deberá ejecutar el proceso
 * @param tamanio_proceso Tamaño del proceso en Memoria
 * @param prioridad Prioridad del hilo principal (TID 0)
 */
t_pcb* crear_proceso(char* nombre_archivo, uint32_t tamanio_proceso, uint32_t prioridad)
{
    // Reservar memoria para el proceso
    t_pcb* nuevo_proceso;

    // Inicializar pcb, incrementar en 1 la variable global ULTIMO_PID. Agregarlo a la variable global `lista_procesos`

    // Crear hilo (tcb) principal. Agregarlo a la variable global `estado_new`

    log_info(logger, "## (%d:0) Se crea el proceso - Estado: NEW ##", nuevo_proceso->pid);

    return nuevo_proceso;
}

/**
 * @brief Libera la memoria del pcb dado
 */
void destruir_pcb(t_pcb* pcb)
{
    list_destroy_and_destroy_elements(pcb->tids, free);
    // TODO: Revisar si es correcto usar solo free para destruir el mutex
    list_destroy_and_destroy_elements(pcb->mutex, free);
    free(pcb);
}

/**
 * @brief Libera la memoria del tcb dado
 */
void destruir_tcb(t_tcb* tcb)
{
    free(tcb);
}