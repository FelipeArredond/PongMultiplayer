#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define SERVER_PORT 8085
#define MAXLINE 4096
#define format_string "%s\n"

typedef struct {
    int horizontal;
    int vertical;
} Data;

typedef struct
{
    char address[MAXLINE + 1];
    int socket;
} Player;

typedef struct
{
    Player player1;
    Player player2;
    int scorePlayer1;
    int scorePlayer2;
    int isWaiting;
} Game;

void *handle_game(void *args);
void init_ball_velocity(Data *ball_vel, int right);
void send_ball_velocity_init(int socket_a, int socket_b, char type[], Data ball_vel_a);

int main(int argc, char const *argv[])
{
    int listenfd, connfd, n;
    struct sockaddr_in serverAddr;

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(SERVER_PORT);

    if (bind(listenfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        perror("Error al vincular el socket a la dirección y puerto");
        close(listenfd);
        exit(EXIT_FAILURE);
    }

    if (listen(listenfd, 10) < 0)
    {
        perror("listen failed");
        close(listenfd);
        exit(EXIT_FAILURE);
    }

    Game game;
    game.scorePlayer1 = 0;
    game.scorePlayer2 = 0;
    game.isWaiting = 0;
    while (1)
    {
        struct sockaddr_in addr;
        socklen_t addrLen;
        char clientAddress[MAXLINE + 1];

        printf("Waiting for a connection on port %d...\n", SERVER_PORT);
        fflush(stdout);
        connfd = accept(listenfd, (struct sockaddr *)&addr, (socklen_t *)&addrLen);
        inet_ntop(AF_INET, (struct sockaddr *)&addr, clientAddress, MAXLINE);

        Player player;
        strcpy(player.address, clientAddress);
        player.socket = connfd;

        if (game.isWaiting)
        {
            game.player2 = player;

            pthread_t thread;
            pthread_create(&thread, NULL, handle_game, &game);

            game.scorePlayer1 = 0;
            game.scorePlayer2 = 0;
            game.isWaiting = 0;
        }
        else
        {
            printf("Waiting for another player...\n");
            game.player1 = player;
            game.isWaiting = 1;
        }
    }

    return 0;
}

void *handle_game(void *args) {
    Game game = *((Game *)args);
    Player player1 = game.player1;
    Player player2 = game.player2;

	char buffer_received_a[1024] = { 0 };
    char buffer_received_b[1024] = { 0 };

    printf("Player 1: %s\n", player1.address);
    printf("Player 2: %s\n", player2.address);

    // Establecer el socket en modo no bloqueante
    int flags_a = fcntl(player1.socket, F_GETFL, 0);
    fcntl(player1.socket, F_SETFL, flags_a | O_NONBLOCK);
    int flags_b = fcntl(player2.socket, F_GETFL, 0);
    fcntl(player2.socket, F_SETFL, flags_b | O_NONBLOCK);

    srand(time(NULL));
    Data ball_velocity;
    // init_ball_velocity(&ball_velocity, (rand()%2));
    init_ball_velocity(&ball_velocity, 0);
    send_ball_velocity_init(player1.socket, player2.socket, "ball_start", ball_velocity);
    fflush(stdout);

	while (1) {
        // Leer datos del cliente A
        int bytes_read_a = read(player1.socket, buffer_received_a, sizeof(buffer_received_a));
        // Leer datos del cliente B
        int bytes_read_b = read(player2.socket, buffer_received_b, sizeof(buffer_received_b));
        if(bytes_read_a == 0 || bytes_read_b == 0){
            close(player1.socket);
            close(player2.socket);
            break;
        }
        if (bytes_read_a > 0) {
            printf(format_string, buffer_received_a);
            if(strcmp(buffer_received_a, "point_made") == 0){
                init_ball_velocity(&ball_velocity, (rand()%2));
                send_ball_velocity_init(player1.socket, player2.socket, "ball_start", ball_velocity);
                continue;
            }
            // Enviar datos del cliente A al cliente B
            printf(format_string, "A -> B");
            fflush(stdout);
            send(player2.socket, buffer_received_a, bytes_read_a, 0);
        }

        if (bytes_read_b > 0) {
            printf(format_string, buffer_received_b);
            if(strcmp(buffer_received_b, "point_made") == 0){
                init_ball_velocity(&ball_velocity, (rand()%2));
                send_ball_velocity_init(player1.socket, player2.socket, "ball_start", ball_velocity);
                continue;
            }
            // Enviar datos del cliente B al cliente A
            printf(format_string, "B -> A");
            fflush(stdout);
            send(player1.socket, buffer_received_b, bytes_read_b, 0);
        }
    }

    shutdown(player1.socket, SHUT_RDWR);
    shutdown(player2.socket, SHUT_RDWR);
    return NULL;
}

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
