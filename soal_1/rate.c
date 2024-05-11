#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SHM_SIZE 4096 // Ukuran shared memory yang diperlukan untuk menyimpan hasil CSV

struct shared_data {
    char data[SHM_SIZE];
};

int main() {
    int shmid;
    key_t key = 1234;

    // Alokasi shared memory yang sudah ada
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

    // Variabel untuk menyimpan rating tertinggi dari setiap CSV
    float max_trashcan_rating = 0.0;
    float max_parkinglot_rating = 0.0;

    // Variabel untuk menyimpan nama tempat dengan rating tertinggi dari setiap CSV
    char trashcan_best_place[256] = "";
    char parkinglot_best_place[256] = "";

    // Salin data shared memory ke dalam buffer lokal
    char buffer[SHM_SIZE];
    strcpy(buffer, shm_data->data);

    // Variabel untuk menyimpan jenis data yang sedang diproses
    char current_csv[256] = "";

    // Memisahkan data menjadi setiap baris
    char *line = strtok(buffer, "\n");
    while (line != NULL) {
        // Cek apakah baris tersebut merupakan nama file CSV
        if (strstr(line, ".csv")) {
            strcpy(current_csv, line);
        } else {
            char name[256];
            float rating;

            // Pisahkan nama tempat dan rating
            if (sscanf(line, "%[^,], %f", name, &rating) == 2) {
                // Periksa jenis CSV (Trash Can atau Parking Lot) berdasarkan nama file CSV
                if (strstr(current_csv, "trashcan")) {
                    // Periksa apakah rating saat ini lebih tinggi dari yang sebelumnya
                    if (rating > max_trashcan_rating) {
                        max_trashcan_rating = rating;
                        strcpy(trashcan_best_place, name);
                    }
                } else if (strstr(current_csv, "parkinglot")) {
                    // Periksa apakah rating saat ini lebih tinggi dari yang sebelumnya
                    if (rating > max_parkinglot_rating) {
                        max_parkinglot_rating = rating;
                        strcpy(parkinglot_best_place, name);
                    }
                }
            }
        }

        // Lanjut ke baris berikutnya
        line = strtok(NULL, "\n");
    }

    // Cetak rating tertinggi untuk setiap jenis CSV
    printf("Type: Trash Can\n");
    printf("Filename: belobog_trashcan.csv\n");
    printf("------------------------------\n");
    printf("Name: %s\n", trashcan_best_place);
    printf("Rating: %.1f\n\n", max_trashcan_rating);

    printf("Type: Parking Lot\n");
    printf("Filename: osaka_parkinglot.csv\n");
    printf("------------------------------\n");
    printf("Name: %s\n", parkinglot_best_place);
    printf("Rating: %.1f\n", max_parkinglot_rating);

    // Memutuskan shared memory
    if (shmdt(shm_data) == -1) {
        perror("shmdt");
        exit(1);
    }

    return 0;
}
