#include "main.h"


/* Para el manejo de instrucciones */
#define MAX_LINE_LENGTH 255

static nodo_proceso* nodo_primer_proceso = NULL;

nodo_proceso* buscar_proceso_por_pid(uint32_t pid){

    nodo_proceso* actual = nodo_primer_proceso;

    while(actual != NULL && actual->proceso.pid != pid){
        actual = actual->siguiente_nodo_proceso;
    }

    return actual;
}

nodo_hilo* buscar_hilo_por_tid(uint32_t pid, uint32_t tid){

    nodo_proceso* nodo_proceso = buscar_proceso_por_pid(pid);
    nodo_hilo* actual = nodo_proceso->proceso.lista_hilos;

    while(actual != NULL && actual->hilo.tid != tid){
        actual = actual->siguiente_nodo_hilo;
    }

    return actual;
}


void iniciar_proceso(t_datos_inicializacion_proceso* datos){
    
    nodo_proceso* nuevo_nodo_proceso = (nodo_proceso*)malloc(sizeof(nodo_proceso));

    nuevo_nodo_proceso->proceso.pid = datos->pid;
    nuevo_nodo_proceso->proceso.tamanio = datos->tamanio;
    nuevo_nodo_proceso->proceso.base = 0;
    nuevo_nodo_proceso->proceso.limite = nuevo_nodo_proceso->proceso.base + datos->tamanio;
    nuevo_nodo_proceso->proceso.lista_hilos = NULL; //No inicializamos ningún hilo todavía

    nuevo_nodo_proceso->siguiente_nodo_proceso = NULL; //Inicializamos el puntero al siguiente elemento de la lista (vacío)

    if(nodo_primer_proceso == NULL){ //Si aún no se creó ningún proceso
        nodo_primer_proceso = nuevo_nodo_proceso;
    } else { //Si ya hay al menos un proceso creado
         nodo_proceso* proceso = buscar_ultimo_proceso();
         proceso->siguiente_nodo_proceso = nuevo_nodo_proceso;
    }

    return;
}

uint32_t finalizar_proceso(t_datos_finalizacion_proceso* datos){

    nodo_proceso* proceso = buscar_proceso_por_pid(datos->pid);

    int tamanio = proceso->proceso.tamanio;

    nodo_hilo* actual = proceso->proceso.lista_hilos;
    nodo_hilo* siguiente;

    while(actual != NULL){ //Eliminamos todos los hilos del programa
        siguiente = actual->siguiente_nodo_hilo;
        free(actual);
        actual = siguiente;
    }

    free(proceso);

    return tamanio;
}

void iniciar_hilo(t_datos_inicializacion_hilo* datos){

    nodo_proceso* nodo_proceso = buscar_proceso_por_pid(datos->pid);
    nodo_hilo* nuevo_nodo_hilo = (nodo_hilo*)malloc(sizeof(nodo_hilo));

    nuevo_nodo_hilo->hilo.tid = datos->tid;
    nuevo_nodo_hilo->hilo.PC = 0;
    nuevo_nodo_hilo->hilo.AX = 0;
    nuevo_nodo_hilo->hilo.BX = 0;
    nuevo_nodo_hilo->hilo.CX = 0;
    nuevo_nodo_hilo->hilo.DX = 0;
    nuevo_nodo_hilo->hilo.EX = 0;
    nuevo_nodo_hilo->hilo.FX = 0;
    nuevo_nodo_hilo->hilo.GX = 0;
    nuevo_nodo_hilo->hilo.HX = 0;

    nuevo_nodo_hilo->hilo.archivo_pseudocodigo = leer_archivo_pseudocodigo(datos->archivo_pseudocodigo);

    nuevo_nodo_hilo->siguiente_nodo_hilo = NULL;

    if(nodo_proceso->proceso.lista_hilos == NULL){   // Aun no se inicializó ningún hilo en el proceso
        nodo_proceso->proceso.lista_hilos = nuevo_nodo_hilo;
    } else {                                // El proceso ya tenía al menos un hilo
        nodo_hilo* ultimo_hilo = buscar_ultimo_hilo(nodo_proceso->proceso.lista_hilos->hilo.tid);
        ultimo_hilo->siguiente_nodo_hilo = nuevo_nodo_hilo;
    }

    return;
}

void finalizar_hilo(t_datos_finalizacion_hilo* datos){

    nodo_hilo* actual = buscar_hilo_por_tid(datos->pid, datos->tid);

    liberar_instrucciones(actual->hilo.archivo_pseudocodigo);
    free(actual);

}


