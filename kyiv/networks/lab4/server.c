#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define HTTP_PORT 8000
#define HTTPS_PORT 8443
#define MAX_REQUEST_SIZE 8192
#define MAX_PATH_LENGTH 1024
#define DOCUMENT_ROOT "."
#define DEFAULT_FILE "index.html"

typedef struct {
        int socket;
        SSL *ssl;
        bool use_ssl;
} client_context;

// Function to get MIME type based on file extension
const char *get_mime_type(const char *file_path) {
    const char *extension = strrchr(file_path, '.');
    if (extension == NULL) {
        return "application/octet-stream";
    }

    if (strcmp(extension, ".html") == 0) {
        return "text/html";
    } else if (strcmp(extension, ".jpg") == 0 ||
               strcmp(extension, ".jpeg") == 0) {
        return "image/jpeg";
    } else if (strcmp(extension, ".png") == 0) {
        return "image/png";
    } else if (strcmp(extension, ".css") == 0) {
        return "text/css";
    } else if (strcmp(extension, ".js") == 0) {
        return "application/javascript";
    } else {
        return "application/octet-stream";
    }
}

// Function to read or send data safely with SSL support
ssize_t safe_read(client_context *client, void *buffer, size_t len) {
    if (client->use_ssl) {
        return SSL_read(client->ssl, buffer, len);
    } else {
        return recv(client->socket, buffer, len, 0);
    }
}

ssize_t safe_write(client_context *client, const void *buffer, size_t len) {
    if (client->use_ssl) {
        return SSL_write(client->ssl, buffer, len);
    } else {
        return send(client->socket, buffer, len, 0);
    }
}

// Function to handle client connections
void *handle_client(void *arg) {
    client_context *client = (client_context *)arg;
    int client_socket = client->socket;
    char request[MAX_REQUEST_SIZE] = {0};
    char response_header[MAX_REQUEST_SIZE] = {0};
    char file_path[MAX_PATH_LENGTH] = {0};
    ssize_t bytes_read;

    // Read the HTTP request
    bytes_read = safe_read(client, request, MAX_REQUEST_SIZE - 1);
    if (bytes_read <= 0) {
        goto cleanup;
    }
    request[bytes_read] = '\0';

    printf("[Запит]\n%s\n", request);

    // Parse the request line
    char *request_line = strtok(request, "\r\n");
    if (request_line == NULL) {
        goto cleanup;
    }

    char method[10], uri[MAX_PATH_LENGTH], http_version[10];
    if (sscanf(request_line, "%9s %1023s %9s", method, uri, http_version) !=
        3) {
        goto cleanup;
    }

    // Handle root URI
    if (strcmp(uri, "/") == 0) {
        snprintf(uri, sizeof(uri), "/%s", DEFAULT_FILE);
    }

    // Construct file path
    snprintf(file_path, sizeof(file_path), "%s%s", DOCUMENT_ROOT, uri);

    // Try to open the file
    FILE *file = fopen(file_path, "rb");
    if (file) {
        // Get file size
        fseek(file, 0, SEEK_END);
        long file_size = ftell(file);
        fseek(file, 0, SEEK_SET);

        // Get content type
        const char *content_type = get_mime_type(file_path);

        // Create HTTP response header
        sprintf(response_header,
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: %s\r\n"
                "Content-Length: %ld\r\n"
                "Connection: close\r\n"
                "\r\n",
                content_type, file_size);

        // Send the header
        safe_write(client, response_header, strlen(response_header));

        // Send the file content
        char buffer[4096];
        size_t bytes_read;
        while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
            safe_write(client, buffer, bytes_read);
        }

        fclose(file);
    } else {
        // File not found - send 404
        const char *not_found = "<h1>404 Not Found</h1>";
        sprintf(response_header,
                "HTTP/1.1 404 Not Found\r\n"
                "Content-Type: text/html\r\n"
                "Content-Length: %zu\r\n"
                "Connection: close\r\n"
                "\r\n",
                strlen(not_found));

        safe_write(client, response_header, strlen(response_header));
        safe_write(client, not_found, strlen(not_found));
    }

cleanup:
    if (client->use_ssl) {
        SSL_shutdown(client->ssl);
        SSL_free(client->ssl);
    }
    close(client_socket);
    free(client);
    pthread_detach(pthread_self());
    return NULL;
}

// Initialize SSL context
SSL_CTX *create_ssl_context() {
    SSL_CTX *ctx;

    // Initialize the SSL library
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();

    // Create SSL context
    const SSL_METHOD *method = TLS_server_method();
    ctx = SSL_CTX_new(method);
    if (ctx == NULL) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    // Load certificate and private key
    if (SSL_CTX_use_certificate_file(ctx, "cert.pem", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, "key.pem", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    // Verify private key
    if (!SSL_CTX_check_private_key(ctx)) {
        fprintf(stderr, "Private key does not match the certificate\n");
        exit(EXIT_FAILURE);
    }

    return ctx;
}

// Function to start server (HTTP or HTTPS)
void *start_server(void *arg) {
    int port = *(int *)arg;
    bool use_https = (port == HTTPS_PORT);

    SSL_CTX *ssl_ctx = NULL;
    if (use_https) {
        ssl_ctx = create_ssl_context();
    }

    // Create socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) <
        0) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    // Bind socket
    if (bind(server_socket, (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0) {
        perror("Binding failed");
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(server_socket, SOMAXCONN) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("%s сервер слухає на порті %d\n", use_https ? "HTTPS" : "HTTP",
           port);

    // Accept connections in a loop
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        int client_socket =
            accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }

        // Create client context
        client_context *client = malloc(sizeof(client_context));
        if (!client) {
            close(client_socket);
            continue;
        }

        client->socket = client_socket;
        client->use_ssl = use_https;
        client->ssl = NULL;

        if (use_https) {
            // Set up SSL connection
            client->ssl = SSL_new(ssl_ctx);
            SSL_set_fd(client->ssl, client_socket);

            if (SSL_accept(client->ssl) <= 0) {
                ERR_print_errors_fp(stderr);
                SSL_free(client->ssl);
                close(client_socket);
                free(client);
                continue;
            }
        }

        // Create a thread to handle the client
        pthread_t client_thread;
        if (pthread_create(&client_thread, NULL, handle_client, client) != 0) {
            perror("Thread creation failed");
            if (use_https) {
                SSL_free(client->ssl);
            }
            close(client_socket);
            free(client);
        }
    }

    // Clean up (this code is unreachable in the current implementation)
    if (use_https) {
        SSL_CTX_free(ssl_ctx);
    }
    close(server_socket);
    return NULL;
}

int main() {
    pthread_t http_thread, https_thread;
    int http_port = HTTP_PORT;
    int https_port = HTTPS_PORT;

    // Start HTTP server in a separate thread
    if (pthread_create(&http_thread, NULL, start_server, &http_port) != 0) {
        perror("HTTP thread creation failed");
        exit(EXIT_FAILURE);
    }

    // Start HTTPS server in the main thread
    start_server(&https_port);

    return 0;
}
