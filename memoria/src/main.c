#include "main.h"
#include <pthread.h>


int main(int argc, char* argv[]) {

    t_config* config;
    t_log* logger;

    char* ip_filesystem;
    char* puerto_filesystem;
    char* puerto_escucha;

    pthread_t thread_kernel;

    config = iniciar_config("memoria.config");

    logger = iniciar_logger(config, "memoria.log", "MEMORIA");

    ip_filesystem = config_get_string_value(config, "IP_FILESYSTEM");
    puerto_filesystem = config_get_string_value(config, "PUERTO_FILESYSTEM");
    puerto_escucha = config_get_string_value(config, "PUERTO_ESCUCHA");


    /* Conexión con el Filesystem */

    int socket_filesystem = conectar_a_socket(ip_filesystem, puerto_filesystem);
    log_info(logger, "Conectado al Filesystem");
    //enviar_mensaje("Hola, soy la Memoria", socket_filesystem);

    /* Conexión con la CPU */

    int fd_escucha = iniciar_servidor(puerto_escucha);
    log_info(logger, "Memoria lista para escuchar al CPU y Kernel");
    
    int socket_cpu = esperar_cliente(fd_escucha);   //  No se está haciendo un check para asegurar que es
    log_info(logger, "Se conectó el CPU");          //  la CPU quien se está conectando - definir

    /* Se crea un hilo para escuchar al kernel */
    parametros_hilo* parametros = malloc(sizeof(parametros_hilo));  //
    parametros->socket = fd_escucha;                                // Esta parte es unicamente para pasarle más de un parametro a la función
    parametros->logger = logger;                                    //
    pthread_create(&thread_kernel, NULL, hilo_kernel, parametros);

    /* Escuchamos las peticiones que el CPU haga hasta que se desconecte */
    while(1) {
        int resultado = atender_cpu(logger, socket_cpu);
        if(resultado == -1) { break; }
    }

    
    pthread_join(thread_kernel, NULL);
    free(parametros);
    terminar_programa(logger, config, socket_filesystem);

    return 0;
}

void* hilo_kernel(void* args)
{
    parametros_hilo* parametros = (parametros_hilo*)args;   // 
    int fd_escucha = parametros->socket;                    // Sacamos los parametros del logger y el socket de escucha de nuestra estructura
    t_log* logger = parametros->logger;                     //

    while(1) {  // Crea un nuevo hilo por cada conexión
        pthread_t kernelThread;
        int socket_kernel = (int)malloc(sizeof(int));       //
        socket_kernel = accept(fd_escucha, NULL, NULL);     //
        parametros->socket = socket_kernel;                 // Se cambia el socket de escucha por el del kernel para poder reeutilizar la estructura
        pthread_create(&kernelThread, NULL, atender_kernel, parametros);
        pthread_detach(kernelThread);
        //free(socket_kernel);
    }
}

void* atender_kernel(void* args)
{
    parametros_hilo* parametros = (parametros_hilo*)args;
    int socket_cliente = parametros->socket;
    t_log* logger = parametros->logger;

    int codigo_operacion = recibir_operacion(socket_cliente);
    switch(codigo_operacion) {
        case -1:
            log_error(logger, "El kernel se desconectó");
            break;
        default:
            atender_peticion_kernel(logger, codigo_operacion);
            break;
    }
}

void* atender_peticion_kernel(t_log* logger, int cod_op)
{
    int mensaje = leer_buffer(cod_op);
    switch(mensaje) {
        case CREAR_PROCESO:
            //crear proceso
            break;
        case FINALIZAR_PROCESO:
            //finalizar proceso
            break;
        case CREAR_HILO:
            //crear hilo
            break;
        case FINALIZAR_HILO:
            //finalizar hilo
            break;
        case MEMORY_DUMP:
            //memory dump
            break;
        default:
            //log_error(logger, "Operación desconocida - kernel");
            break;
    }
}

int atender_cpu(t_log* logger, int socket_cliente)
{
    int codigo_operacion = recibir_operacion(socket_cliente);
    switch(codigo_operacion) {
        case -1:
            log_error(logger, "El CPU se desconectó");
            break;
        default:
            atender_peticion_cpu(codigo_operacion);
            break;
    }
    return codigo_operacion;
}

void* atender_peticion_cpu(int cod_op)
{
    int mensaje = leer_buffer(cod_op);
    switch(mensaje){
        case DEVOLVER_CONTEXTO_EJECUCION:
            //devolver contexto de ejecucion
            break;
        case ACTUALIZAR_CONTEXTO_EJECUCION:
            //actualizar contexto de ejecucion
            break;
        case DEVOLVER_INSTRUCCION:
            //devolver instruccion
            break;
        case LEER_MEMORIA:
            //leer memoria
            break;
        case ESCRIBIR_MEMORIA:
            //escribir memoria
            break;
        default:
            //log_error(logger, "Operación desconocida - CPU");
            break;
    }

    return cod_op;
}

int leer_buffer(int buffer)
{
    return CREAR_PROCESO;
}

void terminar_programa(t_log* logger, t_config* config, int conexion)
{
    log_destroy(logger);
    config_destroy(config);
    close(conexion);
}
