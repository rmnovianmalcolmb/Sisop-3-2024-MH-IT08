#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};

    // Membuat socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation error");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 addresses from text to binary form
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    // Menghubungkan socket ke alamat server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        return -1;
    }

    while (1) {
        printf("Enter command (or 'exit' to quit): ");
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = '\0'; // Menghapus newline dari input

        if (strcmp(buffer, "exit") == 0) {
            break; // Keluar jika pengguna mengetik 'exit'
        }

        // Mengirim perintah ke server
        send(sock, buffer, strlen(buffer), 0);
        printf("Command sent: %s\n", buffer);

        // Menerima respons dari server
        if ((valread = read(sock, buffer, BUFFER_SIZE - 1)) > 0) {
            buffer[valread] = '\0'; // Null-terminate the buffer
            printf("Server response: %s\n", buffer);
        }
    }

    // Menutup socket
    close(sock);
    return 0;
}
