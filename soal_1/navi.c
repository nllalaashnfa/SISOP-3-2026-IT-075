#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include "protocol.h"

int link_g;

void on_stop(int sig) { 
    printf("\n[System] Disconnecting from The Wired...\n");
    close(link_g); _exit(0); 
}

void* listen_srv(void* arg) {
    char txt[BUFFER_SIZE];
    int link = *(int*)arg;
    while (recv(link, txt, BUFFER_SIZE, 0) > 0) {
        if (strcmp(txt, "SERVER_SHUTDOWN") == 0) {
            printf("\n[System] Server initiated shutdown. Closing connection...\n");
            _exit(0); 
        }
        printf("\n%s\n> ", txt); fflush(stdout);
        memset(txt, 0, BUFFER_SIZE);
    }
    return NULL;
}

int main() {
    signal(SIGINT, on_stop);
    int link = socket(AF_INET, SOCK_STREAM, 0); link_g = link;
    struct sockaddr_in srv_addr = {AF_INET, htons(SERVER_PORT), inet_addr(SERVER_IP)};
    if (connect(link, (struct sockaddr*)&srv_addr, sizeof(srv_addr)) < 0) return 1;

    char nama[100], txt[BUFFER_SIZE];
    while (1) {
        memset(nama, 0, 100);
        printf("Enter your name: ");
        if (!fgets(nama, 100, stdin)) break;
        nama[strcspn(nama, "\n")] = 0;

        if (strcmp(nama, ADMIN_NAME) == 0) {
            printf("Enter Password: "); 
            scanf("%s", txt); getchar();
            if (strcmp(txt, ADMIN_PASS)) { 
                printf("Wrong Password! Access Denied.\n"); 
                continue; 
            }
        }

        send(link, nama, strlen(nama), 0);
        memset(txt, 0, BUFFER_SIZE);
        if (recv(link, txt, BUFFER_SIZE, 0) <= 0) break;
        printf("%s\n", txt);
        if (strstr(txt, "Welcome")) break;
    }

    pthread_t t; pthread_create(&t, NULL, listen_srv, &link);

    while (1) {
        usleep(300000); 
        if (strcmp(nama, ADMIN_NAME) == 0) {
            printf("\n=== %s CONSOLE ===\n1. Check Active Entites (Users)\n2. Check Server Uptime\n3. Execute Emergency Shutdown\n4. Disconnect\nCommand >> ", ADMIN_NAME);
        } else {
            printf("> ");
        }

        memset(txt, 0, BUFFER_SIZE);
        if (!fgets(txt, BUFFER_SIZE, stdin)) break;
        txt[strcspn(txt, "\n")] = 0;
        if (strlen(txt) == 0) continue;

        if (strcmp(txt, "/exit") == 0 || (txt[0] == '4' && strcmp(nama, ADMIN_NAME) == 0)) {
            printf("[System] Disconnecting from The Wired...\n");
            _exit(0); 
        }

        send(link, txt, strlen(txt), 0);
        usleep(100000); 
    }
    _exit(0); 
}
