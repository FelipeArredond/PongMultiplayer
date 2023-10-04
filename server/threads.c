#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define SERVER_PORT 8080
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
void initBallVelocity(Data *ballData, int right);
void sendBallVelocity(int socket1, int socket2, char type[], Data ballData);

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
        perror("Listen failed");
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

	char bufferPlayer1[24] = { 0 };
    char bufferPlayer2[24] = { 0 };

    // Establecer el socket en modo no bloqueante
    // int flags_a = fcntl(player1.socket, F_GETFL, 0);
    // fcntl(player1.socket, F_SETFL, flags_a | O_NONBLOCK);
    // int flags_b = fcntl(player2.socket, F_GETFL, 0);
    // fcntl(player2.socket, F_SETFL, flags_b | O_NONBLOCK);

    fcntl(player1.socket, F_SETFL, O_NONBLOCK);
    fcntl(player2.socket, F_SETFL, O_NONBLOCK);

    srand(time(NULL));
    Data ballData;

    initBallVelocity(&ballData, 0);
    sendBallVelocity(player1.socket, player2.socket, "ball_start", ballData);
    fflush(stdout);

	while (1) {
        // Leer datos del cliente A
        memset(bufferPlayer1, 0, sizeof(bufferPlayer1));
        int bytesRead1 = read(player1.socket, bufferPlayer1, sizeof(bufferPlayer1));

        // Leer datos del cliente B
        memset(bufferPlayer2, 0, sizeof(bufferPlayer2));
        int bytesRead2 = read(player2.socket, bufferPlayer2, sizeof(bufferPlayer2));

        if(bytesRead1 == 0 || bytesRead2 == 0){
            break;
        }
        
        if (bytesRead1 > 0) {
            printf(format_string, bufferPlayer1);
            if(strcmp(bufferPlayer1, "point_made") == 0){
                initBallVelocity(&ballData, (rand()%2));
                sendBallVelocity(player1.socket, player2.socket, "ball_start", ballData);
                continue;
            }
            // Enviar datos del cliente A al cliente B
            printf(format_string, "A -> B");
            fflush(stdout);
            send(player2.socket, bufferPlayer1, bytesRead1, 0);
        }

        if (bytesRead2 > 0) {
            //printf(format_string, bufferPlayer2);
            if(strcmp(bufferPlayer2, "point_made") == 0){
                initBallVelocity(&ballData, (rand()%2));
                sendBallVelocity(player1.socket, player2.socket, "ball_start", ballData);
                continue;
            }
            // Enviar datos del cliente B al cliente A
            printf(format_string, "B -> A");
            fflush(stdout);
            send(player1.socket, bufferPlayer2, bytesRead2, 0);
        }
    }

    shutdown(player1.socket, SHUT_RDWR);
    shutdown(player2.socket, SHUT_RDWR);
    return NULL;
}

void initBallVelocity(Data *ballData, int right) {
    ballData->horizontal = rand() % 3 + 2;  // Genera un número entre 2 y 4
    ballData->vertical = rand() % 2 + 1;    // Genera un número entre 1 y 2
    
    if (!right) {
        ballData->horizontal = -ballData->horizontal;
    }
}

void sendBallVelocity(int socket1, int socket2, char type[], Data ballData) {
    char encodedData1[100];
    char encodedData2[100];

    Data ballData2 = ballData;
    ballData2.horizontal = -ballData2.horizontal;

    snprintf(encodedData1, sizeof(encodedData1), "%s;%d;%d", type, ballData.horizontal, ballData.vertical);
    snprintf(encodedData2, sizeof(encodedData2), "%s;%d;%d", type, ballData2.horizontal, ballData2.vertical);
    // Envía la información de ball_vel a ambos sockets
    send(socket1, encodedData1, strlen(encodedData1), 0);
    send(socket2, encodedData2, strlen(encodedData2), 0);
}
