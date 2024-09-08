#ifndef CONEXIONES_H_
#define CONEXIONES_H_

#include<stdio.h>
#include<stdlib.h>
#include<readline/readline.h>
#include <unistd.h>
#include<sys/socket.h>
#include<netdb.h>

int crear_conexion(char *ip, char* puerto);
int iniciar_servidor(char* puerto);
int esperar_cliente(int socket_servidor);
int conectar_a_socket(char *ip, char* puerto);

#endif