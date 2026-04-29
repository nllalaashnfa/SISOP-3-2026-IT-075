#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#include "protocol.h"

int list_link[MAX_CLIENTS], total = 0, is_shutdown = 0;
char list_nama[MAX_CLIENTS][100];
time_t start;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void log_it(const char* tag, const char* msg) {
    FILE* f = fopen("history.log", "a");
    if(!f) return;
    time_t now = time(NULL);
    char jam[25];
    strftime(jam, 25, "%Y-%m-%d %H:%M:%S", localtime(&now));
    fprintf(f, "[%s] [%s] [%s]\n", jam, tag, msg);
    fclose(f);
}

void* handle(void* arg) {
    int link_user = (int)(long)arg; 
    char txt[BUFFER_SIZE], nama[100], box[2048], log_msg[2048];

    while (1) {
        memset(nama, 0, 100);
        if (recv(link_user, nama, 100, 0) <= 0) { close(link_user); return NULL; }
        nama[strcspn(nama, "\r\n")] = 0;
        pthread_mutex_lock(&lock);
        int ada = 0;
        for(int i = 0; i < total; i++) if(strcmp(list_nama[i], nama) == 0) ada = 1;
        if (ada) {
            sprintf(box, "[System] The identity '%s' is already synchronized in The Wired.", nama);
            send(link_user, box, strlen(box), 0);
            pthread_mutex_unlock(&lock); continue; 
        }
        strncpy(list_nama[total], nama, 99);
        list_link[total++] = link_user;
        pthread_mutex_unlock(&lock); break; 
    }

    sprintf(box, "--- Welcome to The Wired, %s ---", nama);
    send(link_user, box, strlen(box), 0);
    sprintf(log_msg, "User '%s' connected", nama);
    log_it("System", log_msg); 

    while (1) {
        memset(txt, 0, BUFFER_SIZE);
        int bytes = recv(link_user, txt, BUFFER_SIZE, 0);
        if (bytes <= 0) {
            pthread_mutex_lock(&lock);
            if (!is_shutdown) { // Hanya log jika bukan shutdown
                for(int i=0; i<total; i++) {
                    if(list_link[i] == link_user) {
                        sprintf(log_msg, "User '%s' disconnected", nama);
                        log_it("System", log_msg); 
                        list_link[i] = list_link[total-1];
                        strcpy(list_nama[i], list_nama[total-1]);
                        total--; break;
                    }
                }
            }
            pthread_mutex_unlock(&lock);
            close(link_user); return NULL;
        }

        if (strcmp(nama, ADMIN_NAME) == 0) {
            if (txt[0] == '1') {
                char list_user[1024] = ""; int count_biasa = 0;
                for(int i = 0; i < total; i++) {
                    if(strcmp(list_nama[i], ADMIN_NAME) != 0) {
                        strcat(list_user, "\n- "); strcat(list_user, list_nama[i]);
                        count_biasa++;
                    }
                }
                sprintf(box, "Active Entities: %d%s", count_biasa, list_user);
                send(link_user, box, strlen(box), 0);
                log_it("Admin", "RPC_GET_USERS");
            } 
            else if (txt[0] == '2') {
                sprintf(box, "Server Uptime: %ld seconds", time(NULL) - start);
                send(link_user, box, strlen(box), 0);
                log_it("Admin", "RPC_GET_UPTIME");
            } 
            else if (txt[0] == '3') {
                log_it("Admin", "RPC_SHUTDOWN");
                log_it("System", "EMERGENCY SHUTDOWN INITIATED");
                pthread_mutex_lock(&lock);
                is_shutdown = 1; // FLAG AKTIF
                for(int i = 0; i < total; i++) {
                    send(list_link[i], "SERVER_SHUTDOWN", 15, 0);
                    sprintf(log_msg, "User '%s' disconnected", list_nama[i]);
                    log_it("System", log_msg);
                }
                pthread_mutex_unlock(&lock);
                sleep(1); _exit(0); 
            }
        } else {
            sprintf(box, "[%s]: %s", nama, txt);
            pthread_mutex_lock(&lock);
            for(int i = 0; i < total; i++) if(list_link[i] != link_user) send(list_link[i], box, strlen(box), 0);
            pthread_mutex_unlock(&lock);
            sprintf(log_msg, "[%s]: %s", nama, txt);
            log_it("User", log_msg); 
        }
    }
}

int main() {
    start = time(NULL);
    log_it("System", "SERVER ONLINE"); 
    int link_srv = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1; setsockopt(link_srv, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in addr = {AF_INET, htons(SERVER_PORT), INADDR_ANY};
    bind(link_srv, (struct sockaddr*)&addr, sizeof(addr));
    listen(link_srv, MAX_CLIENTS);
    printf("Server running on port %d...\n", SERVER_PORT);
    while (1) {
        int masuk = accept(link_srv, NULL, NULL);
        pthread_t t; pthread_create(&t, NULL, handle, (void*)(long)masuk);
        pthread_detach(t);
 
   }
}
