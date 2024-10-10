#include "comunicaciones.h"




#define DEVOLVER_CONTEXTO_EJECUCION 1
#define ACTUALIZAR_CONTEXTO_EJECUCION 2
#define DEVOLVER_INSTRUCCION 3
#define LEER_MEMORIA 4
#define ESCRIBIR_MEMORIA 5

#define CREAR_PROCESO 6
#define FINALIZAR_PROCESO 7
#define CREAR_HILO 8
#define FINALIZAR_HILO 9
#define MEMORY_DUMP 10


void* leer_buffer_kernel(int cod_op){

    void* datos;
    t_buffer* buffer;

    switch(cod_op){

        case CREAR_PROCESO:
            //codigo
            break;

        case FINALIZAR_PROCESO:
            //codigo
            break;

        case CREAR_HILO:
            //codigo
            break;

        case FINALIZAR_HILO:
            //codigo
            break;

        case MEMORY_DUMP:
            //codigo
            break;

        default:
            //codigo
            break;
    }

    return datos;
}

void* leer_buffer_cpu(int cod_op){

    void* datos;
    t_buffer* buffer;

    switch(cod_op){

        case DEVOLVER_CONTEXTO_EJECUCION:
            //codigo
            break;

        case ACTUALIZAR_CONTEXTO_EJECUCION:
            //codigo
            break;

        case DEVOLVER_INSTRUCCION:
            //codigo
            break;

        case LEER_MEMORIA:
            //codigo
            break;

        case ESCRIBIR_MEMORIA:
            //codigo
            break;

        default:
            //codigo
            break;
    }

    return datos;
}

void enviar_buffer(int cod_op, void* datos){

}

estructura_hilo* convertir_struct(datos_contexto_hilo* datos_contexto_hilo){

    estructura_hilo* hilo;

    hilo->tid = datos_contexto_hilo->tid;
    hilo->PC = datos_contexto_hilo->PC;
    hilo->AX = datos_contexto_hilo->AX;
    hilo->BX = datos_contexto_hilo->BX;
    hilo->CX = datos_contexto_hilo->CX;
    hilo->DX = datos_contexto_hilo->DX;
    hilo->EX = datos_contexto_hilo->EX;
    hilo->FX = datos_contexto_hilo->FX;
    hilo->GX = datos_contexto_hilo->GX;
    hilo->HX = datos_contexto_hilo->HX;

    return hilo;
}