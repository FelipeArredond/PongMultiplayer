#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "cJSON.h"
#define PORT 8080
#define format_string "%s\n"

int main(int argc, char const* argv[]){
	int server_fd, new_socket, valread;
	struct sockaddr_in address;
	int opt = 1;
	int addrlen = sizeof(address);
	char buffer_received[1024] = { 0 };
	char* hello = "Hello from server";
	int connected_clients = 0;

	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	if (setsockopt(server_fd, SOL_SOCKET,
				SO_REUSEADDR | SO_REUSEPORT, &opt,
				sizeof(opt))) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);
	printf(format_string, "HOLA1\n");
	fflush(stdout);
	if (bind(server_fd, (struct sockaddr*)&address,
			sizeof(address))
		< 0) {
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	printf(format_string, "HOLA2\n");
	fflush(stdout);
	if (listen(server_fd, 3) < 0) {
		perror("listen");
		exit(EXIT_FAILURE);
	}
	printf(format_string, "HOLA3\n");
	fflush(stdout);
	if ((new_socket
		= accept(server_fd, (struct sockaddr*)&address,
				(socklen_t*)&addrlen))
		< 0) {
		perror("accept");
		exit(EXIT_FAILURE);
	}
	printf(format_string, "HOLA4\n");
    while(read(new_socket, buffer_received, 1024)){
		printf("fasdfsadfsa\n");
		fflush(stdout);
        printf(format_string, buffer_received);
        send(new_socket, "Hola python", strlen(hello), 0);
    }
	shutdown(server_fd, SHUT_RDWR);
	return 0;
}
