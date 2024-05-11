# LAPORAN RESMI SOAL SHIFT MODUL 3 SISTEM OPERASI 2024
## ANGGOTA KELOMPOK IT08

1. Naufal Syafi' Hakim          (5027231022)
2. RM. Novian Malcolm Bayuputra (5027231035)
3. Abid Ubaidillah Adam         (5027231089)

## SOAL NOMOR 1

### auth.c

**1. Membuat deklarasi untuk ukuran shared memory, panjang maks nama file, dan struktur data yang akan disimpan di shared memory**
```c
#define SHM_SIZE 4096
#define FILENAME_MAX_LEN 256 
struct shared_data {
    char data[SHM_SIZE];
};
```

**2. Mendeklarasikan variabel-variabel yang diperlukan seperti pointer ke direktori, entri direktori, id shared memory, dan key untuk shared memory**
```c
int main() {
    DIR *dir;
    struct dirent *ent;
    int shmid;
    key_t key = 1234;
```


**3. Membuat shared memory dengan menggunakan shmget(), dengan menggunakan kunci yang telah ditentukan sebelumnya. Kemudian, shared memory tersebut diattach ke address proses menggunakan shmat() lalu membersihkan shared memory**
```c
    if ((shmid = shmget(key, sizeof(struct shared_data), IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        exit(1);
    }

    struct shared_data *shm_data;
    if ((shm_data = (struct shared_data *)shmat(shmid, NULL, 0)) == (struct shared_data *) -1) {
        perror("shmat");
        exit(1);
    }
    memset(shm_data->data, 0, SHM_SIZE);
```

**4. Membuka direktori `new-data` dan melakukan autentikasi ke file yang ada di direktori tersebut, file yang terautentikasi adalah file yang mengandung `_trashcan.csv` atau `_parkinglot.csv`, jika tidak terautentikasi maka akan dihapus**
```c
dir = opendir("new-data");
    if (dir != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            if (ent->d_type == DT_REG) { 
                char filepath[512];
                char *dot = strrchr(ent->d_name, '.');
                if (dot && !strcmp(dot, ".csv")) {  
                    char *underscore = strchr(ent->d_name, '_');
                    if (underscore) {
                        if (!strcmp(underscore + 1, "trashcan.csv") || !strcmp(underscore + 1, "parkinglot.csv")) {
                            sprintf(filepath, "new-data/%s", ent->d_name);
                            FILE *csv_file = fopen(filepath, "r");
                            if (csv_file == NULL) {
                                perror("Error opening CSV file");
                                exit(1);
                            }
                            strcat(shm_data->data, ent->d_name);
                            strcat(shm_data->data, "\n"); 
                            char line[256];
                            while (fgets(line, sizeof(line), csv_file) != NULL) {
                                strcat(shm_data->data, line);
                            }
                            fclose(csv_file);
                        } else {
                            sprintf(filepath, "new-data/%s", ent->d_name);
                            remove(filepath);
                        }
                    } else {
                        sprintf(filepath, "new-data/%s", ent->d_name);
                        remove(filepath);
                    }
                } else {
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
```

**5. Mendetach atau melepaskan shared memory**
```c
    if (shmdt(shm_data) == -1) {
        perror("shmdt");
        exit(1);
    }
    return EXIT_SUCCESS;
}
```

### rate.c

**1. Membuat deklarasi untuk ukuran shared memory, panjang maks nama file, dan struktur data yang akan disimpan di shared memory**
```c
#define SHM_SIZE 4096
struct shared_data {
    char data[SHM_SIZE];
};
```

**2. Mengalokasikan shared memory yang sudah ada dengan menggunakan key. Kemudian, attach shared memory ke address proses**
```c
int main() {
    int shmid;
    key_t key = 1234;

    if ((shmid = shmget(key, sizeof(struct shared_data), 0666)) < 0) {
        perror("shmget");
        exit(1);
    }

    struct shared_data *shm_data;
    if ((shm_data = (struct shared_data *)shmat(shmid, NULL, 0)) == (struct shared_data *) -1) {
        perror("shmat");
        exit(1);
    }
```

**3. Deklarasi variabel dan menyalin data dari shared memory**
```c

    float max_trashcan_rating = 0.0;
    float max_parkinglot_rating = 0.0;
    char trashcan_best_place[256] = "";
    char parkinglot_best_place[256] = "";

    char buffer[SHM_SIZE];
    strcpy(buffer, shm_data->data);

    char current_csv[256] = "";
```

