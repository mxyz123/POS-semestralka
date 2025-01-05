#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <unistd.h>

int main() {
    //premmenne
    int choice, port, status, clientSocket;
    char ip_addr[19];
    struct sockaddr_in serv_addr;

    //hra
    printf("1. Pripojit\n2. Odist\n");
    scanf("%d", &choice);
    switch(choice) {
        case 1:
            printf("Zadaj adresu ip: ");
            scanf("%s", ip_addr);
            printf("Zadaj port: ");
            scanf("%d", &port);
            clientSocket = socket(AF_INET, SOCK_STREAM, 0);
            serv_addr.sin_family = AF_INET;
            serv_addr.sin_port = htons(port);
            if (inet_pton(AF_INET, ip_addr, &serv_addr.sin_addr) <= 0) {
                printf("\nInvalid address/ Address not supported \n");
                return -1;
            }
            status = connect(clientSocket, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
            break;
        case 2:
            printf("Exiting.\n");
            break;
        default:
            break;
    }

    close(clientSocket);
    return 0;
}