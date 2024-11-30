#ifndef MAIN_H_
#define MAIN_H_

#include <utils/configuracion.h>
#include <utils/conexiones.h>
#include <utils/mensajes.h>
#include <utils/comunicacion_memoria_filesystem.h>
#include <commons/bitarray.h>
#include <pthread.h>
#include <sys/stat.h>
#include <commons/temporal.h>

void inicializar_filesystem();
void atender_memoria(void*);
void atender_peticion_filesystem_memoria(int cod_op, int socket);
void terminar_programa(t_log* logger, t_config* config);
bool crear_archivo_dump(const char* nombre_archivo, void* contenido, size_t tamanio);

typedef struct {
    size_t size;
    uint32_t index_block;
} t_file_metadata;

void persistir_bitmap(char* bitmap_path);

#endif