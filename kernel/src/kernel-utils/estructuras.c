#include <kernel-utils/estructuras.h>

void destruir_pcb(t_pcb* pcb)
{
    list_destroy_and_destroy_elements(pcb->tids, free);
    // TODO: Revisar si es correcto usar solo free para destruir el mutex
    list_destroy_and_destroy_elements(pcb->mutex, free);
    free(pcb);
}

void destruir_tcb(t_tcb* tcb)
{
    free(tcb);
}

void destruir_estado(t_estado* estado)
{
    list_destroy_and_destroy_elements(estado->hilos, (void*) destruir_tcb);
    free(estado);
}