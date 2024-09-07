#include "main.h"

int main(int argc, char* argv[]) {
    t_config* config;
    t_log* logger;
    char* ip_filesystem;
    char* puerto_filesystem;
    char* puerto_escucha;

    config = iniciar_config("memoria.config");

    logger = iniciar_logger(config, "./logs/memoria.log", "MEMORIA");

    ip_filesystem = config_get_string_value(config, "IP_FILESYSTEM");
    puerto_filesystem = config_get_string_value(config, "PUERTO_FILESYSTEM");
    puerto_escucha = config_get_string_value(config, "PUERTO_ESCUCHA");

    int socket_filesystem = conectar_a_socket(ip_filesystem, puerto_filesystem);
    log_info(logger, "Conectado al Filesystem");

    /* Hacemos el handshake con FileSystem*/
    int32_t handshake = 1;
    int32_t result;

    send(socket_filesystem, &handshake, sizeof(int32_t), 0);
    recv(socket_filesystem, &result, sizeof(int32_t), MSG_WAITALL);

    if(result == 0) {
        log_info(logger, "HANDSHAKE OK");
    } else {
        log_error(logger, "Error en el handshake con File System");
        abort();
    }

    int fd_escucha = iniciar_servidor(puerto_escucha);
    log_info(logger, "Memoria lista para escuchar al CPU y Kernel");

    /* Estamos esperando a la CPU */
    int socket_cpu = esperar_cliente(fd_escucha);
    log_info(logger, "Se conectó el CPU");

    /* Estamos esperando al Kernel */
    int socket_kernel = esperar_cliente(fd_escucha);
    log_info(logger, "Se conectó el Kernel");

    terminar_programa(logger, config, socket_filesystem);

    return 0;
}

void terminar_programa(t_log* logger, t_config* config, int conexion)
{
    log_destroy(logger);
    config_destroy(config);
    close(conexion);
}
