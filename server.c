#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <unistd.h>

int main() {
    //premmenne
    int port, serverSocket, clientSocket, status;
    struct sockaddr_in serv_addr;
    struct sockaddr_storage client_addr;
    socklen_t addr_size;
    printf("Zadajte port: ");
    scanf("%d", &port);
    printf("%d\n");

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    inet_pton(AF_INET, "0.0.0.0", &serv_addr.sin_addr);
    serv_addr.sin_port = htons(port);
    
    status = bind(serverSocket, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    listen(serverSocket, 1);

    addr_size = sizeof(client_addr);
    clientSocket = accept(serverSocket, (struct sockaddr *)&client_addr, &addr_size);

    close(clientSocket);
    close(serverSocket);
    return 0;
}