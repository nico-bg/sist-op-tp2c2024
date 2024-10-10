#include <main.h>
#include <pthread.h>
#include <semaphore.h>


typedef struct
{
    int cliente_socket;
    int cliente_memoria;
    t_log *logger;
} t_thread_args;

sem_t sem_ciclo_de_instruccion;
sem_t sem_operaciones;

int main(int argc, char* argv[]) {
 
    char* ip_memoria;
    char* puerto_memoria;
    char* puerto_escucha_dispatch;
    char* puerto_escucha_interrupt;
    
    t_log* logger;
    t_config* config;
    
    iniciar_semaforos();
    
    config = iniciar_config("cpu.config");
    logger = iniciar_logger(config, "cpu.log", "CPU");

    ip_memoria = config_get_string_value(config, "IP_MEMORIA");
    puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
    puerto_escucha_dispatch = config_get_string_value(config, "PUERTO_ESCUCHA_DISPATCH");
    puerto_escucha_interrupt = config_get_string_value(config, "PUERTO_ESCUCHA_INTERRUPT");

    //int socket_memoria = conectar_a_socket(ip_memoria, puerto_memoria);
    
    int socket_memoria = 1;
    
    //log_info(logger, "Conectado a Memoria");    
    //enviar_mensaje("Hola, soy el CPU", socket_memoria);

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


    t_thread_args hilos_args = {socket_dispatch, socket_memoria, logger};  


    pthread_t thread_dispatch;
    pthread_t thread_ciclo_de_instruccion;
    
    pthread_create(&thread_dispatch, NULL, escuchar_dispatch, &hilos_args);    
    pthread_create(&thread_ciclo_de_instruccion, NULL, ciclo_de_instruccion, &hilos_args);    

    pthread_join(thread_dispatch, NULL);
    pthread_join(thread_ciclo_de_instruccion, NULL);

    terminar_programa(logger, config, socket_memoria);

    return 0;
}

void iniciar_semaforos (){
    sem_init(&sem_ciclo_de_instruccion, 0, 0);
    sem_init(&sem_operaciones, 0, 0);
}


void ciclo_de_instruccion (void *args) {
    sem_wait(&sem_ciclo_de_instruccion);

    t_thread_args *thread_args = (t_thread_args *) args;
    t_log *logger =  thread_args -> logger;
    int servidor_memoria =  thread_args -> cliente_memoria;

    log_info(logger, "Hilo ciclo_de_instrucción en ejecucion");
}


void escuchar_dispatch (void *args) {
    t_thread_args *thread_args = (t_thread_args *) args;
    
    int cliente_dispatch = thread_args -> cliente_socket;
    t_log *logger =  thread_args -> logger;
    int servidor_memoria =  thread_args -> cliente_memoria;

    log_info(logger, "Hilo escuchar_dispatch esperando pid y tid del kernel");

	t_buffer* buffer;
    uint32_t size;
    
    while (1) {
        op_code cod_op = recibir_operacion(cliente_dispatch);
        
        log_info(logger, "me llego el codigo de operacion");
        switch (cod_op) {
            case OPERACION_EJECUTAR_HILO:
                
                log_info(logger, "me llegó un OPERACION_EJECUTAR_HILO!!");
                
                buffer = recibir_buffer(&size, cliente_dispatch);

                log_info(logger, "me llego el buffer");

                 pcb = deserializar_hilo_a_cpu(buffer);

                log_info(logger, "me llego el buffer con primer campo:%d", pcb -> tid );

                log_info(logger, "me llego el buffer con segundo campo:%d", pcb -> pid );


                t_buffer* contexto_devuelto = pedir_contexto(servidor_memoria, buffer);

                contexto = deserializar_datos_contexto(contexto_devuelto);

                
                log_info(logger, "me llego el buffer con segundo campo:%d", contexto.AX );


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



t_buffer* pedir_contexto(int servidor_memoria, t_buffer* buffer_pedido_contexto)
{
    // Empaquetamos y serializamos los datos junto con el código de operación
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = DEVOLVER_CONTEXTO_EJECUCION;
    paquete->buffer = buffer_pedido_contexto;
    t_buffer* paquete_serializado = serializar_paquete(paquete);

    send(servidor_memoria, paquete_serializado->stream, paquete_serializado->size, 0);

    buffer_destroy(paquete_serializado);
    eliminar_paquete(paquete);
    recibir_operacion(servidor_memoria);    
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
        sem_wait(&sem_operaciones);
        
        sem_post(&sem_operaciones);
        break;

        case READ_MEM:
         sem_wait(&sem_operaciones);
        //...
        sem_post(&sem_operaciones);
        break;

        case WRITE_MEM:
         sem_wait(&sem_operaciones);
        //...
        sem_post(&sem_operaciones);
        break;

        case SUM:
         sem_wait(&sem_operaciones);
        //...
        sem_post(&sem_operaciones);
        break;

        case SUB:
         sem_wait(&sem_operaciones);
        //...
        sem_post(&sem_operaciones); sem_wait(&sem_operaciones);
        break;

        case JNZ:
         sem_wait(&sem_operaciones);
        //...
        sem_post(&sem_operaciones);
        break;

        case LOG:
         sem_wait(&sem_operaciones);
        //...
        sem_post(&sem_operaciones);
        break;

        case DUMP_MEMORY:
         sem_wait(&sem_operaciones);
        //...
        sem_post(&sem_operaciones);
        break;

        case IO:
         sem_wait(&sem_operaciones);
        //...
        sem_post(&sem_operaciones);
        break;

        case PROCESS_CREATE:
         sem_wait(&sem_operaciones);
        //...
        sem_post(&sem_operaciones);
        break;

        case THREAD_CREATE:
         sem_wait(&sem_operaciones);
        //...
        sem_post(&sem_operaciones);
        break;

        case THREAD_JOIN:
         sem_wait(&sem_operaciones);
        //...
        sem_post(&sem_operaciones);
        break;

        case THREAD_CANCEL:
         sem_wait(&sem_operaciones);
        //...
        sem_post(&sem_operaciones);
        break;

        case MUTEX_CREATE:
         sem_wait(&sem_operaciones);
        //...
        sem_post(&sem_operaciones);
        break;

        case MUTEX_LOCK:
         sem_wait(&sem_operaciones);
        //...
        sem_post(&sem_operaciones);
        break;

        case MUTEX_UNLOCK:
         sem_wait(&sem_operaciones);
        //...
        sem_post(&sem_operaciones);
        break;

        case THREAD_EXIT:
         sem_wait(&sem_operaciones);
        //...
        sem_post(&sem_operaciones);
        break;

        case PROCESS_EXIT:
         sem_wait(&sem_operaciones);
        //...
        sem_post(&sem_operaciones);
        break;

    }

}

/*
void chequeo_interrupcion(){
    //uso de semaforos combinados con hilos que no me queda claro//
    //es una funcion que se la llama al final de cada "case" de la funcion ejecutar_instruccion//
}
*/

void terminar_programa(t_log* logger, t_config* config, int conexion)
{
    log_destroy(logger);
    config_destroy(config);
    close(conexion);
}


