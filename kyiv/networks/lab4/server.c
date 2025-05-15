#include <arpa/inet.h> // Added for inet_ntoa
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#define HTTP_PORT 80
#define HTTPS_PORT 443
#define BUFFER_SIZE 8192
#define MAX_PATH 256
#define MAX_CONNECTIONS 10

// Global SSL context for HTTPS
SSL_CTX *https_ctx = NULL;

// Function prototypes
void handle_sigint(int sig);
void url_decode(const char *src, char *dest);

// Function to URL decode
void url_decode(const char *src, char *dest) {
    char *dest_ptr = dest;
    char hex[3] = {0};
    while (*src) {
        if (*src == '%' && src[1] && src[2]) {
            hex[0] = src[1];
            hex[1] = src[2];
            *dest_ptr++ = (char)strtol(hex, NULL, 16);
            src += 3;
        } else if (*src == '+') {
            *dest_ptr++ = ' ';
            src++;
        } else {
            *dest_ptr++ = *src++;
        }
    }
    *dest_ptr = '\0'; // Null terminate
}

// Function to get MIME type based on file extension
const char *get_mime_type(const char *filename) {
    const char *ext = strrchr(filename, '.');
    if (!ext)
        return "application/octet-stream";

    // Convert to lowercase for case-insensitive comparison
    char ext_lower[10];
    int i = 0;
    while (ext[i] && i < 9) {
        ext_lower[i] = tolower(ext[i]);
        i++;
    }
    ext_lower[i] = '\0';

    if (strcmp(ext_lower, ".html") == 0 || strcmp(ext_lower, ".htm") == 0)
        return "text/html";
    if (strcmp(ext_lower, ".txt") == 0)
        return "text/plain";
    if (strcmp(ext_lower, ".jpg") == 0 || strcmp(ext_lower, ".jpeg") == 0)
        return "image/jpeg";
    if (strcmp(ext_lower, ".png") == 0)
        return "image/png";
    if (strcmp(ext_lower, ".gif") == 0)
        return "image/gif";
    if (strcmp(ext_lower, ".css") == 0)
        return "text/css";
    if (strcmp(ext_lower, ".js") == 0)
        return "application/javascript";
    if (strcmp(ext_lower, ".ico") == 0)
        return "image/x-icon";
    if (strcmp(ext_lower, ".svg") == 0)
        return "image/svg+xml";
    if (strcmp(ext_lower, ".pdf") == 0)
        return "application/pdf";

    return "application/octet-stream";
}

// Handler for SIGINT
void handle_sigint(int sig) {
    (void)sig; // Suppress unused parameter warning
    printf("\nShutting down server...\n");
    if (https_ctx) {
        SSL_CTX_free(https_ctx);
    }
    exit(0);
}

// Function to create a self-signed certificate
void create_self_signed_cert() {
    system("mkdir -p certs");
    system("openssl req -x509 -newkey rsa:4096 -keyout certs/key.pem -out "
           "certs/cert.pem -days 365 -nodes -subj '/CN=localhost'");
}

// Function to initialize HTTPS SSL context
SSL_CTX *init_ssl_context() {
    SSL_CTX *ctx;

    // Initialize OpenSSL
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();

    // Create SSL context
    ctx = SSL_CTX_new(TLS_server_method());
    if (!ctx) {
        perror("Error creating SSL context");
        exit(1);
    }

    // Load certificate and private key
    if (SSL_CTX_use_certificate_file(ctx, "certs/cert.pem", SSL_FILETYPE_PEM) <=
        0) {
        ERR_print_errors_fp(stderr);
        exit(1);
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, "certs/key.pem", SSL_FILETYPE_PEM) <=
        0) {
        ERR_print_errors_fp(stderr);
        exit(1);
    }

    // Verify private key
    if (!SSL_CTX_check_private_key(ctx)) {
        fprintf(stderr, "Private key does not match the certificate\n");
        exit(1);
    }

    return ctx;
}

