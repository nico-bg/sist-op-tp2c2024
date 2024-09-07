#include "main.h"

int main(int argc, char* argv[]) {
    t_config* config;
    t_log* logger;
    char* ip_filesystem;
    char* puerto_filesystem;
    char* puerto_escucha;

    config = iniciar_config_memoria();

    logger = iniciar_logger_memoria(config);

    ip_filesystem = config_get_string_value(config, "IP_FILESYSTEM");
    puerto_filesystem = config_get_string_value(config, "PUERTO_FILESYSTEM");
    puerto_escucha = config_get_string_value(config, "PUERTO_ESCUCHA");

    int socket_filesystem = conectar_a_filesystem(ip_filesystem, puerto_filesystem);
    log_info(logger, "Conectado al Filesystem");

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

/* Levanta el archivo de configuración de Memoria */
t_config* iniciar_config_memoria()
{
    t_config* config;
    config = config_create("memoria.config");

    if(config == NULL) {
        perror("Error inicializando la config de Memoria");
        config_destroy(config);
        abort();
    }

    return config;
}

/* Instancia un nuevo logger con el LOG_LEVEL definido en la configuración */
t_log* iniciar_logger_memoria(t_config* config)
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
int conectar_a_filesystem(char *ip, char* puerto)
{
    int socket_filesystem = crear_conexion(ip, puerto);

    if(socket_filesystem == -1) {
        perror("Error al crear el socket de filesystem");
        abort();
    }

    return socket_filesystem;
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

void terminar_programa(t_log* logger, t_config* config, int conexion)
{
    log_destroy(logger);
    config_destroy(config);
    close(conexion);
}

int esperar_cliente(int socket_servidor)
{
	// Aceptamos un nuevo cliente
	int socket_cliente;
	socket_cliente = accept(socket_servidor, NULL, NULL);

	return socket_cliente;
}