#include <main.h>

t_bitarray* bitmap;
t_log* logger;
t_config* config;

int main(int argc, char* argv[]) {
    char* puerto_escucha;

 
    config = iniciar_config("filesystem.config");
    logger = iniciar_logger(config, "filesystem.log", "FILESYSTEM");

    inicializar_filesystem(config);

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

void inicializar_filesystem(void){

inicializar_bitmap();

}


void inicializar_bitmap(){

log_debug(logger,"Inicialización del bitmap" );

FILE* bitmap_f = fopen("/home/utnso/tp20242c/tp-2024-2c-Grupo-777/filesystem/MOUNT_DIR/bitmap.dat", "r");

int size;
char* buffer;
fseek(bitmap_f, 0, SEEK_END);
size = ftell(bitmap_f);

log_debug(logger,"El tamaño del bitmap es:%d", size );

fseek(bitmap_f, 0, SEEK_SET);

fread(buffer, size, 0, bitmap_f);

//buffer = string_substring_until(buffer, size);

int cantidad_de_bloques = config_get_int_value(config, "BLOCK_COUNT");

bitmap = bitarray_create_with_mode(buffer, cantidad_de_bloques/8, LSB_FIRST);

fclose(bitmap_f);

log_debug(logger,"Inicialización del bitmap finalizada" );

}

void terminar_programa(t_log* logger, t_config* config)
{
    log_destroy(logger);
    config_destroy(config);
}