// Function to send HTTP error response
void send_error_response(int client_socket, int error_code, SSL *ssl,
                         int is_https) {
    char response[BUFFER_SIZE];
    const char *error_message;

    switch (error_code) {
    case 404:
        error_message = "Not Found";
        break;
    case 500:
        error_message = "Internal Server Error";
        break;
    default:
        error_message = "Unknown Error";
    }

    snprintf(response, sizeof(response),
             "HTTP/1.1 %d %s\r\n"
             "Content-Type: text/html\r\n"
             "Connection: close\r\n\r\n"
             "<html><body><h1>%d %s</h1></body></html>",
             error_code, error_message, error_code, error_message);

    if (is_https) {
        SSL_write(ssl, response, strlen(response));
    } else {
        write(client_socket, response, strlen(response));
    }
}

// Improved function to send file contents using low-level file operations
int send_file(int client_socket, const char *filepath, SSL *ssl, int is_https) {
    int file_fd = open(filepath, O_RDONLY);
    if (file_fd < 0) {
        perror("Error opening file");
        send_error_response(client_socket, 404, ssl, is_https);
        return -1;
    }

    // Get file size
    struct stat file_stat;
    if (fstat(file_fd, &file_stat) < 0) {
        perror("Error getting file stats");
        close(file_fd);
        send_error_response(client_socket, 500, ssl, is_https);
        return -1;
    }

    // Prepare HTTP headers
    char headers[BUFFER_SIZE];
    const char *mime_type = get_mime_type(filepath);
    snprintf(headers, sizeof(headers),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: %s\r\n"
             "Content-Length: %ld\r\n"
             "Connection: close\r\n\r\n",
             mime_type, file_stat.st_size);

    // Send headers
    if (is_https) {
        SSL_write(ssl, headers, strlen(headers));
    } else {
        write(client_socket, headers, strlen(headers));
    }

    // Send file contents in binary mode
    unsigned char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    while ((bytes_read = read(file_fd, buffer, sizeof(buffer))) > 0) {
        if (is_https) {
            if (SSL_write(ssl, buffer, bytes_read) <= 0) {
                perror("Error writing to SSL socket");
                break;
            }
        } else {
            if (write(client_socket, buffer, bytes_read) <= 0) {
                perror("Error writing to socket");
                break;
            }
        }
    }

    close(file_fd);
    return 0;
}

// Function to handle a single client connection
void handle_client(int client_socket, SSL *ssl, int is_https) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;

    // Read request
    if (is_https) {
        bytes_received = SSL_read(ssl, buffer, sizeof(buffer) - 1);
    } else {
        bytes_received = read(client_socket, buffer, sizeof(buffer) - 1);
    }

    if (bytes_received <= 0) {
        perror("Error reading from socket");
        if (is_https) {
            SSL_shutdown(ssl);
            SSL_free(ssl);
        }
        close(client_socket);
        return;
    }

    buffer[bytes_received] = '\0';

    // Parse request line
    char method[16], uri[256], http_version[16];
    if (sscanf(buffer, "%15s %255s %15s", method, uri, http_version) != 3) {
        send_error_response(client_socket, 400, ssl, is_https);
        if (is_https) {
            SSL_shutdown(ssl);
            SSL_free(ssl);
        }
        close(client_socket);
        return;
    }

    // Print request info for debugging
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    getpeername(client_socket, (struct sockaddr *)&addr, &addr_len);
    char *client_ip = inet_ntoa(addr.sin_addr); // Fixed type issue here
    printf("[%s] %s %s %s\n", client_ip, method, uri, http_version);

    // URL decode for handling spaces and special characters
    char decoded_uri[MAX_PATH];
    url_decode(uri, decoded_uri);

    // Decode URI and create filepath
    char filepath[MAX_PATH] = "."; // Start from current directory

    // Handle root path
    if (strcmp(decoded_uri, "/") == 0) {
        strcat(filepath, "/index.html");
    } else {
        // Remove leading slash and append to path
        if (decoded_uri[0] == '/') {
            // If URI starts with /, add it properly to avoid double slashes
            strcat(filepath, decoded_uri);
        } else {
            // If URI doesn't start with /, add a slash before it
            strcat(filepath, "/");
            strcat(filepath, decoded_uri);
        }
    }

    // Print the file being accessed for debugging
    printf("Accessing file: %s\n", filepath);

    // Check if file exists and serve it
    if (access(filepath, F_OK) == 0) {
        send_file(client_socket, filepath, ssl, is_https);
    } else {
        printf("File not found: %s\n", filepath);
        send_error_response(client_socket, 404, ssl, is_https);
    }

    // Clean up HTTPS connection
    if (is_https) {
        SSL_shutdown(ssl);
        SSL_free(ssl);
    }
    close(client_socket);
}

