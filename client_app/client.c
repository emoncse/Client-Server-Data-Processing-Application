#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080
#define BUFFER_SIZE 1024

void send_data_to_server();

int main() {
    FILE *log_file = freopen("logs/client.log", "w", stdout);
    if (!log_file) {
        perror("Failed to open log file");
        exit(EXIT_FAILURE);
    }

    send_data_to_server();
    return 0;
}

void send_data_to_server() {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("Invalid server address");
        close(sock);
        exit(EXIT_FAILURE);
    }

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection to server failed");
        close(sock);
        exit(EXIT_FAILURE);
    }

    printf("Connected to server.\n");

    FILE *file = fopen("users.csv", "r");
    if (!file) {
        perror("Failed to open CSV file");
        close(sock);
        exit(EXIT_FAILURE);
    }

    while (fgets(buffer, sizeof(buffer), file)) {
        buffer[strcspn(buffer, "\r\n")] = '\0';

        ssize_t bytes_sent = send(sock, buffer, strlen(buffer), 0);
        if (bytes_sent < 0) {
            perror("Failed to send data");
            fclose(file);
            close(sock);
            exit(EXIT_FAILURE);
        }
        printf("Sent: %s\n", buffer);

        memset(buffer, 0, BUFFER_SIZE);
        ssize_t bytes_received = recv(sock, buffer, BUFFER_SIZE, 0);
        if (bytes_received < 0) {
            perror("Failed to receive data");
            break;
        } else if (bytes_received == 0) {
            printf("Server closed connection\n");
            break;
        }
        printf("Server Response: %s\n", buffer);
    }

    fclose(file);
    close(sock);
}
