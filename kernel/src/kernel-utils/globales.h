#ifndef GLOBALES_H_
#define GLOBALES_H_

#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/queue.h>
#include <commons/collections/dictionary.h>
#include <kernel-utils/estructuras.h>

/* Loggers y Configuración */
extern t_log* logger;
extern t_log* logger_debug;
extern t_config* config;

/* Lista de PCBs creados */
extern t_list* lista_procesos;

/* Estados */
extern t_estado* estado_new;
extern t_estado* estado_ready;
extern t_estado* estado_exec;
extern t_estado* estado_blocked;
extern t_estado* estado_exit;

#endif