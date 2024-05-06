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
        command[strcspn(command, "\n")] = '\0'; // Remove newline 

        if (strcmp(command, "exit") == 0) {
            break;
        }

        printf("Additional Info: ");
        fgets(additionalInfo, MAX_COMMAND_LENGTH, stdin);
        additionalInfo[strcspn(additionalInfo, "\n")] = '\0'; // Remove newline 

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