// Function to create and bind socket
int create_server_socket(int port) {
    int server_socket;
    struct sockaddr_in server_addr;

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Error creating socket");
        exit(1);
    }

    // Allow socket reuse
    int reuse = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &reuse,
                   sizeof(reuse)) < 0) {
        perror("Error setting socket option");
        exit(1);
    }

    // Bind socket
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0) {
        perror("Error binding socket");
        exit(1);
    }

    // Listen for connections
    if (listen(server_socket, MAX_CONNECTIONS) < 0) {
        perror("Error listening");
        exit(1);
    }

    return server_socket;
}

int main() {
    int http_socket, https_socket;
    fd_set read_fds;

    // Handle Ctrl+C gracefully
    signal(SIGINT, handle_sigint);

    // Create self-signed certificate if not exists
    create_self_signed_cert();

    // Initialize HTTPS SSL context
    https_ctx = init_ssl_context();

    // Create HTTP and HTTPS sockets
    http_socket = create_server_socket(HTTP_PORT);
    https_socket = create_server_socket(HTTPS_PORT);

    printf("Web server running...\n");
    printf("HTTP server listening on port %d\n", HTTP_PORT);
    printf("HTTPS server listening on port %d\n", HTTPS_PORT);
    printf("Document root: %s\n", getcwd(NULL, 0));
    printf("Press Ctrl+C to stop server\n\n");

    // Main server loop
    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(http_socket, &read_fds);
        FD_SET(https_socket, &read_fds);
        int max_fd = (http_socket > https_socket) ? http_socket : https_socket;

        // Wait for incoming connections
        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) < 0) {
            if (errno == EINTR) {
                // Interrupted by signal, check if we should exit
                continue;
            }
            perror("Error in select");
            continue;
        }

        // Handle HTTP connection
        if (FD_ISSET(http_socket, &read_fds)) {
            struct sockaddr_in client_addr;
            socklen_t client_addr_len = sizeof(client_addr);
            int client_socket = accept(
                http_socket, (struct sockaddr *)&client_addr, &client_addr_len);

            if (client_socket < 0) {
                perror("Error accepting HTTP connection");
                continue;
            }

            handle_client(client_socket, NULL, 0);
        }

        // Handle HTTPS connection
        if (FD_ISSET(https_socket, &read_fds)) {
            struct sockaddr_in client_addr;
            socklen_t client_addr_len = sizeof(client_addr);
            int client_socket =
                accept(https_socket, (struct sockaddr *)&client_addr,
                       &client_addr_len);

            if (client_socket < 0) {
                perror("Error accepting HTTPS connection");
                continue;
            }

            // Create SSL connection
            SSL *ssl = SSL_new(https_ctx);
            SSL_set_fd(ssl, client_socket);

            // Perform SSL handshake
            if (SSL_accept(ssl) <= 0) {
                ERR_print_errors_fp(stderr);
                close(client_socket);
                SSL_free(ssl);
                continue;
            }

            handle_client(client_socket, ssl, 1);
        }
    }

    // Clean up
    close(http_socket);
    close(https_socket);
    SSL_CTX_free(https_ctx);

    return 0;
}
