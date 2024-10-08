#include "main.h"
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
        int* socket_kernel = malloc(sizeof(int));
        socket_kernel = accept(fd_escucha, NULL, NULL);
        pthread_create(&kernelThread, NULL, atender_kernel, socket_kernel);
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
            atender_peticion_kernel(codigo_operacion, socket);
            break;
    }
}

void atender_peticion_kernel(int cod_op, int socket)
{

    int pid, tid, tamanio;
    uint32_t base, limite;
    char* archivo;

    switch(cod_op) {
        case CREAR_PROCESO:
            //LEER BUFFER
            if(hay_espacio_en_memoria(tamanio)){
                iniciar_proceso(pid, tamanio, base, limite, archivo);
                log_info(logger, "## Proceso Creado -  PID: %d - Tamaño: %d", pid, tamanio);
                //enviar_mensaje("Proceso inicializado con éxito", socket);
            } else {
                enviar_mensaje("No hay suficiente espacio para inicializar proceso", socket);
            }
            break;
        case FINALIZAR_PROCESO:
            finalizar_proceso(pid);
            log_info(logger, "## Proceso Destruido -  PID: %d - Tamaño: %d", pid, tamanio);
            //enviar_mensaje("Proceso finalizado con éxito", socket);
            break;
        case CREAR_HILO:
            iniciar_hilo(pid, tid);
            log_info(logger, "## Hilo Creado - (PID:TID) - (%d:%d)", pid, tid);
            //enviar_mensaje("Hilo inicializado con éxito", socket);
            break;
        case FINALIZAR_HILO:
            finalizar_hilo(pid, tid);
            log_info(logger, "## Hilo Destruido - (PID:TID) - (%d:%d)", pid, tid);
            //enviar_mensaje("Hilo finalizado con éxito", socket);
            break;
        case MEMORY_DUMP:
            log_info(logger, "## Memory Dump solicitado - (PID:TID) - (%d:%d)", pid, tid);
            //enviar_mensaje("Operacion MEM_DUMP realizada con éxito", socket); //temporal - checkpoint-2
            break;
        default:
            //log_error(logger, "Operación desconocida - kernel");
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
    int pid, tid, tamanio;
    uint32_t PC;
    char* archv_codigo, dir_fisica;
    estructura_hilo* hilo;

    int mensaje = leer_buffer(cod_op);
    switch(mensaje){
        case DEVOLVER_CONTEXTO_EJECUCION:
            log_info(logger, "## Contexto Solicitado - (PID:TID) - (%d:%d)", pid, tid);
            hilo = devolver_contexto_ejecucion(pid, tid);
            //ENVIAR CONTEXTO EJECUCION
            break;
        case ACTUALIZAR_CONTEXTO_EJECUCION:
            actualizar_contexto_ejecucion(pid, tid, hilo);
            log_info(logger, "## Contexto Actualizado - (PID:TID) - (%d:%d)", pid, tid);
            enviar_mensaje("Contexto actualizado con éxito", socket);
            break;
        case DEVOLVER_INSTRUCCION:
            char* inst = devolver_instruccion(pid, tid, PC);
            log_info(logger, "## Obtener instrucción - (PID:TID) - (%d:%d) - Instrucción: <%s> <%s>", pid, tid, inst, archv_codigo);
            break;
        case LEER_MEMORIA:
            log_info(logger, "## Lectura - (PID:TID) - (%d:%d) - Dir. Física: %s - Tamaño: %d", pid, tid, dir_fisica, tamanio);
            //leer memoria
            break;
        case ESCRIBIR_MEMORIA:
            log_info(logger, "## Escritura - (PID:TID) - (%d:%d) - Dir. Física: %s - Tamaño: %d", pid, tid, dir_fisica, tamanio);
            //escribir memoria
            break;
        default:
            //log_error(logger, "Operación desconocida - CPU");
            break;
    }
}

int leer_buffer(int buffer)
{
    return CREAR_PROCESO;
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