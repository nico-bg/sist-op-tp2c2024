#include <main.h>

int main(int argc, char* argv[]) {
    t_config* config;
    t_log* logger;
    char* ip_memoria;
    char* puerto_memoria;
    char* ip_cpu;
    char* puerto_cpu_dispatch;
    char* puerto_cpu_interrupt;

    config = iniciar_config_kernel();
    logger = iniciar_logger_kernel(config);

    ip_memoria = config_get_string_value(config, "IP_MEMORIA");
    puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
    ip_cpu = config_get_string_value(config, "IP_CPU");
    puerto_cpu_dispatch = config_get_string_value(config, "PUERTO_CPU_DISPATCH");
    puerto_cpu_interrupt = config_get_string_value(config, "PUERTO_CPU_INTERRUPT");

    int socket_memoria = conectar_a_socket(ip_memoria, puerto_memoria);
    log_info(logger, "Conectado a Memoria");

    int socket_cpu_dispatch = conectar_a_socket(ip_cpu, puerto_cpu_dispatch);
    log_info(logger, "Conectado al CPU en el puerto Dispatch");

    int socket_cpu_interrupt = conectar_a_socket(ip_cpu, puerto_cpu_interrupt);
    log_info(logger, "Conectado al CPU en el puerto Interrupt");

    terminar_programa(logger, config, socket_cpu_dispatch, socket_cpu_interrupt, socket_memoria);

    return 0;
}

/* Levanta el archivo de configuración de Memoria */
t_config* iniciar_config_kernel()
{
    t_config* config;
    config = config_create("kernel.config");

    if(config == NULL) {
        perror("Error inicializando la config de Memoria");
        config_destroy(config);
        abort();
    }

    return config;
}

/* Instancia un nuevo logger con el LOG_LEVEL definido en la configuración */
t_log* iniciar_logger_kernel(t_config* config)
{
    t_log* logger;

    char* log_level_string = config_get_string_value(config, "LOG_LEVEL");
    t_log_level log_level = log_level_from_string(log_level_string);

    logger = log_create("./logs/memoria.log", "MEMORIA", true, log_level);

    if(logger == NULL) {
        perror("Error inicializando el logger de Memoria");
        log_destroy(logger);
        abort();
    }

    return logger;
}

/* Creamos y retornamos el socket de la conexion a FileSystem */
int conectar_a_socket(char *ip, char* puerto)
{
    int socket_conexion = crear_conexion(ip, puerto);

    if(socket_conexion == -1) {
        perror("Error al crear el socket");
        abort();
    }

    return socket_conexion;
}

/* Creamos y retornamos el socket de la conexion */
int crear_conexion(char *ip, char* puerto)
{
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &server_info);

	int fd_conexion = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

	connect(fd_conexion, server_info->ai_addr, server_info->ai_addrlen);

	freeaddrinfo(server_info);

	return fd_conexion;
}

void terminar_programa(t_log* logger, t_config* config, int conexion_cpu_dispatch, int conexion_cpu_interrupt, int conexion_memoria)
{
    log_destroy(logger);
    config_destroy(config);
    close(conexion_cpu_dispatch);
    close(conexion_cpu_interrupt);
    close(conexion_memoria);
}