#include <kernel-utils/conexion-a-memoria.h>

int crear_conexion_a_memoria()
{
    char* ip_memoria = config_get_string_value(config, "IP_MEMORIA");
    char* puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
    int fd_conexion = conectar_a_socket(ip_memoria, puerto_memoria);

    return fd_conexion;
}