#include <main.h>

int main(int argc, char* argv[]) {
    char* puerto_escucha;
    t_config* config;
    t_log* logger;

    config = iniciar_config("filesystem.config");
    logger = iniciar_logger(config, "filesystem.log", "FILESYSTEM");

    /* Inicialización archivo de bitmaps*/
    //t_bitmap* bitmap = iniciar_bitmap("bitmap.dat")

    /* Inicialización archivo de bloques*/
    //t_bloques* bloques = iniciar_bloques("bloques.dat")

    /* Inicializar directorio de montaje*/

    /* Inicializar directorio de montaje*/
    //struct_fcb fcb = crear_fcb(nombre_archivo);
    //crear_entrada_directorio(nombre_archivo);
    //actualizar_tabla_global_archivos_abiertos(fcb);
    //asignacion_bloques(fcb, tamanio);
    //escritura_bloques(bloques, fcb);
    //actualización_atributos(fcb);


    puerto_escucha = config_get_string_value(config, "PUERTO_ESCUCHA");

    int fd_escucha = iniciar_servidor(puerto_escucha);
    log_info(logger, "FileSystem listo para escuchar a la Memoria"); 

    /* Estamos esperando a la Memoria */


    //int socket_memoria = esperar_cliente(fd_escucha);
    //log_info(logger, "Se conectó la Memoria");

    /* Escuchamos las peticiones que la Memoria haga hasta que se desconecte */
    //atender_peticiones(logger, config, socket_memoria);

    while(1) {
        pthread_t thread;
        int *fd_conexion_ptr = malloc(sizeof(int));
        *fd_conexion_ptr = accept(fd_escucha, NULL, NULL);
        pthread_create(&thread, NULL, (void*) atender_peticiones, fd_conexion_ptr);
        pthread_detach(thread);
    }

    terminar_programa(logger, config);

    return EXIT_SUCCESS;
}

void terminar_programa(t_log* logger, t_config* config)
{
    log_destroy(logger);
    config_destroy(config);
}
