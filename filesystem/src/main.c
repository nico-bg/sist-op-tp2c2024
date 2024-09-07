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

    while(1) {
        log_info(logger, "Esperando código de operación");
        int codigo_operacion = recibir_operacion(socket_memoria);

        switch(codigo_operacion) {
            case OPERACION_MENSAJE:
                recibir_mensaje(socket_memoria, logger);
                break;
            case -1:
                log_error(logger, "El cliente se desconectó, terminando servidor");
                terminar_programa(logger, config);
                return EXIT_FAILURE;
            default:
                log_warning(logger, "Operación desconocida: %d", codigo_operacion);
                break;
        }
    }

    terminar_programa(logger, config);

    return 0;
}

void terminar_programa(t_log* logger, t_config* config)
{
    log_destroy(logger);
    config_destroy(config);
}
