#include <stdio.h>
#include <stdlib.h>
#include <termios.h>

#include <string.h>

#include "networking.h"

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

typedef struct data_{
    int port;
    char ip_addr[19];
} data_;

void * network() {
    //premmenne
    int port, status, clientSocket, value;
    char ip_addr[19], buffer[1024] = {0};
    char* msg = "ping";
    struct sockaddr_in serv_addr;

    //sockety
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip_addr, &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
    status = connect(clientSocket, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    send(clientSocket, msg, strlen(msg), 0);
    value = read(clientSocket, buffer, 1023);
    printf("%s\n", buffer);

    close(clientSocket);
}

void * ui() {
    //premmenne
    int choice, port;
    char ip_addr[19];

    //ui
    enableRawMode();
    printf("%d\n", readKeyPress());
    disableRawMode();

    printf("1. Pripojit\n2. Odist\n");
    scanf("%d", &choice);
    switch(choice) {
        case 1:
            printf("Zadaj adresu ip: ");
            scanf("%s", ip_addr);
            printf("Zadaj port: ");
            scanf("%d", &port);

            break;
        case 2:
            printf("Exiting.\n");
            break;
        default:
            break;
    }

}


int main() {
    //terminal
    tcgetattr(cmd, &normal_termios);
    atexit(disableRawMode);
    
    return 0;
}