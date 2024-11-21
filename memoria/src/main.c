#include "main.h"
#include "comunicaciones.h"
#include <utils/sleep.h>
#include <pthread.h>

t_log* logger;
t_config* config;
t_list* particiones;
void* memoria;
int socket_filesystem;

int main(int argc, char* argv[]) {

    config = iniciar_config("memoria.config");
    logger = iniciar_logger(config, "memoria.log", "MEMORIA");

    inicializar_particiones();

    char* ip_filesystem;
    char* puerto_filesystem;
    char* puerto_escucha;

    pthread_t thread_kernel;

    ip_filesystem = config_get_string_value(config, "IP_FILESYSTEM");
    puerto_filesystem = config_get_string_value(config, "PUERTO_FILESYSTEM");
    puerto_escucha = config_get_string_value(config, "PUERTO_ESCUCHA");


    /* Conexión con el Filesystem */
    socket_filesystem = conectar_a_socket(ip_filesystem, puerto_filesystem);
    log_info(logger, "Conectado al Filesystem");
    enviar_mensaje("Hola, soy la Memoria", socket_filesystem);


    /* Conexión con la CPU */
    int fd_escucha = iniciar_servidor(puerto_escucha);
    log_info(logger, "Memoria lista para escuchar al CPU y Kernel");
    
    int socket_cpu = esperar_cliente(fd_escucha);   //  No se está haciendo un check para asegurar que
    log_info(logger, "Se conectó el CPU");          //  es la CPU quien se está conectando - definir

    /* Se crea un hilo para escuchar al kernel */
    pthread_create(&thread_kernel, NULL, hilo_kernel, fd_escucha);

    /* Escuchamos las peticiones que el CPU haga hasta que se desconecte */
    while(1) {
        int resultado = atender_cpu(socket_cpu);
        if(resultado == -1) { break; }
    }
    
    pthread_join(thread_kernel, NULL);
    terminar_programa(config, socket_filesystem);

    return 0;
}

void hilo_kernel(void* fd_escucha)
{
    while(1) {  // Crea un nuevo hilo por cada conexión
        pthread_t kernelThread;
        int *socket_kernel = malloc(sizeof(int));
        *socket_kernel = accept((int)fd_escucha, NULL, NULL);
        log_info(logger, "## Kernel Conectado - FD del socket: %d", *socket_kernel);
        pthread_create(&kernelThread, NULL, (void*) atender_kernel, socket_kernel);
        pthread_detach(kernelThread);
    }
}

void atender_kernel(void* socket_cliente)
{
    int socket = *(int*)socket_cliente;
    free(socket_cliente);

    int cod_op = recibir_operacion(socket);

    switch (cod_op) {
        case -1:
            log_error(logger, "El KERNEL se desconectó");
            break;
        default:
            atender_peticion_kernel(cod_op, socket);
            break;
    }
}

void atender_peticion_kernel(int cod_op, int socket)
{

    switch(cod_op) {

        case OPERACION_CREAR_PROCESO:
            t_datos_inicializacion_proceso* datos_crear_proceso = (t_datos_inicializacion_proceso*)leer_buffer_kernel(cod_op, socket);
            t_particion* particion_libre = buscar_particion_libre(datos_crear_proceso->tamanio);

            if(particion_libre != NULL){
                iniciar_proceso(datos_crear_proceso, particion_libre);
                log_info(logger, "## Proceso Creado -  PID: %d - Tamaño: %d", datos_crear_proceso->pid, datos_crear_proceso->tamanio);
                confirmar_operacion(socket);
            } else {
                log_info(logger, "No hay suficiente espacio para inicializar proceso");
                notificar_error(socket);
            }
            break;

        case OPERACION_FINALIZAR_PROCESO:
            t_datos_finalizacion_proceso* datos_finalizar_proceso = (t_datos_finalizacion_proceso*)leer_buffer_kernel(cod_op, socket);
            int tam = finalizar_proceso(datos_finalizar_proceso);
            log_info(logger, "## Proceso Destruido -  PID: %d - Tamaño: %d", datos_finalizar_proceso->pid, tam);
            confirmar_operacion(socket);
            break;

        case OPERACION_CREAR_HILO:
            t_datos_inicializacion_hilo* datos_crear_hilo = (t_datos_inicializacion_hilo*)leer_buffer_kernel(cod_op, socket);
            iniciar_hilo(datos_crear_hilo);
            log_info(logger, "## Hilo Creado - (PID:TID) - (%d:%d)", datos_crear_hilo->pid, datos_crear_hilo->tid);
            confirmar_operacion(socket);
            break;

        case OPERACION_FINALIZAR_HILO:
            t_datos_finalizacion_hilo* datos_finalizar_hilo = (t_datos_finalizacion_hilo*)leer_buffer_kernel(cod_op, socket);
            finalizar_hilo(datos_finalizar_hilo);
            log_info(logger, "## Hilo Destruido - (PID:TID) - (%d:%d)", datos_finalizar_hilo->pid, datos_finalizar_hilo->tid);
            confirmar_operacion(socket);
            break;

        case OPERACION_DUMP_MEMORY: //Los datos de la struct finalizar hilo & mem dump son los mismos, por ende se reutiliza la estructura
            t_datos_dump_memory* datos_mem_dump = (t_datos_dump_memory*)leer_buffer_kernel(cod_op, socket);
            log_info(logger, "## Memory Dump solicitado - (PID:TID) - (%d:%d)", datos_mem_dump->pid, datos_mem_dump->tid);

            op_code codigo_operacion = enviar_dump_memory(socket_filesystem, datos_mem_dump);

            if(codigo_operacion == OPERACION_CONFIRMAR){
                confirmar_operacion(socket);
            } else {
                notificar_error(socket);
            }
            break;

        default:
            log_error(logger, "Kernel envió un codigo de operacion desconocido: %d", cod_op);
            break;
    }
}

