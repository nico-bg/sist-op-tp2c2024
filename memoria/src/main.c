#include "main.h"
#include "comunicaciones.h"
#include <pthread.h>

t_log* logger;

int main(int argc, char* argv[]) {

    t_config* config;

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
        pthread_create(&kernelThread, NULL, (void*) atender_kernel, socket_kernel);
        pthread_detach(kernelThread);
    }
}

void atender_kernel(void* socket_cliente)
{
    int socket = *(int*)socket_cliente;
    free(socket_cliente);

    int codigo_operacion = recibir_operacion(socket);
    switch(codigo_operacion) {
        case -1:
            log_error(logger, "El kernel se desconectó");
            break;
        default:
            log_info(logger, "Me llegó el codigo de operación: %d", codigo_operacion);
            atender_peticion_kernel(codigo_operacion, socket);
            break;
    }
}

void atender_peticion_kernel(int cod_op, int socket)
{
    switch(cod_op) {

        case CREAR_PROCESO:
            datos_proceso* datos_crear_proceso = (datos_proceso*)leer_buffer_kernel(cod_op);
            if(hay_espacio_en_memoria(datos_crear_proceso->tamanio)){
                iniciar_proceso(datos_crear_proceso->pid, datos_crear_proceso->tamanio, datos_crear_proceso->archivo_pseudocodigo);
                log_info(logger, "## Proceso Creado -  PID: %d - Tamaño: %d", datos_crear_proceso->pid, datos_crear_proceso->tamanio);
                //enviar_mensaje("Proceso inicializado con éxito", socket);
            } else {
                enviar_mensaje("No hay suficiente espacio para inicializar proceso", socket);
            }
            break;

        case FINALIZAR_PROCESO:
            datos_proceso* datos_finalizar_proceso = (datos_proceso*)leer_buffer_kernel(cod_op);
            finalizar_proceso(datos_finalizar_proceso->pid);
            log_info(logger, "## Proceso Destruido -  PID: %d - Tamaño: %d", datos_finalizar_proceso->pid, datos_finalizar_proceso->tamanio);
            //enviar_mensaje("Proceso finalizado con éxito", socket);
            break;

        case CREAR_HILO:
            datos_hilo* datos_crear_hilo = (datos_hilo*)leer_buffer_kernel(cod_op);
            iniciar_hilo(datos_crear_hilo->pid, datos_crear_hilo->tid, datos_crear_hilo->archivo_pseudocodigo);
            log_info(logger, "## Hilo Creado - (PID:TID) - (%d:%d)", datos_crear_hilo->pid, datos_crear_hilo->tid);
            //enviar_mensaje("Hilo inicializado con éxito", socket);
            break;

        case FINALIZAR_HILO:
            datos_hilo* datos_finalizar_hilo = (datos_hilo*)leer_buffer_kernel(cod_op);
            finalizar_hilo(datos_finalizar_hilo->pid, datos_finalizar_hilo->tid);
            log_info(logger, "## Hilo Destruido - (PID:TID) - (%d:%d)", datos_finalizar_hilo->pid, datos_finalizar_hilo->tid);
            //enviar_mensaje("Hilo finalizado con éxito", socket);
            break;

        case MEMORY_DUMP:
            datos_hilo* datos_mem_dump = (datos_hilo*)leer_buffer_kernel(cod_op);
            log_info(logger, "## Memory Dump solicitado - (PID:TID) - (%d:%d)", datos_mem_dump->pid, datos_mem_dump->tid);
            //enviar_mensaje("Operacion MEM_DUMP realizada con éxito", socket); //temporal - checkpoint-2
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

    switch(cod_op) {

        case DEVOLVER_CONTEXTO_EJECUCION:
            datos_hilo* datos_devolver_contexto = (datos_hilo*)leer_buffer_cpu(cod_op);
            log_info(logger, "## Contexto Solicitado - (PID:TID) - (%d:%d)", datos_devolver_contexto->pid, datos_devolver_contexto->tid);
            estructura_hilo* contexto_ejecucion = devolver_contexto_ejecucion(datos_devolver_contexto->pid, datos_devolver_contexto->tid);
            enviar_buffer(cod_op, contexto_ejecucion);
            break;

        case ACTUALIZAR_CONTEXTO_EJECUCION:
            datos_contexto_hilo* datos_actualizar_contexto = (datos_contexto_hilo*)leer_buffer_cpu(cod_op);
            estructura_hilo* hilo = convertir_struct(datos_actualizar_contexto);
            actualizar_contexto_ejecucion(datos_actualizar_contexto->pid, datos_actualizar_contexto->tid, hilo);
            log_info(logger, "## Contexto Actualizado - (PID:TID) - (%d:%d)", datos_actualizar_contexto->pid, datos_actualizar_contexto->tid);
            //enviar_mensaje("Contexto actualizado con éxito", socket);
            break;

        case DEVOLVER_INSTRUCCION:
            datos_hilo* datos_devolver_instruccion = (datos_hilo*)leer_buffer_cpu(cod_op);
            char* inst = devolver_instruccion(datos_devolver_instruccion->pid, datos_devolver_instruccion->tid, datos_devolver_instruccion->PC);
            enviar_buffer(cod_op, inst);
            log_info(logger, "## Obtener instrucción - (PID:TID) - (%d:%d) - Instrucción: <%s> <%s>", datos_devolver_instruccion->pid, datos_devolver_instruccion->tid, inst, datos_devolver_instruccion->archivo_pseudocodigo);
            break;

        case LEER_MEMORIA:
            //log_info(logger, "## Lectura - (PID:TID) - (%d:%d) - Dir. Física: %s - Tamaño: %d", pid, tid, dir_fisica, tamanio);
            //leer memoria
            break;

        case ESCRIBIR_MEMORIA:
            //log_info(logger, "## Escritura - (PID:TID) - (%d:%d) - Dir. Física: %s - Tamaño: %d", pid, tid, dir_fisica, tamanio);
            //escribir memoria
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

bool hay_espacio_en_memoria(int tamanio){
    return true; //checkpoint-2
}