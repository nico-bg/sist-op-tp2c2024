#ifndef MAIN_H_
#define MAIN_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include <utils/hello.h>
#include <unistd.h>
#include<sys/socket.h>
#include<netdb.h>

t_config* iniciar_config_kernel();
t_log* iniciar_logger_kernel(t_config* config);
int crear_conexion(char *ip, char* puerto);
int conectar_a_socket(char *ip, char* puerto);
void terminar_programa(t_log* logger, t_config* config, int conexion_cpu_dispatch, int conexion_cpu_interrupt, int conexion_memoria);

#endif