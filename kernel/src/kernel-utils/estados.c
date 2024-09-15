#include <kernel-utils/estados.h>

t_list* lista_procesos;
t_list* estado_new;
t_list* estado_ready;
t_list* estado_exec;
t_list* estado_blocked;
t_list* estado_exit;

/*
 * @brief Inicializa las variables globales de estados y la lista de procesos
 */
void inicializar_estados()
{
    lista_procesos = list_create();
    estado_new = list_create();
    estado_ready = list_create();
    estado_exec = list_create();
    estado_blocked = list_create();
    estado_exit = list_create();
}

/**
 * @brief Libera la memoria de las variables globales de lista de procesos y listas de estado de hilos
 */
void destruir_estados()
{
    list_destroy_and_destroy_elements(lista_procesos, (void*) destruir_pcb);
    list_destroy_and_destroy_elements(estado_new, (void*) destruir_tcb);
    list_destroy_and_destroy_elements(estado_ready, (void*) destruir_tcb);
    list_destroy_and_destroy_elements(estado_exec, (void*) destruir_tcb);
    list_destroy_and_destroy_elements(estado_blocked, (void*) destruir_tcb);
    list_destroy_and_destroy_elements(estado_exit, (void*) destruir_tcb);
}