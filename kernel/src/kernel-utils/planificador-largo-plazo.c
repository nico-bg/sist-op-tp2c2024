#include <kernel-utils/planificador-largo-plazo.h>
#include <utils/comunicacion_kernel_memoria.h>
#include <kernel-utils/conexion-a-memoria.h>

static void transicion_new_a_ready(t_pcb* proceso, t_tcb* hilo);
static op_code pedir_inicializacion_hilo_a_memoria(t_tcb* hilo);
static op_code pedir_inicializacion_proceso_a_memoria(t_pcb* proceso);
static t_tcb* crear_hilo_principal(t_pcb* proceso_padre);

/**
 * @brief Se mantiene escuchando los cambios en los estados NEW y EXIT
 * para pasarlos a READY o liberarlos respectivamente
 */
void* planificador_largo_plazo()
{
    while(1) {
        // Esperamos hasta que se finalicen procesos para seguir intentando inicializar (solo si en algun momento no hubo memoria suficiente)
        sem_wait(&semaforo_memoria_suficiente);
        // Esperamos hasta que hayan procesos en NEW
        sem_wait(&semaforo_estado_new);

        // Obtenemos el siguiente proceso (cuyo hilo principal debería pasar a READY) de `estado_new` según FIFO
        pthread_mutex_lock(&mutex_estado_new);
        t_pcb* proceso_a_inicializar = list_get(estado_new, 0);
        pthread_mutex_unlock(&mutex_estado_new);

        int resultado_proceso = pedir_inicializacion_proceso_a_memoria(proceso_a_inicializar);

        // Si no hubo memoria suficiente pasamos a la siguiente iteración donde se va a bloquear por el `semaforo_memoria_suficiente`
        if(resultado_proceso == OPERACION_NOTIFICAR_ERROR) {
            sem_post(&semaforo_estado_new);
            continue;
        }

        // Si hubo memoria suficiente liberamos el semáforo de memoria para evitar bloquear el planificador en la siguiente iteración
        // ... y continuamos la ejecución
        sem_post(&semaforo_memoria_suficiente);

        // Inicializamos el hilo principal del proceso
        t_tcb* hilo_principal = crear_hilo_principal(proceso_a_inicializar);

        // Le enviamos el hilo a Memoria para que cree los contextos de ejecucion
        op_code resultado = pedir_inicializacion_hilo_a_memoria(hilo_principal);

        // Si la respuesta es exitosa, pasamos el tcb a `estado_ready` y sacamos el proceso de `estado_new`
        if(resultado == OPERACION_CONFIRMAR) {
            transicion_new_a_ready(proceso_a_inicializar, hilo_principal);
            log_info(logger, "## (%d:%d) Se crea el Hilo - Estado: READY", hilo_principal->pid_padre, hilo_principal->tid);
            log_debug(logger, "LISTA ESTADO READY: %d", list_size(estado_ready));
        }
    }
}

/**
 * @brief Crea una conexión con Memoria y envía el hilo serializado,
 * espera la respuesta y devuelve 0 si se pudo inicializar,
 * caso contrario (memoria insuficiente) devuelve -1
 */
static op_code pedir_inicializacion_hilo_a_memoria(t_tcb* hilo)
{
    // Establecer la conexión con Memoria
    int socket_memoria = crear_conexion_a_memoria();
    if (socket_memoria == -1) {
        log_error(logger, "Error al conectar con Memoria");
    }

    // Serializamos los datos a enviar en un buffer
    t_datos_inicializacion_hilo* datos_inicializacion = malloc(sizeof(t_datos_inicializacion_hilo));
    datos_inicializacion->pid = hilo->pid_padre;
    datos_inicializacion->tid = hilo->tid;
    datos_inicializacion->archivo_pseudocodigo = string_duplicate(hilo->nombre_archivo);
    t_buffer* buffer_inicializacion = serializar_datos_inicializacion_hilo(datos_inicializacion);

    // Empaquetamos y serializamos los datos junto con el código de operación
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = OPERACION_CREAR_HILO;
    paquete->buffer = buffer_inicializacion;
    t_buffer* paquete_serializado = serializar_paquete(paquete);

    send(socket_memoria, paquete_serializado->stream, paquete_serializado->size, 0);

    buffer_destroy(paquete_serializado);
    eliminar_paquete(paquete);
    destruir_datos_inicializacion_hilo(datos_inicializacion);

    // Esperar respuesta de Memoria
    op_code respuesta = recibir_operacion(socket_memoria);

    // Cerrar la conexión con Memoria
    close(socket_memoria);

    // Retornar el resultado
    return respuesta;
}

static void transicion_new_a_ready(t_pcb* proceso, t_tcb* hilo)
{
    // Sacamos el proceso del estado NEW
    pthread_mutex_lock(&mutex_estado_new);
    list_remove_element(estado_new, proceso);
    pthread_mutex_unlock(&mutex_estado_new);

    // Agregamos el hilo al estado READY
    pthread_mutex_lock(&mutex_estado_ready);
    list_add(estado_ready, hilo);
    pthread_mutex_unlock(&mutex_estado_ready);

    // Le notificamos al planificador de corto plazo que tiene mas hilos para planificar
    sem_post(&semaforo_estado_ready);
}

static t_tcb* crear_hilo_principal(t_pcb* proceso_padre)
{
    // Inicializamos el hilo principal
    t_tcb* hilo_principal = malloc(sizeof(t_tcb));
    hilo_principal->pid_padre = proceso_padre->pid;
    hilo_principal->nombre_archivo = string_duplicate(proceso_padre->nombre_archivo);
    hilo_principal->tid = 0;
    hilo_principal->prioridad = proceso_padre->prioridad;
    hilo_principal->hilos_bloqueados = list_create();

    return hilo_principal;
}

static op_code pedir_inicializacion_proceso_a_memoria(t_pcb* proceso)
{
    int socket_memoria = crear_conexion_a_memoria();

    if (socket_memoria == -1) {
        log_error(logger, "No se pudo establecer la coneccion con la memoria");
    }  

    t_datos_inicializacion_proceso* datos_inicializacion = malloc(sizeof(t_datos_inicializacion_proceso));
    datos_inicializacion->pid = proceso->pid;  // PID del proceso
    datos_inicializacion->tamanio = proceso->tamanio;  // Tamaño del proceso

    // Serializar los datos de inicialización del proceso
    t_buffer* buffer_inicializacion = serializar_datos_inicializacion_proceso(datos_inicializacion);

    // Empaquetamos y serializamos los datos junto con el código de operación
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = OPERACION_CREAR_PROCESO;
    paquete->buffer = buffer_inicializacion;
    t_buffer* paquete_serializado = serializar_paquete(paquete);

    send(socket_memoria, paquete_serializado->stream, paquete_serializado->size, 0);

    buffer_destroy(paquete_serializado);
    eliminar_paquete(paquete);
    destruir_datos_inicializacion_proceso(datos_inicializacion);

    // Esperar respuesta de Memoria
    op_code respuesta = recibir_operacion(socket_memoria);

    // Cerrar la conexión con Memoria
    close(socket_memoria);

    return respuesta; 
}
