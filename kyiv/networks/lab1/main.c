#include <arpa/inet.h>
#include <curl/curl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LEN 1024
#define DELEGATION_FILE "delegated-ripencc-latest"

// Structure to store memory for curl response
struct MemoryStruct {
        char *memory;
        size_t size;
};

// Callback function for curl to write received data
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb,
                                  void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if (!ptr) {
        printf("Not enough memory (realloc returned NULL)\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

// Get public IP address using ipify API
char *get_public_ip() {
    CURL *curl;
    CURLcode res;
    struct MemoryStruct chunk;
    char *ip = NULL;

    chunk.memory = malloc(1);
    chunk.size = 0;

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL,
                         "https://api.ipify.org?format=json");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            fprintf(stderr, "Помилка отримання IP: %s\n",
                    curl_easy_strerror(res));
        } else {
            // Extract IP from JSON response like {"ip":"123.456.789.012"}
            char *ip_start = strstr(chunk.memory, "\"ip\":\"");
            if (ip_start) {
                ip_start += 6; // Move past "\"ip\":\""
                char *ip_end = strchr(ip_start, '"');
                if (ip_end) {
                    size_t ip_len = ip_end - ip_start;
                    ip = (char *)malloc(ip_len + 1);
                    strncpy(ip, ip_start, ip_len);
                    ip[ip_len] = '\0';
                }
            }
        }

        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    free(chunk.memory);

    return ip;
}

// Download delegation file from RIPE NCC FTP server
int download_delegation_file() {
    CURL *curl;
    CURLcode res;
    FILE *fp;

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();

    if (!curl) {
        fprintf(stderr, "Помилка ініціалізації CURL\n");
        return 0;
    }

    fp = fopen(DELEGATION_FILE, "wb");
    if (!fp) {
        fprintf(stderr, "Помилка відкриття файлу для запису\n");
        curl_easy_cleanup(curl);
        return 0;
    }

    curl_easy_setopt(
        curl, CURLOPT_URL,
        "ftp://ftp.ripe.net/pub/stats/ripencc/delegated-ripencc-latest");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

    res = curl_easy_perform(curl);

    fclose(fp);
    curl_easy_cleanup(curl);
    curl_global_cleanup();

    if (res != CURLE_OK) {
        fprintf(stderr, "Помилка завантаження файлу делегацій: %s\n",
                curl_easy_strerror(res));
        return 0;
    }

    return 1;
}

// Check if a string is a valid IPv4 address
int is_valid_ipv4(const char *ip_str) {
    struct in_addr addr;
    return inet_pton(AF_INET, ip_str, &addr) == 1;
}

// Check if IP is in the network defined by network address and mask
int is_ip_in_network(const char *ip_str, const char *network_str,
                     int mask_bits) {
    struct in_addr ip_addr, network_addr;

    if (inet_pton(AF_INET, ip_str, &ip_addr) != 1 ||
        inet_pton(AF_INET, network_str, &network_addr) != 1) {
        return 0;
    }

    // Convert to host byte order for bit operations
    uint32_t ip = ntohl(ip_addr.s_addr);
    uint32_t network = ntohl(network_addr.s_addr);

    // Create mask
    uint32_t mask = 0xFFFFFFFF << (32 - mask_bits);

    // Check if IP is in network
    return (ip & mask) == (network & mask);
}

// Find delegation for the given IP address in the delegation file
void find_delegation_for_ip(const char *ip, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Файл делегацій не знайдено.\n");
        return;
    }

    char line[MAX_LINE_LEN];
    while (fgets(line, MAX_LINE_LEN, file)) {
        // Remove newline
        char *newline = strchr(line, '\n');
        if (newline)
            *newline = '\0';

        // Split the line by | delimiter
        char *parts[10];
        int part_count = 0;
        char *token = strtok(line, "|");

        while (token && part_count < 10) {
            parts[part_count++] = token;
            token = strtok(NULL, "|");
        }

        // Check if this is an IPv4 entry
        if (part_count < 6 || strcmp(parts[2], "ipv4") != 0) {
            continue;
        }

        char *network = parts[3];
        char *size_str = parts[4];

        // Ensure network is a valid IPv4 address
        if (!is_valid_ipv4(network)) {
            continue;
        }

        // Ensure size is a number
        char *endptr;
        long size = strtol(size_str, &endptr, 10);
        if (*endptr != '\0') {
            continue;
        }

        // Check if size is a power of 2
        if ((size & (size - 1)) != 0) {
            continue;
        }

        // Calculate mask bits
        int mask_bits = 32 - ((int)log2(size));

        // Check if the IP is in this network
        if (is_ip_in_network(ip, network, mask_bits)) {
            printf("IP-адреса %s належить до делегації:\n", ip);
            printf("%s\n", line);
            fclose(file);
            return;
        }
    }

    printf("IP-адреса %s не знайдена в делегаціях.\n", ip);
    fclose(file);
}

int main() {
    printf("Отримання вашої публічної IP-адреси...\n");
    char *public_ip = get_public_ip();

    if (!public_ip) {
        printf("Не вдалося отримати IP-адресу.\n");
        return 1;
    }

    printf("Ваша публічна IP-адреса: %s\n", public_ip);

    printf("Завантаження файлу делегацій RIPE NCC...\n");
    if (!download_delegation_file()) {
        printf("Не вдалося завантажити файл делегацій.\n");
        free(public_ip);
        return 1;
    }

    printf("Пошук делегації для вашої IP-адреси...\n");
    find_delegation_for_ip(public_ip, DELEGATION_FILE);

    free(public_ip);
    return 0;
}
