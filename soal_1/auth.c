#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SHM_SIZE 4096 // Ukuran shared memory yang diperlukan untuk menyimpan hasil CSV
#define FILENAME_MAX_LEN 256 // Panjang maksimum nama file

struct shared_data {
    char data[SHM_SIZE];
};

int main() {
    DIR *dir;
    struct dirent *ent;
    int shmid;
    key_t key = 1234;

    // Create shared memory
    if ((shmid = shmget(key, sizeof(struct shared_data), IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        exit(1);
    }

    // Attach shared memory
    struct shared_data *shm_data;
    if ((shm_data = (struct shared_data *)shmat(shmid, NULL, 0)) == (struct shared_data *) -1) {
        perror("shmat");
        exit(1);
    }

    // Clear shared memory
    memset(shm_data->data, 0, SHM_SIZE);

    dir = opendir("new-data");
    if (dir != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            if (ent->d_type == DT_REG) {  // Check if it's a regular file
                char filepath[512]; // Increase buffer size to 512
                char *dot = strrchr(ent->d_name, '.');
                if (dot && !strcmp(dot, ".csv")) {  // Check if it's a CSV file
                    char *underscore = strchr(ent->d_name, '_');
                    if (underscore) {
                        if (!strcmp(underscore + 1, "trashcan.csv") || !strcmp(underscore + 1, "parkinglot.csv")) {
                            // File is authenticated, open the CSV file
                            sprintf(filepath, "new-data/%s", ent->d_name);
                            FILE *csv_file = fopen(filepath, "r");
                            if (csv_file == NULL) {
                                perror("Error opening CSV file");
                                exit(1);
                            }

                            // Store filename in shared memory
                            strcat(shm_data->data, ent->d_name);
                            strcat(shm_data->data, "\n"); // Add newline separator

                            // Read content from CSV file and append it to shared memory
                            char line[256];
                            while (fgets(line, sizeof(line), csv_file) != NULL) {
                                strcat(shm_data->data, line);
                            }
			
                            // Close the CSV file
                            fclose(csv_file);
                        } else {
                            // File is not authenticated, delete it
                            sprintf(filepath, "new-data/%s", ent->d_name);
                            remove(filepath);
                        }
                    } else {
                        // File is not authenticated, delete it
                        sprintf(filepath, "new-data/%s", ent->d_name);
                        remove(filepath);
                    }
                } else {
                    // File is not a CSV, delete it
                    sprintf(filepath, "new-data/%s", ent->d_name);
                    remove(filepath);
                }
            }
        }
        closedir(dir);
    } else {
        perror("new-data");
        return EXIT_FAILURE;
    }

    // Detach shared memory
    if (shmdt(shm_data) == -1) {
        perror("shmdt");
        exit(1);
    }

    return EXIT_SUCCESS;
}

