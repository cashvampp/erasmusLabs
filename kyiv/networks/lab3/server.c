#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#define BUFFER_SIZE 65536 // 64KB buffer size

// Function to format transfer speed into appropriate units
void format_speed(double bytes_per_second, char *output) {
    const char *units[] = {"B/s", "KB/s", "MB/s", "GB/s"};
    int unit_index = 0;

    while (bytes_per_second >= 1024.0 && unit_index < 3) {
        bytes_per_second /= 1024.0;
        unit_index++;
    }

    sprintf(output, "%.2f %s", bytes_per_second, units[unit_index]);
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    char speed_str[50];

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(12345);

    // Set socket options to reuse address
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) <
        0) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    // Bind socket to address
    if (bind(server_socket, (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0) {
        perror("Binding failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(server_socket, 1) < 0) {
        perror("Listening failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }
    printf("Чекаю на підключення...\n");

    // Accept connection from client
    client_socket =
        accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
    if (client_socket < 0) {
        perror("Accept failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }
    printf("Підключено до %s:%d\n", inet_ntoa(client_addr.sin_addr),
           ntohs(client_addr.sin_port));

    // Receive initial message from client
    char initial_message[20];
    recv(client_socket, initial_message, sizeof(initial_message), 0);

    if (strcmp(initial_message, "SPEED_TEST_START") == 0) {
        printf("Починаю тест швидкості...\n");

        // Send start signal
        char start_signal[] = "START";
        send(client_socket, start_signal, strlen(start_signal), 0);

        // Speed test - receive data
        struct timeval start_time, end_time;
        gettimeofday(&start_time, NULL);

        long total_bytes_received = 0;
        int bytes_received;
        char end_signal[20];

        while (1) {
            bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);

            if (bytes_received <= 0) {
                break;
            }

            // Check if this is the end signal
            if (bytes_received < 20) {
                buffer[bytes_received] = '\0';
                if (strcmp(buffer, "SPEED_TEST_END") == 0) {
                    break;
                }
            }

            total_bytes_received += bytes_received;
        }

        gettimeofday(&end_time, NULL);

        // Calculate elapsed time in seconds
        double elapsed_time =
            (end_time.tv_sec - start_time.tv_sec) +
            (end_time.tv_usec - start_time.tv_usec) / 1000000.0;

        // Calculate speed
        double bytes_per_second = total_bytes_received / elapsed_time;
        format_speed(bytes_per_second, speed_str);

        printf("Загальний об'єм отриманих даних: %ld байт\n",
               total_bytes_received);
        printf("Час отримання: %.2f секунд\n", elapsed_time);
        printf("Швидкість отримання даних: %s\n", speed_str);

        // Send results back to client
        char result_message[100];
        sprintf(result_message, "Швидкість отримання даних на сервері: %s",
                speed_str);
        send(client_socket, result_message, strlen(result_message), 0);
    }

    // Close sockets
    close(client_socket);
    close(server_socket);

    return 0;
}
