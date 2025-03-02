#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <mysql/mysql.h>
#include "db_handler.h"

#define PORT 8080
#define BUFFER_SIZE 1024
#define DB_HOST "localhost"
#define DB_USER "imran"
#define DB_NAME "user_data"
#define DB_PASS "Emon@123"

void start_server();
void handle_client(int client_socket);
void receive_and_process_data(int client_socket);
void store_data_in_db(int client_socket, const char *data);
void send_acknowledgment(int client_socket, const char *message);

int main() {
    freopen("logs/server.log", "a", stdout);
    start_server();
    return 0;
}

void start_server() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        printf("Socket creation failed\n");
        fflush(stdout);
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        printf("Socket bind failed\n");
        fflush(stdout);
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) < 0) {
        printf("Socket listen failed\n");
        fflush(stdout);
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port 8080...\n");
    fflush(stdout);

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            printf("Client connection failed\n");
            fflush(stdout);
            continue;
        }

        printf("New client connected\n");
        fflush(stdout);
        handle_client(client_fd);
    }
}

void handle_client(int client_socket) {
    receive_and_process_data(client_socket);
    close(client_socket);
    printf("Client disconnected\n");
    fflush(stdout);
}

void receive_and_process_data(int client_socket) {
    char buffer[BUFFER_SIZE];
    int bytes_received;

    while ((bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[bytes_received] = '\0';
        printf("Received data: %s\n", buffer);
        fflush(stdout);

        store_data_in_db(client_socket, buffer);
    }
}

void store_data_in_db(int client_socket, const char *data) {
    char data_copy[BUFFER_SIZE];
    strncpy(data_copy, data, BUFFER_SIZE);
    data_copy[BUFFER_SIZE - 1] = '\0';

    char *tokens[5];
    char *token = strtok(data_copy, ",");
    int i = 0;

    while (token != NULL && i < 5) {
        tokens[i++] = token;
        token = strtok(NULL, ",");
    }

    if (i < 5) {
        printf("Invalid data format: %s\n", data);
        fflush(stdout);
        send(client_socket, "Invalid data format\n", 20, 0);
        return;
    }

    int id = atoi(tokens[0]);
    char *first_name = tokens[1];
    char *last_name = tokens[2];
    char *email = tokens[3];
    char *city = tokens[4];

    printf("Parsed Data - ID: %d, First: %s, Last: %s, Email: %s, City: %s\n", 
            id, first_name, last_name, email, city);
    fflush(stdout);

    MYSQL *conn = connect_to_database();
    if (!conn) {
        send(client_socket, "Database connection failed\n", 28, 0);
        return;
    }

    execute_insert_query(conn, id, first_name, last_name, email, city, client_socket);
    mysql_close(conn);
}

void send_acknowledgment(int client_socket, const char *message) {
    send(client_socket, message, strlen(message), 0);
}

MYSQL *connect_to_database() {
    freopen("logs/server.log", "a", stdout);
    MYSQL *conn = mysql_init(NULL);
    if (!conn) {
        printf("MySQL initialization failed\n");
        fflush(stdout);
        return NULL;
    }

    if (!mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, 0, NULL, 0)) {
        printf("MySQL connection failed: %s\n", mysql_error(conn));
        fflush(stdout);
        mysql_close(conn);
        return NULL;
    }

    printf("Connected to MySQL successfully\n");
    fflush(stdout);
    return conn;
}

int execute_insert_query(MYSQL *conn, int id, const char *first_name, const char *last_name, const char *email, const char *city, int client_fd) {
    char query[512];

    snprintf(query, sizeof(query),
        "INSERT INTO users (id, first_name, last_name, email, city) VALUES (%d, '%s', '%s', '%s', '%s') "
        "ON DUPLICATE KEY UPDATE first_name=VALUES(first_name), last_name=VALUES(last_name), email=VALUES(email), city=VALUES(city)",
        id, first_name, last_name, email, city);

    if (mysql_query(conn, query)) {
        printf("MySQL query failed: %s\n", mysql_error(conn));
        fflush(stdout);
        send(client_fd, "Database error\n", 15, 0);
        return -1;
    }

    if (mysql_affected_rows(conn) == 0) {
        printf("Already in DB - ID: %d, Name: %s %s, Email: %s, City: %s\n", id, first_name, last_name, email, city);
        fflush(stdout);
        send(client_fd, "Already in DB\n", 15, 0);
    } else {
        printf("Stored successfully - ID: %d, Name: %s %s, Email: %s, City: %s\n", id, first_name, last_name, email, city);
        fflush(stdout);
        send(client_fd, "Stored successfully\n", 25, 0);
    }

    return 0;
}
