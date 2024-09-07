#include <main.h>

int main(int argc, char* argv[]) {
    char* puerto_escucha;
    t_config* config;
    t_log* logger;

    config = iniciar_config_filesystem();
    logger = iniciar_logger_filesystem(config);

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

/* Levanta el archivo de configuración de FileSystem */
t_config* iniciar_config_filesystem()
{
    t_config* config;
    config = config_create("filesystem.config");

    if(config == NULL) {
        perror("Error inicializando la config de FileSystem");
        config_destroy(config);
        abort();
    }

    return config;
}

/* Instancia un nuevo logger con el LOG_LEVEL definido en la configuración */
t_log* iniciar_logger_filesystem(t_config* config)
{
    t_log* logger;

    char* log_level_string = config_get_string_value(config, "LOG_LEVEL");
    t_log_level log_level = log_level_from_string(log_level_string);

    logger = log_create("./logs/filesystem.log", "FILESYSTEM", true, log_level);

    if(logger == NULL) {
        perror("Error inicializando el logger de FileSystem");
        log_destroy(logger);
        abort();
    }

    return logger;
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

void terminar_programa(t_log* logger, t_config* config)
{
    log_destroy(logger);
    config_destroy(config);
}

int esperar_cliente(int socket_servidor)
{
	// Aceptamos un nuevo cliente
	int socket_cliente;
	socket_cliente = accept(socket_servidor, NULL, NULL);

	return socket_cliente;
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