**4. Membaca data dan menentukan nama mana yang memiliki rating tertinggi dalam setiap type file csv**
```c
    char *line = strtok(buffer, "\n");
    while (line != NULL) {
        if (strstr(line, ".csv")) {
            strcpy(current_csv, line);
        } else {
            char name[256];
            float rating;
            if (sscanf(line, "%[^,], %f", name, &rating) == 2) {
                if (strstr(current_csv, "trashcan")) {
                    if (rating > max_trashcan_rating) {
                        max_trashcan_rating = rating;
                        strcpy(trashcan_best_place, name);
                    }
                } else if (strstr(current_csv, "parkinglot")) {
                    if (rating > max_parkinglot_rating) {
                        max_parkinglot_rating = rating;
                        strcpy(parkinglot_best_place, name);
                    }
                }
            }
        }
        line = strtok(NULL, "\n");
    }
```

**5. Melakukan print nama dengan rating tertinggi dimulai dengan type Trash Can lalu dilanjutkan dengan type Parking Lot**
```c
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
```

**6. Mendetach atau melepaskan shared memory**
```c
    if (shmdt(shm_data) == -1) {
        perror("shmdt");
        exit(1);
    }
    return EXIT_SUCCESS;
}
```

### db.c

**1. Membuat deklarasi untuk ukuran shared memory, panjang maks nama file, dan struktur data yang akan disimpan di shared memory**
```c
#define SHM_SIZE 4096
struct shared_data {
    char data[SHM_SIZE];
};
```

**2. Mengalokasikan shared memory yang sudah ada dengan menggunakan key. Kemudian, attach shared memory ke address proses**
```c
int main() {
    int shmid;
    key_t key = 1234;

    if ((shmid = shmget(key, sizeof(struct shared_data), 0666)) < 0) {
        perror("shmget");
        exit(1);
    }

    struct shared_data *shm_data;
    if ((shm_data = (struct shared_data *)shmat(shmid, NULL, 0)) == (struct shared_data *) -1) {
        perror("shmat");
        exit(1);
    }
```

**3. Menyalin data dari shared memory lalu memindahkan file yang mengandung nama yang sesuai pada shared memory yaitu `_trashcan.csv` dan `_parkinglot.csv` dari direktori `new-data` ke direktori `microservices/database`, lalu menulis log pada `db.log`**
```c
    char buffer[SHM_SIZE];
    strcpy(buffer, shm_data->data);
    char *line = strtok(buffer, "\n");
    while (line != NULL) {
        char filename[256];
        sscanf(line, "%s", filename);
        char source_path[256] = "/home/ubuntu/sisop3soal1/new-data/";
        strcat(source_path, filename);
        char dest_path[256] = "/home/ubuntu/sisop3soal1/microservices/database/";
        strcat(dest_path, filename);
        if (rename(source_path, dest_path) == 0) {
            time_t now = time(NULL);
            struct tm *timeinfo = localtime(&now);
            char timestamp[20];
            strftime(timestamp, sizeof(timestamp), "%d/%m/%Y %H:%M:%S", timeinfo);
            char file_type[20];
            if (strstr(filename, "trashcan") != NULL) {
                strcpy(file_type, "Trash Can");
            } else if (strstr(filename, "parkinglot") != NULL) {
                strcpy(file_type, "Parking Lot");
            } else {
                strcpy(file_type, "Unknown");
            }
            printf("[%s] [%s] %s\n", timestamp, file_type, filename);
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
```

**4. Mendetach atau melepaskan shared memory**
```c
    if (shmdt(shm_data) == -1) {
        perror("shmdt");
        exit(1);
    }
    return EXIT_SUCCESS;
}
```

