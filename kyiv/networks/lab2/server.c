#include <arpa/inet.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

// Global variable for the socket
int server_socket;
bool running = true;

// Signal handler for graceful shutdown
void handle_signal(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        printf("\nОтримано сигнал завершення. Закриття сервера...\n");
        running = false;
    }
}

int main() {
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    // Set up signal handling for Ctrl+C (SIGINT)
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    // 1. Create socket
    server_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (server_socket < 0) {
        perror("Помилка створення сокету");
        exit(EXIT_FAILURE);
    }

    // Initialize server address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // Listen on any interface
    server_addr.sin_port = htons(PORT);

    // 2. Bind socket to address
    if (bind(server_socket, (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0) {
        perror("Помилка прив'язки сокету");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("UDP сервер запущено на порті: %d...\n", PORT);
    printf("Натисніть Ctrl+C для завершення роботи сервера\n");

    while (running) {
        // 4. Receive data with timeout to check for shutdown
        fd_set read_fds;
        struct timeval tv;
        FD_ZERO(&read_fds);
        FD_SET(server_socket, &read_fds);
        tv.tv_sec = 1; // 1 second timeout
        tv.tv_usec = 0;

        // Check if there's data to read or timeout
        int select_result =
            select(server_socket + 1, &read_fds, NULL, NULL, &tv);

        if (select_result == -1) {
            // Error occurred
            if (running) {
                perror("Помилка у функції select");
            }
            continue;
        } else if (select_result == 0) {
            // Timeout, check if we should still be running
            continue;
        }

        memset(buffer, 0, BUFFER_SIZE);
        int received_bytes =
            recvfrom(server_socket, buffer, BUFFER_SIZE - 1, 0,
                     (struct sockaddr *)&client_addr, &client_addr_len);

        if (received_bytes < 0) {
            perror("Помилка отримання даних");
            continue;
        }

        // Get client IP and port
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        int client_port = ntohs(client_addr.sin_port);

        printf("Отримано повідомлення: %s. Відправник: %s:%d\n", buffer,
               client_ip, client_port);

        // Send response back to client
        // 3. Send data
        if (sendto(server_socket, buffer, received_bytes, 0,
                   (struct sockaddr *)&client_addr, client_addr_len) < 0) {
            perror("Помилка відправки даних");
        }
    }

    // 6. Close socket
    printf("Закриття сокета і завершення роботи сервера...\n");
    close(server_socket);
    return 0;
}
