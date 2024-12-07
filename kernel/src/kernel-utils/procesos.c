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
void crear_proceso(char* nombre_archivo, uint32_t tamanio_proceso, uint32_t prioridad)
{
    // Inicializamos el nuevo proceso con PID incremental
    t_pcb* nuevo_proceso = malloc(sizeof(t_pcb));
    nuevo_proceso->pid = ULTIMO_PID + 1;
    nuevo_proceso->tids = list_create();
    nuevo_proceso->mutex = list_create();
    nuevo_proceso->tamanio = tamanio_proceso;
    nuevo_proceso->ultimo_tid = 0;
    nuevo_proceso->prioridad = prioridad;
    nuevo_proceso->nombre_archivo = string_duplicate(nombre_archivo);

    ULTIMO_PID = nuevo_proceso->pid;

    // Agregamos el proceso a nuestra `lista_procesos`
    pthread_mutex_lock(&mutex_lista_procesos);
    list_add(lista_procesos, nuevo_proceso);
    pthread_mutex_unlock(&mutex_lista_procesos);

    // Agregamos el proceso al estado NEW
    pthread_mutex_lock(&mutex_estado_new);
    list_add(estado_new, nuevo_proceso);
    pthread_mutex_unlock(&mutex_estado_new);

    // Informamos al planificador de largo plazo que tiene hilos para procesar
    sem_post(&semaforo_estado_new);

    log_info(logger, "## (%d:0) Se crea el proceso - Estado: NEW ##", nuevo_proceso->pid);
}

/**
 * @brief Libera la memoria del t_mutex dado
 */
void destruir_mutex(void* elemento)
{
    t_mutex* mutex = (t_mutex*) elemento;
    mutex->hilo_asignado = NULL;
    queue_destroy(mutex->hilos_bloqueados);
    free(mutex->recurso);
    free(mutex);
}

/**
 * @brief Libera la memoria del pcb dado
 */
void destruir_pcb(t_pcb* pcb)
{
    list_destroy(pcb->tids);
    list_destroy_and_destroy_elements(pcb->mutex, destruir_mutex);
    free(pcb->nombre_archivo);
    free(pcb);
}

/**
 * @brief Libera la memoria del tcb dado
 */
void destruir_tcb(t_tcb* tcb)
{
    list_destroy(tcb->hilos_bloqueados);
    free(tcb->nombre_archivo);
    free(tcb);
}