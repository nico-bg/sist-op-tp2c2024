#include <main.h>
#include <pthread.h>


int main(int argc, char* argv[]) {
 
    char* ip_memoria;
    char* puerto_memoria;
    char* puerto_escucha_dispatch;
    char* puerto_escucha_interrupt;
    t_log* logger;
    t_config* config;
    
    
    config = iniciar_config("../cpu.config");
    logger = iniciar_logger(config, "cpu.log", "CPU");

    ip_memoria = config_get_string_value(config, "IP_MEMORIA");
    puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
    puerto_escucha_dispatch = config_get_string_value(config, "PUERTO_ESCUCHA_DISPATCH");
    puerto_escucha_interrupt = config_get_string_value(config, "PUERTO_ESCUCHA_INTERRUPT");

    int socket_memoria = conectar_a_socket(ip_memoria, puerto_memoria);
    log_info(logger, "Conectado a Memoria");    
    enviar_mensaje("Hola, soy el CPU", socket_memoria);

   int fd_dispatch = iniciar_servidor(puerto_escucha_dispatch);
/* Esperamos a que se conecte el Kernel por el puerto dispatch */
    int socket_dispatch = esperar_cliente(fd_dispatch);
    log_info(logger, "Hola, el kernel se conecto por puerto dispatch");

/*
   int fd_interrupt = iniciar_servidor(puerto_escucha_interrupt);
  -- Esperamos a que se conecte el Kernel por el puerto interrupt 
    int socket_interrupt = esperar_cliente(fd_interrupt);
*/
    log_info(logger, "Hola, el kernel se conecto por puerto interrupt");

    t_thread_args dispatch_args = {socket_dispatch, socket_memoria, logger};  

    pthread_t thread_dispatch;
    
    pthread_create(&thread_dispatch, NULL, escuchar_dispatch, &dispatch_args);    

    pthread_join(thread_dispatch, NULL);

    terminar_programa(logger, config, socket_memoria);

    return 0;
}
   
void escuchar_dispatch (void *args) {
    t_thread_args *thread_args = (t_thread_args *) args;
    
    int cliente_dispatch = thread_args -> cliente_socket;
    t_log *logger =  thread_args -> logger;
    int servidor_memoria =  thread_args -> cliente_memoria;

    log_info(logger, "Hilo escuchar_dispatch esperando pid y tid del kernel");


	t_buffer* buffer;
    
    uint32_t* size;

    t_hilo_a_cpu* pcb;
    
    while (1) {
        op_code cod_op = recibir_operacion(cliente_dispatch);
        
        //op_code cod_op = OPERACION_EJECUTAR_HILO;

        
        log_info(logger, "me llego el codigo de operacion");
        switch (cod_op) {
            case OPERACION_EJECUTAR_HILO:
                
                log_info(logger, "me llegó un OPERACION_EJECUTAR_HILO!!");
                
                buffer = recibir_buffer(size, cliente_dispatch);

                log_info(logger, "me llego el buffer");

                 pcb = deserializar_hilo_a_cpu(buffer);

                log_info(logger, "me llego el buffer con primer campo:%d", pcb -> tid );

                log_info(logger, "me llego el buffer con segundo campo:%d", pcb -> pid );

                break;
            case -1:
        
                log_error(logger, "cliente desconectado");
                return EXIT_FAILURE;
            default:
                log_warning(logger, "Operacion desconocida");
                break;    
        }
    }


}
/*

void procesar_pcb (int socket_mem, t_hilo_a_cpu estructura_pcb, t_log*  logger){
 log_info(logger, "Se ejecuta el pcb");
}
*/

void ejecutar_instruccion(t_instruccion *instruccion, uint32_t valor_rw){   //valor_rw seria un valor que tal vez usemos en la ejecucion de read y write al desarrollar una nueva funcion que escriba en memoria//
	int resultado; //variable para almacenar datos de salida luego de la ejecucion de inst. dependiendo la operacion//
    switch(instruccion->instruc){ //uso de semaforos en cada caso para usar logger//
        case SET:
        break;

        case READ_MEM:
        break;

        case WRITE_MEM:
        break;

        case SUM:
        break;

        case SUB:
        break;

        case JNZ:
        break;

        case LOG:
        break;

    }

}

void chequeo_interrupcion(){
    //uso de semaforos combinados con hilos que no me queda claro//
    //es una funcion que se la llama al final de cada "case" de la funcion ejecutar_instruccion//
}


void terminar_programa(t_log* logger, t_config* config, int conexion)
{
    log_destroy(logger);
    config_destroy(config);
    close(conexion);
}


