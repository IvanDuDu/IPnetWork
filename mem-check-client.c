#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define SERVER_IP "127.0.0.1"
#define PORT 8080
#define BUFFER_SIZE 1024
#define XOR_KEY 0x5A  // Khóa XOR để mã hóa và giải mã

void xor_cipher(char *data) {
    for (int i = 0; data[i] != '\0'; i++) {
        data[i] ^= XOR_KEY;
    }
}

int main() {
    int sockfd;
    struct sockaddr_in server_addr, sender_addr;
    char buffer[BUFFER_SIZE];
    socklen_t addr_len = sizeof(server_addr);
    socklen_t sender_addr_len = sizeof(sender_addr);

    // Tạo socket UDP
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    while (1) {
        // Nhập thông điệp từ người dùng
        printf("Nhập thông điệp gửi đến server: ");
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = 0; // Loại bỏ ký tự newline

        if( *buffer==' ') break;

        // Gửi thông điệp đến server
        sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)&server_addr, addr_len);
        printf("Đã gửi: %s\n", buffer);

        // Sử dụng select() để xử lý timeout
        fd_set readfds;
        struct timeval timeout;
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;

        int activity = select(sockfd + 1, &readfds, NULL, NULL, &timeout);
        if (activity == -1) {
            perror("Lỗi select()");
            close(sockfd);
            exit(EXIT_FAILURE);
        } else if (activity == 0) {
            printf("Không nhận được phản hồi từ server trong 5 giây.\n");
            continue;
        }

        // Nhận dữ liệu từ server
        int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&sender_addr, &sender_addr_len);
        if (n < 0) {
            perror("recvfrom failed");
            close(sockfd);
            exit(EXIT_FAILURE);
        }
        buffer[n] = '\0';

        // In địa chỉ IP của máy gửi
        char sender_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &sender_addr.sin_addr, sender_ip, INET_ADDRSTRLEN);
        printf("Nhận dữ liệu từ IP: %s\n", sender_ip);

        // Xác minh danh tính server bằng memcmp()
        struct sockaddr_in expected_addr;
        memset(&expected_addr, 0, sizeof(expected_addr));
        expected_addr.sin_family = AF_INET;
        expected_addr.sin_port = htons(PORT);
        inet_pton(AF_INET, SERVER_IP, &expected_addr.sin_addr);
        
        if (memcmp(&server_addr, &expected_addr, sizeof(struct sockaddr_in)) != 0) {
            printf("Cảnh báo: Phản hồi không đến từ server mong đợi!\n");
            continue;
        }

        // Giải mã dữ liệu nhận được
        xor_cipher(buffer);
        printf("Dữ liệu sau khi giải mã: %s\n", buffer);



    }

    close(sockfd);
    return 0;
}