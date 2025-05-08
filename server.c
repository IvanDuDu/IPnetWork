#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define FTP_PORT 21

int main() {
    int sock, data_sock;
    struct sockaddr_in server, data_server;
    char buffer[1024];

    // Tạo socket cho kết nối điều khiển
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket failed");
        exit(1);
    }

    // Cấu hình server FTP
    server.sin_family = AF_INET;
    server.sin_port = htons(FTP_PORT);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Kết nối đến server
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Connection failed");
        exit(1);
    }

    // Nhận thông báo chào mừng (220)
    recv(sock, buffer, sizeof(buffer), 0);
    printf("Server: %s\n", buffer);

    // Gửi USER và PASS
    char *user = "USER user\r\n";
    send(sock, user, strlen(user), 0);
    recv(sock, buffer, sizeof(buffer), 0);
    printf("Server: %s\n", buffer);

    char *pass = "PASS pass\r\n";
    send(sock, pass, strlen(pass), 0);
    recv(sock, buffer, sizeof(buffer), 0);
    printf("Server: %s\n", buffer);

    // Gửi PASV để lấy thông tin kết nối dữ liệu
    char *pasv = "PASV\r\n";
    send(sock, pasv, strlen(pasv), 0);
    recv(sock, buffer, sizeof(buffer), 0);
    printf("Server: %s\n", buffer);

    // Phân tích phản hồi PASV
    int ip1, ip2, ip3, ip4, port1, port2;
    sscanf(buffer, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)",
           &ip1, &ip2, &ip3, &ip4, &port1, &port2);
    printf("Data connection IP: %d.%d.%d.%d, Port: %d\n", ip1, ip2, ip3, ip4, port1 * 256 + port2);

    // Tạo kết nối dữ liệu
    data_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (data_sock < 0) {
        perror("Data socket failed");
        exit(1);
    }

    // Kết nối đến cổng dữ liệu
    data_server.sin_family = AF_INET;
    data_server.sin_port = htons(port1 * 256 + port2);
    data_server.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(data_sock, (struct sockaddr *)&data_server, sizeof(data_server)) < 0) {
        perror("Data connection failed");
        exit(1);
    }

    // Gửi lệnh LIST
    char *list_cmd = "LIST\r\n";
    send(sock, list_cmd, strlen(list_cmd), 0);
    recv(data_sock, buffer, sizeof(buffer), 0);
    printf("File List:\n%s\n", buffer);

    // Đóng kết nối dữ liệu
    close(data_sock);

    // Gửi lệnh RETR để tải file
    char *retr_cmd = "RETR test.txt\r\n";
    send(sock, retr_cmd, strlen(retr_cmd), 0);
    recv(data_sock, buffer, sizeof(buffer), 0);
    printf("File Content:\n%s\n", buffer);

    // Đóng kết nối điều khiển
    close(sock);

    return 0;
}
