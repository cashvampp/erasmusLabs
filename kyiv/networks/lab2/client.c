#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define SERVER_IP "127.0.0.1" // Default server IP (localhost)

int main() {
    int client_socket;
    struct sockaddr_in server_addr;
    socklen_t server_addr_len = sizeof(server_addr);
    char send_buffer[BUFFER_SIZE];
    char receive_buffer[BUFFER_SIZE];

    // 1. Create socket
    client_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (client_socket < 0) {
        perror("Помилка створення сокету");
        exit(EXIT_FAILURE);
    }

    // Initialize server address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    // Convert IP address from text to binary form
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("Помилка конвертації IP-адреси");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    printf("Введіть повідомлення для відправки ('exit' або 'quit' для виходу): "
           "\n");

    while (1) {
        printf("Клієнт: ");
        fgets(send_buffer, BUFFER_SIZE, stdin);

        // Remove newline character
        size_t len = strlen(send_buffer);
        if (len > 0 && send_buffer[len - 1] == '\n') {
            send_buffer[len - 1] = '\0';
            len--;
        }

        // Check for exit commands
        if (strcmp(send_buffer, "exit") == 0 ||
            strcmp(send_buffer, "quit") == 0) {
            printf("Завершення роботи клієнта...\n");
            break;
        }

        // 3. Send data
        if (sendto(client_socket, send_buffer, len, 0,
                   (struct sockaddr *)&server_addr, server_addr_len) < 0) {
            perror("Помилка відправки даних");
            continue;
        }

        // 4. Receive data
        memset(receive_buffer, 0, BUFFER_SIZE);
        int received_bytes =
            recvfrom(client_socket, receive_buffer, BUFFER_SIZE - 1, 0,
                     (struct sockaddr *)&server_addr, &server_addr_len);

        if (received_bytes < 0) {
            perror("Помилка отримання даних");
            continue;
        }

        receive_buffer[received_bytes] = '\0';
        printf("Сервер отримав повідомлення: %s\n", receive_buffer);
    }

    // 6. Close socket
    close(client_socket);
    return 0;
}