**Hasil akhir setelah menjalankan `./auth`**
![Screenshot from 2024-05-11 21-45-58](https://github.com/rmnovianmalcolmb/Sisop-3-2024-MH-IT08/assets/122516105/f860aaaf-5e14-4a66-a515-970a79751400)

**Hasil akhir setelah menjalankan `./rate`**
![image](https://github.com/rmnovianmalcolmb/Sisop-3-2024-MH-IT08/assets/122516105/1e7fa717-a559-41a7-9297-7e67ca826694)

**Hasil akhir setelah menjalankan `./db`**
![image](https://github.com/rmnovianmalcolmb/Sisop-3-2024-MH-IT08/assets/122516105/25e39614-26b8-4607-b37b-0afe3e5d39a4)

**Hasil `db.log`**

![image](https://github.com/rmnovianmalcolmb/Sisop-3-2024-MH-IT08/assets/122516105/283d672b-a888-42ed-b9c6-b46470a815d2)

## SOAL NOMOR 2

### dudududu.c

**1. Fungsi untuk merubah kata menjadi angka**
```c
int string_to_int(char *str) {
    if (strcmp(str, "nol") == 0) return 0;
    else if (strcmp(str, "satu") == 0) return 1;
    else if (strcmp(str, "dua") == 0) return 2;
    else if (strcmp(str, "tiga") == 0) return 3;
    else if (strcmp(str, "empat") == 0) return 4;
    else if (strcmp(str, "lima") == 0) return 5;
    else if (strcmp(str, "enam") == 0) return 6;
    else if (strcmp(str, "tujuh") == 0) return 7;
    else if (strcmp(str, "delapan") == 0) return 8;
    else if (strcmp(str, "sembilan") == 0) return 9;
    else return -1;
}
```

**2. Fungsi untuk merubah angka menjadi kata**
```c
void int_to_words(int num, char *words) {
    const char *ones[] = {"", "satu", "dua", "tiga", "empat", "lima", "enam", "tujuh", "delapan", "sembilan"};
    const char *tens[] = {"", "sepuluh", "dua puluh", "tiga puluh", "empat puluh", "lima puluh", "enam puluh", "tujuh puluh", "delapan puluh", "sembilan puluh"};
    const char *teens[] = {"sepuluh", "sebelas", "dua belas", "tiga belas", "empat belas", "lima belas", "enam belas", "tujuh belas", "delapan belas", "sembilan belas"};

    if (num < 10) {
        strcpy(words, ones[num]);
    } else if (num < 20) {
        strcpy(words, teens[num - 10]);
    } else {
        int digit1 = num % 10;
        int digit2 = num / 10;
        if (digit1 == 0) {
            strcpy(words, tens[digit2]);
        } else {
            sprintf(words, "%s %s", tens[digit2], ones[digit1]);
        }
    }
}
```

**3. Fungsi agar ./kalkulator hanya bisa dijalankan dengan argumen -kali, -tambah, -kurang, dan -bagi**
```c
int main(int argc, char *argv[]) {
    if (argc != 2 || (strcmp(argv[1], "-kali") != 0 && strcmp(argv[1], "-tambah") != 0 && strcmp(argv[1], "-kurang") != 0 && strcmp(argv[1], "-bagi") != 0)) {
        return 1;
    }
```

**4. Membuat parent dan child process**
```c
    int fd1[2], fd2[2];
    if (pipe(fd1) == -1 || pipe(fd2) == -1) {
        perror("Pipe failed");
        return 1;
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("Fork failed");
        return 1;
    }
```

**5. Mengambil input dan melakukan penghitungan**
```c
    if (pid > 0) { 
        close(fd1[0]); 
        close(fd2[1]); 

        char input_str1[20], input_str2[20];
        fflush(stdout);
        scanf("%s %s", input_str1, input_str2);

        int num1 = string_to_int(input_str1);
        int num2 = string_to_int(input_str2);

        if (num1 == -1 || num2 == -1) {
            printf("Invalid input\n");
            return 1;
        }

        int result;
        if (strcmp(argv[1], "-kali") == 0) {
            result = num1 * num2;
        } else if (strcmp(argv[1], "-tambah") == 0) {
            result = num1 + num2;
        } else if (strcmp(argv[1], "-kurang") == 0) {
            result = num1 - num2;
        } else { 
            result = num1 / num2;
        }
```

**6. Melakukan print output ke terminal**
```c
    write(fd1[1], &result, sizeof(result));
        close(fd1[1]);

        wait(NULL);

        char result_str[100];
        read(fd2[0], result_str, sizeof(result_str));
        if (strcmp(argv[1], "-kali") == 0) {
            printf("hasil perkalian %s dan %s adalah %s\n", input_str1,input_str2,result_str);
        } else if (strcmp(argv[1], "-tambah") == 0) {
           printf("hasil penjumlahan %s dan %s adalah %s\n", input_str1,input_str2,result_str);
        } else if (strcmp(argv[1], "-kurang") == 0) {
           printf("hasil pengurangan %s dan %s adalah %s\n", input_str1,input_str2,result_str);
        } else { 
           printf("hasil pembagian %s dan %s adalah %s\n", input_str1,input_str2,result_str);
        }
        close(fd2[0]);
```

**7. Menulis log di `histori.log`**
```c
    FILE *history = fopen("histori.log", "a");
        if (history != NULL) {
            
            char log_message[256];
            char operation_type[10];
            
            time_t current_time;
            struct tm *local_time;
            char time_str[20];

            time(&current_time);
            local_time = localtime(&current_time);
            strftime(time_str, sizeof(time_str), "%d/%m/%y %H:%M:%S", local_time);
            
            if (strcmp(argv[1], "-kali") == 0) {
                 sprintf(log_message, "[%s] [KALI] %s kali %s sama dengan %s\n", time_str, input_str1, input_str2, result_str);
            } else if (strcmp(argv[1], "-tambah") == 0) {
                sprintf(log_message, "[%s] [TAMBAH] %s tambah %s sama dengan %s\n", time_str, input_str1, input_str2, result_str);
            } else if (strcmp(argv[1], "-kurang") == 0) {
            	if(result < 0) {
            	sprintf(log_message, "[%s] [KURANG] ERROR pada pengurangan\n", time_str);
            	}
            	else {
                sprintf(log_message, "[%s] [KURANG] %s kurang %s sama dengan %s\n", time_str, input_str1, input_str2, result_str);
                }
            } else {  
                sprintf(log_message, "[%s] [BAGI] %s bagi %s sama dengan %s\n", time_str, input_str1, input_str2, result_str);
            }
            fputs(log_message, history);
            fclose(history);
        } else {
            printf("Error writing to history.log\n");
        }
```

**8. Child process untuk merubah hasil angka menjadi kata**
```c
    } else { 
        close(fd1[1]); 
        close(fd2[0]); 

        int result;
        read(fd1[0], &result, sizeof(result));

        char result_str[100];
        if (result == 0) {
            strcpy(result_str, "nol");
        } else if (result < 0) {
            strcpy(result_str, "ERROR");
        } else {
            int_to_words(result, result_str);
        }

        write(fd2[1], result_str, sizeof(result_str));
        close(fd2[1]);

        exit(0);
    }

    return 0;
}
```
**Contoh penggunaan program**
![image](https://github.com/rmnovianmalcolmb/Sisop-3-2024-MH-IT08/assets/122516105/311d81f6-0163-4632-91b6-cecf11b97e09)

**Hasil histori.log**

![image](https://github.com/rmnovianmalcolmb/Sisop-3-2024-MH-IT08/assets/122516105/2507c282-d4f8-4d4e-af5b-134f4bc2365d)



# Soal nomor 3
## actions.c

actions.c berisi fungsi-fungsi yang dapat dipanggil oleh padock.c ketika dijalankan.

```bash
#include <stdio.h>
#include <string.h>

char* handleGap(float gap) {
    if (gap < 3.5) {
        return "Gogogo";
    } else if (gap >= 3.5 && gap <= 10) {
        return "Push";
    } else {
        return "Stay out of trouble";
    }
}

char* handleFuel(float fuel) {
    if (fuel > 80) {
        return "Push Push Push";
    } else if (fuel >= 50 && fuel <= 80) {
        return "You can go";
    } else {
        return "Conserve Fuel";
    }
}

char* handleTire(int tireWear) {
    if (tireWear > 80) {
        return "Go Push Go Push";
    } else if (tireWear >= 50 && tireWear <= 80) {
        return "Good Tire Wear";
    } else if (tireWear >= 30 && tireWear < 50) {
        return "Conserve Your Tire";
    } else {
        return "Box Box Box";
    }
}

char* handleTireChange(char* tireType) {
    if (strcmp(tireType, "Soft") == 0) {
        return "Mediums Ready";
    } else if (strcmp(tireType, "Medium") == 0) {
        return "Box for Softs";
    } else {
        return "Invalid Tire Type";
    }
}
```

## Penjelasan

### 1. `handleGap(float gap)`

- Mengembalikan "Gogogo" jika gap kurang dari 3.5.
- Mengembalikan "Push" jika gap antara 3.5 dan 10.
- Mengembalikan "Stay out of trouble" jika gap lebih dari 10.

### 2. `handleFuel(float fuel)`

- Mengembalikan "Push Push Push" jika bahan bakar lebih dari 80.
- Mengembalikan "You can go" jika bahan bakar antara 50 dan 80.
- Mengembalikan "Conserve Fuel" jika bahan bakar kurang dari 50.

### 3. `handleTire(int tireWear)`

- Mengembalikan "Go Push Go Push" jika keausan ban lebih dari 80.
- Mengembalikan "Good Tire Wear" jika keausan ban antara 50 dan 80.
- Mengembalikan "Conserve Your Tire" jika keausan ban antara 30 dan 50.
- Mengembalikan "Box Box Box" jika keausan ban kurang dari 30.

### 4. `handleTireChange(char* tireType)`

- Mengembalikan "Mediums Ready" jika jenis ban "Soft".
- Mengembalikan "Box for Softs" jika jenis ban "Medium".
- Mengembalikan "Invalid Tire Type" jika jenis ban tidak valid.

## How to Use

1. Include header file `#include "actions.h"` 
2. Call salah satu function di dalamnya

## driver.c
driver.c sebagai klien yang mengirimkan command kepada paddock dan menerima respon
```bash
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>

#define MAX_COMMAND_LENGTH 256
#define PORT 8080
#define SERVER_IP "127.0.0.1"

void sendCommand(int clientSocket) {
    char command[MAX_COMMAND_LENGTH];
    char additionalInfo[MAX_COMMAND_LENGTH];

    while (1) {
        printf("Command: ");
        fgets(command, MAX_COMMAND_LENGTH, stdin);
        command[strcspn(command, "\n")] = '\0'; // Remove newline character

        if (strcmp(command, "exit") == 0) {
            break;
        }

        printf("Additional Info: ");
        fgets(additionalInfo, MAX_COMMAND_LENGTH, stdin);
        additionalInfo[strcspn(additionalInfo, "\n")] = '\0'; // Remove newline character

        // Send command and additional info
        if (send(clientSocket, command, strlen(command), 0) < 0) {
            perror("Error sending command");
            continue;
        }

        if (send(clientSocket, additionalInfo, strlen(additionalInfo), 0) < 0) {
            perror("Error sending additional info");
            continue;
        }

        // Receive response
        char response[MAX_COMMAND_LENGTH];
        memset(response, 0, sizeof(response));
        int bytesReceived = recv(clientSocket, response, sizeof(response) - 1, 0);
        if (bytesReceived <= 0) {
            if (bytesReceived == 0) {
                printf("Server closed the connection\n");
            } else {
                perror("Error receiving response");
            }
            break;
        }
        response[bytesReceived] = '\0'; // Null-terminate the received string

        printf("Response: %s\n", response);
    }
}

int main() {
    int clientSocket;
    struct sockaddr_in serverAddr;

    // Create socket
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        perror("Error creating socket");
        return 1;
    }

    // Configure server address
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    serverAddr.sin_port = htons(PORT);

    // Connect to the server
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Error connecting to server");
        return 1;
    }

    sendCommand(clientSocket);

    close(clientSocket);
    return 0;
}
```

## How to Use

Compile dan jalankan namun harus didahului program paddock
   ```bash
    ./paddock
   gcc driver.c -o driver
   ./driver
   ```
## Penjelasan
- sendCommand(int clientSocket): Function untuk mengirimkan pesan
- main(): membuat socket, connect ke server dan call sendCommand().


## paddock.c
Implementasi sebagai server dalam hal ini mengirimkan response kepada klien.

```bash
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <signal.h>
#include "actions.c"

#define MAX_COMMAND_LENGTH 256
#define PORT 8080
#define LOG_FILE "race.log"

int serverSocket;

void logMessage(char* source, char* command, char* additionalInfo) {
    time_t currentTime = time(NULL);
    struct tm* localTime = localtime(&currentTime);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%d/%m/%Y %H:%M:%S", localTime);

    FILE* logFile = fopen(LOG_FILE, "a");
    if (logFile == NULL) {
        printf("Error opening log file\n");
        return;
    }

    fprintf(logFile, "[%s] [%s]: [%s] [%s]\n", source, timestamp, command, additionalInfo);
    fclose(logFile);
}

void signalHandler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        printf("\nReceived signal %d, closing server...\n", signal);
        close(serverSocket);
        exit(0);
    }
}


void handleClient(int clientSocket) {
    char command[MAX_COMMAND_LENGTH];
    char additionalInfo[MAX_COMMAND_LENGTH];

    while (1) {
        // Receive command
        memset(command, 0, sizeof(command));
        if (recv(clientSocket, command, sizeof(command), 0) <= 0) {
            printf("Client disconnected\n");
            break;
        }

        // Receive additional info
        memset(additionalInfo, 0, sizeof(additionalInfo));
        if (recv(clientSocket, additionalInfo, sizeof(additionalInfo), 0) <= 0) {
            printf("Error receiving additional info\n");
            break;
        }

        char* response = NULL;
        if (strcmp(command, "Gap") == 0) {
            float gap = atof(additionalInfo);
            response = handleGap(gap);
        } else if (strcmp(command, "Fuel") == 0) {
            float fuel = atof(additionalInfo);
            response = handleFuel(fuel);
        } else if (strcmp(command, "Tire") == 0) {
            int tireWear = atoi(additionalInfo);
            response = handleTire(tireWear);
        } else if (strcmp(command, "Tire Change") == 0) {
            response = handleTireChange(additionalInfo);
        } else {
            response = "Invalid command";
        }

        // Send response
        send(clientSocket, response, strlen(response), 0);
        logMessage("Driver", command, additionalInfo);
        logMessage("Paddock", command, response);
    }

    close(clientSocket);
}

int main() {
    pid_t pid, sid;
    
     signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    // Fork the parent process
    pid = fork();
    if (pid < 0) {
        fprintf(stderr, "Error forking process\n");
        exit(EXIT_FAILURE);
    }

    // If we got a good PID, then we can exit the parent process
    if (pid > 0) {
        printf("Daemon process started with PID %d\n", pid);
        exit(EXIT_SUCCESS);
    }

    // Create a new session and process group
    sid = setsid();
    if (sid < 0) {
        fprintf(stderr, "Error creating new session\n");
        exit(EXIT_FAILURE);
    }

    // Change the current working directory
    chdir("/");

    // Close stdin, stdout, and stderr
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t addrSize;

    // Create socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        fprintf(stderr, "Error creating socket\n");
        exit(1);
    }

    // Configure server address
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    // Bind the socket to the server address
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        fprintf(stderr, "Error binding socket\n");
        exit(1);
    }

    // Listen for incoming connections
    if (listen(serverSocket, 5) < 0) {
        fprintf(stderr, "Error listening for connections\n");
        exit(1);
    }

    while (1) {
        addrSize = sizeof(clientAddr);
        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &addrSize);
        if (clientSocket < 0) {
            fprintf(stderr, "Error accepting connection\n");
            continue;
        }

        // Fork a child process to handle the client
        pid_t pid = fork();
        if (pid == 0) {
            // Child process
            close(serverSocket); // Child doesn't need the listener
            handleClient(clientSocket);
            close(clientSocket);
            exit(0);
        } else if (pid < 0) {
            fprintf(stderr, "Error forking child process\n");
        } else {
            // Parent process
            close(clientSocket); // Parent doesn't need this
        }
    }

    close(serverSocket);
    return 0;
}
```

## Penjelasan
- logMessage(char* source, char* command, char* additionalInfo): Function untuk membuat log file.
- signalHandler(int signal): Signal handler function untuk menghandle SIGINT and SIGTERM signals ketika terjadi signal shutdown
- handleClient(int clientSocket): Function untuk menghandle client connections, menerima commands, memproses, and mengirim responses.

## Problem
ketika program dijalankan race.log atau logfile masih belum bisa dibuat dan response dari paddock.c ini terkadang jadi terkadang tidak sehingga agak membingungkan

## SOAL NOMOR 4

### client.c

```c
#define PORT 8080
#define BUFFER_SIZE 1024
```
Ini adalah bagian define yang menentukan konstanta PORT dan BUFFER_SIZE yang akan digunakan dalam program.

```c
int main() {
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};
```
Ini adalah awal dari fungsi main(). Variabel sock akan menyimpan deskriptor socket, valread akan menyimpan jumlah byte yang dibaca dari socket, serv_addr adalah struktur yang menyimpan informasi alamat server, dan buffer adalah array untuk menyimpan data yang dikirim dan diterima melalui socket.

```c
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation error");
        return -1;
    }
```
Bagian ini membuat socket baru menggunakan fungsi socket(). Jika terjadi kesalahan dalam pembuatan socket, pesan error akan dicetak dan program akan keluar.

```c
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
```
Bagian ini mengisi struktur serv_addr dengan informasi alamat server. sin_family diatur ke AF_INET untuk menunjukkan bahwa ini adalah alamat IPv4, dan sin_port diatur ke htons(PORT) yang mengubah nilai PORT ke format jaringan.

```c
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
```
Bagian ini mengonversi alamat IP "127.0.0.1" (localhost) dari bentuk teks ke bentuk biner menggunakan fungsi inet_pton() dan menyimpannya dalam serv_addr.sin_addr. Jika terjadi kesalahan dalam konversi, pesan error akan dicetak dan program akan keluar.

```c
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        return -1;
    }
```
Bagian ini mencoba menghubungkan socket ke alamat server menggunakan fungsi connect(). Jika terjadi kesalahan dalam koneksi, pesan error akan dicetak dan program akan keluar.

```c
    while (1) {
        printf("Enter command (or 'exit' to quit): ");
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = '\0'; // Menghapus newline dari input
        
        if (strcmp(buffer, "exit") == 0) {
            break; // Keluar jika pengguna mengetik 'exit'
        }

        send(sock, buffer, strlen(buffer), 0);
        printf("Command sent: %s\n", buffer);

        if ((valread = read(sock, buffer, BUFFER_SIZE - 1)) > 0) {
            buffer[valread] = '\0'; // Null-terminate the buffer
            printf("Server response: %s\n", buffer);
        }
    }
```
Bagian ini adalah loop utama program. Dalam loop ini, program akan meminta pengguna untuk memasukkan perintah. Jika pengguna memasukkan "exit", loop akan diakhiri. Jika bukan, perintah akan dikirim ke server menggunakan fungsi send(). Kemudian, program akan menunggu respons dari server menggunakan fungsi read() dan mencetak respons tersebut.

```c
    close(sock);
    return 0;
}
```
Setelah keluar dari loop, socket akan ditutup menggunakan fungsi close(), dan program akan keluar dengan mengembalikan nilai 0.

### server.c

```c
#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define FILE_PATH "../myanimelist.csv"
```
Ini adalah bagian define yang menentukan konstanta PORT (port server), MAX_CLIENTS (jumlah maksimum client yang dapat terhubung), BUFFER_SIZE (ukuran buffer untuk menerima dan mengirim data), dan FILE_PATH (lokasi file yang menyimpan data anime).

```c
void logChange(const char *type, const char *message) {
    FILE *file = fopen("../change.log", "a");
    if (!file) {
        perror("Failed to open log file");
        return;
    }
    time_t now = time(NULL);
    char *timestamp = ctime(&now);
    timestamp[strlen(timestamp) - 1] = '\0';  // Remove newline character
    fprintf(file, "[%s] [%s] %s\n", timestamp, type, message);
    fclose(file);
}
```
Fungsi logChange digunakan untuk mencatat perubahan (penambahan atau penghapusan data) ke dalam file log change.log. Fungsi ini membuka file log dalam mode "a" (append), mendapatkan waktu saat ini menggunakan time() dan ctime(), lalu menulis log dengan format [timestamp] [type] message ke dalam file log.

```c
void addDataToFile(const char *data) {
    FILE *file = fopen(FILE_PATH, "a");
    if (!file) {
        perror("Failed to open file for appending");
        return;
    }
    fprintf(file, "%s\n", data);
    fclose(file);
}
```
Fungsi addDataToFile digunakan untuk menambahkan data baru ke file myanimelist.csv. Fungsi ini membuka file dalam mode "a" (append), menulis data baru ke dalam file menggunakan fprintf, lalu menutup file.

```c
void deleteDataFromFile(const char *title) {
    FILE *file = fopen(FILE_PATH, "r");
    if (!file) {
        perror("Failed to open file for reading");
        return;
    }
    FILE *temp = fopen("temp.csv", "w");
    if (!temp) {
        fclose(file);
        perror("Failed to open temp file for writing");
        return;
    }

    char line[BUFFER_SIZE];
    int found = 0;
    while (fgets(line, BUFFER_SIZE, file)) {
        line[strcspn(line, "\n")] = '\0'; // Remove newline
        if (strstr(line, title) == NULL) {
            fprintf(temp, "%s\n", line);
        } else {
            found = 1;
        }
    }

    fclose(file);
    fclose(temp);

    if (found) {
        rename("temp.csv", FILE_PATH); // Replace old file with new temp file
        logChange("DEL", title);
    } else {
        remove("temp.csv"); // Remove temp file if no data found
    }
}
```
Fungsi deleteDataFromFile digunakan untuk menghapus data dari file myanimelist.csv berdasarkan judul anime yang diberikan. Fungsi ini membuka file dalam mode "r" (read), membuat file sementara temp.csv dalam mode "w" (write), membaca setiap baris dari file dan menulis ke file sementara jika baris tersebut tidak mengandung judul anime yang ingin dihapus. Setelah semua baris diproses, file sementara menggantikan file asli jika data ditemukan dan dihapus, atau file sementara dihapus jika tidak ada data yang dihapus. Fungsi logChange dipanggil untuk mencatat penghapusan data ke dalam file log.

```c
void filterDataAndSend(int clientSocket, const char *filterCriteria, int columnIndex) {
    FILE *file = fopen(FILE_PATH, "r");
    if (!file) {
        send(clientSocket, "Failed to open file.\n", 21, 0);
        return;
    }

    char line[BUFFER_SIZE];
    char *token;
    char response[BUFFER_SIZE] = {0};
    int found = 0;

    while (fgets(line, sizeof(line), file) != NULL) {
        char *tmp = strdup(line);
        token = strtok(tmp, ",");
        for (int i = 0; i < columnIndex; ++i) {
            if (token != NULL) {
                token = strtok(NULL, ",");
            }
        }

        if (token && strstr(token, filterCriteria) != NULL) {
            send(clientSocket, line, strlen(line), 0);
            found = 1;
        }
        free(tmp);
    }
    fclose(file);

    if (!found) {
        strcpy(response, "No data found for the criteria.\n");
        send(clientSocket, response, strlen(response), 0);
    } else {
        strcpy(response, "END_OF_DATA\n");
        send(clientSocket, response, strlen(response), 0);
    }
}
```
Fungsi filterDataAndSend digunakan untuk mengirimkan data dari file myanimelist.csv ke client berdasarkan kriteria filter yang diberikan. Fungsi ini membuka file dalam mode "r" (read), lalu membaca setiap baris dari file. Untuk setiap baris, fungsi ini memisahkan baris menjadi token menggunakan strtok dengan pemisah ,. Kemudian, fungsi ini memeriksa token pada indeks kolom yang sesuai dengan columnIndex (0 untuk hari, 1 untuk genre, 3 untuk status) dan membandingkan dengan filterCriteria. Jika sesuai, baris tersebut dikirim ke client menggunakan send. Setelah semua baris diproses, jika tidak ada data yang ditemukan, pesan "No data found for the criteria" dikirim ke client. Jika ada data yang ditemukan, pesan "END_OF_DATA" dikirim ke client untuk menandakan akhir dari data.

```c
void handleCommand(int clientSocket, char *command) {
    char response[1024] = {0};

    if (strncmp(command, "ADD", 3) == 0) {
        addDataToFile(command + 4);  // Command is expected like "ADD Day,Genre,Title,Status"
        sprintf(response, "Data successfully added: %s", command + 4);
        logChange("ADD", command + 4);
    } else if (strncmp(command, "DEL", 3) == 0) {
        deleteDataFromFile(command + 4);
        sprintf(response, "Data successfully deleted if existing: %s", command + 4);
    } else if (strncmp(command, "SHOW_ALL", 8) == 0) {
        filterDataAndSend(clientSocket, "", 0);  // No filter, show all
    } else if (strncmp(command, "SHOW_GENRES ", 12) == 0) {
        filterDataAndSend(clientSocket, command + 12, 1);  // Filter by genre
    } else if (strncmp(command, "SHOW_DAY ", 9) == 0) {
        filterDataAndSend(clientSocket, command + 9, 0);  // Filter by day
    } else if (strncmp(command, "SHOW_STATUS ", 12) == 0) {
        filterDataAndSend(clientSocket, command + 12, 3);  // Filter by status
    } else {
        strcpy(response, "Invalid Command");
        send(clientSocket, response, strlen(response), 0);
    }
}
```
Fungsi handleCommand digunakan untuk memproses perintah yang diterima dari client. Fungsi ini memeriksa perintah yang diterima dan melakukan tindakan yang sesuai berdasarkan perintah tersebut.
Jika perintah dimulai dengan "ADD", fungsi addDataToFile dipanggil dengan data yang diberikan (diharapkan dalam format "ADD Day,Genre,Title,Status"). Kemudian, respons "Data successfully added" dikirim ke client, dan fungsi logChange dipanggil untuk mencatat penambahan data ke dalam file log.
Jika perintah dimulai dengan "DEL", fungsi deleteDataFromFile dipanggil dengan judul anime yang ingin dihapus. Kemudian, respons "Data successfully deleted if existing" dikirim ke client.
Jika perintah adalah "SHOW_ALL", fungsi filterDataAndSend dipanggil dengan kriteria filter kosong dan columnIndex 0 untuk menampilkan semua data.
Jika perintah dimulai dengan "SHOW_GENRES ", fungsi filterDataAndSend dipanggil dengan kriteria filter yang diberikan setelah "SHOW_GENRES " dan columnIndex 1 untuk menampilkan data berdasarkan genre.
Jika perintah dimulai dengan "SHOW_DAY ", fungsi filterDataAndSend dipanggil dengan kriteria filter yang diberikan setelah "SHOW_DAY " dan columnIndex 0 untuk menampilkan data berdasarkan hari.
Jika perintah dimulai dengan "SHOW_STATUS ", fungsi filterDataAndSend dipanggil dengan kriteria filter yang diberikan setelah "SHOW_STATUS " dan columnIndex 3 untuk menampilkan data berdasarkan status.
Jika perintah tidak valid, respons "Invalid Command" dikirim ke client.

```c
int main() {
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrLen;
    char buffer[BUFFER_SIZE];

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(serverSocket, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d\n", PORT);
```
Ini adalah fungsi main yang merupakan titik masuk utama program. Pada bagian ini, program membuat socket server menggunakan socket(), mengikatnya ke alamat dan port tertentu menggunakan bind(), dan mendengarkan koneksi masuk menggunakan listen(). Jika terjadi kesalahan pada setiap langkah, program akan keluar dengan kode error.

```c
 while (1) {
        clientAddrLen = sizeof(clientAddr);
        clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
        if (clientSocket < 0) {
            perror("Accept failed");
            continue;
        }

        printf("New client connected\n");

        while (1) {
            memset(buffer, 0, BUFFER_SIZE);
            int bytesRead = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
            if (bytesRead <= 0) {
                break;
            }

            buffer[bytesRead] = '\0';
            printf("Received command: %s\n", buffer);

            handleCommand(clientSocket, buffer);

            if (strcmp(buffer, "exit") == 0) {
                break;
            }
        }

        close(clientSocket);
        printf("Client disconnected\n");
    }

    close(serverSocket);
    return 0;
}
```
Pada loop utama, program menerima koneksi dari client menggunakan accept(). Setelah terhubung dengan client, program memasuki loop internal untuk membaca perintah dari client menggunakan recv(). Setiap perintah yang diterima diproses oleh fungsi handleCommand(). Jika perintah exit diterima, loop internal keluar dan koneksi dengan client ditutup menggunakan close(). Setelah semua client terputus, socket server ditutup dan program keluar.
Secara keseluruhan, program ini menyediakan server yang dapat menerima koneksi dari beberapa client sekaligus dan memungkinkan client untuk mengelola daftar anime dengan menambahkan, menghapus, dan menampilkan data yang disimpan dalam file myanimelist.csv.
