#include <kernel-utils/estados.h>

t_list* lista_procesos;
t_list* estado_new;
t_list* estado_ready;
t_tcb* estado_exec;
t_list* estado_blocked;
t_list* estado_exit;

sem_t semaforo_estado_ready;
pthread_mutex_t mutex_estado_ready;
pthread_mutex_t mutex_estado_exec;

void crear_hilo_ready_mock(uint32_t pid, uint32_t tid, uint32_t prioridad) {
    t_tcb* hilo = malloc(sizeof(t_tcb));
    hilo->pid_padre = pid;
    hilo->tid = tid;
    hilo->prioridad = prioridad;
    list_add(estado_ready, hilo);
    sem_post(&semaforo_estado_ready);
}

/*
 * @brief Inicializa las variables globales de estados y la lista de procesos
 */
void inicializar_estados()
{
    /* Inicialización de estados*/
    lista_procesos = list_create();
    estado_new = list_create();
    estado_ready = list_create();
    estado_exec = malloc(sizeof(t_tcb));
    estado_blocked = list_create();
    estado_exit = list_create();

    /* Inicialización de Semáforos y Mutex de estados*/
    sem_init(&semaforo_estado_ready, 0, 0);
    pthread_mutex_init(&mutex_estado_ready, NULL);
    pthread_mutex_init(&mutex_estado_exec, NULL);

    /* TODO: Eliminar esto una vez que el planificador a largo plazo funcione */
    /* Solo lo utilizo para tener un hilo en Ready y poder probar el planificador a corto plazo */
    crear_hilo_ready_mock(1, 0, 1);
    crear_hilo_ready_mock(2, 0, 4);
    crear_hilo_ready_mock(1, 1, 2);
    crear_hilo_ready_mock(0, 0, 0);
    crear_hilo_ready_mock(0, 1, 1);
}

/**
 * @brief Libera la memoria de las variables globales de lista de procesos y listas de estado de hilos
 */
void destruir_estados()
{
    /* Liberamos la memoria de los estados */
    list_destroy_and_destroy_elements(lista_procesos, (void*) destruir_pcb);
    list_destroy_and_destroy_elements(estado_new, (void*) destruir_tcb);
    list_destroy_and_destroy_elements(estado_ready, (void*) destruir_tcb);
    destruir_tcb(estado_exec);
    list_destroy_and_destroy_elements(estado_blocked, (void*) destruir_tcb);
    list_destroy_and_destroy_elements(estado_exit, (void*) destruir_tcb);

    /* Liberamos los Semáforos y Mutex de estados */
    sem_destroy(&semaforo_estado_ready);
    pthread_mutex_destroy(&mutex_estado_ready);
    pthread_mutex_destroy(&mutex_estado_exec);
}