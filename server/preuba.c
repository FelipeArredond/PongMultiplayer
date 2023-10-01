#include <stdio.h>
#include <string.h>

typedef struct {
    int horizontal;
    int vertical;
} Data;

typedef struct {
    char type[10];  // 'init' u otro tipo de mensaje
    Data data;
} Message;

void extract_type_value(const char *json_data, char *type_value) {
    const char *type_key = "\"type\":\"";
    const char *type_start = strstr(json_data, type_key);
    if (type_start) {
        // Avanzar al inicio del valor de "type"
        type_start += strlen(type_key);
        // Buscar el siguiente "
        const char *type_value_start = strchr(type_start, '"');
        if (type_value_start) {
            // Calcular la longitud del valor de "type"
            size_t type_length = type_value_start - type_start;
            // Copiar el valor de "type" en type_value
            strncpy(type_value, type_start, type_length);
            type_value[type_length] = '\0';  // Terminar la cadena
        }
    }
}

int main() {
    // Supongamos que buffer_received_a->type contiene un JSON válido
    Message buffer_received_a;
    strcpy(buffer_received_a.type, "{\"type\":\"point_made\"}");

    char type_value[100];
    extract_type_value(buffer_received_a.type, type_value);

    printf("Valor de \"type\": %s\n", type_value);

    return 0;
}
