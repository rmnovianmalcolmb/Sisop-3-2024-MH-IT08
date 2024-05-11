
# LAPORAN RESMI SOAL SHIFT MODUL 1 SISTEM OPERASI 2024
## ANGGOTA KELOMPOK IT08

1. Naufal Syafi' Hakim          (5027231022)
2. RM. Novian Malcolm Bayuputra (5027231035)
3. Abid Ubaidillah Adam         (5027231089)

## SOAL NOMOR 1

### auth.c


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

## How to Use

1. Compile
   ```bash
   gcc paddock.c -o paddock
   ```
2.  Run 
   ```bash
./paddock
```
## Penjelasan
- logMessage(char* source, char* command, char* additionalInfo): Function untuk membuat log file.
- signalHandler(int signal): Signal handler function untuk menghandle SIGINT and SIGTERM signals ketika terjadi signal shutdown
- handleClient(int clientSocket): Function untuk menghandle client connections, menerima commands, memproses, and mengirim responses.

## Problem
ketika program dijalankan race.log atau logfile masih belum bisa dibuat dan response dari paddock.c ini terkadang jadi terkadang tidak sehingga agak membingungkan


