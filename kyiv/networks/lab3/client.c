#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#define BUFFER_SIZE 65536 // 64KB initial buffer size
#define TEST_DURATION 5   // Test duration in seconds

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

int main(int argc, char *argv[]) {
    int client_socket;
    struct sockaddr_in server_addr;
    char initial_message[] = "SPEED_TEST_START";
    char buffer[BUFFER_SIZE];
    char speed_str[50];

    // Fill buffer with random data
    for (int i = 0; i < BUFFER_SIZE; i++) {
        buffer[i] = rand() % 256;
    }

    // Create socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(12345);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Connect to server
    if (connect(client_socket, (struct sockaddr *)&server_addr,
                sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    printf("З'єднано з сервером. Початок тесту швидкості...\n");

    // Send initial message to server
    send(client_socket, initial_message, strlen(initial_message), 0);

    // Wait for start signal from server
    char start_signal[20];
    recv(client_socket, start_signal, sizeof(start_signal), 0);

    // Speed test - send data
    printf("Відправка даних на сервер...\n");

    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);

    long total_bytes_sent = 0;
    time_t end_test_time = time(NULL) + TEST_DURATION;

    while (time(NULL) < end_test_time) {
        int bytes_sent = send(client_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_sent < 0) {
            perror("Помилка при відправці даних");
            break;
        }
        total_bytes_sent += bytes_sent;
    }

    gettimeofday(&end_time, NULL);

    // Calculate elapsed time in seconds
    double elapsed_time = (end_time.tv_sec - start_time.tv_sec) +
                          (end_time.tv_usec - start_time.tv_usec) / 1000000.0;

    // Calculate speed
    double bytes_per_second = total_bytes_sent / elapsed_time;
    format_speed(bytes_per_second, speed_str);

    printf("Загальний об'єм відправлених даних: %ld байт\n", total_bytes_sent);
    printf("Час відправки: %.2f секунд\n", elapsed_time);
    printf("Швидкість відправки даних: %s\n", speed_str);

    // Signal end of test
    char end_signal[] = "SPEED_TEST_END";
    send(client_socket, end_signal, strlen(end_signal), 0);

    // Receive server's measured speed
    char server_result[100];
    recv(client_socket, server_result, sizeof(server_result), 0);
    printf("Результат виміру на сервері: %s\n", server_result);

    // Close socket
    close(client_socket);

    return 0;
}
