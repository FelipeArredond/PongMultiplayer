#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <time.h> 


#define PORT_A 8080
#define PORT_B 8081
#define format_string "%s\n"

typedef struct {
    int horizontal;
    int vertical;
} Data;

typedef struct {
    char type[100];  // 'init' o cualquier otro tipo de mensaje
    Data data;
} Message;

void init_ball_velocity(Data *ball_vel, int right) {
    ball_vel->horizontal = rand() % 3 + 2;  // Genera un número entre 2 y 4
    ball_vel->vertical = rand() % 2 + 1;    // Genera un número entre 1 y 2
    
    if (!right) {
        ball_vel->horizontal = -ball_vel->horizontal;
    }
}

void send_ball_velocity_init(int socket_a, int socket_b, Message ball_vel) {
    char jsonString[200];
    snprintf(jsonString, sizeof(jsonString), "{\"type\":\"%s\",\"data\":[%d,%d]}", ball_vel.type, ball_vel.data.horizontal, ball_vel.data.vertical);

    // Envía la información de ball_vel a ambos sockets
    send(socket_a, jsonString, strlen(jsonString), 0);
    send(socket_b, jsonString, strlen(jsonString), 0);
}

// void extract_type_value(const char *json_data, char *type_value) {
//     cJSON *root = cJSON_Parse(json_data);  // Parsear el JSON
//     if (root) {
//         cJSON *type_item = cJSON_GetObjectItemCaseSensitive(root, "type");
//         if (cJSON_IsString(type_item)) {
//             strcpy(type_value, type_item->valuestring);
//         } else {
//             strcpy(type_value, "");  // No se encontró el campo 'type'
//         }
//         cJSON_Delete(root);  // Liberar memoria
//     } else {
//         strcpy(type_value, "");  // Error al parsear el JSON
//     }
// }

int main(int argc, char const* argv[]) {
    int server_fd_a, server_fd_b, new_socket_a, new_socket_b;
    struct sockaddr_in address_a, address_b;
    int opt = 1;
    int addrlen = sizeof(address_a);
	Message buffer_received_a[1024] = { 0 };
    Message buffer_received_b[1024] = { 0 };
    char* hello = "Hello from server";

    if ((server_fd_a = socket(AF_INET, SOCK_STREAM, 0)) < 0 ||
        (server_fd_b = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd_a, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) ||
        setsockopt(server_fd_b, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address_a.sin_family = AF_INET;
    address_a.sin_addr.s_addr = INADDR_ANY;
    address_a.sin_port = htons(PORT_A);

    address_b.sin_family = AF_INET;
    address_b.sin_addr.s_addr = INADDR_ANY;
    address_b.sin_port = htons(PORT_B);

    if (bind(server_fd_a, (struct sockaddr*)&address_a, sizeof(address_a)) < 0 ||
        bind(server_fd_b, (struct sockaddr*)&address_b, sizeof(address_b)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd_a, 3) < 0 || listen(server_fd_b, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    printf(format_string, "HOLA4");
    fflush(stdout);
  	new_socket_a = accept(server_fd_a, (struct sockaddr*)&address_a, (socklen_t*)&addrlen);
    printf(format_string, "HOLA5");
    fflush(stdout);
	new_socket_b = accept(server_fd_b, (struct sockaddr*)&address_b, (socklen_t*)&addrlen);
    printf(format_string, "HOLA6");
    fflush(stdout);

	if (new_socket_a < 0 || new_socket_b < 0) {
		perror("accept");
		exit(EXIT_FAILURE);
	}

    srand(time(NULL));
    Data ball_velocity;
    // init_ball_velocity(&ball_velocity, (rand()%2));
    init_ball_velocity(&ball_velocity, 0);
    Message initMessage;
    strcpy(initMessage.type, "ball_start");
    initMessage.data = ball_velocity;
    send_ball_velocity_init(new_socket_a, new_socket_b, initMessage);
    fflush(stdout);

	while (1) {
        // Leer datos del cliente A
        int bytes_read_a = read(new_socket_a, buffer_received_a, sizeof(buffer_received_a));
        // Leer datos del cliente B
        int bytes_read_b = read(new_socket_b, buffer_received_b, sizeof(buffer_received_b));
        if(bytes_read_a == 0 || bytes_read_b == 0){
            close(new_socket_a);
            close(new_socket_b);
            break;
        }
        char type_value[100];
        // extract_type_value(buffer_received_a->type, type_value);
        // printf("Comparación: %s\n", type_value);
        // printf("Comparación: %s\n", type_value);
        printf("Comparación: %d\n", strcmp(buffer_received_a->type, "point_made"));
        printf("Comparación: %s\n", buffer_received_a->type);
        printf("Comparación: %d\n", strcmp(buffer_received_b->type, "point_made"));
        printf("Comparación: %s\n", buffer_received_b->type);

        fflush(stdout);
        if (bytes_read_a > 0) {
            if(strcmp(buffer_received_a->type, "point_made") == 0){
                init_ball_velocity(&ball_velocity, (rand()%2));
                Message initMessage;
                strcpy(initMessage.type, "ball_start");
                initMessage.data = ball_velocity;
                printf(format_string, "point_made");
                send_ball_velocity_init(new_socket_a, new_socket_b, initMessage);
                continue;
            }
            // Enviar datos del cliente A al cliente B
            printf(format_string, "A -> B");
            send(new_socket_b, buffer_received_a, bytes_read_a, 0);
        }

        if (bytes_read_b > 0) {
            if(strcmp(buffer_received_b->type, "point_made") == 0){
                init_ball_velocity(&ball_velocity, (rand()%2));
                Message initMessage;
                strcpy(initMessage.type, "ball_start");
                initMessage.data = ball_velocity;
                printf(format_string, "point_made");
                send_ball_velocity_init(new_socket_a, new_socket_b, initMessage);
                continue;
            }
            // Enviar datos del cliente B al cliente A
            printf(format_string, "B -> A");
            send(new_socket_a, buffer_received_b, bytes_read_b, 0);
        }
        fflush(stdout);
    }


    close(server_fd_a);
    close(server_fd_b);
    return 0;
}
