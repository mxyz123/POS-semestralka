#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

static struct termios normal_termios;
static int cmd = STDIN_FILENO;

void disableRawMode() {
    tcsetattr(cmd, TCSAFLUSH, &normal_termios);
}

void enableRawMode() {
    struct termios raw_termios = normal_termios;
    raw_termios.c_lflag &= ~(ECHO | ICANON);

    tcsetattr(cmd, TCSAFLUSH, &raw_termios);
}

int readKeyPress() {
    char c;
    int x = 0;
    read(cmd, &c, 1);
    switch(c) {
        case 'w':
            x = 1;
            break;
        case 's':
            x = 2;
            break;
        case 'a':
            x = 3;
            break;
        case 'd':
            x = 4;
            break;
        case 'q':
            x = -1;
            break;
        default:
            x = 0;
    }
    return x;
}

typedef struct box{
    int socket, signal;
} box;

void * user(void * data) {
    box * d = (box*)data;
    int x;
    char buffer[1024];

    while(d->signal == 0) {
        enableRawMode();
        x = readKeyPress();
        sprintf(buffer, "%d", x);
        disableRawMode();

        if(x == -1)
            d->signal = 1;

        send(d->socket, buffer, strlen(buffer), 0);
    }
}

void * server(void * data) {
    box * d = (box*)data;
    char buffer[1024];
    ssize_t recv_size, recv_size_;

    int tiles[20][20], score;
    
    while(d->signal == 0) {
        printf("\033[H");
        recv_size = recv(d->socket, tiles, 1600ul, 0);
        recv_size_ = recv(d->socket, &score, sizeof(int), 0);
        if(recv_size > 0) {
            for (size_t i = 0; i < 20; i++)
            {
                for (size_t j = 0; j < 20; j++)
                {
                    if(tiles[i][j] == 1)
                        printf("\x1b[32m■ \x1b[0m");
                    else if(tiles[i][j] == 2)
                        printf("\x1b[31m■ \x1b[0m");
                    else
                        printf("■ ");
                }
                printf("\n");
            }
        }
        if(recv_size_ > 0)
            printf("\n[SCORE:%d]", score);
    }
}

int main() {
    printf("\033[H\033[J");
    tcgetattr(cmd, &normal_termios);
    atexit(disableRawMode);

    int port = 0;
    box data;
    data.signal = 0;

    printf("Zadajte port: ");
    scanf("%d", &port);

    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    inet_pton(AF_INET, "0.0.0.0", &address.sin_addr);

    data.socket = clientSocket;

    int result = connect(clientSocket, (struct sockaddr *)&address, sizeof(address));

    pthread_t user_t, server_t;

    pthread_create(&user_t, NULL, user, &data);
    pthread_create(&server_t, NULL, server, &data);

    pthread_join(user_t, NULL);
    pthread_join(server_t, NULL);
    
    close(clientSocket);
    printf("\033[H\033[J");
    return 0;
}