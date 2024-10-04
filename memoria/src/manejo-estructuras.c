#include "main.h"

/* Para el manejo de instrucciones */
#define MAX_LINE_LENGTH 255

estructura_proceso* buscar_proceso_por_pid(int pid){
    
}

estructura_hilo* buscar_hilo_por_tid(int pid, int tid){

    estructura_proceso* proceso = buscar_proceso_por_pid(pid);
    nodo_hilo* actual = proceso->lista_hilos;
    nodo_hilo* anterior = NULL;

    while(actual != NULL && actual->hilo.tid != tid){
        anterior = actual;
        actual = actual->siguiente;
    }

    estructura_hilo* hilo = actual->hilo;

    return hilo;
}


estructura_proceso* iniciar_proceso(int pid, int tamanio, uint32_t base, uint32_t limite, const char* archivo_pseudocodigo){
    
    estructura_proceso* nuevo_proceso = (estructura_proceso*)malloc(sizeof(estructura_proceso));

    nuevo_proceso->pid = pid;
    nuevo_proceso->tamanio = tamanio;
    nuevo_proceso->base = base;
    nuevo_proceso->limite = limite;
    nuevo_proceso->archivo_pseudocodigo = leer_archivo_pseudocodigo(archivo_pseudocodigo);
    nuevo_proceso->lista_hilos = NULL;

    return nuevo_proceso;
}

void finalizar_proceso(int pid){

    estructura_proceso* proceso = buscar_proceso_por_pid(pid);

    nodo_hilo* actual = proceso->lista_hilos;
    nodo_hilo* siguiente;

    while(actual != NULL){ //Eliminamos todos los hilos del programa
        siguiente = actual->siguiente;
        free(actual);
        actual = siguiente;
    }

    liberar_instrucciones(proceso->archivo_pseudocodigo);
    free(proceso);
    return;
}

estructura_hilo* agregar_hilo(int pid, int tid){

    estructura_proceso* proceso = buscar_proceso_por_pid(pid);

    nodo_hilo* nodo_nuevo_hilo = (nodo_hilo*)malloc(sizeof(nodo_hilo));

    estructura_hilo* nuevo_hilo = (estructura_hilo*)malloc(sizeof(estructura_hilo));


}

void finalizar_hilo(int pid, int tid){

    nodo_hilo* actual = buscar_hilo_por_tid(pid, tid);

    free(actual);

}


char** leer_archivo_pseudocodigo(const char* nombre_archivo){

    int cant_lineas = contar_lineas(nombre_archivo);

    FILE* archivo = fopen(nombre_archivo, "r");

    char** instrucciones = (char**)malloc((cant_lineas + 1)*sizeof(char*));

    char* buffer[MAX_LINE_LENGTH];
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

int contar_lineas(const char* nombre_archivo){

    FILE *archivo = fopen(nombre_archivo, "r"); //Abrimos el archivo como solo lectura
    
    int cont = 0;
    char linea[MAX_LINE_LENGTH];

    while(fgets(linea, sizeof(linea), archivo) != NULL){
        cont++;
    }

    fclose(archivo);
    return cont;
}