#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define FTP_PORT 21
#define BUFFER_SIZE 1024

// Hàm connect socket control
int connect_control_socket(const char* ip) {
    int sock;
    struct sockaddr_in server;
    char buffer[BUFFER_SIZE];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    server.sin_family = AF_INET;
    server.sin_port = htons(FTP_PORT);
    server.sin_addr.s_addr = inet_addr(ip);

    if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
        perror("Connection failed");
        exit(1);
    }

    recv(sock, buffer, sizeof(buffer), 0);
    printf("Server: %s\n", buffer);

    return sock;
}

// Hàm login
void ftp_login(int sock, const char* user, const char* pass) {
    char buffer[BUFFER_SIZE];
    char cmd[256];

    sprintf(cmd, "USER %s\r\n", user);
    send(sock, cmd, strlen(cmd), 0);
    recv(sock, buffer, sizeof(buffer), 0);
    printf("%s", buffer);

    sprintf(cmd, "PASS %s\r\n", pass);
    send(sock, cmd, strlen(cmd), 0);
    recv(sock, buffer, sizeof(buffer), 0);
    printf("%s", buffer);
}

// Hàm PASV
int ftp_pasv(int control_sock) {
    char buffer[BUFFER_SIZE];
    int ip1, ip2, ip3, ip4, p1, p2;
    struct sockaddr_in data_addr;
    int data_sock;

    send(control_sock, "PASV\r\n", 6, 0);
    recv(control_sock, buffer, sizeof(buffer), 0);
    printf("%s", buffer);

    sscanf(buffer, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)",
           &ip1, &ip2, &ip3, &ip4, &p1, &p2);

    int port = p1 * 256 + p2;

    data_sock = socket(AF_INET, SOCK_STREAM, 0);
    data_addr.sin_family = AF_INET;
    data_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    data_addr.sin_port = htons(port);

    if (connect(data_sock, (struct sockaddr*)&data_addr, sizeof(data_addr)) < 0) {
        perror("Data connection failed");
        return -1;
    }
    return data_sock;
}

// LIST
void ftp_list(int control_sock) {
    char buffer[BUFFER_SIZE];
    int data_sock = ftp_pasv(control_sock);
    if (data_sock < 0) return;

    send(control_sock, "LIST\r\n", 6, 0);
    recv(control_sock, buffer, sizeof(buffer), 0);
    printf("%s", buffer);

    printf("File list:\n");
    int n;
    while ((n = recv(data_sock, buffer, sizeof(buffer), 0)) > 0) {
        write(1, buffer, n);
    }
    close(data_sock);
    recv(control_sock, buffer, sizeof(buffer), 0);
    printf("%s", buffer);
}

// RETR
void ftp_retr(int control_sock, const char* filename) {
    char buffer[BUFFER_SIZE];
    char cmd[256];
    int data_sock = ftp_pasv(control_sock);
    if (data_sock < 0) return;

    sprintf(cmd, "RETR %s\r\n", filename);
    send(control_sock, cmd, strlen(cmd), 0);
    recv(control_sock, buffer, sizeof(buffer), 0);
    printf("%s", buffer);

    FILE* fp = fopen(filename, "w");
    int n;
    while ((n = recv(data_sock, buffer, sizeof(buffer), 0)) > 0) {
        fwrite(buffer, 1, n, fp);
    }
    fclose(fp);
    close(data_sock);
    recv(control_sock, buffer, sizeof(buffer), 0);
    printf("%s", buffer);
}

