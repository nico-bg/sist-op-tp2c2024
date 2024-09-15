#include <main.h>

t_log* logger;
t_log* logger_debug;
t_config* config;

int main(int argc, char* argv[]) {
    char* ip_cpu;
    char* puerto_cpu_dispatch;
    char* puerto_cpu_interrupt;

    // Levanto el archivo de configuración e inicializo los loggers
    config = iniciar_config("kernel.config");
    logger = iniciar_logger(config, "kernel.log", "KERNEL");
    logger_debug = iniciar_logger_debug("debug.log", "KERNEL");

    // Guardo las configuraciones en variables
    ip_cpu = config_get_string_value(config, "IP_CPU");
    puerto_cpu_dispatch = config_get_string_value(config, "PUERTO_CPU_DISPATCH");
    puerto_cpu_interrupt = config_get_string_value(config, "PUERTO_CPU_INTERRUPT");

    // Inicializo las conexiones a los sockets que deben estar siempre conectados
    int socket_cpu_dispatch = conectar_a_socket(ip_cpu, puerto_cpu_dispatch);
    int socket_cpu_interrupt = conectar_a_socket(ip_cpu, puerto_cpu_interrupt);

    // Inicializamos las variables globales de estados y la lista de procesos
    inicializar_estados();

    // Verifico y proceso los argumentos recibidos en `main`
    t_argumentos* argumentos = procesar_argumentos(argc, argv);

    // Creamos el primer proceso
    crear_proceso(argumentos->archivo_pseudocodigo, argumentos->tamanio_proceso, 0);

    // Iniciamos el planificador de corto plazo
    // TODO: Considerar crear un hilo para ejecutarlo en paralelo
    planificador_corto_plazo();

    // Iniciamos el planificador de largo plazo
    // TODO: Considerar crear un hilo para ejecutarlo en paralelo
    planificador_largo_plazo();

    // Escuchamos las Syscalls que recibimos de la CPU
    // TODO: Considerar crear un hilo para ejecutarlo en paralelo
    atender_syscalls();

    // destruir_argumentos(argumentos);
    // terminar_programa(logger, config, socket_cpu_dispatch, socket_cpu_interrupt);

    return 0;
}

// void terminar_programa(t_log* logger, t_config* config, int conexion_cpu_dispatch, int conexion_cpu_interrupt)
// {
//     log_destroy(logger);
//     config_destroy(config);
//     close(conexion_cpu_dispatch);
//     close(conexion_cpu_interrupt);
// }