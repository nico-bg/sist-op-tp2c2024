#include <main.h>
#include <pthread.h>
#include <semaphore.h>

t_config* config;
t_log* logger;
int socket_memoria;
int socket_dispatch;
int socket_interrupt;

sem_t sem_ciclo_de_instruccion;

int main(int argc, char* argv[]) {
 
    char* ip_memoria;
    char* puerto_memoria;
    char* puerto_escucha_dispatch;
    char* puerto_escucha_interrupt;
    

    iniciar_semaforos();
    
    config = iniciar_config("cpu.config");
    logger = iniciar_logger(config, "cpu.log", "CPU");

    ip_memoria = config_get_string_value(config, "IP_MEMORIA");
    puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
    puerto_escucha_dispatch = config_get_string_value(config, "PUERTO_ESCUCHA_DISPATCH");
    puerto_escucha_interrupt = config_get_string_value(config, "PUERTO_ESCUCHA_INTERRUPT");

    socket_memoria = conectar_a_socket(ip_memoria, puerto_memoria);

    log_info(logger, "Conectado a Memoria");    
   // enviar_mensaje("Hola, soy el CPU", socket_memoria);

   int fd_dispatch = iniciar_servidor(puerto_escucha_dispatch);
   int fd_interrupt = iniciar_servidor(puerto_escucha_interrupt);
/* Esperamos a que se conecte el Kernel por el puerto dispatch */
    socket_dispatch = esperar_cliente(fd_dispatch);
    log_info(logger, "Hola, el kernel se conecto por puerto dispatch");


/* Esperamos a que se conecte el Kernel por el puerto interrupt */
    socket_interrupt = esperar_cliente(fd_interrupt);

    log_info(logger, "Hola, el kernel se conecto por puerto interrupt");


    pthread_t thread_dispatch;
    pthread_t thread_ciclo_de_instruccion;
    
    pthread_create(&thread_dispatch, NULL, escuchar_dispatch, NULL);    
    pthread_create(&thread_ciclo_de_instruccion, NULL, ciclo_de_instruccion, NULL);    

    pthread_join(thread_dispatch, NULL);
    pthread_join(thread_ciclo_de_instruccion, NULL);

    terminar_programa();

    return 0;
}

void iniciar_semaforos (){
    sem_init(&sem_ciclo_de_instruccion, 0, 0);
}


