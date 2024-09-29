#ifndef GLOBALES_H_
#define GLOBALES_H_

#include <semaphore.h>
#include <pthread.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/queue.h>
#include <commons/collections/dictionary.h>
#include <kernel-utils/estructuras.h>

/* Loggers y Configuración */
extern t_log* logger;
extern t_log* logger_debug;
extern t_config* config;

/* Sockets de Conexión a CPU */
extern int socket_cpu_dispatch;
extern int socket_cpu_interrupt;

/* Lista de PCBs/Procesos creados */
extern t_list* lista_procesos;
extern int ULTIMO_PID;

/* Estados para los hilos gestionados por los planificadores */
extern t_list* estado_new;
extern t_list* estado_ready;
extern t_tcb* estado_exec; // Solo podemos tener 1 hilo en este estado
extern t_list* estado_blocked;
extern t_list* estado_exit;

/* Semáforos y Mutex para los estados que los requieren */
extern sem_t semaforo_estado_ready;
extern pthread_mutex_t mutex_lista_procesos;
extern pthread_mutex_t mutex_estado_ready;
extern pthread_mutex_t mutex_estado_exec;
extern pthread_mutex_t mutex_estado_blocked;
extern pthread_mutex_t mutex_estado_exit;

#endif