char** leer_archivo_pseudocodigo(const char* nombre_archivo_codigo){

    char* path_archivo = obtener_path_completo(nombre_archivo_codigo);
    int cant_lineas = contar_lineas(path_archivo);

    FILE* archivo = fopen(path_archivo, "r");

    char** instrucciones = (char**)malloc((cant_lineas + 1)*sizeof(char*));

    char buffer[MAX_LINE_LENGTH];
    int cont = 0;

    while(fgets(buffer, MAX_LINE_LENGTH, archivo) != NULL && cont < cant_lineas){
        buffer[strcspn(buffer, "\n")] = 0;

        instrucciones[cont] = (char*)malloc((strlen(buffer) + 1) * sizeof(char));

        strcpy(instrucciones[cont], buffer);
        cont++;
    }

    instrucciones[cont] = NULL; //La ultima linea del array será NULL, indica que terminan las instrucciones

    fclose(archivo);
    return instrucciones;

}

void liberar_instrucciones(char** instrucciones){
    int i = 0;
    while(instrucciones[i]!=NULL){
        free(instrucciones[i]);
        i++;
    }
    free(instrucciones);
}

int contar_lineas(const char* path_archivo){

    FILE *archivo = fopen(path_archivo, "r"); //Abrimos el archivo como solo lectura
    
    int cont = 0;
    char linea[MAX_LINE_LENGTH];

    while(fgets(linea, sizeof(linea), archivo) != NULL){
        cont++;
    }

    fclose(archivo);
    return cont;
}

char* obtener_path_completo(const char* nombre_archivo){

    t_config* config = iniciar_config("memoria.config");
    
    char* path_config = config_get_string_value(config, "PATH_INSTRUCCIONES");

    char* path_completo = malloc(strlen(path_config) + strlen(nombre_archivo) + 2); // +1 para '/' y +1 para '\0'

    strcpy(path_completo, path_config);
    strcpy(path_completo, "/");
    strcpy(path_completo, nombre_archivo);

    return path_completo;
}

nodo_proceso* buscar_ultimo_proceso(void){
    if(nodo_primer_proceso == NULL){
        return nodo_primer_proceso;
    }

    nodo_proceso* actual = nodo_primer_proceso;
    
    while(actual->siguiente_nodo_proceso != NULL){
        actual = actual->siguiente_nodo_proceso;
    }
    return actual;
}

nodo_hilo* buscar_ultimo_hilo(uint32_t pid){

    nodo_proceso* nodo_proceso = buscar_proceso_por_pid(pid);
    nodo_hilo* actual = nodo_proceso->proceso.lista_hilos;

    while(actual->siguiente_nodo_hilo != NULL){
        actual = actual->siguiente_nodo_hilo;
    }

    return actual;
}


t_contexto* devolver_contexto_ejecucion(t_cpu_solicitar_contexto* datos){

    nodo_proceso* proceso = buscar_proceso_por_pid(datos->pid);
    nodo_hilo* hilo = buscar_hilo_por_tid(datos->pid, datos->tid);

    t_contexto* contexto;

    contexto->pid = proceso->proceso.pid;
    contexto->base = proceso->proceso.base;
    contexto->limite = proceso->proceso.limite;
    contexto->tid = hilo->hilo.tid;
    contexto->PC = hilo->hilo.PC;
    contexto->AX = hilo->hilo.AX;
    contexto->BX = hilo->hilo.BX;
    contexto->CX = hilo->hilo.CX;
    contexto->DX = hilo->hilo.DX;
    contexto->EX = hilo->hilo.EX;
    contexto->FX = hilo->hilo.FX;
    contexto->GX = hilo->hilo.GX;
    contexto->HX = hilo->hilo.HX;

    return contexto;
}

void actualizar_contexto_ejecucion(t_contexto* datos){

    nodo_hilo* hilo_a_actualizar = buscar_hilo_por_tid(datos->pid, datos->tid);

    hilo_a_actualizar->hilo.AX = datos->AX;
    hilo_a_actualizar->hilo.BX = datos->BX;
    hilo_a_actualizar->hilo.CX = datos->CX;
    hilo_a_actualizar->hilo.DX = datos->DX;
    hilo_a_actualizar->hilo.EX = datos->EX;
    hilo_a_actualizar->hilo.FX = datos->FX;
    hilo_a_actualizar->hilo.GX = datos->GX;
    hilo_a_actualizar->hilo.HX = datos->HX;
    hilo_a_actualizar->hilo.PC = datos->PC;

    return;
}

char* devolver_instruccion(t_datos_obtener_instruccion* datos){

    char* inst = NULL;

    nodo_hilo* nodo_hilo = buscar_hilo_por_tid(datos->pid, datos->tid);
    inst = nodo_hilo->hilo.archivo_pseudocodigo[datos->PC];

    return inst;
}

estructura_hilo* convertir_struct(t_contexto* contexto) {
    estructura_hilo* hilo = malloc(sizeof(estructura_hilo));
    hilo->tid = contexto->tid;
    hilo->PC = contexto->PC;
    hilo->AX = contexto->AX;
    hilo->BX = contexto->BX;
    hilo->CX = contexto->CX;
    hilo->DX = contexto->DX;
    hilo->EX = contexto->EX;
    hilo->FX = contexto->FX;
    hilo->GX = contexto->GX;
    hilo->HX = contexto->HX;
    hilo->archivo_pseudocodigo = NULL;

    return hilo;
}
