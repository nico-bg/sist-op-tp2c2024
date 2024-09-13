#include <kernel-utils/inicializacion.h>

/*
 * @brief Convierte los argumentos recibidos de la funcion `main` a la estructura t_argumentos y la retorna
 */
t_argumentos* procesar_argumentos(int argc, char* argv[])
{
    t_argumentos* argumentos = malloc(sizeof(t_argumentos*));

    // Se requieren 2 argumentos, validamos con 3 porque el primero es la ejecución del script ""./bin/kernel"
    if(argc < 3) {
        log_debug(logger_debug, "Argumentos insuficientes. Se deben pasar el archivo de pseudocódigo y el tamaño del proceso");
        abort();
    }

    argumentos->archivo_pseudocodigo = string_duplicate(argv[1]);
    argumentos->tamanio_proceso = (u_int32_t) atoi(argv[2]);

    return argumentos;
}

/*
 * @brief Libera la memoria de la estructura t_argumentos
 */
void destruir_argumentos(t_argumentos* argumentos)
{
    free(argumentos->archivo_pseudocodigo);
    free(argumentos);
}