void escuchar_dispatch () {

    log_info(logger, "Hilo escuchar_dispatch esperando pid y tid del kernel");
 
	t_buffer* buffer;    
    uint32_t size;
    
    while (1) {
        op_code cod_op = recibir_operacion(socket_dispatch);

        log_info(logger, "me llego el codigo de operacion");
        
        switch (cod_op) {
            case OPERACION_EJECUTAR_HILO:
                
                log_info(logger, "me llegó un OPERACION_EJECUTAR_HILO!!");
                
                buffer = recibir_buffer(&size, socket_dispatch);

               log_info(logger, "me llego el buffer");

    // pcb estructura global
                pcb = deserializar_hilo_a_cpu(buffer);

        
                log_info(logger, "me llego el buffer con primer campo:%d", pcb->tid );

                log_info(logger, "me llego el buffer con segundo campo:%d", pcb->pid );

                t_buffer* contexto_devuelto = pedir_contexto(socket_memoria, buffer);

                contexto = deserializar_datos_contexto(contexto_devuelto);


//READ_MEM AX BX
               //   contexto.PC = 1;
               //   contexto.AX = 2;
               //   contexto.BX = 3;
               //   contexto.CX = 0;
               //   contexto.DX = 2;
               //   contexto.EX = 0;
               //   contexto.FX = 0;
               //   contexto.GX = 9;
               //   contexto.Base = 1000;
               //   contexto.Limite = 2000;

                log_info(logger, "El valor de GX es:%d",contexto.GX);

                sem_post(&sem_ciclo_de_instruccion);

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


void ciclo_de_instruccion () {
    
    while(true){
    
    sem_wait(&sem_ciclo_de_instruccion);

    log_info(logger, "Ciclo_de_instrucción en ejecucion");

    //Fetch

    t_datos_obtener_instruccion* datos = malloc(sizeof(t_datos_obtener_instruccion));

    datos->PC = contexto.PC;
    datos->pid = contexto.pid;
    datos->tid= contexto.tid;
    

    t_buffer* buffer_pedido_instruccion = serializar_datos_solicitar_instruccion(datos);

    char* intruccion = pedir_proxima_instruccion(socket_memoria, buffer_pedido_instruccion);

   // char* instruccion = "LOG AX";

    log_info(logger, "Fetch finalizado");

    //Decode

    char** estructura_instruccion = string_split(instruccion, " ");

    if(strcmp(estructura_instruccion[0], "SET") == 0)
    {
        setear_registro(estructura_instruccion[1], estructura_instruccion[2]);
    }

    if(strcmp(estructura_instruccion[0], "SUM") == 0)
    {
        sum_registro(estructura_instruccion[1], estructura_instruccion[2]);
    }

    if(strcmp(estructura_instruccion[0], "SUB") == 0)
    {
        sub_registro(estructura_instruccion[1], estructura_instruccion[2]);
    }

    if(strcmp(estructura_instruccion[0], "READ_MEM") == 0)
    {
      log_info(logger, "Entra por read_mem");

      read_mem(estructura_instruccion[1], estructura_instruccion[2]);

      log_info(logger, "El valor de AX es:%d", contexto.AX);
    }

    if(strcmp(estructura_instruccion[0], "WRITE_MEM") == 0)
    {
      write_mem(estructura_instruccion[1], estructura_instruccion[2]);

    }

    if(strcmp(estructura_instruccion[0], "JNZ") == 0)
    {
    
      jnz_pc(estructura_instruccion[1], estructura_instruccion[2]);
    
    }

    if(strcmp(estructura_instruccion[0], "LOG") == 0)

    {

     log_info(logger, "El valor leido por instruccion LOG es:%d", obtener_registro(estructura_instruccion[1]));
    
    }

    if(strcmp(estructura_instruccion[0], "MUTEX_CREATE") == 0)
    {
     //actualizacion_contexto(socket_memoria, pid, tid, contexto);

     //devolver_control();


    }

    if(strcmp(estructura_instruccion[0], "MUTEX_LOCK") == 0)
    {
    //actualizacion_contexto(socket_memoria, pid, tid, contexto);

     //devolver_control();
    }

    if(strcmp(estructura_instruccion[0], "MUTEX_UNLOCK") == 0)
    {
    //actualizacion_contexto(socket_memoria, pid, tid, contexto);

     //devolver_control();
    }

    if(strcmp(estructura_instruccion[0], "DUMP_MEMORY") == 0)
    {
    //actualizacion_contexto(socket_memoria, pid, tid, contexto);

     //devolver_control();
    }

    if(strcmp(estructura_instruccion[0], "IO") == 0)
    {
    //actualizacion_contexto(socket_memoria, pid, tid, contexto);

     //devolver_control();
    }


    if(strcmp(estructura_instruccion[0], "PROCESS_CREATE") == 0)
    {
    //actualizacion_contexto(socket_memoria, pid, tid, contexto);

     //devolver_control();
    }

    if(strcmp(estructura_instruccion[0], "THREAD_CREATE") == 0)
    {
    //actualizacion_contexto(socket_memoria, pid, tid, contexto);

     //devolver_control();
    }

    if(strcmp(estructura_instruccion[0], "THREAD_CANCEL") == 0)
    {

    }

    if(strcmp(estructura_instruccion[0], "THREAD_JOIN") == 0)
    {

    }

    if(strcmp(estructura_instruccion[0], "THREAD_EXIT") == 0)
    {

    }

    if(strcmp(estructura_instruccion[0], "PROCESS_EXIT") == 0)
    {

    }

    }


    log_info(logger, "Decode finalizado");

}


t_buffer* pedir_contexto(int servidor_memoria, t_buffer* buffer_pedido_contexto)
{
    // Empaquetamos y serializamos los datos junto con el código de operación
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = OPERACION_DEVOLVER_CONTEXTO_EJECUCION;
    paquete->buffer = buffer_pedido_contexto;
    t_buffer* paquete_serializado = serializar_paquete(paquete);

    send(servidor_memoria, paquete_serializado->stream, paquete_serializado->size, 0);

    buffer_destroy(paquete_serializado);
    eliminar_paquete(paquete);
    recibir_operacion(servidor_memoria);   

    u_int32_t tamaño_buffer;

    return recibir_buffer(&tamaño_buffer, socket_memoria); 
}

void terminar_programa()
{
    log_destroy(logger);
    config_destroy(config);
    close(socket_memoria);
    close(socket_dispatch);
    close(socket_interrupt);
}

void setear_registro(char * registro, char * valor)
{
    if(strcmp(registro, "PC") == 0)
    {
       contexto.PC = atoi(valor);
    }

    if(strcmp(registro, "AX") == 0)
    {
       contexto.AX = atoi(valor);
    }

    if(strcmp(registro, "BX") == 0)
    {
       contexto.BX = atoi(valor);
    }

    if(strcmp(registro, "CX") == 0)
    {
       contexto.CX = atoi(valor);
    }

    if(strcmp(registro, "DX") == 0)
    {
       contexto.DX = atoi(valor);
    }

    if(strcmp(registro, "EX") == 0)
    {
       contexto.EX = atoi(valor);
    }

    if(strcmp(registro, "FX") == 0)
    {
       contexto.FX = atoi(valor);
    }

    if(strcmp(registro, "GX") == 0)
    {
       contexto.GX = atoi(valor);
    }

    if(strcmp(registro, "HX") == 0)
    {
       contexto.HX = atoi(valor);
    }

    if(strcmp(registro, "Base") == 0)
    {
       contexto.Base = atoi(valor);
    }

    if(strcmp(registro, "Limite") == 0)
    {
       contexto.Limite = atoi(valor);
    }

}

 void sum_registro(char * registro1, char * registro2)
{
//SUM PC otroRegistro
   if((strcmp(registro1, "PC") == 0)&&(strcmp(registro2, "PC") == 0))
    {
         contexto.PC = contexto.PC + contexto.PC;
    }

   if((strcmp(registro1, "PC") == 0)&&(strcmp(registro2, "AX") == 0))
    {
         contexto.PC = contexto.PC + contexto.AX;
    }

   if((strcmp(registro1, "PC") == 0)&&(strcmp(registro2, "BX") == 0))
    {
         contexto.PC = contexto.PC + contexto.BX;
    }
   if((strcmp(registro1, "PC") == 0)&&(strcmp(registro2, "CX") == 0))
    {
         contexto.PC = contexto.PC + contexto.CX;
    }
   if((strcmp(registro1, "PC") == 0)&&(strcmp(registro2, "DX") == 0))
    {
         contexto.PC = contexto.PC + contexto.DX;
    }
   if((strcmp(registro1, "PC") == 0)&&(strcmp(registro2, "EX") == 0))
    {
         contexto.PC = contexto.PC + contexto.EX;
    }
   if((strcmp(registro1, "PC") == 0)&&(strcmp(registro2, "FX") == 0))
    {
         contexto.PC = contexto.PC + contexto.FX;
    }
   if((strcmp(registro1, "PC") == 0)&&(strcmp(registro2, "GX") == 0))
    {
         contexto.PC = contexto.PC + contexto.GX;
    }
   if((strcmp(registro1, "PC") == 0)&&(strcmp(registro2, "HX") == 0))
    {
         contexto.PC = contexto.PC + contexto.HX;
    }
   if((strcmp(registro1, "PC") == 0)&&(strcmp(registro2, "Base") == 0))
    {
         contexto.PC = contexto.PC + contexto.Base;
    }
   if((strcmp(registro1, "PC") == 0)&&(strcmp(registro2, "Limite") == 0))
    {
         contexto.PC = contexto.PC + contexto.Limite;
    }

//SUM AX otroRegistro
   if((strcmp(registro1, "AX") == 0)&&(strcmp(registro2, "PC") == 0))
    {
         contexto.AX = contexto.AX + contexto.PC;
    }

   if((strcmp(registro1, "AX") == 0)&&(strcmp(registro2, "AX") == 0))
    {
         contexto.AX = contexto.AX + contexto.AX;
    }

   if((strcmp(registro1, "AX") == 0)&&(strcmp(registro2, "BX") == 0))
    {
         contexto.AX = contexto.AX + contexto.BX;
    }
   if((strcmp(registro1, "AX") == 0)&&(strcmp(registro2, "CX") == 0))
    {
         contexto.AX = contexto.AX + contexto.CX;
    }
   if((strcmp(registro1, "AX") == 0)&&(strcmp(registro2, "DX") == 0))
    {
         contexto.AX = contexto.AX + contexto.DX;
    }
   if((strcmp(registro1, "AX") == 0)&&(strcmp(registro2, "EX") == 0))
    {
         contexto.AX = contexto.AX + contexto.EX;
    }
   if((strcmp(registro1, "AX") == 0)&&(strcmp(registro2, "FX") == 0))
    {
         contexto.AX = contexto.AX + contexto.FX;
    }
   if((strcmp(registro1, "AX") == 0)&&(strcmp(registro2, "GX") == 0))
    {
         contexto.AX = contexto.AX + contexto.GX;
    }
   if((strcmp(registro1, "AX") == 0)&&(strcmp(registro2, "HX") == 0))
    {
         contexto.AX = contexto.AX + contexto.HX;
    }
   if((strcmp(registro1, "AX") == 0)&&(strcmp(registro2, "Base") == 0))
    {
         contexto.AX = contexto.AX + contexto.Base;
    }
   if((strcmp(registro1, "AX") == 0)&&(strcmp(registro2, "Limite") == 0))
    {
         contexto.AX = contexto.AX + contexto.Limite;
    }

//SUM BX otroRegistro
   if((strcmp(registro1, "BX") == 0)&&(strcmp(registro2, "PC") == 0))
    {
         contexto.BX = contexto.BX + contexto.PC;
    }

   if((strcmp(registro1, "BX") == 0)&&(strcmp(registro2, "AX") == 0))
    {
         contexto.BX = contexto.BX + contexto.AX;
    }

   if((strcmp(registro1, "BX") == 0)&&(strcmp(registro2, "BX") == 0))
    {
         contexto.BX = contexto.BX + contexto.BX;
    }
   if((strcmp(registro1, "BX") == 0)&&(strcmp(registro2, "CX") == 0))
    {
         contexto.BX = contexto.BX + contexto.CX;
    }
   if((strcmp(registro1, "BX") == 0)&&(strcmp(registro2, "DX") == 0))
    {
         contexto.BX = contexto.BX + contexto.DX;
    }
   if((strcmp(registro1, "BX") == 0)&&(strcmp(registro2, "EX") == 0))
    {
         contexto.BX = contexto.BX + contexto.EX;
    }
   if((strcmp(registro1, "BX") == 0)&&(strcmp(registro2, "FX") == 0))
    {
         contexto.BX = contexto.BX + contexto.FX;
    }
   if((strcmp(registro1, "BX") == 0)&&(strcmp(registro2, "GX") == 0))
    {
         contexto.BX = contexto.BX + contexto.GX;
    }
   if((strcmp(registro1, "BX") == 0)&&(strcmp(registro2, "HX") == 0))
    {
         contexto.BX = contexto.BX + contexto.HX;
    }
   if((strcmp(registro1, "BX") == 0)&&(strcmp(registro2, "Base") == 0))
    {
         contexto.BX = contexto.BX + contexto.Base;
    }
   if((strcmp(registro1, "BX") == 0)&&(strcmp(registro2, "Limite") == 0))
    {
         contexto.BX = contexto.BX + contexto.Limite;
    }

//SUM CX otroRegistro
   if((strcmp(registro1, "CX") == 0)&&(strcmp(registro2, "PC") == 0))
    {
         contexto.CX = contexto.CX + contexto.PC;
    }

   if((strcmp(registro1, "CX") == 0)&&(strcmp(registro2, "AX") == 0))
    {
         contexto.CX = contexto.CX + contexto.AX;
    }

   if((strcmp(registro1, "CX") == 0)&&(strcmp(registro2, "BX") == 0))
    {
         contexto.CX = contexto.CX + contexto.BX;
    }
   if((strcmp(registro1, "CX") == 0)&&(strcmp(registro2, "CX") == 0))
    {
         contexto.CX = contexto.CX + contexto.CX;
    }
   if((strcmp(registro1, "CX") == 0)&&(strcmp(registro2, "DX") == 0))
    {
         contexto.CX = contexto.CX + contexto.DX;
    }
   if((strcmp(registro1, "CX") == 0)&&(strcmp(registro2, "EX") == 0))
    {
         contexto.CX = contexto.CX + contexto.EX;
    }
   if((strcmp(registro1, "CX") == 0)&&(strcmp(registro2, "FX") == 0))
    {
         contexto.CX = contexto.CX + contexto.FX;
    }
   if((strcmp(registro1, "CX") == 0)&&(strcmp(registro2, "GX") == 0))
    {
         contexto.CX = contexto.CX + contexto.GX;
    }
   if((strcmp(registro1, "CX") == 0)&&(strcmp(registro2, "HX") == 0))
    {
         contexto.CX = contexto.CX + contexto.HX;
    }
   if((strcmp(registro1, "CX") == 0)&&(strcmp(registro2, "Base") == 0))
    {
         contexto.CX = contexto.CX + contexto.Base;
    }
   if((strcmp(registro1, "CX") == 0)&&(strcmp(registro2, "Limite") == 0))
    {
         contexto.CX = contexto.CX + contexto.Limite;
    }

//SUM DX otroRegistro
   if((strcmp(registro1, "DX") == 0)&&(strcmp(registro2, "PC") == 0))
    {
         contexto.DX = contexto.DX + contexto.PC;
    }

   if((strcmp(registro1, "DX") == 0)&&(strcmp(registro2, "AX") == 0))
    {
         contexto.DX = contexto.DX + contexto.AX;
    }

   if((strcmp(registro1, "DX") == 0)&&(strcmp(registro2, "BX") == 0))
    {
         contexto.DX = contexto.DX + contexto.BX;
    }
   if((strcmp(registro1, "DX") == 0)&&(strcmp(registro2, "CX") == 0))
    {
         contexto.DX = contexto.DX + contexto.CX;
    }
   if((strcmp(registro1, "DX") == 0)&&(strcmp(registro2, "DX") == 0))
    {
         contexto.DX = contexto.DX + contexto.DX;
    }
   if((strcmp(registro1, "DX") == 0)&&(strcmp(registro2, "EX") == 0))
    {
         contexto.DX = contexto.DX + contexto.EX;
    }
   if((strcmp(registro1, "DX") == 0)&&(strcmp(registro2, "FX") == 0))
    {
         contexto.DX = contexto.DX + contexto.FX;
    }
   if((strcmp(registro1, "DX") == 0)&&(strcmp(registro2, "GX") == 0))
    {
         contexto.DX = contexto.DX + contexto.GX;
    }
   if((strcmp(registro1, "DX") == 0)&&(strcmp(registro2, "HX") == 0))
    {
         contexto.DX = contexto.DX + contexto.HX;
    }
   if((strcmp(registro1, "DX") == 0)&&(strcmp(registro2, "Base") == 0))
    {
         contexto.DX = contexto.DX + contexto.Base;
    }
   if((strcmp(registro1, "DX") == 0)&&(strcmp(registro2, "Limite") == 0))
    {
         contexto.DX = contexto.DX + contexto.Limite;
    }

//SUM EX otroRegistro
   if((strcmp(registro1, "EX") == 0)&&(strcmp(registro2, "PC") == 0))
    {
         contexto.EX = contexto.EX + contexto.PC;
    }

   if((strcmp(registro1, "EX") == 0)&&(strcmp(registro2, "AX") == 0))
    {
         contexto.EX = contexto.EX + contexto.AX;
    }

   if((strcmp(registro1, "EX") == 0)&&(strcmp(registro2, "BX") == 0))
    {
         contexto.EX = contexto.EX + contexto.BX;
    }
   if((strcmp(registro1, "EX") == 0)&&(strcmp(registro2, "CX") == 0))
    {
         contexto.EX = contexto.EX + contexto.CX;
    }
   if((strcmp(registro1, "EX") == 0)&&(strcmp(registro2, "DX") == 0))
    {
         contexto.EX = contexto.EX + contexto.DX;
    }
   if((strcmp(registro1, "EX") == 0)&&(strcmp(registro2, "EX") == 0))
    {
         contexto.EX = contexto.EX + contexto.EX;
    }
   if((strcmp(registro1, "EX") == 0)&&(strcmp(registro2, "FX") == 0))
    {
         contexto.EX = contexto.EX + contexto.FX;
    }
   if((strcmp(registro1, "EX") == 0)&&(strcmp(registro2, "GX") == 0))
    {
         contexto.EX = contexto.EX + contexto.GX;
    }
   if((strcmp(registro1, "EX") == 0)&&(strcmp(registro2, "HX") == 0))
    {
         contexto.EX = contexto.EX + contexto.HX;
    }
   if((strcmp(registro1, "EX") == 0)&&(strcmp(registro2, "Base") == 0))
    {
         contexto.EX = contexto.EX + contexto.Base;
    }
   if((strcmp(registro1, "EX") == 0)&&(strcmp(registro2, "Limite") == 0))
    {
         contexto.EX = contexto.EX + contexto.Limite;
    }

//SUM FX otroRegistro
   if((strcmp(registro1, "FX") == 0)&&(strcmp(registro2, "PC") == 0))
    {
         contexto.FX = contexto.FX + contexto.PC;
    }

   if((strcmp(registro1, "FX") == 0)&&(strcmp(registro2, "AX") == 0))
    {
         contexto.FX = contexto.FX + contexto.AX;
    }

   if((strcmp(registro1, "FX") == 0)&&(strcmp(registro2, "BX") == 0))
    {
         contexto.FX = contexto.FX + contexto.BX;
    }
   if((strcmp(registro1, "FX") == 0)&&(strcmp(registro2, "CX") == 0))
    {
         contexto.FX = contexto.FX + contexto.CX;
    }
   if((strcmp(registro1, "FX") == 0)&&(strcmp(registro2, "DX") == 0))
    {
         contexto.FX = contexto.FX + contexto.DX;
    }
   if((strcmp(registro1, "FX") == 0)&&(strcmp(registro2, "EX") == 0))
    {
         contexto.FX = contexto.FX + contexto.EX;
    }
   if((strcmp(registro1, "FX") == 0)&&(strcmp(registro2, "FX") == 0))
    {
         contexto.FX = contexto.FX + contexto.FX;
    }
   if((strcmp(registro1, "FX") == 0)&&(strcmp(registro2, "GX") == 0))
    {
         contexto.FX = contexto.FX + contexto.GX;
    }
   if((strcmp(registro1, "FX") == 0)&&(strcmp(registro2, "HX") == 0))
    {
         contexto.FX = contexto.FX + contexto.HX;
    }
   if((strcmp(registro1, "FX") == 0)&&(strcmp(registro2, "Base") == 0))
    {
         contexto.FX = contexto.FX + contexto.Base;
    }
   if((strcmp(registro1, "FX") == 0)&&(strcmp(registro2, "Limite") == 0))
    {
         contexto.FX = contexto.FX + contexto.Limite;
    }

//SUM GX otroRegistro
   if((strcmp(registro1, "GX") == 0)&&(strcmp(registro2, "PC") == 0))
    {
         contexto.GX = contexto.GX + contexto.PC;
    }

   if((strcmp(registro1, "GX") == 0)&&(strcmp(registro2, "AX") == 0))
    {
         contexto.GX = contexto.GX + contexto.AX;
    }

   if((strcmp(registro1, "GX") == 0)&&(strcmp(registro2, "BX") == 0))
    {
         contexto.GX = contexto.GX + contexto.BX;
    }
   if((strcmp(registro1, "GX") == 0)&&(strcmp(registro2, "CX") == 0))
    {
         contexto.GX = contexto.GX + contexto.CX;
    }
   if((strcmp(registro1, "GX") == 0)&&(strcmp(registro2, "DX") == 0))
    {
         contexto.GX = contexto.GX + contexto.DX;
    }
   if((strcmp(registro1, "GX") == 0)&&(strcmp(registro2, "EX") == 0))
    {
         contexto.GX = contexto.GX + contexto.EX;
    }
   if((strcmp(registro1, "GX") == 0)&&(strcmp(registro2, "FX") == 0))
    {
         contexto.GX = contexto.GX + contexto.FX;
    }
   if((strcmp(registro1, "GX") == 0)&&(strcmp(registro2, "GX") == 0))
    {
         contexto.GX = contexto.GX + contexto.GX;
    }
   if((strcmp(registro1, "GX") == 0)&&(strcmp(registro2, "HX") == 0))
    {
         contexto.GX = contexto.GX + contexto.HX;
    }
   if((strcmp(registro1, "GX") == 0)&&(strcmp(registro2, "Base") == 0))
    {
         contexto.GX = contexto.GX + contexto.Base;
    }
   if((strcmp(registro1, "GX") == 0)&&(strcmp(registro2, "Limite") == 0))
    {
         contexto.GX = contexto.GX + contexto.Limite;
    }

//SUM HX otroRegistro
   if((strcmp(registro1, "HX") == 0)&&(strcmp(registro2, "PC") == 0))
    {
         contexto.HX = contexto.HX + contexto.PC;
    }

   if((strcmp(registro1, "HX") == 0)&&(strcmp(registro2, "AX") == 0))
    {
         contexto.HX = contexto.HX + contexto.AX;
    }

   if((strcmp(registro1, "HX") == 0)&&(strcmp(registro2, "BX") == 0))
    {
         contexto.HX = contexto.HX + contexto.BX;
    }
   if((strcmp(registro1, "HX") == 0)&&(strcmp(registro2, "CX") == 0))
    {
         contexto.HX = contexto.HX + contexto.CX;
    }
   if((strcmp(registro1, "HX") == 0)&&(strcmp(registro2, "DX") == 0))
    {
         contexto.HX = contexto.HX + contexto.DX;
    }
   if((strcmp(registro1, "HX") == 0)&&(strcmp(registro2, "EX") == 0))
    {
         contexto.HX = contexto.HX + contexto.EX;
    }
   if((strcmp(registro1, "HX") == 0)&&(strcmp(registro2, "FX") == 0))
    {
         contexto.HX = contexto.HX + contexto.FX;
    }
   if((strcmp(registro1, "HX") == 0)&&(strcmp(registro2, "GX") == 0))
    {
         contexto.HX = contexto.HX + contexto.GX;
    }
   if((strcmp(registro1, "HX") == 0)&&(strcmp(registro2, "HX") == 0))
    {
         contexto.HX = contexto.HX + contexto.HX;
    }
   if((strcmp(registro1, "HX") == 0)&&(strcmp(registro2, "Base") == 0))
    {
         contexto.HX = contexto.HX + contexto.Base;
    }
   if((strcmp(registro1, "HX") == 0)&&(strcmp(registro2, "Limite") == 0))
    {
         contexto.HX = contexto.HX + contexto.Limite;
    }

//SUM Base otroRegistro
   if((strcmp(registro1, "Base") == 0)&&(strcmp(registro2, "PC") == 0))
    {
         contexto.Base = contexto.Base + contexto.PC;
    }

   if((strcmp(registro1, "Base") == 0)&&(strcmp(registro2, "AX") == 0))
    {
         contexto.Base = contexto.Base + contexto.AX;
    }

   if((strcmp(registro1, "Base") == 0)&&(strcmp(registro2, "BX") == 0))
    {
         contexto.Base = contexto.Base + contexto.BX;
    }
   if((strcmp(registro1, "Base") == 0)&&(strcmp(registro2, "CX") == 0))
    {
         contexto.Base = contexto.Base + contexto.CX;
    }
   if((strcmp(registro1, "Base") == 0)&&(strcmp(registro2, "DX") == 0))
    {
         contexto.Base = contexto.Base + contexto.DX;
    }
   if((strcmp(registro1, "Base") == 0)&&(strcmp(registro2, "EX") == 0))
    {
         contexto.Base = contexto.Base + contexto.EX;
    }
   if((strcmp(registro1, "Base") == 0)&&(strcmp(registro2, "FX") == 0))
    {
         contexto.Base = contexto.Base + contexto.FX;
    }
   if((strcmp(registro1, "Base") == 0)&&(strcmp(registro2, "GX") == 0))
    {
         contexto.Base = contexto.Base + contexto.GX;
    }
   if((strcmp(registro1, "Base") == 0)&&(strcmp(registro2, "HX") == 0))
    {
         contexto.Base = contexto.Base + contexto.HX;
    }
   if((strcmp(registro1, "Base") == 0)&&(strcmp(registro2, "Base") == 0))
    {
         contexto.Base = contexto.Base + contexto.Base;
    }
   if((strcmp(registro1, "Base") == 0)&&(strcmp(registro2, "Limite") == 0))
    {
         contexto.Base = contexto.Base + contexto.Limite;
    }

//SUM Limite otroRegistro
   if((strcmp(registro1, "Limite") == 0)&&(strcmp(registro2, "PC") == 0))
    {
         contexto.Limite = contexto.Limite + contexto.PC;
    }

   if((strcmp(registro1, "Limite") == 0)&&(strcmp(registro2, "AX") == 0))
    {
         contexto.Limite = contexto.Limite + contexto.AX;
    }

   if((strcmp(registro1, "Limite") == 0)&&(strcmp(registro2, "BX") == 0))
    {
         contexto.Limite = contexto.Limite + contexto.BX;
    }
   if((strcmp(registro1, "Limite") == 0)&&(strcmp(registro2, "CX") == 0))
    {
         contexto.Limite = contexto.Limite + contexto.CX;
    }
   if((strcmp(registro1, "Limite") == 0)&&(strcmp(registro2, "DX") == 0))
    {
         contexto.Limite = contexto.Limite + contexto.DX;
    }
   if((strcmp(registro1, "Limite") == 0)&&(strcmp(registro2, "EX") == 0))
    {
         contexto.Limite = contexto.Limite + contexto.EX;
    }
   if((strcmp(registro1, "Limite") == 0)&&(strcmp(registro2, "FX") == 0))
    {
         contexto.Limite = contexto.Limite + contexto.FX;
    }
   if((strcmp(registro1, "Limite") == 0)&&(strcmp(registro2, "GX") == 0))
    {
         contexto.Limite = contexto.Limite + contexto.GX;
    }
   if((strcmp(registro1, "Limite") == 0)&&(strcmp(registro2, "HX") == 0))
    {
         contexto.Limite = contexto.Limite + contexto.HX;
    }
   if((strcmp(registro1, "Limite") == 0)&&(strcmp(registro2, "Base") == 0))
    {
         contexto.Limite = contexto.Limite + contexto.Base;
    }
   if((strcmp(registro1, "Limite") == 0)&&(strcmp(registro2, "Limite") == 0))
    {
         contexto.Limite = contexto.Limite + contexto.Limite;
    }

}
    
void sub_registro(char * registro1, char * registro2)
{
//SUB PC otroRegistro
   if((strcmp(registro1, "PC") == 0)&&(strcmp(registro2, "PC") == 0))
    {
         contexto.PC = contexto.PC - contexto.PC;
    }

   if((strcmp(registro1, "PC") == 0)&&(strcmp(registro2, "AX") == 0))
    {
         contexto.PC = contexto.PC - contexto.AX;
    }

   if((strcmp(registro1, "PC") == 0)&&(strcmp(registro2, "BX") == 0))
    {
         contexto.PC = contexto.PC - contexto.BX;
    }
   if((strcmp(registro1, "PC") == 0)&&(strcmp(registro2, "CX") == 0))
    {
         contexto.PC = contexto.PC - contexto.CX;
    }
   if((strcmp(registro1, "PC") == 0)&&(strcmp(registro2, "DX") == 0))
    {
         contexto.PC = contexto.PC - contexto.DX;
    }
   if((strcmp(registro1, "PC") == 0)&&(strcmp(registro2, "EX") == 0))
    {
         contexto.PC = contexto.PC - contexto.EX;
    }
   if((strcmp(registro1, "PC") == 0)&&(strcmp(registro2, "FX") == 0))
    {
         contexto.PC = contexto.PC - contexto.FX;
    }
   if((strcmp(registro1, "PC") == 0)&&(strcmp(registro2, "GX") == 0))
    {
         contexto.PC = contexto.PC - contexto.GX;
    }
   if((strcmp(registro1, "PC") == 0)&&(strcmp(registro2, "HX") == 0))
    {
         contexto.PC = contexto.PC - contexto.HX;
    }
   if((strcmp(registro1, "PC") == 0)&&(strcmp(registro2, "Base") == 0))


   if((strcmp(registro1, "AX") == 0)&&(strcmp(registro2, "AX") == 0))
    {
         contexto.AX = contexto.AX - contexto.AX;
    }

   if((strcmp(registro1, "AX") == 0)&&(strcmp(registro2, "BX") == 0))
    {
         contexto.AX = contexto.AX - contexto.BX;
    }
   if((strcmp(registro1, "AX") == 0)&&(strcmp(registro2, "CX") == 0))
    {
         contexto.AX = contexto.AX - contexto.CX;
    }
   if((strcmp(registro1, "AX") == 0)&&(strcmp(registro2, "DX") == 0))
    {
         contexto.AX = contexto.AX - contexto.DX;
    }
   if((strcmp(registro1, "AX") == 0)&&(strcmp(registro2, "EX") == 0))
    {
         contexto.AX = contexto.AX - contexto.EX;
    }
   if((strcmp(registro1, "AX") == 0)&&(strcmp(registro2, "FX") == 0))
    {
         contexto.AX = contexto.AX - contexto.FX;
    }
   if((strcmp(registro1, "AX") == 0)&&(strcmp(registro2, "GX") == 0))
    {
         contexto.AX = contexto.AX - contexto.GX;
    }
   if((strcmp(registro1, "AX") == 0)&&(strcmp(registro2, "HX") == 0))
    {
         contexto.AX = contexto.AX - contexto.HX;
    }
   if((strcmp(registro1, "AX") == 0)&&(strcmp(registro2, "Base") == 0))
    {
         contexto.AX = contexto.AX - contexto.Base;
    }
   if((strcmp(registro1, "AX") == 0)&&(strcmp(registro2, "Limite") == 0))
    {
         contexto.AX = contexto.AX - contexto.Limite;
    }

//SUB BX otroRegistro
   if((strcmp(registro1, "BX") == 0)&&(strcmp(registro2, "PC") == 0))
    {
         contexto.BX = contexto.BX - contexto.PC;
    }

   if((strcmp(registro1, "BX") == 0)&&(strcmp(registro2, "AX") == 0))
    {
         contexto.BX = contexto.BX - contexto.AX;
    }

   if((strcmp(registro1, "BX") == 0)&&(strcmp(registro2, "BX") == 0))
    {
         contexto.BX = contexto.BX - contexto.BX;
    }
   if((strcmp(registro1, "BX") == 0)&&(strcmp(registro2, "CX") == 0))
    {
         contexto.BX = contexto.BX - contexto.CX;
    }
   if((strcmp(registro1, "BX") == 0)&&(strcmp(registro2, "DX") == 0))
    {
         contexto.BX = contexto.BX - contexto.DX;
    }
   if((strcmp(registro1, "BX") == 0)&&(strcmp(registro2, "EX") == 0))
    {
         contexto.BX = contexto.BX - contexto.EX;
    }
   if((strcmp(registro1, "BX") == 0)&&(strcmp(registro2, "FX") == 0))
    {
         contexto.BX = contexto.BX - contexto.FX;
    }
   if((strcmp(registro1, "BX") == 0)&&(strcmp(registro2, "GX") == 0))
    {
         contexto.BX = contexto.BX - contexto.GX;
    }
   if((strcmp(registro1, "BX") == 0)&&(strcmp(registro2, "HX") == 0))
    {
         contexto.BX = contexto.BX - contexto.HX;
    }
   if((strcmp(registro1, "BX") == 0)&&(strcmp(registro2, "Base") == 0))
    {
         contexto.BX = contexto.BX - contexto.Base;
    }
   if((strcmp(registro1, "BX") == 0)&&(strcmp(registro2, "Limite") == 0))
    {
         contexto.BX = contexto.BX - contexto.Limite;
    }

//SUB CX otroRegistro
   if((strcmp(registro1, "CX") == 0)&&(strcmp(registro2, "PC") == 0))
    {
         contexto.CX = contexto.CX - contexto.PC;
    }

   if((strcmp(registro1, "CX") == 0)&&(strcmp(registro2, "AX") == 0))
    {
         contexto.CX = contexto.CX - contexto.AX;
    }

   if((strcmp(registro1, "CX") == 0)&&(strcmp(registro2, "BX") == 0))
    {
         contexto.CX = contexto.CX - contexto.BX;
    }
   if((strcmp(registro1, "CX") == 0)&&(strcmp(registro2, "CX") == 0))
    {
         contexto.CX = contexto.CX - contexto.CX;
    }
   if((strcmp(registro1, "CX") == 0)&&(strcmp(registro2, "DX") == 0))
    {
         contexto.CX = contexto.CX - contexto.DX;
    }
   if((strcmp(registro1, "CX") == 0)&&(strcmp(registro2, "EX") == 0))
    {
         contexto.CX = contexto.CX - contexto.EX;
    }
   if((strcmp(registro1, "CX") == 0)&&(strcmp(registro2, "FX") == 0))
    {
         contexto.CX = contexto.CX - contexto.FX;
    }
   if((strcmp(registro1, "CX") == 0)&&(strcmp(registro2, "GX") == 0))
    {
         contexto.CX = contexto.CX - contexto.GX;
    }
   if((strcmp(registro1, "CX") == 0)&&(strcmp(registro2, "HX") == 0))
    {
         contexto.CX = contexto.CX - contexto.HX;
    }
   if((strcmp(registro1, "CX") == 0)&&(strcmp(registro2, "Base") == 0))
    {
         contexto.CX = contexto.CX - contexto.Base;
    }
   if((strcmp(registro1, "CX") == 0)&&(strcmp(registro2, "Limite") == 0))
    {
         contexto.CX = contexto.CX - contexto.Limite;
    }

//SUB DX otroRegistro
   if((strcmp(registro1, "DX") == 0)&&(strcmp(registro2, "PC") == 0))
    {
         contexto.DX = contexto.DX - contexto.PC;
    }

   if((strcmp(registro1, "DX") == 0)&&(strcmp(registro2, "AX") == 0))
    {
         contexto.DX = contexto.DX - contexto.AX;
    }

   if((strcmp(registro1, "DX") == 0)&&(strcmp(registro2, "BX") == 0))
    {
         contexto.DX = contexto.DX - contexto.BX;
    }
   if((strcmp(registro1, "DX") == 0)&&(strcmp(registro2, "CX") == 0))
    {
         contexto.DX = contexto.DX - contexto.CX;
    }
   if((strcmp(registro1, "DX") == 0)&&(strcmp(registro2, "DX") == 0))
    {
         contexto.DX = contexto.DX - contexto.DX;
    }
   if((strcmp(registro1, "DX") == 0)&&(strcmp(registro2, "EX") == 0))
    {
         contexto.DX = contexto.DX - contexto.EX;
    }
   if((strcmp(registro1, "DX") == 0)&&(strcmp(registro2, "FX") == 0))
    {
         contexto.DX = contexto.DX - contexto.FX;
    }
   if((strcmp(registro1, "DX") == 0)&&(strcmp(registro2, "GX") == 0))
    {
         contexto.DX = contexto.DX - contexto.GX;
    }
   if((strcmp(registro1, "DX") == 0)&&(strcmp(registro2, "HX") == 0))
    {
         contexto.DX = contexto.DX - contexto.HX;
    }
   if((strcmp(registro1, "DX") == 0)&&(strcmp(registro2, "Base") == 0))
    {
         contexto.DX = contexto.DX - contexto.Base;
    }
   if((strcmp(registro1, "DX") == 0)&&(strcmp(registro2, "Limite") == 0))
    {
         contexto.DX = contexto.DX - contexto.Limite;
    }

//SUB EX otroRegistro
   if((strcmp(registro1, "EX") == 0)&&(strcmp(registro2, "PC") == 0))
    {
         contexto.EX = contexto.EX - contexto.PC;
    }

   if((strcmp(registro1, "EX") == 0)&&(strcmp(registro2, "AX") == 0))
    {
         contexto.EX = contexto.EX - contexto.AX;
    }

   if((strcmp(registro1, "EX") == 0)&&(strcmp(registro2, "BX") == 0))
    {
         contexto.EX = contexto.EX - contexto.BX;
    }
   if((strcmp(registro1, "EX") == 0)&&(strcmp(registro2, "CX") == 0))
    {
         contexto.EX = contexto.EX - contexto.CX;
    }
   if((strcmp(registro1, "EX") == 0)&&(strcmp(registro2, "DX") == 0))
    {
         contexto.EX = contexto.EX - contexto.DX;
    }
   if((strcmp(registro1, "EX") == 0)&&(strcmp(registro2, "EX") == 0))
    {
         contexto.EX = contexto.EX - contexto.EX;
    }
   if((strcmp(registro1, "EX") == 0)&&(strcmp(registro2, "GX") == 0))
    {
         contexto.EX = contexto.EX - contexto.GX;
    }       
   if((strcmp(registro1, "EX") == 0)&&(strcmp(registro2, "HX") == 0))
    {
         contexto.EX = contexto.EX - contexto.HX;
    }       
   if((strcmp(registro1, "EX") == 0)&&(strcmp(registro2, "Base") == 0))
    {
         contexto.EX = contexto.EX - contexto.Base;
    }
   if((strcmp(registro1, "EX") == 0)&&(strcmp(registro2, "Limite") == 0))
    {
         contexto.EX = contexto.EX - contexto.Limite;
    }

//SUB FX otroRegistro
   if((strcmp(registro1, "FX") == 0)&&(strcmp(registro2, "PC") == 0))
    {
         contexto.FX = contexto.FX - contexto.PC;
    }

   if((strcmp(registro1, "FX") == 0)&&(strcmp(registro2, "AX") == 0))
    {
         contexto.FX = contexto.FX - contexto.AX;
    }

   if((strcmp(registro1, "FX") == 0)&&(strcmp(registro2, "BX") == 0))
    {
         contexto.FX = contexto.FX - contexto.BX;
    }
   if((strcmp(registro1, "FX") == 0)&&(strcmp(registro2, "CX") == 0))
    {
         contexto.FX = contexto.FX - contexto.CX;
    }
   if((strcmp(registro1, "FX") == 0)&&(strcmp(registro2, "DX") == 0))
    {
         contexto.FX = contexto.FX - contexto.DX;
    }
   if((strcmp(registro1, "FX") == 0)&&(strcmp(registro2, "EX") == 0))
    {
         contexto.FX = contexto.FX - contexto.EX;
    }
   if((strcmp(registro1, "FX") == 0)&&(strcmp(registro2, "FX") == 0))
    {
         contexto.FX = contexto.FX - contexto.FX;
    }
   if((strcmp(registro1, "FX") == 0)&&(strcmp(registro2, "GX") == 0))
    {
         contexto.FX = contexto.FX - contexto.GX;
    }
   if((strcmp(registro1, "FX") == 0)&&(strcmp(registro2, "HX") == 0))
    {
         contexto.FX = contexto.FX - contexto.HX;
    }
   if((strcmp(registro1, "FX") == 0)&&(strcmp(registro2, "Base") == 0))
    {
         contexto.FX = contexto.FX - contexto.Base;
    }
   if((strcmp(registro1, "FX") == 0)&&(strcmp(registro2, "Limite") == 0))
    {
         contexto.FX = contexto.FX - contexto.Limite;
    }

//SUB GX otroRegistro
   if((strcmp(registro1, "GX") == 0)&&(strcmp(registro2, "PC") == 0))
    {
         contexto.GX = contexto.GX - contexto.PC;
    }

   if((strcmp(registro1, "GX") == 0)&&(strcmp(registro2, "AX") == 0))
    {
         contexto.GX = contexto.GX - contexto.AX;
    }

   if((strcmp(registro1, "GX") == 0)&&(strcmp(registro2, "BX") == 0))
    {
         contexto.GX = contexto.GX - contexto.BX;
    }
   if((strcmp(registro1, "GX") == 0)&&(strcmp(registro2, "CX") == 0))
    {
         contexto.GX = contexto.GX - contexto.CX;
    }
   if((strcmp(registro1, "GX") == 0)&&(strcmp(registro2, "DX") == 0))
    {
         contexto.GX = contexto.GX - contexto.DX;
    }
   if((strcmp(registro1, "GX") == 0)&&(strcmp(registro2, "EX") == 0))
    {
         contexto.GX = contexto.GX - contexto.EX;
    }
   if((strcmp(registro1, "GX") == 0)&&(strcmp(registro2, "FX") == 0))
    {
         contexto.GX = contexto.GX - contexto.FX;
    }
   if((strcmp(registro1, "GX") == 0)&&(strcmp(registro2, "GX") == 0))
    {
         contexto.GX = contexto.GX - contexto.GX;
    }
   if((strcmp(registro1, "GX") == 0)&&(strcmp(registro2, "HX") == 0))
    {
         contexto.GX = contexto.GX - contexto.HX;
    }
   if((strcmp(registro1, "GX") == 0)&&(strcmp(registro2, "Base") == 0))
    {
         contexto.GX = contexto.GX - contexto.Base;
    }
   if((strcmp(registro1, "GX") == 0)&&(strcmp(registro2, "Limite") == 0))
    {
         contexto.GX = contexto.GX - contexto.Limite;
    }

//SUB HX otroRegistro
   if((strcmp(registro1, "HX") == 0)&&(strcmp(registro2, "PC") == 0))
    {
         contexto.HX = contexto.HX - contexto.PC;
    }

   if((strcmp(registro1, "HX") == 0)&&(strcmp(registro2, "AX") == 0))
    {
         contexto.HX = contexto.HX - contexto.AX;
    }

   if((strcmp(registro1, "HX") == 0)&&(strcmp(registro2, "BX") == 0))
    {
         contexto.HX = contexto.HX - contexto.BX;
    }
   if((strcmp(registro1, "HX") == 0)&&(strcmp(registro2, "CX") == 0))
    {
         contexto.HX = contexto.HX - contexto.CX;
    }
   if((strcmp(registro1, "HX") == 0)&&(strcmp(registro2, "DX") == 0))
    {
         contexto.HX = contexto.HX - contexto.DX;
    }
   if((strcmp(registro1, "HX") == 0)&&(strcmp(registro2, "EX") == 0))
    {
         contexto.HX = contexto.HX - contexto.EX;
    }
   if((strcmp(registro1, "HX") == 0)&&(strcmp(registro2, "FX") == 0))
    {
         contexto.HX = contexto.HX - contexto.FX;
    }
   if((strcmp(registro1, "HX") == 0)&&(strcmp(registro2, "GX") == 0))
    {
         contexto.HX = contexto.HX - contexto.GX;
    }
   if((strcmp(registro1, "HX") == 0)&&(strcmp(registro2, "HX") == 0))
    {
         contexto.HX = contexto.HX - contexto.HX;
    }
   if((strcmp(registro1, "HX") == 0)&&(strcmp(registro2, "Base") == 0))
    {
         contexto.HX = contexto.HX - contexto.Base;
    }
   if((strcmp(registro1, "HX") == 0)&&(strcmp(registro2, "Limite") == 0))
    {
         contexto.HX = contexto.HX - contexto.Limite;
    }

//SUB Base otroRegistro
   if((strcmp(registro1, "Base") == 0)&&(strcmp(registro2, "PC") == 0))
    {
         contexto.Base = contexto.Base - contexto.PC;
    }

   if((strcmp(registro1, "Base") == 0)&&(strcmp(registro2, "AX") == 0))
    {
         contexto.Base = contexto.Base - contexto.AX;
    }

   if((strcmp(registro1, "Base") == 0)&&(strcmp(registro2, "BX") == 0))
    {
         contexto.Base = contexto.Base - contexto.BX;
    }
   if((strcmp(registro1, "Base") == 0)&&(strcmp(registro2, "CX") == 0))
    {
         contexto.Base = contexto.Base - contexto.CX;
    }
   if((strcmp(registro1, "Base") == 0)&&(strcmp(registro2, "DX") == 0))
    {
         contexto.Base = contexto.Base - contexto.DX;
    }
   if((strcmp(registro1, "Base") == 0)&&(strcmp(registro2, "EX") == 0))
    {
         contexto.Base = contexto.Base - contexto.EX;
    }
   if((strcmp(registro1, "Base") == 0)&&(strcmp(registro2, "FX") == 0))
    {
         contexto.Base = contexto.Base - contexto.FX;
    }
   if((strcmp(registro1, "Base") == 0)&&(strcmp(registro2, "GX") == 0))
    {
         contexto.Base = contexto.Base - contexto.GX;
    }
   if((strcmp(registro1, "Base") == 0)&&(strcmp(registro2, "HX") == 0))
    {
         contexto.Base = contexto.Base - contexto.HX;
    }
   if((strcmp(registro1, "Base") == 0)&&(strcmp(registro2, "Base") == 0))
    {
         contexto.Base = contexto.Base - contexto.Base;
    }
   if((strcmp(registro1, "Base") == 0)&&(strcmp(registro2, "Limite") == 0))
    {
         contexto.Base = contexto.Base - contexto.Limite;
    }

//SUB Limite otroRegistro
   if((strcmp(registro1, "Limite") == 0)&&(strcmp(registro2, "PC") == 0))
    {
         contexto.Limite = contexto.Limite - contexto.PC;
    }

   if((strcmp(registro1, "Limite") == 0)&&(strcmp(registro2, "AX") == 0))
    {
         contexto.Limite = contexto.Limite - contexto.AX;
    }

   if((strcmp(registro1, "Limite") == 0)&&(strcmp(registro2, "BX") == 0))
    {
         contexto.Limite = contexto.Limite - contexto.BX;
    }
   if((strcmp(registro1, "Limite") == 0)&&(strcmp(registro2, "CX") == 0))
    {
         contexto.Limite = contexto.Limite - contexto.CX;
    }
   if((strcmp(registro1, "Limite") == 0)&&(strcmp(registro2, "DX") == 0))
    {
         contexto.Limite = contexto.Limite - contexto.DX;
    }
   if((strcmp(registro1, "Limite") == 0)&&(strcmp(registro2, "EX") == 0))
    {
         contexto.Limite = contexto.Limite - contexto.EX;
    }
   if((strcmp(registro1, "Limite") == 0)&&(strcmp(registro2, "FX") == 0))
    {
         contexto.Limite = contexto.Limite - contexto.FX;
    }
   if((strcmp(registro1, "Limite") == 0)&&(strcmp(registro2, "GX") == 0))
    {
         contexto.Limite = contexto.Limite - contexto.GX;
    }
   if((strcmp(registro1, "Limite") == 0)&&(strcmp(registro2, "HX") == 0))
    {
         contexto.Limite = contexto.Limite - contexto.HX;
    }
   if((strcmp(registro1, "Limite") == 0)&&(strcmp(registro2, "Base") == 0))
    {
         contexto.Limite = contexto.Limite - contexto.Base;
    }
   if((strcmp(registro1, "Limite") == 0)&&(strcmp(registro2, "Limite") == 0))
    {
         contexto.Limite = contexto.Limite - contexto.Limite;
    }
}

 void read_mem(char * registro1, char * registro2)
 {
      u_int32_t dir_logica;
      //u_int32_t dir_logica = lectura_memoria(pid, tid, dir_fisica);

      dir_logica = 300; 
// READ_MEM AX OtroRegistro
      if((strcmp(registro1, "AX") == 0)&& (strcmp(registro2, "AX") == 0))
            {
               if(mmu(contexto.AX) == 1)
                     {    
                        contexto.AX = dir_logica;
                   }
       }
if((strcmp(registro1, "AX") == 0)&& (strcmp(registro2, "BX") == 0))
    {
      if(mmu(contexto.BX) == 1)
      {    
       log_info(logger, "Va a actualizar la direccion lógica de AX");
       contexto.AX = dir_logica;
       log_info(logger, "Direccion lógica de AX:%d", contexto.AX);
       
      }
    }
if((strcmp(registro1, "AX") == 0)&& (strcmp(registro2, "CX") == 0))
    {
      if(mmu(contexto.CX) == 1)
      {    
       contexto.AX = dir_logica;
      }
    }
if((strcmp(registro1, "AX") == 0)&& (strcmp(registro2, "DX") == 0))
    {
      if(mmu(contexto.DX) == 1)
      {    
       contexto.AX = dir_logica;
      }
    }
if((strcmp(registro1, "AX") == 0)&& (strcmp(registro2, "EX") == 0))
    {
      if(mmu(contexto.EX) == 1)
      {    
       contexto.AX = dir_logica;
      }
    }
if((strcmp(registro1, "AX") == 0)&& (strcmp(registro2, "FX") == 0))
    {
      if(mmu(contexto.FX) == 1)
      {    
       contexto.AX = dir_logica;
      }
    }
if((strcmp(registro1, "AX") == 0)&& (strcmp(registro2, "GX") == 0))
    {
      if(mmu(contexto.GX) == 1)
      {    
       contexto.AX = dir_logica;
      }
    }
if((strcmp(registro1, "AX") == 0)&& (strcmp(registro2, "HX") == 0))
    {
      if(mmu(contexto.HX) == 1)
      {    
       contexto.AX = dir_logica;
      }
    }


// READ_MEM BX OtroRegistro
if((strcmp(registro1, "BX") == 0)&& (strcmp(registro2, "AX") == 0))
    {
      if(mmu(contexto.AX) == 1)
      {    
       contexto.BX = dir_logica;
      }
    }
if((strcmp(registro1, "BX") == 0)&& (strcmp(registro2, "BX") == 0))
    {
      if(mmu(contexto.BX) == 1)
      {    
       contexto.BX = dir_logica;
      }
    }
if((strcmp(registro1, "BX") == 0)&& (strcmp(registro2, "CX") == 0))
    {
      if(mmu(contexto.CX) == 1)
      {    
       contexto.BX = dir_logica;
      }
    }
if((strcmp(registro1, "BX") == 0)&& (strcmp(registro2, "DX") == 0))
    {
      if(mmu(contexto.DX) == 1)
      {    
return 1;
    }
if((strcmp(registro1, "BX") == 0)&& (strcmp(registro2, "EX") == 0))
    {
      if(mmu(contexto.EX) == 1)
      {    
       contexto.BX = dir_logica;
      }
    }
if((strcmp(registro1, "BX") == 0)&& (strcmp(registro2, "FX") == 0))
    {
      if(mmu(contexto.FX) == 1)
      {    
       contexto.BX = dir_logica;
      }
    }
if((strcmp(registro1, "BX") == 0)&& (strcmp(registro2, "GX") == 0))
    {
      if(mmu(contexto.GX) == 1)
      {    
       contexto.BX = dir_logica;
      }
    }
if((strcmp(registro1, "BX") == 0)&& (strcmp(registro2, "HX") == 0))
    {
      if(mmu(contexto.HX) == 1)
      {    
       contexto.BX = dir_logica;
      }
    }

// READ_MEM CX OtroRegistro
if((strcmp(registro1, "CX") == 0)&& (strcmp(registro2, "AX") == 0))
    {
      if(mmu(contexto.AX) == 1)
      {    
       contexto.CX = dir_logica;
      }
    }
if((strcmp(registro1, "CX") == 0)&& (strcmp(registro2, "BX") == 0))
    {
      if(mmu(contexto.BX) == 1)
      {    
       contexto.CX = dir_logica;
      }
    }
if((strcmp(registro1, "CX") == 0)&& (strcmp(registro2, "CX") == 0))
    {
      if(mmu(contexto.CX) == 1)
      {    
       contexto.CX = dir_logica;
      }
    }
if((strcmp(registro1, "CX") == 0)&& (strcmp(registro2, "DX") == 0))
    {
      if(mmu(contexto.DX) == 1)
      {    
       contexto.CX = dir_logica;
      }
    }
if((strcmp(registro1, "CX") == 0)&& (strcmp(registro2, "EX") == 0))
    {
      if(mmu(contexto.EX) == 1)
      {    
       contexto.CX = dir_logica;
      }
    }
if((strcmp(registro1, "CX") == 0)&& (strcmp(registro2, "FX") == 0))
    {
      if(mmu(contexto.FX) == 1)
      {    
       contexto.CX = dir_logica;
      }
    }
if((strcmp(registro1, "CX") == 0)&& (strcmp(registro2, "GX") == 0))
    {
      if(mmu(contexto.GX) == 1)
      {    
       contexto.CX = dir_logica;
      }
    }
if((strcmp(registro1, "CX") == 0)&& (strcmp(registro2, "HX") == 0))
    {
      if(mmu(contexto.HX) == 1)
      {    
       contexto.CX = dir_logica;
      }
    }

// READ_MEM DX OtroRegistro
if((strcmp(registro1, "DX") == 0)&& (strcmp(registro2, "AX") == 0))
    {
      if(mmu(contexto.AX) == 1)
      {    
       contexto.DX = dir_logica;
      }
    }
if((strcmp(registro1, "DX") == 0)&& (strcmp(registro2, "BX") == 0))
    {
      if(mmu(contexto.BX) == 1)
      {    
       contexto.DX = dir_logica;
      }
    }
if((strcmp(registro1, "DX") == 0)&& (strcmp(registro2, "CX") == 0))
    {
      if(mmu(contexto.CX) == 1)
      {    
       contexto.DX = dir_logica;
      }
    }
if((strcmp(registro1, "DX") == 0)&& (strcmp(registro2, "DX") == 0))
    {
      if(mmu(contexto.DX) == 1)
      {    
       contexto.DX = dir_logica;
      }
    }
if((strcmp(registro1, "DX") == 0)&& (strcmp(registro2, "EX") == 0))
    {
      if(mmu(contexto.EX) == 1)
      {    
       contexto.DX = dir_logica;
      }
    }
if((strcmp(registro1, "DX") == 0)&& (strcmp(registro2, "FX") == 0))
    {
      if(mmu(contexto.FX) == 1)
      {    
       contexto.DX = dir_logica;
      }
    }
if((strcmp(registro1, "DX") == 0)&& (strcmp(registro2, "GX") == 0))
    {
      if(mmu(contexto.GX) == 1)
      {    
       contexto.DX = dir_logica;
      }
    }
if((strcmp(registro1, "DX") == 0)&& (strcmp(registro2, "HX") == 0))
    {
      if(mmu(contexto.HX) == 1)
      {    
       contexto.DX = dir_logica;
      }
    }


// READ_MEM EX OtroRegistro
if((strcmp(registro1, "EX") == 0)&& (strcmp(registro2, "AX") == 0))
    {
      if(mmu(contexto.AX) == 1)
      {    
       contexto.EX = dir_logica;
      }
    }
if((strcmp(registro1, "EX") == 0)&& (strcmp(registro2, "BX") == 0))
    {
      if(mmu(contexto.BX) == 1)
      {    
       contexto.EX = dir_logica;
      }
    }
if((strcmp(registro1, "EX") == 0)&& (strcmp(registro2, "CX") == 0))
    {
      if(mmu(contexto.CX) == 1)
      {    
       contexto.EX = dir_logica;
      }
    }
if((strcmp(registro1, "EX") == 0)&& (strcmp(registro2, "DX") == 0))
    {
      if(mmu(contexto.DX) == 1)
      {    
       contexto.EX = dir_logica;
      }
    }
if((strcmp(registro1, "EX") == 0)&& (strcmp(registro2, "EX") == 0))
    {
      if(mmu(contexto.EX) == 1)
      {    
       contexto.EX = dir_logica;
      }
    }
if((strcmp(registro1, "EX") == 0)&& (strcmp(registro2, "FX") == 0))
    {
      if(mmu(contexto.FX) == 1)
      {    
       contexto.EX = dir_logica;
      }
    }
if((strcmp(registro1, "EX") == 0)&& (strcmp(registro2, "GX") == 0))
    {
      if(mmu(contexto.GX) == 1)
      {    
       contexto.EX = dir_logica;
      }
    }
if((strcmp(registro1, "EX") == 0)&& (strcmp(registro2, "HX") == 0))
    {
      if(mmu(contexto.HX) == 1)
      {    
       contexto.EX = dir_logica;
      }
    }

// READ_MEM FX OtroRegistro
if((strcmp(registro1, "FX") == 0)&& (strcmp(registro2, "AX") == 0))
    {
      if(mmu(contexto.AX) == 1)
      {    
       contexto.FX = dir_logica;
      }
    }
if((strcmp(registro1, "FX") == 0)&& (strcmp(registro2, "BX") == 0))
    {
      if(mmu(contexto.BX) == 1)
      {    
       contexto.FX = dir_logica;
      }
    }
if((strcmp(registro1, "FX") == 0)&& (strcmp(registro2, "CX") == 0))
    {
      if(mmu(contexto.CX) == 1)
      {    
       contexto.FX = dir_logica;
      }
    }
if((strcmp(registro1, "FX") == 0)&& (strcmp(registro2, "DX") == 0))
    {
      if(mmu(contexto.DX) == 1)
      {    
       contexto.FX = dir_logica;
      }
    }
if((strcmp(registro1, "FX") == 0)&& (strcmp(registro2, "EX") == 0))
    {
      if(mmu(contexto.EX) == 1)
      {    
       contexto.FX = dir_logica;
      }
    }
if((strcmp(registro1, "FX") == 0)&& (strcmp(registro2, "FX") == 0))
    {
      if(mmu(contexto.FX) == 1)
      {    
       contexto.FX = dir_logica;
      }
    }
if((strcmp(registro1, "FX") == 0)&& (strcmp(registro2, "GX") == 0))
    {
      if(mmu(contexto.GX) == 1)
      {    
       contexto.FX = dir_logica;
      }
    }
if((strcmp(registro1, "FX") == 0)&& (strcmp(registro2, "HX") == 0))
    {
      if(mmu(contexto.HX) == 1)
      {    
       contexto.FX = dir_logica;
      }
    }

// READ_MEM GX OtroRegistro
if((strcmp(registro1, "GX") == 0)&& (strcmp(registro2, "AX") == 0))
    {
      if(mmu(contexto.AX) == 1)
      {    
       contexto.GX = dir_logica;
      }
    }
if((strcmp(registro1, "GX") == 0)&& (strcmp(registro2, "BX") == 0))
    {
      if(mmu(contexto.BX) == 1)
      {    
       contexto.GX = dir_logica;
      }
    }
if((strcmp(registro1, "GX") == 0)&& (strcmp(registro2, "CX") == 0))
    {
      if(mmu(contexto.CX) == 1)
      {    
       contexto.GX = dir_logica;
      }
    }
if((strcmp(registro1, "GX") == 0)&& (strcmp(registro2, "DX") == 0))
    {
      if(mmu(contexto.DX) == 1)
      {    
       contexto.GX = dir_logica;
      }
    }
if((strcmp(registro1, "GX") == 0)&& (strcmp(registro2, "EX") == 0))
    {
      if(mmu(contexto.EX) == 1)
      {    
       contexto.GX = dir_logica;
      }
    }
if((strcmp(registro1, "GX") == 0)&& (strcmp(registro2, "FX") == 0))
    {
      if(mmu(contexto.FX) == 1)
      {    
       contexto.GX = dir_logica;
      }
    }
if((strcmp(registro1, "GX") == 0)&& (strcmp(registro2, "GX") == 0))
    {
      if(mmu(contexto.GX) == 1)
      {    
       contexto.GX = dir_logica;
      }
    }
if((strcmp(registro1, "GX") == 0)&& (strcmp(registro2, "HX") == 0))
    {
      if(mmu(contexto.HX) == 1)
      {    
       contexto.GX = dir_logica;
      }
    }

// READ_MEM HX OtroRegistro
if((strcmp(registro1, "HX") == 0)&& (strcmp(registro2, "AX") == 0))
    {
      if(mmu(contexto.AX) == 1)
      {    
       contexto.HX = dir_logica;
      }
    }
if((strcmp(registro1, "HX") == 0)&& (strcmp(registro2, "BX") == 0))
    {
      if(mmu(contexto.BX) == 1)
      {    
       contexto.HX = dir_logica;
      }
    }
if((strcmp(registro1, "HX") == 0)&& (strcmp(registro2, "CX") == 0))
    {
      if(mmu(contexto.CX) == 1)
      {    
       contexto.HX = dir_logica;
      }
    }
if((strcmp(registro1, "HX") == 0)&& (strcmp(registro2, "DX") == 0))
    {
      if(mmu(contexto.DX) == 1)
      {    
       contexto.HX = dir_logica;
      }
    }
if((strcmp(registro1, "HX") == 0)&& (strcmp(registro2, "EX") == 0))
    {
      if(mmu(contexto.EX) == 1)
      {    
       contexto.HX = dir_logica;
      }
    }
if((strcmp(registro1, "HX") == 0)&& (strcmp(registro2, "FX") == 0))
    {
      if(mmu(contexto.FX) == 1)
      {    
       contexto.HX = dir_logica;
      }
    }
if((strcmp(registro1, "HX") == 0)&& (strcmp(registro2, "GX") == 0))
    {
      if(mmu(contexto.GX) == 1)
      {    
       contexto.HX = dir_logica;
      }
    }
if((strcmp(registro1, "HX") == 0)&& (strcmp(registro2, "HX") == 0))
    {
      if(mmu(contexto.HX) == 1)
      {    
       contexto.HX = dir_logica;
      }
    }

else
{
log_info(logger, "Segmentation Fault casero!!!");
/*
devolver el contexto a la memoria
devolver el tid al kernel con motivo de seg fault

actualizar_contexto(socket_memoria, pid, tid, contexto);

terminar_hilo(socket_kernel, tid);

*/

}
}
}

int mmu(int dir_logica)
{
if(( (dir_logica + contexto.Base) <= (contexto.Limite) ))
{
return 1;
log_info(logger, "Esta devolviendo 1");
}
else
{
   log_info(logger, "Esta devolviendo 0");
return 0;
}
}

void write_mem(char * registro1, char * registro2)
 {

u_int32_t dir_fisica;

 //u_int32_t dir_logica = lectura_memoria(pid, tid, dir_fisica);

      dir_fisica = 300; 

// WRITE_MEM AX OtroRegistro
if((strcmp(registro1, "AX") == 0)&& (strcmp(registro2, "AX") == 0))
    {

    int dir_fisica = mmu_dirLog_dirfis(contexto.AX);

    //////escritura_memoria(socket_memoria, pid, dir_fisica, contexto.AX);
      
    }
if((strcmp(registro1, "AX") == 0)&& (strcmp(registro2, "BX") == 0))
    {
       int dir_fisica = mmu_dirLog_dirfis(contexto.AX);

    ////escritura_memoria(socket_memoria, pid, dir_fisica, contexto.BX);
    }
if((strcmp(registro1, "AX") == 0)&& (strcmp(registro2, "CX") == 0))
    {
       int dir_fisica = mmu_dirLog_dirfis(contexto.AX);

    ////escritura_memoria(socket_memoria, pid, dir_fisica, contexto.CX);
    }
if((strcmp(registro1, "AX") == 0)&& (strcmp(registro2, "DX") == 0))
    {
      int dir_fisica = mmu_dirLog_dirfis(contexto.AX);

    ////escritura_memoria(socket_memoria, pid, dir_fisica, contexto.DX);
    }
if((strcmp(registro1, "AX") == 0)&& (strcmp(registro2, "EX") == 0))
    {
        int dir_fisica = mmu_dirLog_dirfis(contexto.AX);

    ////escritura_memoria(socket_memoria, pid, dir_fisica, contexto.EX);
    }
if((strcmp(registro1, "AX") == 0)&& (strcmp(registro2, "FX") == 0))
    {
      int dir_fisica = mmu_dirLog_dirfis(contexto.AX);

    ////escritura_memoria(socket_memoria, pid, dir_fisica, contexto.FX);
    }
if((strcmp(registro1, "AX") == 0)&& (strcmp(registro2, "GX") == 0))
    {
     int dir_fisica = mmu_dirLog_dirfis(contexto.AX);

    ////escritura_memoria(socket_memoria, pid, dir_fisica, contexto.GX);
    }
if((strcmp(registro1, "AX") == 0)&& (strcmp(registro2, "HX") == 0))
    {
   int dir_fisica = mmu_dirLog_dirfis(contexto.AX);

    ////escritura_memoria(socket_memoria, pid, dir_fisica, contexto.HX);
    }


// WRITE_MEM BX OtroRegistro
if((strcmp(registro1, "BX") == 0)&& (strcmp(registro2, "AX") == 0))
    {

    int dir_fisica = mmu_dirLog_dirfis(contexto.BX);

    ////escritura_memoria(socket_memoria, pid, dir_fisica, contexto.AX);
      
    }
if((strcmp(registro1, "BX") == 0)&& (strcmp(registro2, "BX") == 0))
    {
       int dir_fisica = mmu_dirLog_dirfis(contexto.BX);

    //escritura_memoria(socket_memoria, pid, dir_fisica, contexto.BX);
    }
if((strcmp(registro1, "BX") == 0)&& (strcmp(registro2, "CX") == 0))
    {
       int dir_fisica = mmu_dirLog_dirfis(contexto.BX);

    //escritura_memoria(socket_memoria, pid, dir_fisica, contexto.CX);
    }
if((strcmp(registro1, "BX") == 0)&& (strcmp(registro2, "DX") == 0))
    {
      int dir_fisica = mmu_dirLog_dirfis(contexto.BX);

    //escritura_memoria(socket_memoria, pid, dir_fisica, contexto.DX);
    }
if((strcmp(registro1, "BX") == 0)&& (strcmp(registro2, "EX") == 0))
    {
        int dir_fisica = mmu_dirLog_dirfis(contexto.BX);

    //escritura_memoria(socket_memoria, pid, dir_fisica, contexto.EX);
    }
if((strcmp(registro1, "BX") == 0)&& (strcmp(registro2, "FX") == 0))
    {
      int dir_fisica = mmu_dirLog_dirfis(contexto.BX);

    //escritura_memoria(socket_memoria, pid, dir_fisica, contexto.FX);
    }
if((strcmp(registro1, "BX") == 0)&& (strcmp(registro2, "GX") == 0))
    {
     int dir_fisica = mmu_dirLog_dirfis(contexto.BX);

    //escritura_memoria(socket_memoria, pid, dir_fisica, contexto.GX);
    }
if((strcmp(registro1, "BX") == 0)&& (strcmp(registro2, "HX") == 0))
    {
   int dir_fisica = mmu_dirLog_dirfis(contexto.BX);

    //escritura_memoria(socket_memoria, pid, dir_fisica, contexto.HX);
    }

// WRITE_MEM CX OtroRegistro
if((strcmp(registro1, "CX") == 0)&& (strcmp(registro2, "AX") == 0))
    {

    int dir_fisica = mmu_dirLog_dirfis(contexto.CX);

    //escritura_memoria(socket_memoria, pid, dir_fisica, contexto.AX);
      
    }
if((strcmp(registro1, "CX") == 0)&& (strcmp(registro2, "BX") == 0))
    {
       int dir_fisica = mmu_dirLog_dirfis(contexto.CX);

    //escritura_memoria(socket_memoria, pid, dir_fisica, contexto.BX);
    }
if((strcmp(registro1, "CX") == 0)&& (strcmp(registro2, "CX") == 0))
    {
       int dir_fisica = mmu_dirLog_dirfis(contexto.CX);

    //escritura_memoria(socket_memoria, pid, dir_fisica, contexto.CX);
    }
if((strcmp(registro1, "CX") == 0)&& (strcmp(registro2, "DX") == 0))
    {
      int dir_fisica = mmu_dirLog_dirfis(contexto.CX);

    //escritura_memoria(socket_memoria, pid, dir_fisica, contexto.DX);
    }
if((strcmp(registro1, "CX") == 0)&& (strcmp(registro2, "EX") == 0))
    {
        int dir_fisica = mmu_dirLog_dirfis(contexto.CX);

    //escritura_memoria(socket_memoria, pid, dir_fisica, contexto.EX);
    }
if((strcmp(registro1, "CX") == 0)&& (strcmp(registro2, "FX") == 0))
    {
      int dir_fisica = mmu_dirLog_dirfis(contexto.CX);

    //escritura_memoria(socket_memoria, pid, dir_fisica, contexto.FX);
    }
if((strcmp(registro1, "CX") == 0)&& (strcmp(registro2, "GX") == 0))
    {
     int dir_fisica = mmu_dirLog_dirfis(contexto.CX);

    //escritura_memoria(socket_memoria, pid, dir_fisica, contexto.GX);
    }
if((strcmp(registro1, "CX") == 0)&& (strcmp(registro2, "HX") == 0))
    {
   int dir_fisica = mmu_dirLog_dirfis(contexto.CX);

    //escritura_memoria(socket_memoria, pid, dir_fisica, contexto.HX);
    }

// WRITE_MEM DX OtroRegistro
if((strcmp(registro1, "DX") == 0)&& (strcmp(registro2, "AX") == 0))
    {

    int dir_fisica = mmu_dirLog_dirfis(contexto.DX);

    //escritura_memoria(socket_memoria, pid, dir_fisica, contexto.AX);
      
    }
if((strcmp(registro1, "DX") == 0)&& (strcmp(registro2, "BX") == 0))
    {
       int dir_fisica = mmu_dirLog_dirfis(contexto.DX);

    //escritura_memoria(socket_memoria, pid, dir_fisica, contexto.BX);
    }
if((strcmp(registro1, "DX") == 0)&& (strcmp(registro2, "CX") == 0))
    {
       int dir_fisica = mmu_dirLog_dirfis(contexto.DX);

    //escritura_memoria(socket_memoria, pid, dir_fisica, contexto.CX);
    }
if((strcmp(registro1, "DX") == 0)&& (strcmp(registro2, "DX") == 0))
    {
      int dir_fisica = mmu_dirLog_dirfis(contexto.DX);

    //escritura_memoria(socket_memoria, pid, dir_fisica, contexto.DX);
    }
if((strcmp(registro1, "DX") == 0)&& (strcmp(registro2, "EX") == 0))
    {
        int dir_fisica = mmu_dirLog_dirfis(contexto.DX);

    //escritura_memoria(socket_memoria, pid, dir_fisica, contexto.EX);
    }
if((strcmp(registro1, "DX") == 0)&& (strcmp(registro2, "FX") == 0))
    {
      int dir_fisica = mmu_dirLog_dirfis(contexto.DX);

    //escritura_memoria(socket_memoria, pid, dir_fisica, contexto.FX);
    }
if((strcmp(registro1, "DX") == 0)&& (strcmp(registro2, "GX") == 0))
    {
     int dir_fisica = mmu_dirLog_dirfis(contexto.DX);

    //escritura_memoria(socket_memoria, pid, dir_fisica, contexto.GX);
    }
if((strcmp(registro1, "DX") == 0)&& (strcmp(registro2, "HX") == 0))
    {
   int dir_fisica = mmu_dirLog_dirfis(contexto.DX);

    //escritura_memoria(socket_memoria, pid, dir_fisica, contexto.HX);
    }

// WRITE_MEM EX OtroRegistro
if((strcmp(registro1, "EX") == 0)&& (strcmp(registro2, "AX") == 0))
    {

    int dir_fisica = mmu_dirLog_dirfis(contexto.EX);

    //escritura_memoria(socket_memoria, pid, dir_fisica, contexto.AX);
      
    }
if((strcmp(registro1, "EX") == 0)&& (strcmp(registro2, "BX") == 0))
    {
       int dir_fisica = mmu_dirLog_dirfis(contexto.EX);

    //escritura_memoria(socket_memoria, pid, dir_fisica, contexto.BX);
    }
if((strcmp(registro1, "EX") == 0)&& (strcmp(registro2, "CX") == 0))
    {
       int dir_fisica = mmu_dirLog_dirfis(contexto.EX);

    //escritura_memoria(socket_memoria, pid, dir_fisica, contexto.CX);
    }
if((strcmp(registro1, "EX") == 0)&& (strcmp(registro2, "DX") == 0))
    {
      int dir_fisica = mmu_dirLog_dirfis(contexto.EX);

    //escritura_memoria(socket_memoria, pid, dir_fisica, contexto.DX);
    }
if((strcmp(registro1, "EX") == 0)&& (strcmp(registro2, "EX") == 0))
    {
        int dir_fisica = mmu_dirLog_dirfis(contexto.EX);

    //escritura_memoria(socket_memoria, pid, dir_fisica, contexto.EX);
    }
if((strcmp(registro1, "EX") == 0)&& (strcmp(registro2, "FX") == 0))
    {
      int dir_fisica = mmu_dirLog_dirfis(contexto.AX);

    //escritura_memoria(socket_memoria, pid, dir_fisica, contexto.FX);
    }
if((strcmp(registro1, "EX") == 0)&& (strcmp(registro2, "GX") == 0))
    {
     int dir_fisica = mmu_dirLog_dirfis(contexto.EX);

    //escritura_memoria(socket_memoria, pid, dir_fisica, contexto.GX);
    }
if((strcmp(registro1, "EX") == 0)&& (strcmp(registro2, "HX") == 0))
    {
   int dir_fisica = mmu_dirLog_dirfis(contexto.EX);

    //escritura_memoria(socket_memoria, pid, dir_fisica, contexto.HX);
    }

// WRITE_MEM FX OtroRegistro
if((strcmp(registro1, "FX") == 0)&& (strcmp(registro2, "FX") == 0))
    {
    int dir_fisica = mmu_dirLog_dirfis(contexto.FX);

    //escritura_memoria(socket_memoria, pid, dir_fisica, contexto.AX);
      
    }
if((strcmp(registro1, "FX") == 0)&& (strcmp(registro2, "BX") == 0))
    {
       int dir_fisica = mmu_dirLog_dirfis(contexto.FX);

    //escritura_memoria(socket_memoria, pid, dir_fisica, contexto.BX);
    }
if((strcmp(registro1, "FX") == 0)&& (strcmp(registro2, "CX") == 0))
    {
       int dir_fisica = mmu_dirLog_dirfis(contexto.FX);

    //escritura_memoria(socket_memoria, pid, dir_fisica, contexto.CX);
    }
if((strcmp(registro1, "FX") == 0)&& (strcmp(registro2, "DX") == 0))
    {
      int dir_fisica = mmu_dirLog_dirfis(contexto.FX);

    //escritura_memoria(socket_memoria, pid, dir_fisica, contexto.DX);
    }
if((strcmp(registro1, "FX") == 0)&& (strcmp(registro2, "EX") == 0))
    {
        int dir_fisica = mmu_dirLog_dirfis(contexto.FX);

    //escritura_memoria(socket_memoria, pid, dir_fisica, contexto.EX);
    }
if((strcmp(registro1, "FX") == 0)&& (strcmp(registro2, "FX") == 0))
    {
      int dir_fisica = mmu_dirLog_dirfis(contexto.FX);

    //escritura_memoria(socket_memoria, pid, dir_fisica, contexto.FX);
    }
if((strcmp(registro1, "FX") == 0)&& (strcmp(registro2, "GX") == 0))
    {
     int dir_fisica = mmu_dirLog_dirfis(contexto.FX);

    //escritura_memoria(socket_memoria, pid, dir_fisica, contexto.GX);
    }
if((strcmp(registro1, "FX") == 0)&& (strcmp(registro2, "HX") == 0))
    {
   int dir_fisica = mmu_dirLog_dirfis(contexto.FX);

    //escritura_memoria(socket_memoria, pid, dir_fisica, contexto.HX);
    }

// WRITE_MEM GX OtroRegistro
if((strcmp(registro1, "GX") == 0)&& (strcmp(registro2, "AX") == 0))
    {

    int dir_fisica = mmu_dirLog_dirfis(contexto.GX);

    //escritura_memoria(socket_memoria, pid, dir_fisica, contexto.AX);
      
    }
if((strcmp(registro1, "GX") == 0)&& (strcmp(registro2, "BX") == 0))
    {
       int dir_fisica = mmu_dirLog_dirfis(contexto.GX);

    //escritura_memoria(socket_memoria, pid, dir_fisica, contexto.BX);
    }
if((strcmp(registro1, "GX") == 0)&& (strcmp(registro2, "CX") == 0))
    {
       int dir_fisica = mmu_dirLog_dirfis(contexto.GX);

    //escritura_memoria(socket_memoria, pid, dir_fisica, contexto.CX);
    }
if((strcmp(registro1, "GX") == 0)&& (strcmp(registro2, "DX") == 0))
    {
      int dir_fisica = mmu_dirLog_dirfis(contexto.GX);

    //escritura_memoria(socket_memoria, pid, dir_fisica, contexto.DX);
    }
if((strcmp(registro1, "GX") == 0)&& (strcmp(registro2, "EX") == 0))
    {
        int dir_fisica = mmu_dirLog_dirfis(contexto.GX);

    //escritura_memoria(socket_memoria, pid, dir_fisica, contexto.EX);
    }
if((strcmp(registro1, "GX") == 0)&& (strcmp(registro2, "FX") == 0))
    {
      int dir_fisica = mmu_dirLog_dirfis(contexto.GX);

    //escritura_memoria(socket_memoria, pid, dir_fisica, contexto.FX);
    }
if((strcmp(registro1, "GX") == 0)&& (strcmp(registro2, "GX") == 0))
    {
     int dir_fisica = mmu_dirLog_dirfis(contexto.GX);

    //escritura_memoria(socket_memoria, pid, dir_fisica, contexto.GX);
    }
if((strcmp(registro1, "GX") == 0)&& (strcmp(registro2, "HX") == 0))
    {
   int dir_fisica = mmu_dirLog_dirfis(contexto.GX);

    //escritura_memoria(socket_memoria, pid, dir_fisica, contexto.HX);
    }

// WRITE_MEM HX OtroRegistro
if((strcmp(registro1, "HX") == 0)&& (strcmp(registro2, "AX") == 0))
    {

    int dir_fisica = mmu_dirLog_dirfis(contexto.HX);

    //escritura_memoria(socket_memoria, pid, dir_fisica, contexto.AX);
      
    }
if((strcmp(registro1, "HX") == 0)&& (strcmp(registro2, "BX") == 0))
    {
       int dir_fisica = mmu_dirLog_dirfis(contexto.HX);

    //escritura_memoria(socket_memoria, pid, dir_fisica, contexto.BX);
    }
if((strcmp(registro1, "HX") == 0)&& (strcmp(registro2, "CX") == 0))
    {
       int dir_fisica = mmu_dirLog_dirfis(contexto.HX);

    //escritura_memoria(socket_memoria, pid, dir_fisica, contexto.CX);
    }
if((strcmp(registro1, "HX") == 0)&& (strcmp(registro2, "DX") == 0))
    {
      int dir_fisica = mmu_dirLog_dirfis(contexto.HX);

    //escritura_memoria(socket_memoria, pid, dir_fisica, contexto.DX);
    }
if((strcmp(registro1, "HX") == 0)&& (strcmp(registro2, "EX") == 0))
    {
        int dir_fisica = mmu_dirLog_dirfis(contexto.HX);

    //escritura_memoria(socket_memoria, pid, dir_fisica, contexto.EX);
    }
if((strcmp(registro1, "HX") == 0)&& (strcmp(registro2, "FX") == 0))
    {
      int dir_fisica = mmu_dirLog_dirfis(contexto.HX);

    //escritura_memoria(socket_memoria, pid, dir_fisica, contexto.FX);
    }
if((strcmp(registro1, "HX") == 0)&& (strcmp(registro2, "GX") == 0))
    {
     int dir_fisica = mmu_dirLog_dirfis(contexto.HX);

    //escritura_memoria(socket_memoria, pid, dir_fisica, contexto.GX);
    }
if((strcmp(registro1, "HX") == 0)&& (strcmp(registro2, "HX") == 0))
    {
   int dir_fisica = mmu_dirLog_dirfis(contexto.HX);

    //escritura_memoria(socket_memoria, pid, dir_fisica, contexto.HX);
    }
 }

 int mmu_dirLog_dirfis(int dir_logica)
{
return (dir_logica + contexto.Base);
}

void jnz_pc(char* registro, char* valor)
{
if(strcmp(registro, "PC") == 0)
    {
       if(contexto.PC != 0)
       {
       contexto.PC = atoi(valor);
       }  
    }
if(strcmp(registro, "AX") == 0)
    {
       if(contexto.AX != 0)
       {
       contexto.PC = atoi(valor);
       }
    } 
if(strcmp(registro, "BX") == 0)
    {
       if(contexto.BX != 0)
          {
       contexto.PC = atoi(valor);
       }
    } 
if(strcmp(registro, "CX") == 0)
    {
       if(contexto.CX != 0)
          {
       contexto.PC = atoi(valor);
       }
    } 
if(strcmp(registro, "DX") == 0)
    {
       if(contexto.DX != 0)
          {
       contexto.PC = atoi(valor);
       }
    } 
if(strcmp(registro, "EX") == 0)
    {
       if(contexto.EX != 0)
          {
       contexto.PC = atoi(valor);
       }
    } 
if(strcmp(registro, "FX") == 0)
    {
       if(contexto.FX != 0)
          {
       contexto.PC = atoi(valor);
       }
    } 
if(strcmp(registro, "GX") == 0)
    {
       if(contexto.GX != 0)
          {
       contexto.PC = atoi(valor);
       }
    } 
if(strcmp(registro, "HX") == 0)
    {
       if(contexto.HX != 0)
          {
       contexto.PC = atoi(valor);
       }
    } 
if(strcmp(registro, "Base") == 0)
    {
       if(contexto.Base != 0)
          {
       contexto.PC = atoi(valor);
       }
    } 
if(strcmp(registro, "Limite") == 0)
    {
       if(contexto.Limite != 0)
          {
       contexto.PC = atoi(valor);
       }
    } 
}
uint32_t obtener_registro(char* registro)
{
     if(strcmp(registro, "PC") == 0 ){
          return contexto.PC;
     }
     if(strcmp(registro, "AX") == 0 ){
          return contexto.AX;
     }
     if(strcmp(registro, "BX") == 0 ){
          return contexto.BX;
     }
     if(strcmp(registro, "CX") == 0 ){
          return contexto.CX;
     }
     if(strcmp(registro, "DX") == 0 ){
          return contexto.DX;
     }
     if(strcmp(registro, "EX") == 0 ){
          return contexto.EX;
     }
     if(strcmp(registro, "FX") == 0 ){
          return contexto.FX;
     }
     if(strcmp(registro, "GX") == 0 ){
          return contexto.GX;
     }
     if(strcmp(registro, "HX") == 0 ){
          return contexto.HX;
     }
      if(strcmp(registro, "Base") == 0 ){
          return contexto.Base;
     }
     if(strcmp(registro, "Limite") == 0 ){
          return contexto.Limite;
     }

}


 char* pedir_proxima_instruccion(int servidor_memoria, t_buffer* buffer_pedido_devolver_instruccion)
 {
     // Empaquetamos y serializamos los datos junto con el código de operación
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = OPERACION_DEVOLVER_INSTRUCCION;
    paquete->buffer = buffer_pedido_devolver_instruccion;
    t_buffer* paquete_serializado = serializar_paquete(paquete);

    send(servidor_memoria, paquete_serializado->stream, paquete_serializado->size, 0);

    buffer_destroy(paquete_serializado);
    eliminar_paquete(paquete);
    recibir_operacion(servidor_memoria);   

    u_int32_t tamaño_buffer;

    t_buffer* instruccion_paquete_recibido = recibir_buffer(&tamaño_buffer, socket_memoria);

    t_datos_devolver_instruccion* datos = deserializar_datos_devolver_instruccion(instruccion_paquete_recibido);

     return datos->instruccion;
 }