// STOR
void ftp_stor(int control_sock, const char* filename) {
    char buffer[1024];
    char cmd[256];
    int data_sock;
    struct sockaddr_in data_server;
    int ip1, ip2, ip3, ip4, port1, port2;

    // Gửi PASV để lấy data connection
    send(control_sock, "PASV\r\n", strlen("PASV\r\n"), 0);
    recv(control_sock, buffer, sizeof(buffer), 0);
    printf("%s\n", buffer);

    sscanf(buffer, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)",
           &ip1, &ip2, &ip3, &ip4, &port1, &port2);

    // Tạo data connection
    data_sock = socket(AF_INET, SOCK_STREAM, 0);
    data_server.sin_family = AF_INET;
    data_server.sin_addr.s_addr = inet_addr("127.0.0.1");
    data_server.sin_port = htons(port1 * 256 + port2);

    if (connect(data_sock, (struct sockaddr *)&data_server, sizeof(data_server)) < 0) {
        perror("Data connection failed");
        close(data_sock);
        return;
    }

    // Gửi lệnh STOR
    snprintf(cmd, sizeof(cmd), "STOR %s\r\n", filename);
    send(control_sock, cmd, strlen(cmd), 0);
    recv(control_sock, buffer, sizeof(buffer), 0);
    printf("%s\n", buffer);

    // Mở file để đọc nội dung
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("File open failed");
        close(data_sock);
        return;
    }

    // Gửi từng đoạn nội dung file qua data connection
    while (fgets(buffer, sizeof(buffer), file)) {
        send(data_sock, buffer, strlen(buffer), 0);
    }

    fclose(file);
    close(data_sock);

    // Nhận kết quả từ server
    recv(control_sock, buffer, sizeof(buffer), 0);
    printf("%s\n", buffer);
}

// DELE
void ftp_delete(int control_sock, const char* filename) {
    char buffer[BUFFER_SIZE];
    char cmd[256];

    sprintf(cmd, "DELE %s\r\n", filename);
    send(control_sock, cmd, strlen(cmd), 0);
    recv(control_sock, buffer, sizeof(buffer), 0);
    printf("%s", buffer);
}

// CWD
void ftp_cwd(int control_sock, const char* dirname) {
    char buffer[BUFFER_SIZE];
    char cmd[256];

    sprintf(cmd, "CWD %s\r\n", dirname);
    send(control_sock, cmd, strlen(cmd), 0);
    recv(control_sock, buffer, sizeof(buffer), 0);
    printf("%s", buffer);
}

// MENU
void show_menu() {
    printf("\n========== FTP Client Menu ==========\n");
    printf("1. LIST file trên server\n");
    printf("2. Tải file từ server (RETR)\n");
    printf("3. Gửi file lên server (STOR)\n");
    printf("4. Xóa file trên server (DELE)\n");
    printf("5. Đổi thư mục làm việc (CWD)\n");
    printf("0. Thoát\n");
    printf("Chọn: ");
}

int main() {
    int control_sock = connect_control_socket("127.0.0.1");
    ftp_login(control_sock, "myuser", "123456");

    int choice;
    char name[256];

    while (1) {
        show_menu();
        scanf("%d", &choice);
        getchar(); // clear newline

        switch (choice) {
            case 1:
                ftp_list(control_sock);
                break;
            case 2:
                printf("Nhập tên file cần tải: ");
                fgets(name, sizeof(name), stdin);
                name[strcspn(name, "\n")] = 0;
                ftp_retr(control_sock, name);
                break;
            case 3:
                printf("Nhập tên file cần gửi: ");
                fgets(name, sizeof(name), stdin);
                name[strcspn(name, "\n")] = 0;
                ftp_stor(control_sock, name);
                break;
            case 4:
                printf("Nhập tên file cần xóa: ");
                fgets(name, sizeof(name), stdin);
                name[strcspn(name, "\n")] = 0;
                ftp_delete(control_sock, name);
                break;
            case 5:
                printf("Nhập thư mục cần chuyển đến: ");
                fgets(name, sizeof(name), stdin);
                name[strcspn(name, "\n")] = 0;
                ftp_cwd(control_sock, name);
                break;
            case 0:
                send(control_sock, "QUIT\r\n", 6, 0);
                close(control_sock);
                exit(0);
            default:
                printf("Lựa chọn không hợp lệ!\n");
        }
    }
    return 0;
}
