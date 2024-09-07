#include <main.h>

int main(int argc, char* argv[]) {
    t_config* config;
    t_log* logger;
    char* ip_memoria;
    char* puerto_memoria;
    char* ip_cpu;
    char* puerto_cpu_dispatch;
    char* puerto_cpu_interrupt;

    config = iniciar_config("kernel.config");
    logger = iniciar_logger(config, "./logs/memoria.log", "MEMORIA");

    ip_memoria = config_get_string_value(config, "IP_MEMORIA");
    puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
    ip_cpu = config_get_string_value(config, "IP_CPU");
    puerto_cpu_dispatch = config_get_string_value(config, "PUERTO_CPU_DISPATCH");
    puerto_cpu_interrupt = config_get_string_value(config, "PUERTO_CPU_INTERRUPT");

    int socket_memoria = conectar_a_socket(ip_memoria, puerto_memoria);
    log_info(logger, "Conectado a Memoria");

    enviar_mensaje("Hola, soy el Kernel", socket_memoria);

    int socket_cpu_dispatch = conectar_a_socket(ip_cpu, puerto_cpu_dispatch);
    log_info(logger, "Conectado al CPU en el puerto Dispatch");

    enviar_mensaje("Hola, soy el Kernel desde el puerto Dispatch", socket_cpu_dispatch);

    int socket_cpu_interrupt = conectar_a_socket(ip_cpu, puerto_cpu_interrupt);
    log_info(logger, "Conectado al CPU en el puerto Interrupt");

    enviar_mensaje("Hola, soy el Kernel desde el puerto Interrupt", socket_cpu_interrupt);

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