# Soal nomor 3
## actions.c

This repository contains a set of functions to assist with racing strategy decisions based on various factors such as gap to leader, fuel level, tire wear, and tire type.

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

## Fungsi Functions

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

1. Include the header file `#include "racing_strategy.h"` in your C program.
2. Call the appropriate function with the relevant parameters to get the racing strategy advice.

## driver.c
This C program demonstrates a simple client-server communication using sockets. The client sends commands to the server and receives responses.
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

Compile the program using a C compiler (e.g., gcc) dan run it.
   ```bash
   gcc client.c -o client
   ./client
   ```
## Code Overview
sendCommand(int clientSocket): Function to send commands and additional information to the server and receive responses.
main(): Entry point of the program. Creates a socket, connects to the server, and calls sendCommand().
## Requirements
C compiler (e.g., gcc)
Standard C libraries (stdio.h, stdlib.h, string.h, unistd.h, arpa/inet.h, sys/socket.h, errno.h)
## Configuration
Server IP: 127.0.0.1
Server Port: 8080
Maximum Command Length: 256
## Notes
This client program assumes a server is running locally on IP address 127.0.0.1 and port 8080. Modify the SERVER_IP and PORT constants in the code to match your server configuration.
Ensure the server is running and ready to accept connections before running the client.

