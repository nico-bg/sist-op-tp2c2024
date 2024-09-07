#include <main.h>

int main(int argc, char* argv[]) {
    t_config* config;
    t_log* logger;
    char* puerto_escucha_dispatch;
    char* puerto_escucha_interrupt;
    char* ip_memoria;
    char* puerto_memoria;

    config = iniciar_config("cpu.config");
    logger = iniciar_logger(config, "./logs/cpu.log", "CPU");

    ip_memoria = config_get_string_value(config, "IP_MEMORIA");
    puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
    puerto_escucha_dispatch = config_get_string_value(config, "PUERTO_ESCUCHA_DISPATCH");
    puerto_escucha_interrupt = config_get_string_value(config, "PUERTO_ESCUCHA_INTERRUPT");

    int socket_memoria = conectar_a_socket(ip_memoria, puerto_memoria);
    log_info(logger, "Conectado a Memoria");
    
    enviar_mensaje("Hola, soy el CPU", socket_memoria);

    int fd_dispatch = iniciar_servidor(puerto_escucha_dispatch);
    int fd_interrupt = iniciar_servidor(puerto_escucha_interrupt);

    /* Esperamos a que se conecte el Kernel por el puerto dispatch */
    int socket_dispatch = esperar_cliente(fd_dispatch);
    log_info(logger, "Se conectó el Kernel por el puerto Dispatch");

    /* Esperamos a que se conecte el Kernel por el puerto interrupt */
    int socket_interrupt = esperar_cliente(fd_interrupt);
    log_info(logger, "Se conectó el Kernel por el puerto Interrupt");

    terminar_programa(logger, config, socket_memoria);

    return 0;
}

void terminar_programa(t_log* logger, t_config* config, int conexion)
{
    log_destroy(logger);
    config_destroy(config);
    close(conexion);
}