#ifndef PARTICIONES_H
#define PARTICIONES_H_

#include <globales.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
    uint32_t base;
    uint32_t limite;
    uint32_t tamanio;
    uint32_t pid;
    bool esta_libre;
} t_particion;

void inicializar_particiones();
t_particion* crear_particion_en_indice(uint32_t tamanio, int indice);
t_particion* crear_particion(uint32_t tamanio);
void asignar_particion(t_particion* particion, uint32_t tamanio_proceso, uint32_t pid);
void desasignar_particion(t_particion* particion);
void consolidar_particiones_libres();
t_particion* buscar_particion_libre(uint32_t tamanio);
t_particion* buscar_particion_por_pid(uint32_t pid);
int buscar_indice_particion(t_particion* particion);
void destruir_particion(t_particion* particion);

#endif