int atender_cpu(int socket_cliente)
{
    int codigo_operacion = recibir_operacion(socket_cliente);
    switch(codigo_operacion) {
        case -1:
            log_error(logger, "El CPU se desconectó");
            break;
        default:
            atender_peticion_cpu(codigo_operacion, socket_cliente);
            break;
    }
    return codigo_operacion;
}

void atender_peticion_cpu(int cod_op, int socket)
{
    int espera = config_get_int_value(config, "RETARDO_RESPUESTA");

    esperar_ms(espera);
 
    switch(cod_op) {

        case OPERACION_DEVOLVER_CONTEXTO_EJECUCION:
            t_cpu_solicitar_contexto* datos_devolver_contexto = (t_cpu_solicitar_contexto*)leer_buffer_cpu(cod_op, socket);
            log_info(logger, "## Contexto Solicitado - (PID:TID) - (%d:%d)", datos_devolver_contexto->pid, datos_devolver_contexto->tid);
            t_contexto* contexto_ejecucion = devolver_contexto_ejecucion(datos_devolver_contexto);
            enviar_buffer(cod_op, socket, contexto_ejecucion);
            break;

        case OPERACION_ACTUALIZAR_CONTEXTO:
            t_contexto* datos_actualizar_contexto = (t_contexto*)leer_buffer_cpu(cod_op, socket);
            actualizar_contexto_ejecucion(datos_actualizar_contexto);
            log_info(logger, "## Contexto Actualizado - (PID:TID) - (%d:%d)", datos_actualizar_contexto->pid, datos_actualizar_contexto->tid);
            confirmar_operacion(socket);
            break;

        case OPERACION_DEVOLVER_INSTRUCCION:
            t_datos_obtener_instruccion* datos_devolver_instruccion = (t_datos_obtener_instruccion*)leer_buffer_cpu(cod_op, socket);
            char* nombre_archivo = obtener_archivo_pseudocodigo(datos_devolver_instruccion->pid, datos_devolver_instruccion->tid, PATH);
            char* inst = devolver_instruccion(datos_devolver_instruccion);
            log_info(logger, "## Obtener instrucción - (PID:TID) - (%d:%d) - Instrucción: <%s> <%s>", datos_devolver_instruccion->pid, datos_devolver_instruccion->tid, inst, nombre_archivo);
            enviar_buffer(cod_op, socket, inst);
            break;

        case OPERACION_LEER_MEMORIA:
            t_datos_leer_memoria* datos_leer_memoria = (t_datos_leer_memoria*)leer_buffer_cpu(cod_op, socket);
            log_info(logger, "## Lectura - (PID:TID) - (%d:%d) - Dir. Física: %d - Tamaño: %d", datos_leer_memoria->pid, datos_leer_memoria->tid, datos_leer_memoria->dir_fisica, datos_leer_memoria->tamanio);
            uint32_t dato_leido = leer_memoria(datos_leer_memoria);
            enviar_buffer(cod_op, socket, dato_leido);
            break;

        case OPERACION_ESCRIBIR_MEMORIA:
            t_datos_escribir_memoria* datos_escribir_memoria = (t_datos_escribir_memoria*)leer_buffer_cpu(cod_op, socket);
            log_info(logger, "## Escritura - (PID:TID) - (%d:%d) - Dir. Física: %d - Tamaño: %d", datos_escribir_memoria->pid, datos_escribir_memoria->tid, datos_escribir_memoria->dir_fisica, datos_escribir_memoria->tamanio);
            escribir_memoria(datos_escribir_memoria);
            confirmar_operacion(socket);
            break;

        default:
            log_error(logger, "CPU envió un codigo de operación desconocido: %d", cod_op);
            break;
    }

}

void terminar_programa(t_config* config, int conexion)
{
    log_destroy(logger);
    config_destroy(config);
    close(conexion);
}

uint32_t leer_memoria(t_datos_leer_memoria* datos){

    uint32_t dato_leido;

    memcpy(&dato_leido, memoria + datos->dir_fisica, sizeof(uint32_t));

    return dato_leido;
}

void escribir_memoria(t_datos_escribir_memoria* datos){
    memcpy(memoria + datos->dir_fisica, &datos->dato_a_escribir, datos->tamanio);
}
