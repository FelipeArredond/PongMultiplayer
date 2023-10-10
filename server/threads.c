#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define MAXLINE 4096
#define format_string "%s\n"
#define POINT_LIMIT 5

typedef struct {
    int horizontal;
    int vertical;
} Data;

typedef struct
{
    char address[MAXLINE + 1];
    int socket;
    char name[24];
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


    if (argc != 3) {
        // Verificar si se proporcionaron los argumentos requeridos
        printf("Uso: %s <PORT> <Log File>\n", argv[0]);
        return 1; // Salir con un código de error
    } 

    int SERVER_PORT = atoi(argv[1]);
    char *logFile = argv[2];


    if (SERVER_PORT <= 0 || SERVER_PORT > 65535) {
        printf("Puerto inválido\n");
        return 1; // Salir con un código de error
    }

    // Abrir el archivo de registro en modo escritura (si no existe, se creará)
    FILE *log = fopen(logFile, "w");

    if (log == NULL) {
        perror("Error al abrir el archivo de registro");
        return 1; // Salir con un código de error
    }

    // Si se desea ver los outputs en consola se la pasa cualquier argumento adicional
    if( argc == 3){
        dup2(fileno(log), STDOUT_FILENO);
    }
    
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
            fflush(stdout);
            game.player1 = player;
            game.isWaiting = 1;
        }
    }
    // Cerrar el archivo de registro cuando hayas terminado de usarlo
    fclose(log);
    return 0;
}

void *handle_game(void *args) {
    Game game = *((Game *)args);
    Player player1 = game.player1;
    Player player2 = game.player2;

	char bufferPlayer1[100] = { 0 };
    char bufferPlayer2[100] = { 0 };
    // Leer nombres de usuario.
    int bytesRead1 = read(player1.socket, bufferPlayer1, sizeof(bufferPlayer1));
    int bytesRead2 = read(player2.socket, bufferPlayer2, sizeof(bufferPlayer2));
    strcpy(player1.name, bufferPlayer1);
    strcpy(player2.name, bufferPlayer2);
    printf("Player 1: %s | VS | Player 2: %s\n", player1.name, player2.name);
    fflush(stdout);

    send(player1.socket, player2.name, strlen(player2.name), 0);
    send(player2.socket, player1.name, strlen(player1.name), 0);
    printf("Names were sent to players\n");
    sleep(2);
    // Establecer el socket en modo no bloqueante
    // int flags_a = fcntl(player1.socket, F_GETFL, 0);
    // fcntl(player1.socket, F_SETFL, flags_a | O_NONBLOCK);
    // int flags_b = fcntl(player2.socket, F_GETFL, 0);
    // fcntl(player2.socket, F_SETFL, flags_b | O_NONBLOCK);

    fcntl(player1.socket, F_SETFL, O_NONBLOCK);
    fcntl(player2.socket, F_SETFL, O_NONBLOCK);
    // Enviar la información de ball_vel a ambos sockets
    srand(time(NULL));
    Data ballData;
    initBallVelocity(&ballData, 0);
    sendBallVelocity(player1.socket, player2.socket, "bs", ballData);
    printf("Game started!\n");
    fflush(stdout);

	while (game.scorePlayer1 < POINT_LIMIT && game.scorePlayer2 < POINT_LIMIT) {
        // Leer datos del cliente A
        memset(bufferPlayer1, 0, sizeof(bufferPlayer1));
        bytesRead1 = read(player1.socket, bufferPlayer1, sizeof(bufferPlayer1));

        memset(bufferPlayer2, 0, sizeof(bufferPlayer2));
        // Leer datos del cliente B
        bytesRead2 = read(player2.socket, bufferPlayer2, sizeof(bufferPlayer2));

        if(bytesRead1 == 0 || bytesRead2 == 0){
            break;
        }
        
        if (bytesRead1 > 0) {
            printf("Player 1 Buffer: %s\n", bufferPlayer1);
            if(strstr(bufferPlayer1, "pmd")!= NULL){
                game.scorePlayer2++;
                memset(bufferPlayer1, 0, sizeof(bufferPlayer1));
                initBallVelocity(&ballData, (rand()%2));
                sendBallVelocity(player1.socket, player2.socket, "bs", ballData);
                continue;
            }
            // Enviar datos del cliente A al cliente B
            send(player2.socket, bufferPlayer1, bytesRead1, 0);
            printf("Player1 %s send buffer to Player2 %s\n", player1.name, player2.name);
            fflush(stdout);
        }

        if (bytesRead2 > 0) {
            printf("Player 2 Buffer: %s\n", bufferPlayer2);
            if(strstr(bufferPlayer2, "pmd") != NULL){
                game.scorePlayer1++;
                memset(bufferPlayer2, 0, sizeof(bufferPlayer2));
                initBallVelocity(&ballData, (rand()%2));
                sendBallVelocity(player1.socket, player2.socket, "bs", ballData);
                continue;
            }
            // Enviar datos del cliente B al cliente A
            send(player1.socket, bufferPlayer2, bytesRead2, 0);
            printf("Player2 %s send buffer to Player1 %s\n", player2.name, player1.name);
            fflush(stdout);
        }
    }

    //Enviar datos de fin de juego
    send(player1.socket, "fg;;", strlen("fg;;"), 0);
    send(player2.socket, "fg;;", strlen("fg;;"), 0);
    printf("Game finished. FeedBack was sent to clients.\n");
    fflush(stdout);
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
    snprintf(encodedData1, sizeof(encodedData1), "%s;%d;%d|", type, ballData.horizontal, ballData.vertical);
    snprintf(encodedData2, sizeof(encodedData2), "%s;%d;%d|", type, ballData2.horizontal, ballData2.vertical);
    // Envía la información de ball_vel a ambos sockets
    send(socket1, encodedData1, strlen(encodedData1), 0);
    send(socket2, encodedData2, strlen(encodedData2), 0);
    printf("Ball Velocity sent to each player: %d, %d\n", ballData.horizontal, ballData.vertical);
}
