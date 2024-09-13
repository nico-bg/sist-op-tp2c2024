#include <kernel-utils/procesos.h>

/*
 * @brief Crea un proceso y su hilo principal, los agrega a sus respectivas colas de NEW.
 * @brief Representa a la syscall de PROCESS_CREATE.
 * @param nombre_archivo Nombre del archivo de pseudocódigo que deberá ejecutar el proceso
 * @param tamanio_proceso Tamaño del proceso en Memoria
 * @param prioridad Prioridad del hilo principal (TID 0)
 */
t_pcb* crear_proceso(char* nombre_archivo, u_int32_t tamanio_proceso, u_int32_t prioridad)
{
    t_pcb* nuevo_proceso;

    log_info(logger, "## (%d:0) Se crea el proceso - Estado: NEW ##", nuevo_proceso->pid);

    return nuevo_proceso;
}