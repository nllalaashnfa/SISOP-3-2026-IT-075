#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#include "protocol.h"

// Daftar untuk menampung koneksi dan nama yang sedang aktif
int koneksi_user[MAX_CLIENTS], total_user = 0, sedang_shutdown = 0;
char nama_user[MAX_CLIENTS][100];
time_t waktu_mulai;
pthread_mutex_t kunci_server = PTHREAD_MUTEX_INITIALIZER;

void catat_log(const char* tag, const char* pesan) { FILE* f = fopen("history.log", "a");
    if(!f) return; time_t sekarang = time(NULL); char jam[25];
strftime(jam, 25, "%Y-%m-%d %H:%M:%S", localtime(&sekarang)); fprintf(f, "[%s] [%s] [%s]\n", jam, tag, pesan);
  fclose(f);
} void* tangani_client(void* arg) {
int socket_saya = (int)(long)arg; 
 char buffer[BUFFER_SIZE], nama[100], balasan[2048], pesan_log[2048];
		while (1) {memset(nama, 0, 100);
  if (recv(socket_saya, nama, 100, 0) <= 0) { close(socket_saya); return NULL; } nama[strcspn(nama, "\r\n")] = 0;

pthread_mutex_lock(&kunci_server);
 int sudah_ada = 0;
        for(int i = 0; i < total_user; i++) {
            if(strcmp(nama_user[i], nama) == 0) sudah_ada = 1;
        } if (sudah_ada) { sprintf(balasan, "[System] The identity '%s' is already synchronized in The Wired.", nama); send(socket_saya, balasan, strlen(balasan), 0); pthread_mutex_unlock(&kunci_server);
      continue; 
        }

strncpy(nama_user[total_user], nama, 99); koneksi_user[total_user++] = socket_saya; pthread_mutex_unlock(&kunci_server);
        break; 
} sprintf(balasan, "--- Welcome to The Wired, %s ---", nama); send(socket_saya, balasan, strlen(balasan), 0);
  sprintf(pesan_log, "User '%s' connected", nama); catat_log("System", pesan_log); 

    while (1) { memset(buffer, 0, BUFFER_SIZE);
 int byte_masuk = recv(socket_saya, buffer, BUFFER_SIZE, 0);
if (byte_masuk <= 0) { pthread_mutex_lock(&kunci_server);
   if (!sedang_shutdown) {
 for(int i=0; i < total_user; i++) {
if(koneksi_user[i] == socket_saya) { sprintf(pesan_log, "User '%s' disconnected", nama); catat_log("System", pesan_log); 

koneksi_user[i] = koneksi_user[total_user-1]; strcpy(nama_user[i], nama_user[total_user-1]); total_user--; break;
                    }
                }
            } pthread_mutex_unlock(&kunci_server); close(socket_saya); return NULL;
        } if (strcmp(nama, ADMIN_NAME) == 0) {
 if (buffer[0] == '1') {
char daftar[1024] = ""; int jumlah = 0;
	for(int i = 0; i < total_user; i++) {
if(strcmp(nama_user[i], ADMIN_NAME) != 0) { strcat(daftar, "\n- "); strcat(daftar, nama_user[i]);
                        jumlah++;
                    }
                } sprintf(balasan, "Active Entities: %d%s", jumlah, daftar);
 send(socket_saya, balasan, strlen(balasan), 0); catat_log("Admin", "RPC_GET_USERS");
            } else if (buffer[0] == '2') { sprintf(balasan, "Server Uptime: %ld seconds", time(NULL) - waktu_mulai);
        send(socket_saya, balasan, strlen(balasan), 0); catat_log("Admin", "RPC_GET_UPTIME");
            } else if (buffer[0] == '3') { catat_log("Admin", "RPC_SHUTDOWN");
catat_log("System", "EMERGENCY SHUTDOWN INITIATED"); pthread_mutex_lock(&kunci_server); sedang_shutdown = 1;
                

for(int i = 0; i < total_user; i++) {
 send(koneksi_user[i], "SERVER_SHUTDOWN", 15, 0); sprintf(pesan_log, "User '%s' disconnected", nama_user[i]);
  catat_log("System", pesan_log);
                } pthread_mutex_unlock(&kunci_server);
                sleep(1); _exit(0); }
        } else {
	sprintf(balasan, "[%s]: %s", nama, buffer); pthread_mutex_lock(&kunci_server);
for(int i = 0; i < total_user; i++) {
     if(koneksi_user[i] != socket_saya) send(koneksi_user[i], balasan, strlen(balasan), 0);
            } pthread_mutex_unlock(&kunci_server); sprintf(pesan_log, "[%s]: %s", nama, buffer);
   catat_log("User", pesan_log); }
    }
}

int main() 


{

waktu_mulai = time(NULL); catat_log("System", "SERVER ONLINE"); 
int socket_server = socket(AF_INET, SOCK_STREAM, 0);
int opsi = 1; setsockopt(socket_server, SOL_SOCKET, SO_REUSEADDR, &opsi, sizeof(opsi));
struct sockaddr_in alamat_server = {AF_INET, htons(SERVER_PORT), INADDR_ANY};
    
bind(socket_server, (struct sockaddr*)&alamat_server, sizeof(alamat_server)); listen(socket_server, MAX_CLIENTS);
    
	printf("Server running on port %d...\n", SERVER_PORT);
    

	while (1) {
int socket_baru = accept(socket_server, NULL, NULL);
pthread_t id_thread;
pthread_create(&id_thread, NULL, tangani_client, (void*)(long)socket_baru); pthread_detach(id_thread);
    }



















}
//:::::::)))))))))
