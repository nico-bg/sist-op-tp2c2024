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

void syscall_finalizar_hilo();
void syscall_crear_hilo(char* archivo_pseudocodigo, uint32_t prioridad);
void syscall_crear_proceso(char* archivo_pseudocodigo, uint32_t tamanio_proceso, uint32_t prioridad);
void syscall_finalizar_proceso();

#endif