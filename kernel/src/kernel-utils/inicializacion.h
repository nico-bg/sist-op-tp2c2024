#ifndef INICIALIZACION_H_
#define INICIALIZACION_H_

#include <stdlib.h>
#include <stdio.h>
#include <commons/string.h>
#include <kernel-utils/globales.h>

typedef struct {
    char* archivo_pseudocodigo;
    u_int32_t tamanio_proceso;
} t_argumentos;

t_argumentos* procesar_argumentos(int argc, char* argv[]);
void destruir_argumentos(t_argumentos* argumentos);

#endif