#include <main.h>

t_bitarray* bitmap;

t_log* logger;
t_config* config;

int main(int argc, char* argv[]) {
    char* puerto_escucha;

 
    config = iniciar_config("filesystem.config");
    logger = iniciar_logger(config, "filesystem.log", "FILESYSTEM");

    inicializar_filesystem(config);

    puerto_escucha = config_get_string_value(config, "PUERTO_ESCUCHA");

    int fd_escucha = iniciar_servidor(puerto_escucha);
    log_info(logger, "FileSystem listo para escuchar a la Memoria"); 

    /* Estamos esperando a la Memoria */

    int socket_memoria = esperar_cliente(fd_escucha);
    log_info(logger, "Se conectó la Memoria");

    /* Escuchamos las peticiones que la Memoria*/
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

void inicializar_filesystem(){

//verifica_existencia_path();

inicializar_bitmap();

//inicializar_bloques();


}

/*
void verifica_existencia_path(){

}
*/

/*
void inicializar_bloques(){

}
*/

void inicializar_bitmap(){

log_debug(logger,"Inicialización del bitmap" );

FILE* bitmap_f = fopen("/home/utnso/Desktop/tp-2024-2c-Grupo-777/filesystem/MOUNT_DIR/", "r");
if (bitmap_f == NULL) {
    log_error(logger, "No se pudo abrir el archivo bitmap.dat");
    return;
}

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

log_debug(logger,"bitmap creado" );

if (size == 0){
    //Inicializar el bitmap en 0 si el archivo está vacio
    for (int i = 0; i < cantidad_de_bloques; i++){
      //  bitmap[i] = 0;
         bitarray_set_bit(bitmap, i);  //Da segmentation fault --- VER!!!
    }
}

fclose(bitmap_f);

log_debug(logger,"Inicialización del bitmap finalizada" );

}

void terminar_programa(t_log* logger, t_config* config)
{
    log_destroy(logger);
    config_destroy(config);
}


