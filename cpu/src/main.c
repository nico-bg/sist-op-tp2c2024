#include <main.h>
#include <pthread.h>
#include <semaphore.h>

t_config *config;
t_log *logger;
int socket_memoria;
int socket_dispatch;
int socket_interrupt;
bool hay_interrupcion;

sem_t sem_ciclo_de_instruccion;
sem_t sem_fin_ciclo_de_instruccion;

pthread_mutex_t mutex_interrupciones;
pthread_mutex_t mutex_socket_memoria;

int main(int argc, char *argv[])
{

     char *ip_memoria;
     char *puerto_memoria;
     char *puerto_escucha_dispatch;
     char *puerto_escucha_interrupt;

     iniciar_semaforos();

     config = iniciar_config("cpu.config");
     logger = iniciar_logger(config, "cpu.log", "CPU");

     ip_memoria = config_get_string_value(config, "IP_MEMORIA");
     puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
     puerto_escucha_dispatch = config_get_string_value(config, "PUERTO_ESCUCHA_DISPATCH");
     puerto_escucha_interrupt = config_get_string_value(config, "PUERTO_ESCUCHA_INTERRUPT");

     socket_memoria = conectar_a_socket(ip_memoria, puerto_memoria);

     log_info(logger, "Conectado a Memoria");

     int fd_dispatch = iniciar_servidor(puerto_escucha_dispatch);
     int fd_interrupt = iniciar_servidor(puerto_escucha_interrupt);

     /* Esperamos a que se conecte el Kernel por el puerto dispatch */
     socket_dispatch = esperar_cliente(fd_dispatch);

     /* Esperamos a que se conecte el Kernel por el puerto interrupt */
     socket_interrupt = esperar_cliente(fd_interrupt);

     pthread_t thread_dispatch;
     pthread_t thread_ciclo_de_instruccion;
     pthread_t thread_interrupt;

     pthread_create(&thread_dispatch, NULL, escuchar_dispatch, NULL);
     pthread_create(&thread_ciclo_de_instruccion, NULL, ciclo_de_instruccion, NULL);
     pthread_create(&thread_interrupt, NULL, escuchar_interrupciones, NULL);

     pthread_join(thread_dispatch, NULL);
     pthread_join(thread_ciclo_de_instruccion, NULL);
     pthread_join(thread_interrupt, NULL);


     terminar_programa();

     return 0;
}

void iniciar_semaforos (){
    sem_init(&sem_ciclo_de_instruccion, 0, 0);
    sem_init(&sem_fin_ciclo_de_instruccion, 0, 1);
    pthread_mutex_init(&mutex_interrupciones, NULL);
}

