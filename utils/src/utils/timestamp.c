#include <utils/timestamp.h>

char* obtener_timestamp() {
    struct timespec ts;

    if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
        perror("Error obteniendo el tiempo actual");
        return NULL;
    }

    struct tm *local_time = localtime(&ts.tv_sec);
    int milliseconds = ts.tv_nsec / 1000000;

    // Reservar memoria para la cadena de timestamp (HH:MM:SS:MMM + null terminator)
    char* timestamp = malloc(13);

    // Formatear el timestamp en HH:MM:SS:MMM
    snprintf(timestamp, 13, "%02d:%02d:%02d:%03d",
             local_time->tm_hour,
             local_time->tm_min,
             local_time->tm_sec,
             milliseconds);

    return timestamp;
}