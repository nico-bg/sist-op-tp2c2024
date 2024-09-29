#include <kernel-utils/planificador-largo-plazo.h>

static void transicion_new_a_ready(t_pcb* proceso, t_tcb* hilo);
static int pedir_inicializacion_proceso_a_memoria(t_pcb* proceso);

/**
 * @brief Se mantiene escuchando los cambios en los estados NEW y EXIT
 * para pasarlos a READY o liberarlos respectivamente
 */
void* planificador_largo_plazo()
{
    // liberar_hilos_en_exit();

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
        t_tcb* hilo_principal = malloc(sizeof(t_tcb));
        hilo_principal->tid = 0;
        hilo_principal->pid_padre = proceso_a_inicializar->pid;
        hilo_principal->prioridad = proceso_a_inicializar->prioridad;
        hilo_principal->nombre_archivo = string_duplicate(proceso_a_inicializar->nombre_archivo);

        // Le enviamos el hilo a Memoria para que cree los contextos de ejecucion
        int resultado = pedir_inicializacion_hilo_a_memoria(hilo_principal);

        // Si la respuesta es exitosa, pasamos el tcb a `estado_ready` y sacamos el proceso de `estado_new`
        if(resultado == 0) {
            transicion_new_a_ready(proceso_a_inicializar, hilo_principal);
        }
    }
}

/**
 * @brief Libera la memoria de todos los hilos que esten en exit.
 * Si alguno tenía TID 0, buscar a su proceso padre y liberarlo de lista_procesos
 */
void liberar_hilos_en_exit()
{

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

// static void crear_hilo()
// {
//     // Inicializamos el hilo con los datos recibidos
//     t_tcb* hilo_principal = malloc(sizeof(t_tcb));
//     hilo_principal->pid_padre = nuevo_proceso->pid;
//     hilo_principal->nombre_archivo = nombre_archivo;
//     hilo_principal->tid = 0;
//     hilo_principal->prioridad = prioridad;
// }

static int pedir_inicializacion_proceso_a_memoria(t_pcb* proceso)
{
    return 0;
}