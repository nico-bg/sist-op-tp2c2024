#include <utils/sleep.h>

/**
 * @brief Espera (sleep) durante la cantidad de milisegundos pasados por parámetro
 */
void esperar_ms(int tiempo_ms)
{
    const int FACTOR_NANOSEGUNDOS = 1000000;
    const int FACTOR_MILISEGUNDOS = 1000;

    struct timespec timespec;
    timespec.tv_sec = tiempo_ms / FACTOR_MILISEGUNDOS;
    timespec.tv_nsec = (tiempo_ms % FACTOR_MILISEGUNDOS) * FACTOR_NANOSEGUNDOS;

    nanosleep(&timespec, NULL);
}