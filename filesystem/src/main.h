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
#include <utils/configuracion.h>
#include <utils/conexiones.h>

void terminar_programa(t_log* logger, t_config* config);
void esperar_handshake(int socket_cliente);

#endif