#include <main.h>

int main(int argc, char* argv[]) {
    t_config* config;
    t_log* logger;
    char* puerto_escucha_dispatch;
    char* puerto_escucha_interrupt;
    char* ip_memoria;
    char* puerto_memoria;

    config = iniciar_config_cpu();
    logger = iniciar_logger_cpu(config);

    ip_memoria = config_get_string_value(config, "IP_MEMORIA");
    puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
    puerto_escucha_dispatch = config_get_string_value(config, "PUERTO_ESCUCHA_DISPATCH");
    puerto_escucha_interrupt = config_get_string_value(config, "PUERTO_ESCUCHA_INTERRUPT");

    int socket_memoria = conectar_a_memoria(ip_memoria, puerto_memoria);
    log_info(logger, "Conectado a Memoria");

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

t_config* iniciar_config_cpu() {
    t_config* config;
    config = config_create("cpu.config");

    if(config == NULL) {
        perror("Error inicializando la config de CPU");
        config_destroy(config);
        abort();
    }

    return config;
}

t_log* iniciar_logger_cpu(t_config* config) {
    t_log* logger;

    char* log_level_string = config_get_string_value(config, "LOG_LEVEL");
    t_log_level log_level = log_level_from_string(log_level_string);

    logger = log_create("./logs/cpu.log", "CPU", true, log_level);

    if(logger == NULL) {
        perror("Error inicializando el logger de CPU");
        log_destroy(logger);
        abort();
    }

    return logger;
}

int iniciar_servidor(char* puerto)
{
	int socket_servidor;

	struct addrinfo hints, *servinfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

    getaddrinfo(NULL, puerto, &hints, &servinfo);

	// Creamos el socket de escucha del servidor
	socket_servidor = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);

	// Asociamos el socket a un puerto
	setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEPORT, &(int){1}, sizeof(int));

	bind(socket_servidor, servinfo->ai_addr, servinfo->ai_addrlen);

	// Escuchamos las conexiones entrantes
	listen(socket_servidor, SOMAXCONN);

	freeaddrinfo(servinfo);

	return socket_servidor;
}

int esperar_cliente(int socket_servidor)
{
	// Aceptamos un nuevo cliente
	int socket_cliente;
	socket_cliente = accept(socket_servidor, NULL, NULL);

	return socket_cliente;
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

/* Creamos y retornamos el socket de la conexion a Memoria */
int conectar_a_memoria(char *ip, char* puerto)
{
    int socket_filesystem = crear_conexion(ip, puerto);

    if(socket_filesystem == -1) {
        perror("Error al crear el socket de filesystem");
        abort();
    }

    return socket_filesystem;
}

void terminar_programa(t_log* logger, t_config* config, int conexion)
{
    log_destroy(logger);
    config_destroy(config);
    close(conexion);
}