void escuchar_dispatch()
{
     log_info(logger, "Hilo escuchar_dispatch esperando pid y tid del kernel");

     t_buffer *buffer;
     uint32_t size;

     while (1)
     {
          op_code cod_op = recibir_operacion(socket_dispatch);

          switch (cod_op)
          {
          case OPERACION_EJECUTAR_HILO:
               sem_wait(&sem_fin_ciclo_de_instruccion);
               buffer = recibir_buffer(&size, socket_dispatch);

               pcb = deserializar_hilo_a_cpu(buffer);

               pthread_mutex_lock(&mutex_socket_memoria);
               t_buffer *contexto_devuelto = pedir_contexto(socket_memoria, buffer);
               pthread_mutex_unlock(&mutex_socket_memoria);

               contexto = deserializar_datos_contexto(contexto_devuelto);

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

void ciclo_de_instruccion()
{

     while (true)
     {
          sem_wait(&sem_ciclo_de_instruccion);

          // Fetch
          
          log_info(logger, " ## TID: %d  - FETCH - Program Counter: %d", contexto.tid, contexto.PC);
    

          t_datos_obtener_instruccion *datos = malloc(sizeof(t_datos_obtener_instruccion));

          datos->PC = contexto.PC;
          datos->pid = contexto.pid;
          datos->tid = contexto.tid;

          t_buffer *buffer_pedido_instruccion = serializar_datos_solicitar_instruccion(datos);

          char *instruccion = pedir_proxima_instruccion(socket_memoria, buffer_pedido_instruccion);

          if (strlen(instruccion) == 0)
          {
               abort();
          }


          // Decode

          char **estructura_instruccion = string_split(instruccion, " ");

          if (strcmp(estructura_instruccion[0], "SET") == 0)
          {     
               log_info(logger, " ## TID: %d  - Ejecutando: %s - Parametros: %s %d ", contexto.tid, estructura_instruccion[0], estructura_instruccion[1], atoi(estructura_instruccion[2]));

               setear_registro(estructura_instruccion[1], atoi(estructura_instruccion[2]));
               incrementar_pc();

               sem_post(&sem_ciclo_de_instruccion);
          }

          if (strcmp(estructura_instruccion[0], "SUM") == 0)
          {
               log_info(logger, " ## TID: %d  - Ejecutando: %s - Parametros: %s %s ", contexto.tid, estructura_instruccion[0], estructura_instruccion[1], estructura_instruccion[2]);

               sum_registro(estructura_instruccion[1], estructura_instruccion[2]);
               incrementar_pc();

               sem_post(&sem_ciclo_de_instruccion);
          }

          if (strcmp(estructura_instruccion[0], "SUB") == 0)
          {
               log_info(logger, " ## TID: %d  - Ejecutando: %s - Parametros: %s %s ", contexto.tid, estructura_instruccion[0], estructura_instruccion[1], estructura_instruccion[2]);
               
               sub_registro(estructura_instruccion[1], estructura_instruccion[2]);
               incrementar_pc();

               sem_post(&sem_ciclo_de_instruccion);
          }

          if (strcmp(estructura_instruccion[0], "READ_MEM") == 0)
          {
        
               log_info(logger, " ## TID: %d  - Ejecutando: %s - Parametros: %s %s ", contexto.tid, estructura_instruccion[0], estructura_instruccion[1], estructura_instruccion[2]);

               read_mem(estructura_instruccion[1], estructura_instruccion[2]);
               incrementar_pc();

               sem_post(&sem_ciclo_de_instruccion);
          }

          if (strcmp(estructura_instruccion[0], "WRITE_MEM") == 0)
          {
               log_info(logger, " ## TID: %d  - Ejecutando: %s - Parametros: %s %s ", contexto.tid, estructura_instruccion[0], estructura_instruccion[1], estructura_instruccion[2]);
               
               write_mem(estructura_instruccion[1], estructura_instruccion[2]);
               incrementar_pc();

               sem_post(&sem_ciclo_de_instruccion);
          }

          if (strcmp(estructura_instruccion[0], "JNZ") == 0)
          {

               log_info(logger, " ## TID: %d  - Ejecutando: %s - Parametros: %s %d ", contexto.tid, estructura_instruccion[0], estructura_instruccion[1], atoi(estructura_instruccion[2]));

               jnz_pc(estructura_instruccion[1], estructura_instruccion[2]);

               sem_post(&sem_ciclo_de_instruccion);
          }

          if (strcmp(estructura_instruccion[0], "LOG") == 0)

          {

               log_info(logger, " ## TID: %d  - Ejecutando: %s - Parametros: %s", contexto.tid, estructura_instruccion[0], estructura_instruccion[1]);

               log_info(logger, "El valor leido por instruccion LOG es: %d", obtener_registro(estructura_instruccion[1]));
               incrementar_pc();

               sem_post(&sem_ciclo_de_instruccion);
          }

          if (strcmp(estructura_instruccion[0], "MUTEX_CREATE") == 0)
          {
               log_info(logger, " ## TID: %d  - Ejecutando: %s - Parametros: %s ", contexto.tid, estructura_instruccion[0], estructura_instruccion[1]);

               incrementar_pc();
               actualizar_contexto();
               ejecutar_instruccion_mutex(OPERACION_CREAR_MUTEX, estructura_instruccion[1]);
          }

          if (strcmp(estructura_instruccion[0], "MUTEX_LOCK") == 0)
          {
               log_info(logger, " ## TID: %d  - Ejecutando: %s - Parametros: %s ", contexto.tid, estructura_instruccion[0], estructura_instruccion[1]);

               incrementar_pc();
               actualizar_contexto();
               ejecutar_instruccion_mutex(OPERACION_BLOQUEAR_MUTEX, estructura_instruccion[1]);
          }

          if (strcmp(estructura_instruccion[0], "MUTEX_UNLOCK") == 0)
          {
               log_info(logger, " ## TID: %d  - Ejecutando: %s - Parametros: %s ", contexto.tid, estructura_instruccion[0], estructura_instruccion[1]);

               incrementar_pc();
               actualizar_contexto();
               ejecutar_instruccion_mutex(OPERACION_DESBLOQUEAR_MUTEX, estructura_instruccion[1]);
          }

          if (strcmp(estructura_instruccion[0], "DUMP_MEMORY") == 0)
          {
               log_info(logger, " ## TID: %d  - Ejecutando: %s - Parametros:  ", contexto.tid, estructura_instruccion[0]);

               incrementar_pc();
               actualizar_contexto();
               enviar_operacion_a_kernel(OPERACION_DUMP_MEMORY);
          }

          if (strcmp(estructura_instruccion[0], "IO") == 0)
          {
               log_info(logger, " ## TID: %d  - Ejecutando: %s - Parametros: %d ", contexto.tid, estructura_instruccion[0], atoi(estructura_instruccion[1]));

               incrementar_pc();
               actualizar_contexto();

               t_datos_operacion_io* datos = malloc(sizeof(t_datos_operacion_io));
               datos->tiempo = atoi(estructura_instruccion[1]);

               t_paquete* paquete = malloc(sizeof(t_paquete));
               paquete->codigo_operacion = OPERACION_IO;
               paquete->buffer = serializar_datos_operacion_io(datos);
               t_buffer* paquete_serializado = serializar_paquete(paquete);

               send(socket_dispatch, paquete_serializado->stream, paquete_serializado->size, 0);

               buffer_destroy(paquete_serializado);
               eliminar_paquete(paquete);
               destruir_datos_operacion_io(datos);
          }

          if (strcmp(estructura_instruccion[0], "PROCESS_CREATE") == 0)
          {
               log_info(logger, " ## TID: %d  - Ejecutando: %s - Parametros: %s %d %d ", contexto.tid, estructura_instruccion[0], estructura_instruccion[1], atoi(estructura_instruccion[2]), atoi(estructura_instruccion[3]));

               incrementar_pc();
               actualizar_contexto();

               t_datos_crear_proceso* datos = malloc(sizeof(t_datos_crear_proceso));
               datos->archivo_pseudocodigo = string_duplicate(estructura_instruccion[1]);
               datos->tamanio_proceso = atoi(estructura_instruccion[2]);
               datos->prioridad = atoi(estructura_instruccion[3]);

               t_paquete* paquete = malloc(sizeof(t_paquete));
               paquete->codigo_operacion = OPERACION_CREAR_PROCESO;
               paquete->buffer = serializar_datos_crear_proceso(datos);
               t_buffer* paquete_serializado = serializar_paquete(paquete);

               send(socket_dispatch, paquete_serializado->stream, paquete_serializado->size, 0);

               buffer_destroy(paquete_serializado);
               eliminar_paquete(paquete);
               destruir_datos_crear_proceso(datos);
          }

          if (strcmp(estructura_instruccion[0], "THREAD_CREATE") == 0)
          {
               log_info(logger, " ## TID: %d  - Ejecutando: %s - Parametros: %s %d", contexto.tid, estructura_instruccion[0], estructura_instruccion[1], atoi(estructura_instruccion[2]));

               incrementar_pc();
               actualizar_contexto();

               t_datos_crear_hilo* datos = malloc(sizeof(t_datos_crear_hilo));
               datos->archivo_pseudocodigo = string_duplicate(estructura_instruccion[1]);
               datos->prioridad = atoi(estructura_instruccion[2]);

               t_paquete* paquete = malloc(sizeof(t_paquete));
               paquete->codigo_operacion = OPERACION_CREAR_HILO;
               paquete->buffer = serializar_datos_crear_hilo(datos);
               t_buffer* paquete_serializado = serializar_paquete(paquete);

               send(socket_dispatch, paquete_serializado->stream, paquete_serializado->size, 0);

               buffer_destroy(paquete_serializado);
               eliminar_paquete(paquete);
               destruir_datos_crear_hilo(datos);
          }

          if (strcmp(estructura_instruccion[0], "THREAD_CANCEL") == 0)
          {
               log_info(logger, " ## TID: %d  - Ejecutando: %s - Parametros: %d  ", contexto.tid, estructura_instruccion[0], atoi(estructura_instruccion[1]));

               incrementar_pc();
               actualizar_contexto();
               ejecutar_instruccion_hilo(OPERACION_CANCELAR_HILO, atoi(estructura_instruccion[1]));
          }

          if (strcmp(estructura_instruccion[0], "THREAD_JOIN") == 0)
          {
               log_info(logger, " ## TID: %d  - Ejecutando: %s - Parametros: %d  ", contexto.tid, estructura_instruccion[0], atoi(estructura_instruccion[1]));

               incrementar_pc();
               actualizar_contexto();
               ejecutar_instruccion_hilo(OPERACION_ESPERAR_HILO, atoi(estructura_instruccion[1]));
          }

          if (strcmp(estructura_instruccion[0], "THREAD_EXIT") == 0)
          {
               log_info(logger, " ## TID: %d  - Ejecutando: %s - Parametros:  ", contexto.tid, estructura_instruccion[0]);

               incrementar_pc();
               actualizar_contexto();
               enviar_operacion_a_kernel(OPERACION_FINALIZAR_HILO);
          }

          if (strcmp(estructura_instruccion[0], "PROCESS_EXIT") == 0)
          {
               log_info(logger, " ## TID: %d  - Ejecutando: %s - Parametros:  ", contexto.tid, estructura_instruccion[0]);

               incrementar_pc();
               actualizar_contexto();
               enviar_operacion_a_kernel(OPERACION_FINALIZAR_PROCESO);
          }

     // CHECK INTERRUPT
     pthread_mutex_lock(&mutex_interrupciones);
     bool es_necesario_interrupir = hay_interrupcion;
     hay_interrupcion = false;
     pthread_mutex_unlock(&mutex_interrupciones);

     if(es_necesario_interrupir) {
        
          log_info(logger, "## Llega interrupción al puerto Interrupt");

          actualizar_contexto();

          // Notificamos al Kernel que ya desalojamos el hilo
          t_buffer* buffer_interrupcion = buffer_create(sizeof(uint32_t));
          buffer_add_uint32(buffer_interrupcion, OPERACION_DESALOJAR_HILO);

          send(socket_dispatch, buffer_interrupcion->stream, buffer_interrupcion->size, 0);

          buffer_destroy(buffer_interrupcion);

          // Si se supone que se debe seguir ejecutando la siguiente instrucción (semáforo con valor 1)
          // ...decrementamos el semáforo para que se bloquee al inicio del while
          // Si el semáforo se iba a bloquear al inicio del while (semáforo con valor 0), no hace nada
          // ...continúa ejecutando para que se bloquee como estaba previsto
          sem_trywait(&sem_ciclo_de_instruccion);
     }

          sem_post(&sem_fin_ciclo_de_instruccion);
     }
}

t_buffer *pedir_contexto(int servidor_memoria, t_buffer *buffer_pedido_contexto)
{
     // Empaquetamos y serializamos los datos junto con el código de operación
     t_paquete *paquete = malloc(sizeof(t_paquete));
     paquete->codigo_operacion = OPERACION_DEVOLVER_CONTEXTO_EJECUCION;
     paquete->buffer = buffer_pedido_contexto;
     t_buffer *paquete_serializado = serializar_paquete(paquete);

     log_info(logger, "#TID: %d  - Solicito Contexto Ejecución", pcb->tid);

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

void setear_registro(char *registro, uint32_t valor)
{
     if (strcmp(registro, "PC") == 0)
     {

          contexto.PC = valor;

     }

     if (strcmp(registro, "AX") == 0)
     {
          contexto.AX = valor;
     }

     if (strcmp(registro, "BX") == 0)
     {
          contexto.BX = valor;
     }

     if (strcmp(registro, "CX") == 0)
     {
          contexto.CX = valor;
     }

     if (strcmp(registro, "DX") == 0)
     {
          contexto.DX = valor;
     }

     if (strcmp(registro, "EX") == 0)
     {
          contexto.EX = valor;
     }

     if (strcmp(registro, "FX") == 0)
     {
          contexto.FX = valor;
     }

     if (strcmp(registro, "GX") == 0)
     {
          contexto.GX = valor;
     }

     if (strcmp(registro, "HX") == 0)
     {
          contexto.HX = valor;
     }

     if (strcmp(registro, "Base") == 0)
     {
          contexto.Base = valor;
     }

     if (strcmp(registro, "Limite") == 0)
     {
          contexto.Limite = valor;
     }
}

void sum_registro(char *registro1, char *registro2)
{
     uint32_t valor_registro1 = obtener_registro(registro1);
     uint32_t valor_registro2 = obtener_registro(registro2);

     setear_registro(registro1, valor_registro1 + valor_registro2);
}

void sub_registro(char *registro1, char *registro2)
{
     uint32_t valor_registro1 = obtener_registro(registro1);
     uint32_t valor_registro2 = obtener_registro(registro2);

     setear_registro(registro1, valor_registro1 - valor_registro2);
}


void read_mem(char *registro1, char *registro2)
{
     int dir_logica = 300;
     // READ_MEM AX OtroRegistro
     uint32_t valor_registro2 = obtener_registro(registro2);

     if(mmu(valor_registro2) == 1) {
          int dir_fisica = mmu_dirLog_dirfis(valor_registro2);

          uint32_t valor_leido = lectura_memoria(dir_fisica);

          setear_registro(registro1, valor_leido);
     }
     else
     {
          log_info(logger, "Segmentation Fault casero!!!");


          log_info(logger, "#TID: %d  - Actualizo Contexto Ejecución", contexto.tid);


          /*
          devolver el contexto a la memoria
          devolver el tid al kernel con motivo de seg fault

          actualizar_contexto(socket_memoria, pid, tid, contexto);

          terminar_hilo(socket_kernel, tid);

          */
     }
}

int mmu(int dir_logica)
{
     if (((dir_logica + contexto.Base) <= (contexto.Limite)))
     {
          return 1;
 
     }
     else
     {
          return 0;
     }
}

void write_mem(char *registro1, char *registro2)
{
     uint32_t valor_registro1 = obtener_registro(registro1);
     int dir_fisica = mmu_dirLog_dirfis(valor_registro1);


     log_info(logger, " ## TID: %d - Accion: ESCRITURA - Dirección Fisica: %d", contexto.tid, dir_fisica);

     //////escritura_memoria(socket_memoria, pid, dir_fisica, contexto.AX);
}

int mmu_dirLog_dirfis(int dir_logica)
{
     return (dir_logica + contexto.Base);
}

void jnz_pc(char *registro, char *valor)
{
     uint32_t valor_registro = obtener_registro(registro);

     if(valor_registro != 0) {
          contexto.PC = atoi(valor);
     } else {
          incrementar_pc();
     }
}
uint32_t obtener_registro(char *registro)
{
     if (strcmp(registro, "PC") == 0)
     {
          return contexto.PC;
     }
     if (strcmp(registro, "AX") == 0)
     {
          return contexto.AX;
     }
     if (strcmp(registro, "BX") == 0)
     {
          return contexto.BX;
     }
     if (strcmp(registro, "CX") == 0)
     {
          return contexto.CX;
     }
     if (strcmp(registro, "DX") == 0)
     {
          return contexto.DX;
     }
     if (strcmp(registro, "EX") == 0)
     {
          return contexto.EX;
     }
     if (strcmp(registro, "FX") == 0)
     {
          return contexto.FX;
     }
     if (strcmp(registro, "GX") == 0)
     {
          return contexto.GX;
     }
     if (strcmp(registro, "HX") == 0)
     {
          return contexto.HX;
     }
     if (strcmp(registro, "Base") == 0)
     {
          return contexto.Base;
     }
     if (strcmp(registro, "Limite") == 0)
     {
          return contexto.Limite;
     }
}

char *pedir_proxima_instruccion(int servidor_memoria, t_buffer *buffer_pedido_devolver_instruccion)
{
     // Empaquetamos y serializamos los datos junto con el código de operación
     t_paquete *paquete = malloc(sizeof(t_paquete));
     paquete->codigo_operacion = OPERACION_DEVOLVER_INSTRUCCION;
     paquete->buffer = buffer_pedido_devolver_instruccion;
     t_buffer *paquete_serializado = serializar_paquete(paquete);

     send(servidor_memoria, paquete_serializado->stream, paquete_serializado->size, 0);

     buffer_destroy(paquete_serializado);
     eliminar_paquete(paquete);
     recibir_operacion(servidor_memoria);

     u_int32_t tamaño_buffer;

     t_buffer *instruccion_paquete_recibido = recibir_buffer(&tamaño_buffer, socket_memoria);

     t_datos_devolver_instruccion *datos = deserializar_datos_devolver_instruccion(instruccion_paquete_recibido);

     return datos->instruccion;
}

u_int32_t lectura_memoria(u_int32_t dir_fisica)
{
     // Empaquetamos y serializamos los datos junto con el código de operación
     t_paquete *paquete = malloc(sizeof(t_paquete));

     t_datos_leer_memoria *datos = malloc(sizeof(t_datos_leer_memoria));
     
     datos->pid = contexto.pid;
     datos->tid = contexto.tid;
     datos->dir_fisica = dir_fisica;
     datos->tamanio = sizeof(u_int32_t);

     t_buffer *buffer_pedido_leer_memoria = serializar_datos_solicitar_instruccion(datos);

     paquete->buffer = buffer_pedido_leer_memoria;
     t_buffer *paquete_serializado = serializar_paquete(paquete);


     log_info(logger, " ## TID: %d - Accion: LEER - Dirección Fisica: %d", contexto.tid, dir_fisica);


      send(socket_memoria, paquete_serializado->stream, paquete_serializado->size, 0);

     buffer_destroy(paquete_serializado);
     eliminar_paquete(paquete);send(socket_memoria, paquete_serializado->stream, paquete_serializado->size, 0);

     buffer_destroy(paquete_serializado);
     eliminar_paquete(paquete);
     

     u_int32_t valor;

     recv(socket_memoria, &valor, sizeof(u_int32_t), MSG_WAITALL);

     return valor;
}

void actualizar_contexto()
{
     // Armamos los datos que debemos enviar a Memoria
     t_contexto* contexto_a_memoria = malloc(sizeof(t_contexto));
     contexto_a_memoria->pid = contexto.pid;
     contexto_a_memoria->tid = contexto.tid;
     contexto_a_memoria->PC = contexto.PC;
     contexto_a_memoria->AX = contexto.AX;
     contexto_a_memoria->BX = contexto.BX;
     contexto_a_memoria->CX = contexto.CX;
     contexto_a_memoria->DX = contexto.DX;
     contexto_a_memoria->EX = contexto.EX;
     contexto_a_memoria->FX = contexto.FX;
     contexto_a_memoria->GX = contexto.GX;
     contexto_a_memoria->HX = contexto.HX;
     contexto_a_memoria->Base = contexto.Base;
     contexto_a_memoria->Limite = contexto.Limite;

     // Armamos y serializamos el paquete con los datos a enviar
     t_paquete* paquete = malloc(sizeof(t_paquete));
     paquete->codigo_operacion = OPERACION_ACTUALIZAR_CONTEXTO;
     paquete->buffer = serializar_datos_contexto(contexto_a_memoria);
     t_buffer* paquete_serializado = serializar_paquete(paquete);


     log_info(logger, "#TID: %d  - Actualizo Contexto Ejecución", contexto.tid);

     pthread_mutex_lock(&mutex_socket_memoria);
     send(socket_memoria, paquete_serializado->stream, paquete_serializado->size, 0);

     op_code operacion = recibir_operacion(socket_memoria);
     pthread_mutex_unlock(&mutex_socket_memoria);

     if(operacion != OPERACION_CONFIRMAR) {
          log_error(logger, "Error al actualizar contexto de ejecución. Cod: %d", operacion);
          abort();
     }

     buffer_destroy(paquete_serializado);
     eliminar_paquete(paquete);   
     destruir_datos_contexto(contexto_a_memoria);
}




void escuchar_interrupciones() {
     while(1) {
          op_code operacion = recibir_operacion(socket_interrupt);

          switch (operacion)
          {
          case OPERACION_DESALOJAR_HILO:
               pthread_mutex_lock(&mutex_interrupciones);
               hay_interrupcion = true;
               pthread_mutex_unlock(&mutex_interrupciones);
               break;
          default:
               log_debug(logger, "Operación desconocida en interrupt: %d", operacion);
               break;
          }
     }
}

void ejecutar_instruccion_mutex(op_code operacion, char* recurso)
{
     t_datos_operacion_mutex* datos = malloc(sizeof(t_datos_operacion_mutex));
     datos->recurso = string_duplicate(recurso);

     t_paquete* paquete = malloc(sizeof(t_paquete));
     paquete->codigo_operacion = operacion;
     paquete->buffer = serializar_datos_operacion_mutex(datos);
     t_buffer* paquete_serializado = serializar_paquete(paquete);

     send(socket_dispatch, paquete_serializado->stream, paquete_serializado->size, 0);

     buffer_destroy(paquete_serializado);
     eliminar_paquete(paquete);
     destruir_datos_operacion_mutex(datos);
}

void enviar_operacion_a_kernel(op_code operacion)
{
     t_buffer* buffer = buffer_create(sizeof(uint32_t));
     buffer_add_uint32(buffer, operacion);

     send(socket_dispatch, buffer->stream, buffer->size, 0);

     buffer_destroy(buffer);
}

void ejecutar_instruccion_hilo(op_code operacion, uint32_t tid)
{
     t_datos_operacion_hilo* datos = malloc(sizeof(t_datos_operacion_hilo));
     datos->tid = tid;

     t_paquete* paquete = malloc(sizeof(t_paquete));
     paquete->codigo_operacion = operacion;
     paquete->buffer = serializar_datos_operacion_hilo(datos->tid);
     t_buffer* paquete_serializado = serializar_paquete(paquete);

     send(socket_dispatch, paquete_serializado->stream, paquete_serializado->size, 0);

     buffer_destroy(paquete_serializado);
     eliminar_paquete(paquete);
     destruir_datos_operacion_hilo(datos);
}

void incrementar_pc()
{
     contexto.PC += 1;
}