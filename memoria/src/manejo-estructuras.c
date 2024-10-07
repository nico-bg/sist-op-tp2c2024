#include "main.h"


/* Para el manejo de instrucciones */
#define MAX_LINE_LENGTH 255

nodo_proceso* buscar_proceso_por_pid(int pid){

    nodo_proceso* actual = nodo_primer_proceso;

    while(actual != NULL && actual->proceso.pid != pid){
        actual = actual->siguiente_nodo_proceso;
    }

    return actual;
}

nodo_hilo* buscar_hilo_por_tid(int pid, int tid){

    nodo_proceso* nodo_proceso = buscar_proceso_por_pid(pid);
    nodo_hilo* actual = nodo_proceso->proceso.lista_hilos;

    while(actual != NULL && actual->hilo.tid != tid){
        actual = actual->siguiente_nodo_hilo;
    }

    return actual;
}


void iniciar_proceso(int pid, int tamanio, uint32_t base, uint32_t limite, const char* archivo_pseudocodigo){
    
    nodo_proceso* nuevo_nodo_proceso = (nodo_proceso*)malloc(sizeof(nodo_proceso));

    nuevo_nodo_proceso->proceso.pid = pid;
    nuevo_nodo_proceso->proceso.tamanio = tamanio;
    nuevo_nodo_proceso->proceso.base = base;
    nuevo_nodo_proceso->proceso.limite = limite;
    nuevo_nodo_proceso->proceso.archivo_pseudocodigo = leer_archivo_pseudocodigo(archivo_pseudocodigo);
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

void finalizar_proceso(int pid){

    nodo_proceso* proceso = buscar_proceso_por_pid(pid);

    nodo_hilo* actual = proceso->proceso.lista_hilos;
    nodo_hilo* siguiente;

    while(actual != NULL){ //Eliminamos todos los hilos del programa
        siguiente = actual->siguiente_nodo_hilo;
        free(actual);
        actual = siguiente;
    }

    liberar_instrucciones(proceso->proceso.archivo_pseudocodigo);
    free(proceso);
    return;
}

void iniciar_hilo(int pid, int tid){

    nodo_proceso* nodo_proceso = buscar_proceso_por_pid(pid);
    nodo_hilo* nuevo_nodo_hilo = (nodo_hilo*)malloc(sizeof(nodo_hilo));

    nuevo_nodo_hilo->hilo.tid = tid;
    nuevo_nodo_hilo->hilo.PC = 0;
    nuevo_nodo_hilo->hilo.AX = 0;
    nuevo_nodo_hilo->hilo.BX = 0;
    nuevo_nodo_hilo->hilo.CX = 0;
    nuevo_nodo_hilo->hilo.DX = 0;
    nuevo_nodo_hilo->hilo.EX = 0;
    nuevo_nodo_hilo->hilo.FX = 0;
    nuevo_nodo_hilo->hilo.GX = 0;
    nuevo_nodo_hilo->hilo.HX = 0;

    nuevo_nodo_hilo->siguiente_nodo_hilo = NULL;

    if(nodo_proceso->proceso.lista_hilos == NULL){   // Aun no se inicializó ningún hilo en el proceso
        nodo_proceso->proceso.lista_hilos = nuevo_nodo_hilo;
    } else {                                // El proceso ya tenía al menos un hilo
        nodo_hilo* ultimo_hilo = buscar_ultimo_hilo(nodo_proceso->proceso.lista_hilos->hilo.tid);
        ultimo_hilo->siguiente_nodo_hilo = nuevo_nodo_hilo;
    }

    return;
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

nodo_hilo* buscar_ultimo_hilo(int pid){

    nodo_proceso* nodo_proceso = buscar_proceso_por_pid(pid);
    nodo_hilo* actual = nodo_proceso->proceso.lista_hilos;

    while(actual->siguiente_nodo_hilo != NULL){
        actual = actual->siguiente_nodo_hilo;
    }

    return actual;
}