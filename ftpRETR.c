#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

#define BUFFER_SIZE 8192

// Nhận một dòng phản hồi từ server
int recv_response(int sock, char *response) {
    memset(response, 0, BUFFER_SIZE);
    int n = recv(sock, response, BUFFER_SIZE - 1, 0);
    if (n <= 0) {
        perror("Recv response failed");
        return -1;
    }
    printf("%s", response);
    return 0;
}

// Gửi lệnh FTP tới server
int send_cmd(int sock, const char *cmd) {
    printf(">> %s", cmd);
    if (send(sock, cmd, strlen(cmd), 0) < 0) {
        perror("Send command failed");
        return -1;
    }
    return 0;
}

// Phân tích phản hồi PASV để lấy địa chỉ IP và port
int parse_pasv_response(const char *response, char *ip, int *port) {
    int h1, h2, h3, h4, p1, p2;
    const char *p = strchr(response, '(');
    if (!p) return -1;
    sscanf(p, "(%d,%d,%d,%d,%d,%d", &h1, &h2, &h3, &h4, &p1, &p2);
    sprintf(ip, "%d.%d.%d.%d", h1, h2, h3, h4);
    *port = p1 * 256 + p2;
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        printf("Usage: %s <server> <username> <password> <filename>\n", argv[0]);
        return 1;
    }

    const char *server = argv[1];
    const char *username = argv[2];
    const char *password = argv[3];
    const char *filename = argv[4];

    int control_sock, data_sock;
    struct sockaddr_in server_addr, data_addr;
    struct hostent *host;
    char buffer[BUFFER_SIZE];
    char ip[64];
    int port;

    // Lấy IP từ tên miền
    if ((host = gethostbyname(server)) == NULL) {
        perror("Resolve host failed");
        return 1;
    }

    // Tạo socket điều khiển
    control_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (control_sock < 0) {
        perror("Create control socket failed");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(21);
    server_addr.sin_addr = *((struct in_addr *)host->h_addr);
    memset(&(server_addr.sin_zero), 0, 8);

    // Kết nối server FTP port 21
    if (connect(control_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connect to server failed");
        close(control_sock);
        return 1;
    }

    // Nhận banner
    if (recv_response(control_sock, buffer) < 0) return 1;

    // Gửi USER
    sprintf(buffer, "USER %s\r\n", username);
    if (send_cmd(control_sock, buffer) < 0) return 1;
    if (recv_response(control_sock, buffer) < 0) return 1;
    if (strncmp(buffer, "331", 3) != 0) {
        printf("Invalid username\n");
        return 1;
    }

    // Gửi PASS
    sprintf(buffer, "PASS %s\r\n", password);
    if (send_cmd(control_sock, buffer) < 0) return 1;
    if (recv_response(control_sock, buffer) < 0) return 1;
    if (strncmp(buffer, "230", 3) != 0) {
        printf("Login failed\n");
        return 1;
    }

    // Gửi PASV
    sprintf(buffer, "PASV\r\n");
    if (send_cmd(control_sock, buffer) < 0) return 1;
    if (recv_response(control_sock, buffer) < 0) return 1;

    // Phân tích địa chỉ IP và port từ PASV response
    if (parse_pasv_response(buffer, ip, &port) < 0) {
        printf("Parse PASV response failed\n");
        return 1;
    }
    printf("Passive mode at %s:%d\n", ip, port);

    // Tạo socket data
    data_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (data_sock < 0) {
        perror("Create data socket failed");
        return 1;
    }

    data_addr.sin_family = AF_INET;
    data_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &data_addr.sin_addr);
    memset(&(data_addr.sin_zero), 0, 8);

    if (connect(data_sock, (struct sockaddr *)&data_addr, sizeof(data_addr)) < 0) {
        perror("Connect data socket failed");
        close(data_sock);
        return 1;
    }

    // Gửi RETR filename
    sprintf(buffer, "RETR %s\r\n", filename);
    if (send_cmd(control_sock, buffer) < 0) return 1;
    if (recv_response(control_sock, buffer) < 0) return 1;
    if (strncmp(buffer, "150", 3) != 0) {
        printf("File not found or cannot open file\n");
        return 1;
    }

    // Mở file local để lưu
    FILE *file = fopen(filename, "wb");
    if (!file) {
        perror("Cannot create local file");
        return 1;
    }

    // Nhận dữ liệu file
    int n;
    while ((n = recv(data_sock, buffer, BUFFER_SIZE, 0)) > 0) {
        fwrite(buffer, 1, n, file);
    }

    printf("Download completed: %s\n", filename);

    fclose(file);
    close(data_sock);

    // Nhận phản hồi kết thúc
    if (recv_response(control_sock, buffer) < 0) return 1;

    // Gửi QUIT
    sprintf(buffer, "QUIT\r\n");
    send_cmd(control_sock, buffer);
    recv_response(control_sock, buffer);

    close(control_sock);
    return 0;
}
