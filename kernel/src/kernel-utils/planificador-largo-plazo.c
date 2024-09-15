#include <kernel-utils/planificador-largo-plazo.h>

/**
 * @brief Se mantiene escuchando los cambios en los estados NEW y EXIT
 * para pasarlos a READY o liberarlos respectivamente
 */
void planificador_largo_plazo()
{
    // ?? Evaluar si es necesario ejecutarlo en un hilo a parte
    // ?? ...para que escuche constantemente la lista de EXIT
    liberar_hilos_en_exit();

    while(1) {
        // Obtenemos el siguiente hilo (que debería pasar a READY) de `estado_new` según FIFO
        t_tcb* hilo_a_ready;

        // Le enviamos el hilo a Memoria para que cree las estructuras necesarias
        // Considerar utilizar un semáforo para llamar solo cuando haya memoria suficiente
        int resultado = pedir_inicializacion_hilo_a_memoria(hilo_a_ready);

        // Si la respuesta es exitosa, pasamos el tcb a `estado_ready`
        // caso contrario, bloqueamos el semáforo para esperar hasta que se libere la memoria
        if(resultado == 0) {
            // Sacamos el hilo de `estado_new` y lo pasamos a `estado_ready`
        } else {
            // Bloqueamos el semáforo
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
    return -1;
}
