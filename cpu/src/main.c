#include <main.h>

t_log* logger_cpu;
char* puerto_escucha_dispatch;
char* puerto_escucha_interrupt;
t_config* config;

int main(int argc, char* argv[]) {
 
    char* ip_memoria;
    char* puerto_memoria;

    config = iniciar_config("cpu.config");
    logger_cpu = iniciar_logger(config, "cpu.log", "CPU");

    ip_memoria = config_get_string_value(config, "IP_MEMORIA");
    puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
    puerto_escucha_dispatch = config_get_string_value(config, "PUERTO_ESCUCHA_DISPATCH");
    puerto_escucha_interrupt = config_get_string_value(config, "PUERTO_ESCUCHA_INTERRUPT");

    int socket_memoria = conectar_a_socket(ip_memoria, puerto_memoria);
    log_info(logger_cpu, "Conectado a Memoria");
    
    enviar_mensaje("Hola, soy el CPU", socket_memoria);


    iniciar();


    terminar_programa(logger, config, socket_memoria);

    return 0;
}

void terminar_programa(t_log* logger, t_config* config, int conexion)
{
    log_destroy(logger);
    config_destroy(config);
    close(conexion);
}

void iniciar(){
    esperar_a_kernel();
}

void esperar_a_kernel(){
    pthread_t *hilo_dispatch = malloc(sizeof(pthread_t));
    pthread_t *hilo_interrupt = malloc(sizeof(pthread_t));

    pthread_create(hilo_dispatch, NULL, &ejecutar_pid,NULL);
    pthread_create(hilo_interrupt, NULL, &ejecutar_interrupcion,NULL);

    pthread_join(*hilo_dispatch, NULL);
    pthread_join(*hilo_interrupt, NULL);   
}

void ejecutar_pid(){
    
    int fd_dispatch = iniciar_servidor(puerto_escucha_dispatch);
/* Esperamos a que se conecte el Kernel por el puerto dispatch */
    int socket_dispatch = esperar_cliente(fd_dispatch);
    log_info(logger_cpu, "Se conectó el Kernel por el puerto Dispatch");

    /* Escuchamos una sola petición del Kernel por el puerto Dispatch */
    //atender_peticion(logger_cpu, config, socket_dispatch);

    while(1) {
    int operacion = recibir_operacion(socket_dispatch); 

    switch(operacion){
        case EJEC:

    }

    }

}

void ejecutar_interrupcion(){

    int fd_interrupt = iniciar_servidor(puerto_escucha_interrupt);
    /* Esperamos a que se conecte el Kernel por el puerto interrupt */
    int socket_interrupt = esperar_cliente(fd_interrupt);
    log_info(logger_cpu, "Se conectó el Kernel por el puerto Interrupt");

    /* Escuchamos una sola petición del Kernel por el puerto Interrupt */
    atender_peticion(logger_cpu, config, socket_interrupt);
}


