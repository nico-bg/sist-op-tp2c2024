#include <main.h>

t_bitarray* bitmap;

t_log* logger;
t_config* config;

int main(int argc, char* argv[]) {

    config = iniciar_config("filesystem.config");
    logger = iniciar_logger(config, "filesystem.log", "FILESYSTEM");
    char* puerto_escucha;

    puerto_escucha = config_get_string_value(config, "PUERTO_ESCUCHA");

    inicializar_filesystem(config);

    /* Conexión con la memoria */
    int fd_escucha = iniciar_servidor(puerto_escucha);
    log_debug(logger, "FileSystem listo para escuchar a la Memoria"); 

    /* Escuchamos las peticiones de la memoria */
    while(1) {
        pthread_t thread_memoria;
        int *socket_memoria = malloc(sizeof(int));
        *socket_memoria = accept((int)fd_escucha, NULL, NULL);
        log_debug(logger, "Conexion de memoria aceptada en socket: %d", *socket_memoria);
        //log_debug(logger, "Memoria conectada");
        pthread_create(&thread_memoria, NULL, (void*) atender_memoria, socket_memoria); 
        pthread_detach(thread_memoria);
    }

    terminar_programa(logger, config);

    return EXIT_SUCCESS;
}

// Funciones de Comunicación
void atender_memoria(void* socket_cliente) {
    int socket = *(int*)socket_cliente;
    free(socket_cliente);

    int cod_op = recibir_operacion(socket);

    switch (cod_op) {
        case -1:
            log_error(logger, "La memoria se desconectó");
            break;
        default:
            atender_peticion_filesystem_memoria(cod_op, socket);
            break;
    }
}


void atender_peticion_filesystem_memoria(int cod_op, int socket)
{
    switch(cod_op) {

        case OPERACION_MENSAJE:
            recibir_mensaje(socket,logger);
            break;

        case OPERACION_DUMP_MEMORY:
            log_info(logger, "## Memory Dump solicitado ");

            t_buffer* buffer;
            uint32_t length;
            void* datos;

            buffer = recibir_buffer(&length, socket);
            datos = (t_datos_dump_memory_fs*)deserializar_datos_dump_memory_fs(buffer);

            t_datos_dump_memory_fs* datos_dump = (t_datos_dump_memory_fs*)datos;

            if(datos_dump == NULL) {
                log_error(logger, "Error al recibir información del dump");
                enviar_respuesta_operacion(socket, false);
            } else {
            // Crear el archivo usando la información recibida
                bool resultado = crear_archivo_dump(
                    datos_dump->nombre_archivo,
                    datos_dump->contenido, 
                    datos_dump->tamanio
                );
            
                // Enviar resultado a memoria
                enviar_respuesta_operacion(socket, resultado);
            
                // Liberar recursos
                destruir_datos_dump_memory_fs(datos_dump);
                buffer_destroy(buffer);
            }
        break;

        default:
        log_error(logger, "Memoria envió un código de operación desconocido: %d", cod_op);
        enviar_respuesta_operacion(socket, false);
        break;
    }
    
}



