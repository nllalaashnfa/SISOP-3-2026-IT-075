#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include "protocol.h"



int socket_global;
void tangani_keluar(int sig) 

{ 
    

printf("\n[System] Disconnecting from The Wired...\n"); close(socket_global); _exit(0); }


void* dengarkan_pesan(void* arg) {
 char buffer[BUFFER_SIZE];
  int koneksi_ke_server = *(int*)arg;
    

while (recv(koneksi_ke_server, buffer, BUFFER_SIZE, 0) > 0) {
        
	if (strcmp(buffer, "SERVER_SHUTDOWN") == 0) { printf("\n[System] Server initiated shutdown. Closing connection...\n");
            _exit(0); 
        } printf("\n%s\n> ", buffer); fflush(stdout); memset(buffer, 0, BUFFER_SIZE);
    } return NULL;
}

int main() 

{ signal(SIGINT, tangani_keluar);
 int koneksi_ke_server = socket(AF_INET, SOCK_STREAM, 0); socket_global = koneksi_ke_server;
struct sockaddr_in alamat_server = 
	{AF_INET, htons(SERVER_PORT), inet_addr(SERVER_IP)};
    
if (connect(koneksi_ke_server, (struct sockaddr*)&alamat_server, sizeof(alamat_server)) < 0) return 1;


char nama_saya[100], input_user[BUFFER_SIZE];
    
	while (1) { memset(nama_saya, 0, 100);
 printf("Enter your name: ");
        
if (!fgets(nama_saya, 100, stdin)) break; nama_saya[strcspn(nama_saya, "\n")] = 0;

	if (strcmp(nama_saya, ADMIN_NAME) == 0) { printf("Enter Password: "); scanf("%s", input_user); getchar();
            
  if (strcmp(input_user, ADMIN_PASS)) { 
printf("Wrong Password! Access Denied.\n"); continue; 
            }
        } send(koneksi_ke_server, nama_saya, strlen(nama_saya), 0);
    memset(input_user, 0, BUFFER_SIZE);
        

 if (recv(koneksi_ke_server, input_user, BUFFER_SIZE, 0) <= 0) break;
	printf("%s\n", input_user);
        

if (strstr(input_user, "Welcome")) break;
    } pthread_t id_thread; pthread_create(&id_thread, NULL, dengarkan_pesan, &koneksi_ke_server);


    while (1) { usleep(300000); 
        
if (strcmp(nama_saya, ADMIN_NAME) == 0) {
   printf("\n=== %s CONSOLE ===\n1. Check Active Entites (Users)\n2. Check Server Uptime\n3. Execute Emergency Shutdown\n4. Disconnect\nCommand >> ", ADMIN_NAME);
        } else { printf("> ");
        } memset(input_user, 0, BUFFER_SIZE);
        

if (!fgets(input_user, BUFFER_SIZE, stdin)) break; input_user[strcspn(input_user, "\n")] = 0;
 if (strlen(input_user) == 0) continue;
  if (strcmp(input_user, "/exit") == 0 || (input_user[0] == '4' && strcmp(nama_saya, ADMIN_NAME) == 0)) {
	printf("[System] Disconnecting from The Wired...\n");
            _exit(0); 
        } send(koneksi_ke_server, input_user, strlen(input_user), 0); usleep(100000); 
    }
    _exit(0); 


























}
//:::::::::::))))))))))))))))))
