#include <particiones.h>

/**
 * @brief Inicializa la lista de particiones y la memoria (con TAM_MEMORIA) según el esquema de la config
 */
void inicializar_particiones()
{

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

}

/**
 * @brief Busca la partición donde se debería asignar el nuevo proceso, según el tamaño del mismo y el algoritmo de asignación
 * Devuelve NULL si no se encuentra una partición libre
 */
t_particion* buscar_particion_libre(uint32_t tamanio)
{

}

/**
 * @brief Busca la partición asignada al pid recibido y la retorna. Devuelve NULL si no se encuentra
 */
t_particion* buscar_particion_por_pid(uint32_t pid)
{

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