void verifica_existencia_path(char* mount_dir) {
    struct stat st = {0};
    
    // Verifica si el directorio existe
    if (stat(mount_dir, &st) == -1) {
        log_debug(logger, "Creando mount dir: %s", mount_dir);
        if (mkdir(mount_dir, 0700) == -1) {
            log_error(logger, "Error creando mount dir");
            exit(EXIT_FAILURE);
        }
    }
    
   char* ruta_files = string_from_format("%s/files", mount_dir);
    if (stat(ruta_files, &st) == -1) {
        log_debug(logger, "Creando directorio de files: %s", ruta_files);
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
        log_debug(logger, "Creando archivo de bloques: %s", bloques_path);
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
    free(bloques_path);
    fclose(bloques_f);
}

void inicializar_bitmap(char* mount_dir, int block_count) {
    char bitmap_path[256];
    snprintf(bitmap_path, sizeof(bitmap_path), "%s/bitmap.dat", mount_dir);

    int bitmap_size = (block_count +7)/8;  // Tamaño esperado del bitmap en bytes
    FILE* bitmap_f = fopen(bitmap_path, "r+");

    if (bitmap_f != NULL) {
        // Verificar tamaño actual del archivo
        fseek(bitmap_f, 0, SEEK_END);
        int archivo_size = ftell(bitmap_f);
        rewind(bitmap_f);

        if (archivo_size != bitmap_size) {
            log_warning(logger, "Tamaño del archivo bitmap no coincide. Será ajustado.");
            int fd = fileno(bitmap_f);  // Obtener el descriptor del archivo
            if (ftruncate(fd, bitmap_size) != 0) {
                log_error(logger, "Error al truncar el archivo bitmap.");
                fclose(bitmap_f);
                exit(EXIT_FAILURE);
            }
            log_debug(logger, "Archivo bitmap truncado al nuevo tamaño: %d bytes", bitmap_size);
        }
    } else {
        log_debug(logger, "Creando archivo bitmap: %s", bitmap_path);
        bitmap_f = fopen(bitmap_path, "w+");
        if (bitmap_f == NULL) {
            log_error(logger, "Error creando archivo bitmap");
            exit(EXIT_FAILURE);
        }

        // Inicializar el archivo con ceros
        char* buffer = calloc(bitmap_size, 1);
        fwrite(buffer, bitmap_size, 1, bitmap_f);
        free(buffer);
    }

    // Leer o inicializar el buffer en memoria
    char* buffer = malloc(bitmap_size);
    rewind(bitmap_f);
    fread(buffer, bitmap_size, 1, bitmap_f);
    bitmap = bitarray_create_with_mode(buffer, bitmap_size, LSB_FIRST);

    fclose(bitmap_f);
    log_debug(logger, "Bitmap inicializado correctamente.");
}

void inicializar_filesystem(t_config* config) {
    char* mount_dir = config_get_string_value(config, "MOUNT_DIR");
    int block_size = config_get_int_value(config, "BLOCK_SIZE");
    int block_count = config_get_int_value(config, "BLOCK_COUNT");

    
    verifica_existencia_path(mount_dir);
    inicializar_bloques(mount_dir, block_size, block_count);
    inicializar_bitmap(mount_dir, block_count);
    
    log_debug(logger, "Filesystem inicializado correctamente");
}

// Funciones de Manejo de Bloques
int* encontrar_bloques_libres(int cantidad_bloques, int* bloques_encontrados) {
    int block_count = config_get_int_value(config, "BLOCK_COUNT");
    int* bloques = malloc(sizeof(int) * cantidad_bloques);
    *bloques_encontrados = 0;
    
    for(int i = 0; i < block_count && *bloques_encontrados < cantidad_bloques; i++) {
        if(!bitarray_test_bit(bitmap, i)) {
            bloques[*bloques_encontrados] = i;
            (*bloques_encontrados)++;
        }
    }
    
    if(*bloques_encontrados < cantidad_bloques) {
        free(bloques);
        return NULL;
    }
    
    return bloques;
}

void escribir_bloque(int nro_bloque, void* datos, size_t size) {
    char* mount_dir = config_get_string_value(config, "MOUNT_DIR");
    char* path_bloques = string_from_format("%s/bloques.dat", mount_dir);
    int block_size = config_get_int_value(config, "BLOCK_SIZE");
    int retardo = config_get_int_value(config, "RETARDO_ACCESO_BLOQUE");
    
   FILE* archivo = fopen(path_bloques, "r+");
if (archivo == NULL) {
    log_error(logger, "Error al abrir archivo de bloques: %s", strerror(errno));
} else {
    log_debug(logger, "Archivo de bloques abierto correctamente.");
}
    fseek(archivo, nro_bloque * block_size, SEEK_SET);
    fwrite(datos, size, 1, archivo);
    
    fclose(archivo);
    free(path_bloques);
    
    usleep(retardo * 1000);
}

int calcular_bloques_necesarios(size_t tamanio) {
    int block_size = config_get_int_value(config, "BLOCK_SIZE");
    return (tamanio + block_size - 1) / block_size;
}

int contar_bloques_libres(void) {
    int block_count = config_get_int_value(config, "BLOCK_COUNT");
    int count = 0;
    
    for(int i = 0; i < block_count; i++) {
        if(!bitarray_test_bit(bitmap, i)) {
            count++;
        }
    }
    
    return count;
}

// Funciones de Manejo de Archivos
void crear_archivo_metadata(const char* nombre_archivo, t_file_metadata* metadata) {
    char* mount_dir = config_get_string_value(config, "MOUNT_DIR");
    char* path_metadata = string_from_format("%s/files/%s", mount_dir, nombre_archivo);
    
    FILE* archivo = fopen(path_metadata, "w");
    if(archivo == NULL) {
        log_error(logger, "Error al crear archivo de metadata");
        free(path_metadata);
        return;
    }
    
    fprintf(archivo, "SIZE=%zu\nINDEX_BLOCK=%u\n", metadata->size, metadata->index_block);
    
    fclose(archivo);
    free(path_metadata);
}

bool crear_archivo_dump(const char* nombre_archivo, void* contenido, size_t tamanio) {
  
    int bloques_datos = calcular_bloques_necesarios(tamanio);
    int bloques_encontrados;
    
    int* bloques = encontrar_bloques_libres(bloques_datos + 1, &bloques_encontrados);
    if(bloques == NULL) {
        log_error(logger, "No hay suficientes bloques disponibles");
        return false;
    }
    
    log_info(logger, "## Archivo Creado: %s - Tamaño: %zu", nombre_archivo, tamanio);
    
    int bloque_indice = bloques[0];
    for(int i = 0; i < bloques_encontrados; i++) {
        bitarray_set_bit(bitmap, bloques[i]);
        log_info(logger, "## Bloque asignado: %d - Archivo: %s - Bloques Libres: %d",
                bloques[i], nombre_archivo, contar_bloques_libres());
    }
    
    char* mount_dir = config_get_string_value(config, "MOUNT_DIR");
    char bitmap_path[256];
    snprintf(bitmap_path, sizeof(bitmap_path), "%s/bitmap.dat", mount_dir);

    // Persistir cambios en el archivo bitmap
    persistir_bitmap(bitmap_path);
    
    t_file_metadata metadata = {
        .size = tamanio,
        .index_block = bloque_indice
    };
    crear_archivo_metadata(nombre_archivo, &metadata);
    
    log_info(logger, "## Acceso Bloque - Archivo: %s - Tipo Bloque: ÍNDICE - Bloque File System %d",
             nombre_archivo, bloque_indice);
    escribir_bloque(bloque_indice, &bloques[1], bloques_datos * sizeof(int));
    
    int block_size = config_get_int_value(config, "BLOCK_SIZE");
    for(int i = 0; i < bloques_datos; i++) {
        size_t offset = i * block_size;
        size_t size_to_write = (offset + block_size > tamanio) ? (tamanio - offset) : block_size;
        
        log_info(logger, "## Acceso Bloque - Archivo: %s - Tipo Bloque: DATOS - Bloque File System %d",
                 nombre_archivo, bloques[i + 1]);
        escribir_bloque(bloques[i + 1], contenido + offset, size_to_write);
    }
    
    log_info(logger, "## Fin de solicitud - Archivo: %s", nombre_archivo);
    
    free(bloques);
    return true;
}



void terminar_programa(t_log* logger, t_config* config)
{
    log_destroy(logger);
    config_destroy(config);
}


void persistir_bitmap(char* bitmap_path) {
   
   
    FILE* bitmap_f = fopen(bitmap_path, "w");
    if (bitmap_f == NULL) {
        log_error(logger, "Error al guardar el archivo bitmap: %s", bitmap_path);
        return;
    }

   size_t bitmap_size = (bitarray_get_max_bit(bitmap) + 7) / 8;
    fwrite(bitmap->bitarray, bitmap_size, 1, bitmap_f);
    fclose(bitmap_f);
    log_debug(logger, "Bitmap guardado correctamente en %s", bitmap_path);
}
