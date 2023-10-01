#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT_A 8080
#define PORT_B 8081
#define format_string "%s\n"

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
	char buffer_received[1024] = { 0 };

	while (read(new_socket_a, buffer_received_a, sizeof(buffer_received_a)) && read(new_socket_b, buffer_received_b, sizeof(buffer_received_b))) {
		
	}


    close(server_fd_a);
    close(server_fd_b);
    return 0;
}
