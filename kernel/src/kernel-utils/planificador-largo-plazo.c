#include <kernel-utils/planificador-largo-plazo.h>

static void transicion_new_a_ready(t_pcb* proceso, t_tcb* hilo);
static int pedir_inicializacion_proceso_a_memoria(t_pcb* proceso);
static t_tcb* crear_hilo_principal(t_pcb* proceso_padre);

/**
 * @brief Se mantiene escuchando los cambios en los estados NEW y EXIT
 * para pasarlos a READY o liberarlos respectivamente
 */
void* planificador_largo_plazo()
{
    // Paralelizamos la liberación de hilos que estén en estado EXIT
    pthread_t hilo_liberacion_hilos_en_exit;
    pthread_create(&hilo_liberacion_hilos_en_exit, NULL, liberar_hilos_en_exit, NULL);
    pthread_detach(hilo_liberacion_hilos_en_exit);

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
        if(resultado_proceso != 0) {
            continue;
        }

        // Si hubo memoria suficiente liberamos el semáforo de memoria para evitar bloquear el planificador en la siguiente iteración
        // ... y continuamos la ejecución
        sem_post(&semaforo_memoria_suficiente);

        // Inicializamos el hilo principal del proceso
        t_tcb* hilo_principal = crear_hilo_principal(proceso_a_inicializar);

        // Le enviamos el hilo a Memoria para que cree los contextos de ejecucion
        int resultado = pedir_inicializacion_hilo_a_memoria(hilo_principal);

        // Si la respuesta es exitosa, pasamos el tcb a `estado_ready` y sacamos el proceso de `estado_new`
        if(resultado == 0) {
            transicion_new_a_ready(proceso_a_inicializar, hilo_principal);
        }
    }
}

/**
 * @brief Se mantiene escuchando los cambios en el estado EXIT y libera la memoria de sus elementos
 * Si alguno tenía TID 0, buscar a su proceso padre y liberarlo de lista_procesos
 */
void* liberar_hilos_en_exit()
{
    while(1) {
        // Esperamos hasta que hayan TCBs en EXIT
        sem_wait(&semaforo_estado_exit);

        // Obtenemos el siguiente hilo a liberar
        pthread_mutex_lock(&mutex_estado_exit);
        t_tcb* hilo_a_liberar = list_get(estado_exit, 0);
        pthread_mutex_unlock(&mutex_estado_exit);

        // Si el TID corresponde al hilo principal de un proceso, lo buscamos y liberamos de `lista_procesos`
        if(hilo_a_liberar->tid == 0) {
            // TODO: Buscar proceso y liberarlo
        }

        destruir_tcb(hilo_a_liberar);
    }
}

/**
 * @brief Crea una conexión con Memoria y envía el hilo serializado,
 * espera la respuesta y devuelve 0 si se pudo inicializar,
 * caso contrario (memoria insuficiente) devuelve -1
 */
int pedir_inicializacion_hilo_a_memoria(t_tcb* hilo)
{
    return 0;
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

    return hilo_principal;
}

static int pedir_inicializacion_proceso_a_memoria(t_pcb* proceso)
{
    return 0;
}
