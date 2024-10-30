#include <particiones.h>

/**
 * @brief Inicializa la lista de particiones y la memoria (con TAM_MEMORIA) según el esquema de la config
 */
void inicializar_particiones()
{
    char* esquema = config_get_string_value(config, "ESQUEMA");
    int tam_memoria = config_get_int_value(config, "TAM_MEMORIA");
    memoria = malloc(tam_memoria);
    particiones = list_create();

    if(strcmp(esquema, "FIJAS") == 0) {
        char** particiones_fijas = config_get_array_value(config, "PARTICIONES");

        int i = 0;

        while(particiones_fijas[i] != NULL) {
            uint32_t tamanio_particion = atoi(particiones_fijas[i]);
            crear_particion(tamanio_particion);
            i++;
        }
    }

    if(strcmp(esquema, "DINAMICAS") == 0) {
        crear_particion(tam_memoria);
    }
}

/**
 * @brief Crea y agrega la partición a la lista de particiones, en la posición indicada
 */
t_particion* crear_particion_en_indice(uint32_t tamanio, int indice)
{

}

/**
 * @brief Crea y agrega la partición al final de la lista de particiones
 */
t_particion* crear_particion(uint32_t tamanio) {

}

/**
 * @brief Asigna el proceso (PID) a la partición, según el esquema de la config
 */
void asignar_particion(t_particion* particion, uint32_t tamanio_proceso, uint32_t pid)
{

}

/**
 * @brief Libera la partición, modificando su estado de `esta_libre` y `pid`
 * Si el esquema es de particiones dinámicas, se consolidan las particiones libres luego de liberar una
 */
void desasignar_particion(t_particion* particion)
{
    char* esquema = config_get_string_value(config, "ESQUEMA");

    particion->esta_libre = true;
    particion->pid = UINT32_MAX; // Simulamos un valor nulo usando el maximo de UINT32

    if(strcmp(esquema, "DINAMICAS") == 0) {
        consolidar_particiones_libres();
        consolidar_particiones_libres();
    }
}

/**
 * @brief En caso de encontrar 2 particiones libres y contiguas, las consolida en una sola
 * @brief Solo se va a usar para el esquema de particiones dinámicas
 */
void consolidar_particiones_libres()
{
    t_particion* particion_actual;
    t_particion* particion_siguiente;

    for(int i = 0; i < list_size(particiones); i++) {
        particion_actual = list_get(particiones, i);
        particion_siguiente = list_get(particiones, i + 1);

        if(particion_actual->esta_libre && particion_siguiente->esta_libre) {
            list_remove_element(particiones, particion_siguiente);

            particion_actual->tamanio += particion_siguiente->tamanio;
            particion_actual->limite = particion_actual->base + particion_actual->tamanio;

            destruir_particion(particion_siguiente);

            // Cortamos el bucle
            i = list_size(particiones);
        }
    }
}

/**
 * @brief Busca la partición donde se debería asignar el nuevo proceso, según el tamaño del mismo y el algoritmo de asignación
 * Devuelve NULL si no se encuentra una partición libre
 */
t_particion* buscar_particion_libre(uint32_t tamanio)
{

}

uint32_t pid_auxiliar;

bool tiene_pid_auxiliar(void* elemento)
{
    t_particion* particion = (t_particion*) elemento;

    return particion->pid == pid_auxiliar;
}

/**
 * @brief Busca la partición asignada al pid recibido y la retorna. Devuelve NULL si no se encuentra
 */
t_particion* buscar_particion_por_pid(uint32_t pid)
{
    pid_auxiliar = pid;
    t_particion* particion_encontrada = list_find(particiones, tiene_pid_auxiliar);

    return particion_encontrada;
}

/**
 * @brief Itera sobre la lista de particiones y retorna el indice de la partición recibida
 * Devuelve -1 si la partición no se encuentra en la lista
 */
int buscar_indice_particion(t_particion* particion)
{

}

/**
 * @brief Libera la memoria de la partición
 */
void destruir_particion(t_particion* particion) {
    free(particion);
}