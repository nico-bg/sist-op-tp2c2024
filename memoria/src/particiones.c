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
    t_particion* particion_nueva = malloc(sizeof(t_particion));
    t_particion* particion_original = list_get(particiones, indice);

    particion_nueva->base = particion_original->base;
    particion_nueva->limite = (particion_nueva->base + tamanio) - 1;
    particion_nueva->tamanio = tamanio;
    particion_nueva->esta_libre = true;
    particion_nueva->pid = UINT16_MAX;

    particion_original->base = particion_nueva->limite + 1;
    particion_original->limite = particion_original->limite; //el limite de la particion original queda igual;
    particion_original->tamanio = particion_original->tamanio - particion_nueva->tamanio;

    list_add_in_index(particiones, indice, particion_nueva);

    return particion_nueva;
}

/**
 * @brief Crea y agrega la partición al final de la lista de particiones
 */
t_particion* crear_particion(uint32_t tamanio) {

    t_particion* particion = malloc(sizeof(t_particion));
    uint32_t base = 0;

    if(!list_is_empty(particiones)){
        t_particion* ultima_particion = list_get(particiones, list_size(particiones) - 1);
        base = ultima_particion->limite + 1;
    }

    particion->base = base;
    particion->limite = (base + tamanio) - 1;
    particion->tamanio = tamanio;

    particion->pid = UINT32_MAX;
    particion->esta_libre = true;

    list_add(particiones, particion);

    return particion;
}

/**
 * @brief Asigna el proceso (PID) a la partición, según el esquema de la config
 */
void asignar_particion(t_particion* particion, uint32_t tamanio_proceso, uint32_t pid)
{
    particion->pid = pid;
    particion->esta_libre = false;

    log_debug(logger, "Se asigna particion con tamaño %d a proceso con PID <%d> y tamaño %d", particion->tamanio, pid, tamanio_proceso);
    int index = 0;
    t_particion* particion_debug;
    particion_debug = (t_particion*)list_get(particiones, 0);
    while(particion_debug->pid != particion->pid){
        index++;
        particion_debug = list_get(particiones, index);
    }
    log_debug(logger, "particion con indice %d, base %d y limite %d", index, particion->base, particion->limite);

}

/**
 * @brief Libera la partición, modificando su estado de `esta_libre` y `pid`
 * Si el esquema es de particiones dinámicas, se consolidan las particiones libres luego de liberar una
 */
void desasignar_particion(t_particion* particion)
{
    char* esquema = config_get_string_value(config, "ESQUEMA");

    log_debug(logger, "Se desasigna particion con tamaño %d a proceso con PID <%d>", particion->tamanio, particion->pid);
    int index = 0;
    t_particion* particion_debug;
    particion_debug = list_get(particiones, 0);
    while(particion_debug->pid != particion->pid){
        index++;
        particion_debug = list_get(particiones, index);
    }
    log_debug(logger, "particion con indice %d, base %d y limite %d", index, particion->base, particion->limite);

    particion->esta_libre = true;
    particion->pid = UINT32_MAX; // Simulamos un valor nulo usando el maximo de UINT32

    if(strcmp(esquema, "DINAMICAS") == 0) {
        consolidar_particiones_libres();

        if(list_size(particiones) > 1) {
            consolidar_particiones_libres();
        }
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

    for(int i = 0; i < list_size(particiones) - 1; i++) {
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
    t_particion* particion_seleccionada = NULL;
    char* algoritmo = config_get_string_value(config, "ALGORITMO_BUSQUEDA");
    char* esquema =config_get_string_value(config, "ESQUEMA");

    int indice = 0;

    if(strcmp(algoritmo, "FIRST") == 0){
        for(int i = 0; i < list_size(particiones); i++){
            t_particion* particion_actual = list_get(particiones, i);
            if(particion_actual->esta_libre && particion_actual->tamanio >= tamanio){
                particion_seleccionada = particion_actual;
                indice = i;
                break;
            }
        }
    } else if (strcmp(algoritmo, "BEST") == 0) {
        for(int i = 0; i < list_size(particiones); i++){
            t_particion* particion_actual = list_get(particiones, i);
            if(particion_actual->esta_libre && particion_actual->tamanio >= tamanio){
                if(particion_seleccionada == NULL){
                    particion_seleccionada = particion_actual;
                    indice = i;
                } else {
                    if(particion_actual->tamanio < particion_seleccionada->tamanio){
                        particion_seleccionada = particion_actual;
                        indice = i;
                    }
                }
            }
        }
    } else if (strcmp(algoritmo, "WORST") == 0) {
        for(int i = 0; i < list_size(particiones); i++){
            t_particion* particion_actual = list_get(particiones, i);
            if(particion_actual->esta_libre && particion_actual->tamanio >= tamanio){
                if(particion_seleccionada == NULL){
                    particion_seleccionada = particion_actual;
                    indice = i;
                } else {
                    if(particion_actual->tamanio > particion_seleccionada->tamanio){
                        particion_seleccionada = particion_actual;
                        indice = i;
                    }
                }
            }
        }
    }

    if(strcmp(esquema, "DINAMICAS") == 0 && particion_seleccionada != NULL) {
        particion_seleccionada = crear_particion_en_indice(tamanio, indice);
    }

    return particion_seleccionada;
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
/*int buscar_indice_particion(t_particion* particion)
{

}*/

/**
 * @brief Libera la memoria de la partición
 */
void destruir_particion(t_particion* particion) {
    free(particion);
}