#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <errno.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define NAME_SIZE 50

typedef struct {
    int socket;
    char name[NAME_SIZE];
    struct sockaddr_in address;
} ClientInfo;

void broadcast_message(int sender_fd, ClientInfo *clients, int num_clients, char *message, int include_sender) {
    char full_message[BUFFER_SIZE + NAME_SIZE];

    // Xác định tên người gửi
    char sender_name[NAME_SIZE] = "Unknown";
    for (int i = 0; i < num_clients; i++) {
        if (clients[i].socket == sender_fd) {
            strcpy(sender_name, clients[i].name);
            break;
        }
    }

    // Tạo nội dung tin nhắn
    snprintf(full_message, sizeof(full_message), "%s: %s", sender_name, message);

    // Gửi tin nhắn đến tất cả client khác
    for (int i = 0; i < num_clients; i++) {
        if (clients[i].socket > 0 && (include_sender || clients[i].socket != sender_fd)) {
            send(clients[i].socket, full_message, strlen(full_message), 0);
        }
    }
}

int main() {
    int server_fd, new_socket, max_sd, activity, i;
    struct sockaddr_in address;
    int opt = 1;
    fd_set readfds;
    char buffer[BUFFER_SIZE];
    ClientInfo clients[MAX_CLIENTS] = {0};

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // Define server address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Listening on port %d...\n", PORT);

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        max_sd = server_fd;

        for (i = 0; i < MAX_CLIENTS; i++) {
            int sd = clients[i].socket;
            if (sd > 0) FD_SET(sd, &readfds);
            if (sd > max_sd) max_sd = sd;
        }

        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR)) {
            perror("select error");
        }

        // Incoming connection
        if (FD_ISSET(server_fd, &readfds)) {
            socklen_t addrlen = sizeof(address);
            new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
            if (new_socket < 0) {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            // Tìm vị trí trống trong danh sách client
            for (i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i].socket == 0) {
                    clients[i].socket = new_socket;
                    clients[i].address = address;
                    send(new_socket, "Enter your display name: ", 25, 0);
                    read(new_socket, clients[i].name, NAME_SIZE);
                    strtok(clients[i].name, "\n"); // Xoá newline

                    printf("New client [%s] connected.\n", clients[i].name);

                    // Thông báo đến tất cả client khác
                    char join_msg[BUFFER_SIZE];
                    snprintf(join_msg, sizeof(join_msg), "%s has joined the chat.\n", clients[i].name);
                    broadcast_message(new_socket, clients, MAX_CLIENTS, join_msg, 1);
                    break;
                }
            }
        }

        // Kiểm tra tin nhắn từ client
        for (i = 0; i < MAX_CLIENTS; i++) {
            int sd = clients[i].socket;
            if (FD_ISSET(sd, &readfds)) {
                int valread = read(sd, buffer, BUFFER_SIZE);
                if (valread == 0) {
                    // Client rời đi
                    printf("Client [%s] disconnected.\n", clients[i].name);
                    char leave_msg[BUFFER_SIZE];
                    snprintf(leave_msg, sizeof(leave_msg), "%s has left the chat.\n", clients[i].name);
                    broadcast_message(sd, clients, MAX_CLIENTS, leave_msg, 1);

                    close(sd);
                    clients[i].socket = 0;
                } else {
                    buffer[valread] = '\0';
                    broadcast_message(sd, clients, MAX_CLIENTS, buffer, 0);
                }
            }
        }
    }
    return 0;
}









#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    fd_set readfds;

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Set up server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Connect to server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Connected to the server.\n");

    // Nhập tên hiển thị
    recv(sockfd, buffer, BUFFER_SIZE, 0);
    printf("%s", buffer);
    fgets(buffer, BUFFER_SIZE, stdin);
    send(sockfd, buffer, strlen(buffer), 0);

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        FD_SET(STDIN_FILENO, &readfds);

        int activity = select(sockfd + 1, &readfds, NULL, NULL, NULL);
        if (activity < 0) {
            perror("select");
            break;
        }

        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            fgets(buffer, sizeof(buffer), stdin);
            send(sockfd, buffer, strlen(buffer), 0);
        }

        if (FD_ISSET(sockfd, &readfds)) {
            int bytes_received = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
            if (bytes_received > 0) {
                buffer[bytes_received] = '\0';
                printf("%s\n", buffer);
            } else {
                printf("Server disconnected.\n");
                break;
            }
        }
    }

    close(sockfd);
    return 0;
}
