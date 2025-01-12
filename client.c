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
#include <poll.h>

typedef struct box {
    int clientSocket, running, paused, choice, connected, port, side_size, opened, score, died;
    pthread_t connection_t;
} box;

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
        case 'p':
            x = 10;
            break;
        default:
            x = 0;
    }
    return x;
}

int showMenu() {
    int ch = 0;
    printf("\033[H\033[J");
    printf("1 - hlavne menu\n2 - pokracovat\n3 - odist\n>");
    disableRawMode();
    scanf("%d", &ch);
    return ch;
}

void * user(void * data_) {
    box * data = (box*)data_;
    //char buffer[1024];
    int tiles[data->side_size][data->side_size];
    int time = 0, max_time = 0;
    while(data->running == 1) { 
        if(data->paused == 0) {
            printf("\033[H");                
            ssize_t recv_size = recv(data->clientSocket, tiles, sizeof(tiles), 0);
            ssize_t recv_score = recv(data->clientSocket, &data->score, sizeof(int), 0);
            ssize_t recv_died = recv(data->clientSocket, &data->died, sizeof(int), 0);
            ssize_t recv_time = recv(data->clientSocket, &time, sizeof(int), 0);
            ssize_t recv_max_time = recv(data->clientSocket, &max_time, sizeof(int), 0);
            if(recv_size > 0 && data->paused == 0) {
                for (size_t i = 0; i < data->side_size; i++)
                {
                    for (size_t j = 0; j < data->side_size; j++)
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
            if(recv_score > 0 && data->paused == 0) {
                 printf("\n[SCORE:%d]\n", data->score);
            }   
            if(recv_time > 0 && recv_max_time > 0) {
                 printf("[TIME: %d/%d]\n", time, max_time);
            }  
            if(recv_died > 0 && data->paused == 0 && data->died != 0) {
                 printf("[GAME OVER]\n");
            } 
        } else {
            sleep(1);
        }
    }
    return NULL;
}

void * server(void * data_) {
    box * data = (box*)data_;
    int x;
    char buffer[1024];

    while(data->running == 1) {
        if(data->paused == 0) {
            enableRawMode();
            x = readKeyPress();
            sprintf(buffer, "%d", x);
            disableRawMode();
            if(x == 1 || x == 2 || x== 3 || x == 4 || x == -1 || x == 10 || x == 11)
                send(data->clientSocket, buffer, strlen(buffer), 0);

            if(x == -1)
                data->running = 0;

            if(x == 10) {
                data->paused = 1;
                //data->choice = 0;
            }
        } else {
            char* unpause_str = "11";
            char* exit_str = "-1";
            int ch = showMenu();
            switch (ch)
            {
            case 2:
                data->paused = 0;
                send(data->clientSocket, unpause_str, strlen(unpause_str), 0);
                break;
            case 1:
                data->paused = 0;
                send(data->clientSocket, unpause_str, strlen(unpause_str), 0);
                send(data->clientSocket, exit_str, strlen(exit_str), 0);
                data->running = 0;
                data->choice = ch;
                break;
            case 3:
                data->paused = 0;
                send(data->clientSocket, unpause_str, strlen(unpause_str), 0);
                send(data->clientSocket, exit_str, strlen(exit_str), 0);
                data->running = 0;
                data->opened = 0;
                break;
            default:
                break;
            }
            printf("\033[H\033[J");
        }
    }
    return NULL;
}

void closeConnection(int soc) {
    char* exit_str = "-1";
    send(soc, exit_str, strlen(exit_str), 0);
    close(soc);
}

void connectTo(void* data_, char* ip) {
    box * data = (box*)data_;

    data->running = 1;
    data->score = 0;
    data->died = 0;
    data->clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(data->port);
    inet_pton(AF_INET, ip, &address.sin_addr);
    if (connect(data->clientSocket, (struct sockaddr *)&address, sizeof(address)) != -1) {
        recv(data->clientSocket, &data->side_size, sizeof(int), 0);

        pthread_t c_t, s_t;
        pthread_create(&c_t, NULL, user, data);
        pthread_create(&s_t, NULL, server, data);

        pthread_join(c_t, NULL);
        pthread_join(s_t, NULL);
    }
    
    close(data->clientSocket);
    data->choice = 0;
    return;
}

int main() {
    printf("\033[H\033[J");
    tcgetattr(cmd, &normal_termios);
    atexit(disableRawMode);

    box data;
    data.running = 1;
    data.paused = 1;
    data.choice = 0;
    data.connected = 0;
    data.opened = 1;
    int max_time, mode, side_size;    

    while(data.opened == 1) {
        switch (data.choice)
        {
        case 0:
            printf("\033[H\033[J");
            printf("1 - nova hra\n2 - pripojit\n3 - odist\n>");
            disableRawMode();
            scanf("%d", &data.choice);
            break;
        case 1:
            printf("Zadajte port: ");
            scanf("%d", &data.port);
            printf("Zadajte herny mod [0 - standard, 1 - casovany]: ");
            scanf("%d", &mode);
            if(mode == 1) {
                printf("Zadajte dlzku hry v sekundach: ");
                scanf("%d", &max_time);
            } else {
                max_time = 0;
            }
            printf("Zadajte dlzku strany mapy: ");
            scanf("%d", &side_size);
            printf("\033[H\033[J");
            pid_t pid = fork();
            if(pid < 0){
                exit(1);
            } else if (pid == 0) {
                char port_buf[50], mode_buf[50], time_buf[50], side_buf[50];
                sprintf(port_buf, "%d", data.port);
                sprintf(mode_buf, "%d", mode);
                sprintf(time_buf, "%d", max_time);
                sprintf(side_buf, "%d", side_size);
                char* args[] = {"./Server", port_buf, mode_buf, time_buf, side_buf, NULL};
                execv(args[0], args);
            }
            sleep(1);
            data.paused = 0;
            data.connected = 1;
            connectTo(&data, "0.0.0.0");
            break;
        case 2:
            char ip[20];
            printf("Zadajte ip: ");
            scanf("%s", ip); 
            printf("Zadajte port: ");
            scanf("%d", &data.port); 
            data.paused = 0;
            data.connected = 1;
            connectTo(&data, ip);
            break;
        case 3:
            data.running = 0;
            data.opened = 0;
            closeConnection(data.clientSocket);
        default:
            data.choice = 0;
            break;
        }
    }
    printf("\033[H\033[J");
    return 0;
}