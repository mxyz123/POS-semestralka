#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include "microsleep.h"

#define SIDE_LENGTH 20

/*
    0 - nič
    1 - had
    2 - jedlo
*/

typedef struct coord {
    int x;
    int y;
} coord;

coord getRandomCoord() {
    coord c;
    c.x = rand() % 20; 
    c.y = rand() % 20;
    return c;
}

typedef struct box{
    int key, socket, signal, died, score, snake_len;
} box;

int coordNotInArray(coord c, coord* cArray, int cArrayLen) {
    int x = 0;
    for(int i = 0; i <= cArrayLen; i++) {
        if(c.x != cArray[i].x && c.y != cArray[i].y)
            x++;
    }
    printf(" [x:%d snake_len:%d] ", x, cArrayLen);
    if(x >= cArrayLen)
        return 1;
    else
        return 0;
}

/*
    W - 1
    S - 2
    A - 3
    D - 4
    Q - -1
*/

void * out_f(void * data) {
    box * d = (box*)data;
    coord food, lastCoord;
    coord snake[SIDE_LENGTH*SIDE_LENGTH];
    int foodCheck;
    int tiles[SIDE_LENGTH][SIDE_LENGTH];
    d->snake_len = 0;

    //init hry
    snake[0].x = SIDE_LENGTH/2;
    snake[0].y = SIDE_LENGTH/2;
    food = getRandomCoord();

    while(d->signal == 0) {
        if(d->died == 0) {
            lastCoord = snake[d->snake_len];
            for (int i = d->snake_len+1; i >= 1; i--)
            {
                snake[i] = snake[i-1];
            }

            switch (d->key)
            {
            case 1:
                snake[0].x--;
                break;
            case 2:
                snake[0].x++;
                break;
            case 3:
                snake[0].y--;
                break;
            case 4:
                snake[0].y++;
                break;
            default:
                break;
            }
        }

        for(int i = 0; i < SIDE_LENGTH; i++) {
            for(int j = 0; j <SIDE_LENGTH; j++) {
                tiles[i][j] = 0;
            }
        }

        tiles[food.x][food.y] = 2;

        snake[0].x %= SIDE_LENGTH;
        snake[0].y %= SIDE_LENGTH;
        if(snake[0].x < 0)
            snake[0].x = SIDE_LENGTH-1;
        if(snake[0].y < 0)
            snake[0].y = SIDE_LENGTH-1;
        for (size_t i = 0; i <= d->snake_len; i++)
        {
            tiles[snake[i].x][snake[i].y] = 1;
        }
        

        if(snake[0].x == food.x && snake[0].y == food.y) {
            d->score++;
            d->snake_len++;
            snake[d->snake_len] = lastCoord;
            do {
                food = getRandomCoord();
                printf("[generated food x:%d y:%d]", food.x, food.y);
            } while (coordNotInArray(food, snake, d->snake_len) != 1);
        }

        if(d->snake_len > 0)
            for (int i = 1; i < d->snake_len; i++)
            {
                if (snake[0].x == snake[i].x && snake[0].y == snake[i].y){
                    printf("[game over] ");
                    d->died = 1;
                }                        
            }

        printf("snake{x:%d y:%d len:%d} food{x:%d y:%d}\n", snake[0].x, snake[0].y, d->snake_len, food.x, food.y);

        send(d->socket, tiles, sizeof(tiles), 0);
        send(d->socket, &d->score, sizeof(int), 0);
        microsleep(400000);
    } 
}

void * in_f(void * data) {
    box * d = (box*)data;
    char buffer[1024];
    int new_key = d->key;
    while(d->signal == 0) {
        ssize_t recv_size = recv(d->socket, buffer, 1024, 0);
        new_key = atoi(buffer);
        if(d->snake_len == 0 || (new_key == 1 && d->key != 2) || (new_key == 2 && d->key != 1) || (new_key == 3 && d->key != 4) || (new_key == 4 && d->key != 3)){
            d->key = new_key;
        }
        if(d->key == -1)
            d->signal = 1;
        if(recv_size > 0) {
            buffer[recv_size] = 0;
            printf("%s\n", buffer);
        }
    }
}

int main() {
    //premmenne
    int serverSocket, clientSocket, status, port = 0;
    
    ssize_t value;
    struct sockaddr_in serv_addr;
    struct sockaddr_storage client_addr;
    socklen_t addr_size;

    srand(time(NULL));

    box data;
    data.key = 1;
    data.signal = 0;
    data.died = 0;
    data.score = 0;

    printf("Zadajte port: ");
    scanf("%d", &port);

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    inet_pton(AF_INET, "0.0.0.0", &serv_addr.sin_addr);
    serv_addr.sin_port = htons(port);
    
    status = bind(serverSocket, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    listen(serverSocket, 1);

    addr_size = sizeof(client_addr);
    clientSocket = accept(serverSocket, (struct sockaddr *)&client_addr, &addr_size);

    pthread_t in_t, out_t;
    data.socket = clientSocket;

    pthread_create(&in_t, NULL, in_f, &data);
    pthread_create(&out_t, NULL, out_f, &data);

    pthread_join(in_t, NULL);
    pthread_join(out_t, NULL);

    close(clientSocket);
    close(serverSocket);
    return 0;
}