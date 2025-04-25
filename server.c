// concurrent_server.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define PORT 8080
#define BUFFER_SIZE 1024

// Signal handler to prevent zombie processes
void sigchld_handler(int sig) {
    (void)sig; // Ignore unused parameter warning
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void handle_client(int client_sock) {
    char buffer[BUFFER_SIZE] = {0};
    int bytes_received = recv(client_sock, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received <= 0) {
        perror("recv failed");
        close(client_sock);
        return;
    }

    char method[8], uri[1024];
    sscanf(buffer, "%s %s", method, uri);
    printf("Method: %s\n", method);
    printf("URI: %s\n", uri);

    if (strcmp(method, "GET") == 0) {
        if (strcmp(uri, "/") == 0) {
            const char *body =
                "<!DOCTYPE html>\n"
                "<html>\n"
                "<head>\n"
                "<link rel=\"stylesheet\" href=\"/assets/style.css\">\n"
                "</head>\n"
                "<body>\n"
                "<h1>GET: Hello from your server!</h1>\n"
                "</body>\n"
                "</html>\n";

            int body_len = strlen(body);
            char header[256];
            snprintf(header, sizeof(header),
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/html\r\n"
                "Content-Length: %d\r\n"
                "Connection: close\r\n\r\n", body_len);

            send(client_sock, header, strlen(header), 0);
            send(client_sock, body, body_len, 0);
        }
        else if (strcmp(uri, "/assets/style.css") == 0) {
            FILE *fp = fopen("assets/style.css", "r");
            if (fp == NULL) {
                const char *not_found = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
                send(client_sock, not_found, strlen(not_found), 0);
                close(client_sock);
                exit(0);
            }

            struct stat st;
            stat("assets/style.css", &st);
            int size = st.st_size;
            char *css_content = malloc(size + 1);
            fread(css_content, 1, size, fp);
            css_content[size] = '\0';
            fclose(fp);

            char header[256];
            snprintf(header, sizeof(header),
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/css\r\n"
                "Content-Length: %d\r\n"
                "Connection: close\r\n\r\n", size);

            send(client_sock, header, strlen(header), 0);
            send(client_sock, css_content, size, 0);
            free(css_content);
        } else {
            const char *not_found = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
            send(client_sock, not_found, strlen(not_found), 0);
        }
    }

    else if (strcmp(method, "POST") == 0) {
        const char *body = "<h1>POST request received</h1>";
        int body_len = strlen(body);
        char header[256];
        snprintf(header, sizeof(header),
            "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %d\r\nConnection: close\r\n\r\n", body_len);
        send(client_sock, header, strlen(header), 0);
        send(client_sock, body, body_len, 0);
    }

    else if (strcmp(method, "PUT") == 0) {
        const char *body = "<h1>PUT request received</h1>";
        int body_len = strlen(body);
        char header[256];
        snprintf(header, sizeof(header),
            "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %d\r\nConnection: close\r\n\r\n", body_len);
        send(client_sock, header, strlen(header), 0);
        send(client_sock, body, body_len, 0);
    }

    else if (strcmp(method, "DELETE") == 0) {
        const char *body = "<h1>DELETE request received</h1>";
        int body_len = strlen(body);
        char header[256];
        snprintf(header, sizeof(header),
            "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %d\r\nConnection: close\r\n\r\n", body_len);
        send(client_sock, header, strlen(header), 0);
        send(client_sock, body, body_len, 0);
    }

    else if (strcmp(method, "HEAD") == 0) {
        const char *body = "<h1>HEAD request</h1>"; // thực ra không gửi body
        int body_len = strlen(body);
        char header[256];
        snprintf(header, sizeof(header),
            "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %d\r\nConnection: close\r\n\r\n", body_len);
        send(client_sock, header, strlen(header), 0);
    }

    else {
        const char *not_supported =
            "HTTP/1.1 405 Method Not Allowed\r\n"
            "Content-Length: 0\r\n"
            "Connection: close\r\n\r\n";
        send(client_sock, not_supported, strlen(not_supported), 0);
    }

    close(client_sock);
    exit(0);
}


int main() {
    int listenfd, connfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    pid_t pid;

    // Create the listening socket
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Setup the server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind the listening socket to the specified port
    if (bind(listenfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(listenfd);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(listenfd, 5) < 0) {
        perror("Listen failed");
        close(listenfd);
        exit(EXIT_FAILURE);
    }

    // Handle SIGCHLD to prevent zombie processes
    signal(SIGCHLD, sigchld_handler);

    printf("Server is listening on port %d...\n", PORT);

    // Server loop to accept multiple clients
    while (1) {
        // Accept an incoming connection
        connfd = accept(listenfd, (struct sockaddr *)&client_addr, &addr_len);
        if (connfd < 0) {
            perror("Accept failed");
            continue;
        }

        // Fork a child process to handle the client
        pid = fork();
        if (pid < 0) {
            perror("Fork failed");
            close(connfd);
        } else if (pid == 0) {
            // Child process: handle the client

            close(listenfd);  // Close the listening socket in the child process
            handle_client(connfd);
            close(connfd);
            exit(0);
        } else {
            // Parent process: continue accepting new clients
            close(connfd);  // Close the client socket in the parent process
        }
    }

    return 0;
}
