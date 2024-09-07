#include <main.h>

int main(int argc, char* argv[]) {
    char* puerto_escucha;
    t_config* config;
    t_log* logger;

    config = iniciar_config("filesystem.config");
    logger = iniciar_logger(config, "./logs/filesystem.log", "FILESYSTEM");

    puerto_escucha = config_get_string_value(config, "PUERTO_ESCUCHA");

    int fd_escucha = iniciar_servidor(puerto_escucha);
    log_info(logger, "FileSystem listo para escuchar a la Memoria");

    /* Estamos esperando a la Memoria */
    int socket_memoria = esperar_cliente(fd_escucha);
    log_info(logger, "Se conectó la Memoria");

    log_info(logger, "Esperando handshake de la Memoria");
    esperar_handshake(socket_memoria);
    log_info(logger, "Handshake exitoso");

    terminar_programa(logger, config);

    return 0;
}

void terminar_programa(t_log* logger, t_config* config)
{
    log_destroy(logger);
    config_destroy(config);
}

void esperar_handshake(int socket_cliente) {
    int32_t handshake;
    int32_t resultOk = 0;
    int32_t resultError = -1;

    recv(socket_cliente, &handshake, sizeof(int32_t), MSG_WAITALL);
    if(handshake == 1) {
        send(socket_cliente, &resultOk, sizeof(int32_t), 0);
    } else {
        send(socket_cliente, &resultError, sizeof(int32_t), 0);
    }
}