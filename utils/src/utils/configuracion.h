#ifndef CONFIGURACION_H_
#define CONFIGURACION_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>

t_config* iniciar_config(char* path);

/*
* @brief Instancia un nuevo logger con el LOG_LEVEL definido en la configuración
* @param config Instancia de la configuracion, que debe tener definida la clave LOG_LEVEL. En caso de no existir dicha clave, se utilizará el nivel LOG_LEVEL_TRACE
* @param path Ruta hacia el archivo donde se van a generar los logs, tener en cuenta que si se indica dentro de un directorio, el mismo debe existir
* @param name El nombre a mostrar en los logs
*/
t_log* iniciar_logger(t_config* config, char* path, char* name);

#endif