#include <kernel-utils/dispositivo-io.h>

static void transicion_blocked_a_ready(t_tcb* hilo);

sem_t semaforo_io;
pthread_mutex_t mutex_io;
t_queue* cola_io;

void* dispositivo_io()
{
    sem_init(&semaforo_io, 0, 0);
    pthread_mutex_init(&mutex_io, NULL);
    cola_io = queue_create();

    while (true)
    {
        sem_wait(&semaforo_io);
        pthread_mutex_lock(&mutex_io);
        t_solicitud_io* solicitud = queue_pop(cola_io);
        pthread_mutex_unlock(&mutex_io);

        esperar_ms(solicitud->tiempo);

        transicion_blocked_a_ready(solicitud->hilo);

        free(solicitud);
    }
}

/* UTILIDADES */

static void transicion_blocked_a_ready(t_tcb* hilo)
{
    // Saco el hilo del estado BLOCKED
    pthread_mutex_lock(&mutex_estado_blocked);
    list_remove_element(estado_blocked, hilo);
    pthread_mutex_unlock(&mutex_estado_blocked);

    // Agrego el hilo al estado READY
    pthread_mutex_lock(&mutex_estado_ready);
    list_add(estado_ready, hilo);
    pthread_mutex_unlock(&mutex_estado_ready);

    sem_post(&semaforo_estado_ready);
}