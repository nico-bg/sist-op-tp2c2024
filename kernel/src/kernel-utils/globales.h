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

/* Lista de PCBs/Procesos creados */
extern t_list* lista_procesos;
extern int ULTIMO_PID;

/* Estados para los hilos gestionados por los planificadores */
extern t_list* estado_new;
extern t_list* estado_ready;
extern t_list* estado_exec;
extern t_list* estado_blocked;
extern t_list* estado_exit;

#endif