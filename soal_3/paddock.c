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

// creating a logfile
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

// if we kill the process we print it
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

        // Fork a child to handle the client
        pid_t pid = fork();
        if (pid == 0) {
            // Child process
            close(serverSocket); 
            handleClient(clientSocket);
            close(clientSocket);
            exit(0);
        } else if (pid < 0) {
            fprintf(stderr, "Error forking child process\n");
        } else {
          // parent process
            close(clientSocket); 
        }
    }

    close(serverSocket);
    return 0;
}
