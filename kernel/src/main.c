#include <main.h>

t_log* logger;
t_log* logger_debug;
t_config* config;

int main(int argc, char* argv[]) {
    char* ip_memoria;
    char* puerto_memoria;
    char* ip_cpu;
    char* puerto_cpu_dispatch;
    char* puerto_cpu_interrupt;

    // Levanto el archivo de configuración e inicializo los loggers
    config = iniciar_config("kernel.config");
    logger = iniciar_logger(config, "kernel.log", "KERNEL");
    logger_debug = iniciar_logger_debug("debug.log", "KERNEL");

    // Guardo las configuraciones en variables
    ip_memoria = config_get_string_value(config, "IP_MEMORIA");
    puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
    ip_cpu = config_get_string_value(config, "IP_CPU");
    puerto_cpu_dispatch = config_get_string_value(config, "PUERTO_CPU_DISPATCH");
    puerto_cpu_interrupt = config_get_string_value(config, "PUERTO_CPU_INTERRUPT");

    // Inicializo las conexiones a los sockets que deben estar siempre conectados
    int socket_memoria = conectar_a_socket(ip_memoria, puerto_memoria);
    int socket_cpu_dispatch = conectar_a_socket(ip_cpu, puerto_cpu_dispatch);
    int socket_cpu_interrupt = conectar_a_socket(ip_cpu, puerto_cpu_interrupt);

    // Verifico y proceso los argumentos recibidos
    t_argumentos* argumentos = procesar_argumentos(argc, argv);

    liberar_argumentos(argumentos);
    terminar_programa(logger, config, socket_cpu_dispatch, socket_cpu_interrupt, socket_memoria);

    return 0;
}

void terminar_programa(t_log* logger, t_config* config, int conexion_cpu_dispatch, int conexion_cpu_interrupt, int conexion_memoria)
{
    log_destroy(logger);
    config_destroy(config);
    close(conexion_cpu_dispatch);
    close(conexion_cpu_interrupt);
    close(conexion_memoria);
}