#ifndef SYSCALLS_H_
#define SYSCALLS_H_

#include <kernel-utils/estructuras.h>
#include <kernel-utils/globales.h>
#include <kernel-utils/procesos.h>
#include <kernel-utils/conexion-a-memoria.h>
#include <utils/conexiones.h>
#include <utils/buffer.h>
#include <utils/mensajes.h>
#include <utils/comunicacion_kernel_memoria.h>
#include <commons/collections/queue.h>

typedef struct {
    int fd_conexion;
    t_tcb* hilo;
} t_args_esperar_respuesta_dump_memory;

void syscall_finalizar_hilo();
void syscall_crear_hilo(char* archivo_pseudocodigo, uint32_t prioridad);
bool syscall_esperar_hilo(uint32_t tid);
void syscall_crear_proceso(char* archivo_pseudocodigo, uint32_t tamanio_proceso, uint32_t prioridad);
void syscall_finalizar_proceso();
void syscall_crear_mutex(char* recurso);
bool syscall_bloquear_mutex(char* recurso);
void syscall_desbloquear_mutex(char* recurso);
void syscall_io(uint32_t tiempo);
void syscall_dump_memory();

#endif