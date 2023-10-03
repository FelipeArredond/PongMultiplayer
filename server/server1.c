#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>


#define PORT_A 8080
#define PORT_B 8081
#define format_string "%s\n"

typedef struct {
    int horizontal;
    int vertical;
} Data;

void init_ball_velocity(Data *ball_vel, int right) {
    ball_vel->horizontal = rand() % 3 + 2;  // Genera un número entre 2 y 4
    ball_vel->vertical = rand() % 2 + 1;    // Genera un número entre 1 y 2
    
    if (!right) {
        ball_vel->horizontal = -ball_vel->horizontal;
    }
}

void send_ball_velocity_init(int socket_a, int socket_b, char type[], Data ball_vel_a) {
    char encodedData_a[100];
    char encodedData_b[100];
    Data ball_vel_b = ball_vel_a;
    ball_vel_b.horizontal = -ball_vel_b.horizontal;

    snprintf(encodedData_a, sizeof(encodedData_a), "%s;%d;%d", type, ball_vel_a.horizontal, ball_vel_a.vertical);
    snprintf(encodedData_b, sizeof(encodedData_b), "%s;%d;%d", type, ball_vel_b.horizontal, ball_vel_b.vertical);
    // Envía la información de ball_vel a ambos sockets
    send(socket_a, encodedData_a, strlen(encodedData_a), 0);
    send(socket_b, encodedData_b, strlen(encodedData_b), 0);
}


int main(int argc, char const* argv[]) {
    int server_fd_a, server_fd_b, new_socket_a, new_socket_b;
    struct sockaddr_in address_a, address_b;
    int opt = 1;
    int addrlen = sizeof(address_a);
	char buffer_received_a[1024] = { 0 };
    char buffer_received_b[1024] = { 0 };
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

    // Establecer el socket en modo no bloqueante
    int flags_a = fcntl(new_socket_a, F_GETFL, 0);
    fcntl(new_socket_a, F_SETFL, flags_a | O_NONBLOCK);
    int flags_b = fcntl(new_socket_b, F_GETFL, 0);
    fcntl(new_socket_b, F_SETFL, flags_b | O_NONBLOCK);

    srand(time(NULL));
    Data ball_velocity;
    // init_ball_velocity(&ball_velocity, (rand()%2));
    init_ball_velocity(&ball_velocity, 0);
    send_ball_velocity_init(new_socket_a, new_socket_b, "ball_start", ball_velocity);
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
        if (bytes_read_a > 0) {
            printf(format_string, buffer_received_a);
            if(strcmp(buffer_received_a, "point_made") == 0){
                init_ball_velocity(&ball_velocity, (rand()%2));
                send_ball_velocity_init(new_socket_a, new_socket_b, "ball_start", ball_velocity);
                continue;
            }
            // Enviar datos del cliente A al cliente B
            printf(format_string, "A -> B");
            fflush(stdout);
            send(new_socket_b, buffer_received_a, bytes_read_a, 0);
        }

        if (bytes_read_b > 0) {
            printf(format_string, buffer_received_b);
            if(strcmp(buffer_received_b, "point_made") == 0){
                init_ball_velocity(&ball_velocity, (rand()%2));
                send_ball_velocity_init(new_socket_a, new_socket_b, "ball_start", ball_velocity);
                continue;
            }
            // Enviar datos del cliente B al cliente A
            printf(format_string, "B -> A");
            fflush(stdout);
            send(new_socket_a, buffer_received_b, bytes_read_b, 0);
        }
    }


    close(server_fd_a);
    close(server_fd_b);
    return 0;
}
