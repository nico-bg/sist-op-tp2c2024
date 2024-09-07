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

t_config* iniciar_config_memoria();
t_log* iniciar_logger_memoria(t_config* config);
int crear_conexion(char *ip, char* puerto);
int conectar_a_filesystem(char *ip, char* puerto);
int iniciar_servidor(char* puerto);
void terminar_programa(t_log* logger, t_config* config, int conexion);
int esperar_cliente(int socket_servidor);

#endif