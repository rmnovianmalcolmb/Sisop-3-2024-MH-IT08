#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <time.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define FILE_PATH "../myanimelist.csv"

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

void addDataToFile(const char *data) {
    FILE *file = fopen(FILE_PATH, "a");
    if (!file) {
        perror("Failed to open file for appending");
        return;
    }
    fprintf(file, "%s\n", data);
    fclose(file);
}

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
