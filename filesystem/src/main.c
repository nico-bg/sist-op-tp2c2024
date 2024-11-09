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

void verifica_existencia_path(char* mount_dir) {
    struct stat st = {0};
    
    // Verifica si el directorio existe
    if (stat(mount_dir, &st) == -1) {
        log_info(logger, "Creando mount dir: %s", mount_dir);
        if (mkdir(mount_dir, 0700) == -1) {
            log_error(logger, "Error creando mount dir");
            exit(EXIT_FAILURE);
        }
    }
    
   char* ruta_files = string_from_format("%s/files", mount_dir);
    if (stat(ruta_files, &st) == -1) {
        log_info(logger, "Creando directorio de files: %s", ruta_files);
        if (mkdir(ruta_files, 0700) == -1) {
            log_error(logger, "Error al crear directorio de files");
            free(ruta_files);
            exit(EXIT_FAILURE);
        }
    }
    free(ruta_files);
}
void inicializar_bloques(char* mount_dir, int block_size, int block_count) {
     char* bloques_path = string_from_format("%s/bloques.dat", mount_dir);
     FILE* bloques_f = fopen(bloques_path, "r");
    
    if (bloques_f == NULL) {
        log_info(logger, "Creando archivo de bloques: %s", bloques_path);
        bloques_f = fopen(bloques_path, "w+");
        if (bloques_f == NULL) {
            log_error(logger, "Error creando archivo de bloques");
            free(bloques_path);
            exit(EXIT_FAILURE);
        }
        
        // Inicializar el archivo con ceros hasta alcanzar el tamaño deseado
        char* buffer = calloc(block_size, 1);
        for (int i = 0; i < block_count; i++) {
            if (fwrite(buffer, block_size, 1, bloques_f) != 1) {
                log_error(logger, "Error escribiendo bloque");
                free(buffer);
                free(bloques_path);
                fclose(bloques_f);
                exit(EXIT_FAILURE);
            }
        }
        free(buffer);
    }
    fclose(bloques_f);
}

void inicializar_bitmap(char* mount_dir, int block_count) {
    char bitmap_path[256];
    snprintf(bitmap_path, sizeof(bitmap_path), "%s/bitmap.dat", mount_dir);
    
    // Calcular tamaño del bitmap en bytes 
    int bitmap_size = block_count / 8;
    
    FILE* bitmap_f = fopen(bitmap_path, "r+");
    if (bitmap_f == NULL) {
        log_info(logger, "Creando archivo bitmap: %s", bitmap_path);
        bitmap_f = fopen(bitmap_path, "w+");
        if (bitmap_f == NULL) {
            log_error(logger, "Error creando archivo bitmap");
            exit(EXIT_FAILURE);
        }
        
        // Inicializar el archivo con ceros
        char* buffer = calloc(bitmap_size, 1);
        if (fwrite(buffer, bitmap_size, 1, bitmap_f) != 1) {
            log_error(logger, "Error escribiendo bitmap inicial");
            free(buffer);
            fclose(bitmap_f);
            exit(EXIT_FAILURE);
        }
        free(buffer);
    }
    
    // Cargar el bitmap en memoria
    char* buffer = malloc(bitmap_size);
    fseek(bitmap_f, 0, SEEK_SET);
    if (fread(buffer, bitmap_size, 1, bitmap_f) != 1) {
        log_error(logger, "Error leyendo bitmap");
        free(buffer);
        fclose(bitmap_f);
        exit(EXIT_FAILURE);
    }
    
    bitmap = bitarray_create_with_mode(buffer, bitmap_size, LSB_FIRST);
    log_debug(logger, "Bitmap inicializado correctamente");
    
    fclose(bitmap_f);
}

void inicializar_filesystem(t_config* config) {
    int block_size = config_get_int_value(config, "BLOCK_SIZE");
    int block_count = config_get_int_value(config, "BLOCK_COUNT");

    
    verifica_existencia_path("/home/utnso/Desktop/tp-2024-2c-Grupo-777/filesystem/MOUNT_DIR/");
    inicializar_bloques("/home/utnso/Desktop/tp-2024-2c-Grupo-777/filesystem/MOUNT_DIR/", block_size, block_count);
    inicializar_bitmap("/home/utnso/Desktop/tp-2024-2c-Grupo-777/filesystem/MOUNT_DIR/", block_count);
    
    log_info(logger, "Filesystem inicializado correctamente");
}

void terminar_programa(t_log* logger, t_config* config)
{
    log_destroy(logger);
    config_destroy(config);
}


