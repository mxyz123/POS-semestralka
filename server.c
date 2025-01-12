#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <poll.h>
#include "microsleep.h"

typedef struct coord {
    int x;
    int y;
} coord;

coord getRandomCoord(int max) {
    coord c;
    c.x = rand() % max; 
    c.y = rand() % max;
    return c;
}

int coordNotInArray(coord c, coord* cArray, int cArrayLen) {
    int x = cArrayLen;
    for(int i = 0; i <= cArrayLen; i++) {
        if(c.x == cArray[i].x && c.y == cArray[i].y)
            x--;
    }
    if(x == cArrayLen)
        return 1;
    else
        return 0;
}

typedef struct box {
    int serverSocket, clientSocket, running, connected, paused, mode, max_time, time, side_size, exitting, snake_len, key,
    died, score;
    coord food, lastCoord;
    coord* snake;
} box;

void * user(void * data_) {
    box * data = (box*)data_;
    char buffer[1024];
    int new_key = data->key;
    while(data->running == 1) { 
        ssize_t recv_size = recv(data->clientSocket, buffer, 1024, MSG_DONTWAIT);
        if(recv_size > 0) {
            new_key = atoi(buffer);
            if(data->snake_len == 0 || (new_key == 1 && data->key != 2) || (new_key == 2 && data->key != 1) || (new_key == 3 && data->key != 4) || (new_key == 4 && data->key != 3)){
                data->key = new_key;
            }
            buffer[recv_size] = 0;
        }
        if(atoi(buffer) == -1) {
            data->running = 0;
        }
        if(atoi(buffer) == 10) {
            data->paused = 1;
        }
        if(atoi(buffer) == 11) {
            data->paused = 0;
        }
    }
    return NULL;
}

void * server(void * data_) {
    box * data = (box*)data_;
    int tiles[data->side_size][data->side_size];
    while(data->running == 1) { 
        if(data->paused != 1) {
            if(data->died == 0) {
                data->time++;
                if(data->snake_len > 0) {
                    for(int i = 1; i < data->snake_len; i++) {
                        if(data->snake[0].x == data->snake[i].x && data->snake[0].y == data->snake[i].y)
                        {
                            data->died = 1;
                        }
                    }
                }
                if(data->died == 0) {
                    data->lastCoord = data->snake[data->snake_len];
                    for (size_t i = data->snake_len+1; i >= 1 ; i--)
                    {
                        data->snake[i] = data->snake[i-1];
                    }
                    int way = data->key;
                    switch (way)
                    {
                    case 1:
                        if(way != 2)
                            data->snake[0].x--;
                        break;
                    case 2:
                        if(way!= 1)
                            data->snake[0].x++;
                        break;
                    case 3:
                        if(way != 4)
                            data->snake[0].y--;
                        break;
                    case 4:
                        if(way != 3)
                            data->snake[0].y++;
                        break;
                    default:
                        break;
                    }
                    if(data->mode == 1 && data->time/4 == data->max_time) {
                        data->died = 1;
                    }
                }
            }

            data->snake[0].x %= data->side_size;
            data->snake[0].y %= data->side_size;
            if(data->snake[0].x < 0)
                data->snake[0].x = data->side_size-1;
            if(data->snake[0].y < 0)
                data->snake[0].y = data->side_size-1;

            if (data->snake[0].x == data->food.x && data->snake[0].y == data->food.y && data->snake_len != data->side_size*data->side_size)
            {
                data->score++;
                data->snake_len++;
                data->snake[data->snake_len] = data->lastCoord;
                while (1)
                {
                    data->food = getRandomCoord(data->side_size);
                    if(coordNotInArray(data->food, data->snake, data->snake_len) == 1)
                        break;
                }
            }

            if(data->snake_len == data->side_size*data->side_size)
                data->died = 1;

            for(int i = 0; i < data->side_size; i++) {
                for(int j = 0; j < data->side_size; j++) {
                    tiles[i][j] = 0;
                }
            }
            if(data->food.x >= 0 && data->food.y >= 0)
                tiles[data->food.x][data->food.y] = 2;
            for (size_t i = 0; i < data->snake_len+1; i++)
            {
                tiles[data->snake[i].x][data->snake[i].y] = 1;
            }
            
            send(data->clientSocket, tiles, sizeof(tiles), 0);
            send(data->clientSocket, &data->score, sizeof(int), 0);
            send(data->clientSocket, &data->died, sizeof(int), 0);
            int qtime = data->time/4;
            send(data->clientSocket, &qtime, sizeof(int), 0);
            send(data->clientSocket, &data->max_time, sizeof(int), 0);
        }
        microsleep(250000);
    }
    return NULL;   
}

int main(int argc, char **argv) {
    if (argc < 5)
        return 1;
    srand(time(NULL));
    signal(SIGPIPE, SIG_IGN);
    int serverSocket, clientSocket;

    struct sockaddr_in serv_addr;
    struct sockaddr_storage client_addr;
    socklen_t addr_size;

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    inet_pton(AF_INET, "0.0.0.0", &serv_addr.sin_addr);
    serv_addr.sin_port = htons(atoi(argv[1]));
    
    bind(serverSocket, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    listen(serverSocket, 1);

    box data;
    data.connected = 1;
    data.serverSocket = serverSocket;
    data.mode = atoi(argv[2]);
    data.time = 0;
    data.max_time = atoi(argv[3]);
    data.side_size = atoi(argv[4]);
    data.exitting = 0;
    data.food = getRandomCoord(data.side_size);
    data.died = 0;
    data.score = 0;

    data.snake = calloc(data.side_size * data.side_size, sizeof(coord));
    data.snake[0].x = data.side_size/2;
    data.snake[0].y = data.side_size/2;
    data.snake_len = 0;

    data.key = 1;

    struct pollfd testPoll;
    testPoll.fd = data.serverSocket;
    testPoll.events = POLLIN | POLLPRI;

    int counter = 0;

    addr_size = sizeof(client_addr);
    while(data.connected == 1) {
        if(poll(&testPoll, 1, 1000) > 0) {
            clientSocket = accept(serverSocket, (struct sockaddr *)&client_addr, &addr_size);
            data.clientSocket = clientSocket;
            data.running = 1;
            data.paused = 0;

            send(data.clientSocket, &data.side_size, sizeof(int), 0);
            
            pthread_t c_t, s_t;
            pthread_create(&c_t, NULL, user, &data);
            pthread_create(&s_t, NULL, server, &data);
            pthread_join(c_t, NULL);
            pthread_join(s_t, NULL);
            
            close(clientSocket);
            counter = 0;
        } else {
            counter++;
        }
        if(counter >= 10) {
            data.connected = 0;
            close(clientSocket);
        }
    }
    close(serverSocket);
    free(data.snake);
    return 0;
}