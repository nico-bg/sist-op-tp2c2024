#include <utils/configuracion.h>

/*
 * @brief Levanta el archivo de configuración y valida que se haya creado correctamente
 * @param path Ruta hacia el archivo de configuración que se quiere levantar
 */
t_config* iniciar_config(char* path)
{
    t_config* config;
    config = config_create(path);

    if(config == NULL) {
        perror("Error al levantar el archivo de configuración");
        config_destroy(config);
        abort();
    }

    return config;
}

t_log* iniciar_logger(t_config* config, char* path, char* name)
{
    t_log* logger;

    char* log_level_string = config_get_string_value(config, "LOG_LEVEL");
    t_log_level log_level = log_level_from_string(log_level_string);

    // Si no se pudo obtener correctamente el log_level, continuamos con LOG_LEVEL_TRACE por defecto
    if(log_level == -1) {
        perror("LOG_LEVEL inválido, continuando con LOG_LEVEL_TRACE por defecto");
        log_level = LOG_LEVEL_TRACE;
    }

    logger = log_create(path, name, true, log_level);

    if(logger == NULL) {
        perror("Error inicializando el logger");
        log_destroy(logger);
        config_destroy(config);
        abort();
    }

    return logger;
}

t_log* iniciar_logger_debug(char* path, char* name)
{
    t_log* logger;

    logger = log_create(path, name, true, LOG_LEVEL_DEBUG);

    if(logger == NULL) {
        perror("Error inicializando el logger");
        log_destroy(logger);
        abort();
    }

    return logger;
}