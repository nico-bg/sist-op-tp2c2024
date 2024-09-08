#include <main.h>

int main(int argc, char* argv[]) {
    char* puerto_escucha;
    t_config* config;
    t_log* logger;

    config = iniciar_config("filesystem.config");
    logger = iniciar_logger(config, "filesystem.log", "FILESYSTEM");

    puerto_escucha = config_get_string_value(config, "PUERTO_ESCUCHA");

    int fd_escucha = iniciar_servidor(puerto_escucha);
    log_info(logger, "FileSystem listo para escuchar a la Memoria");

    /* Estamos esperando a la Memoria */
    int socket_memoria = esperar_cliente(fd_escucha);
    log_info(logger, "Se conectó la Memoria");

    /* Escuchamos las peticiones que la Memoria haga hasta que se desconecte */
    atender_peticiones(logger, config, socket_memoria);

    terminar_programa(logger, config);

    return EXIT_SUCCESS;
}

void terminar_programa(t_log* logger, t_config* config)
{
    log_destroy(logger);
    config_destroy(config);
}
