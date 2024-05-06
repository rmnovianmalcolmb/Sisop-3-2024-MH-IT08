#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <time.h>

#define SHM_SIZE 4096 // Ukuran shared memory yang diperlukan untuk menyimpan informasi CSV

struct shared_data {
    char data[SHM_SIZE];
};

int main() {
    int shmid;
    key_t key = 1234;

    // Alokasi shared memory
    if ((shmid = shmget(key, sizeof(struct shared_data), 0666)) < 0) {
        perror("shmget");
        exit(1);
    }

    // Mengaitkan shared memory ke ruang alamat proses
    struct shared_data *shm_data;
    if ((shm_data = (struct shared_data *)shmat(shmid, NULL, 0)) == (struct shared_data *) -1) {
        perror("shmat");
        exit(1);
    }

    // Salin data shared memory ke dalam buffer lokal
    char buffer[SHM_SIZE];
    strcpy(buffer, shm_data->data);

    // Memindahkan file yang sesuai dengan nama dari shared memory
    char *line = strtok(buffer, "\n");
    while (line != NULL) {
        char filename[256];
        sscanf(line, "%s", filename);
        // Membuat path lengkap file di new-data
        char source_path[256] = "/home/ubuntu/sisop3soal1/new-data/";
        strcat(source_path, filename);
        // Membuat path lengkap file di microservices/database
        char dest_path[256] = "/home/ubuntu/sisop3soal1/microservices/database/";
        strcat(dest_path, filename);
        // Memindahkan file
        if (rename(source_path, dest_path) == 0) {
            // Mencatat log
            time_t now = time(NULL);
            struct tm *timeinfo = localtime(&now);
            char timestamp[20];
            strftime(timestamp, sizeof(timestamp), "%d/%m/%Y %H:%M:%S", timeinfo);
            // Mendapatkan jenis file
            char file_type[20];
            if (strstr(filename, "trashcan") != NULL) {
                strcpy(file_type, "Trash Can");
            } else if (strstr(filename, "parkinglot") != NULL) {
                strcpy(file_type, "Parking Lot");
            } else {
                strcpy(file_type, "Unknown");
            }
            printf("[%s] [%s] %s\n", timestamp, file_type, filename);
            
            // Menulis log ke db.log
            FILE *log_file = fopen("/home/ubuntu/sisop3soal1/microservices/database/db.log", "a");
            if (log_file != NULL) {
                fprintf(log_file, "[%s] [%s] [%s]\n", timestamp, file_type, filename);
                fclose(log_file);
            } else {
                perror("fopen");
            }
        } else {
            perror("rename");
        }
        line = strtok(NULL, "\n");
    }

    // Memutuskan shared memory
    if (shmdt(shm_data) == -1) {
        perror("shmdt");
        exit(1);
    }

    return 0;
}

