#ifndef MAIN_H_
#define MAIN_H_

#include <utils/configuracion.h>
#include <utils/conexiones.h>
#include <utils/mensajes.h>
#include <commons/bitarray.h>
#include <pthread.h>
#include <sys/stat.h>

void inicializar_filesystem();

void atender_memoria(void*);
void atender_peticion_filesystem_memoria(int cod_op, int socket);

void terminar_programa(t_log* logger, t_config* config);




#endif