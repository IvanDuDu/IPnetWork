#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define FTP_PORT 21

void recv_response(int sock) {
    char buffer[1024];
    int bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);
    buffer[bytes] = '\0';
    printf("Server: %s\n", buffer);
}

int open_data_connection(int control_sock) {
    char buffer[1024];
    char *pasv = "PASV\r\n";
    send(control_sock, pasv, strlen(pasv), 0);
    recv(control_sock, buffer, sizeof(buffer), 0);
    printf("Server: %s\n", buffer);

    int ip1, ip2, ip3, ip4, port1, port2;
    sscanf(buffer, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)",
           &ip1, &ip2, &ip3, &ip4, &port1, &port2);
    int data_port = port1 * 256 + port2;

    struct sockaddr_in data_addr;
    int data_sock = socket(AF_INET, SOCK_STREAM, 0);
    data_addr.sin_family = AF_INET;
    data_addr.sin_port = htons(data_port);
    data_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(data_sock, (struct sockaddr *)&data_addr, sizeof(data_addr)) < 0) {
        perror("Data connection failed");
        exit(1);
    }

    return data_sock;
}

void ftp_login(int control_sock, const char* user, const char* pass) {
    char buffer[1024];
    recv_response(control_sock);

    char cmd[256];
    sprintf(cmd, "USER %s\r\n", user);
    send(control_sock, cmd, strlen(cmd), 0);
    recv_response(control_sock);

    sprintf(cmd, "PASS %s\r\n", pass);
    send(control_sock, cmd, strlen(cmd), 0);
    recv_response(control_sock);
}

void ftp_list(int control_sock) {
    int data_sock = open_data_connection(control_sock);

    char *list_cmd = "LIST\r\n";
    send(control_sock, list_cmd, strlen(list_cmd), 0);

    char buffer[1024];
    int bytes;
    printf("File List:\n");
    while ((bytes = recv(data_sock, buffer, sizeof(buffer)-1, 0)) > 0) {
        buffer[bytes] = '\0';
        printf("%s", buffer);
    }

    close(data_sock);
    recv_response(control_sock);
}

void ftp_retr(int control_sock, const char* filename) {
    int data_sock = open_data_connection(control_sock);

    char cmd[256];
    sprintf(cmd, "RETR %s\r\n", filename);
    send(control_sock, cmd, strlen(cmd), 0);

    char buffer[1024];
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        perror("File open error");
        close(data_sock);
        return;
    }

    int bytes;
    while ((bytes = recv(data_sock, buffer, sizeof(buffer), 0)) > 0) {
        fwrite(buffer, 1, bytes, fp);
    }

    fclose(fp);
    close(data_sock);
    recv_response(control_sock);
}

void ftp_stor(int control_sock, const char* filename) {
    int data_sock = open_data_connection(control_sock);

    char cmd[256];
    sprintf(cmd, "STOR %s\r\n", filename);
    send(control_sock, cmd, strlen(cmd), 0);

    char buffer[1024];
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        perror("File open error");
        close(data_sock);
        return;
    }

    int bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        send(data_sock, buffer, bytes, 0);
    }

    fclose(fp);
    close(data_sock);
    recv_response(control_sock);
}

void ftp_delete(int control_sock, const char* filename) {
    char cmd[256];
    sprintf(cmd, "DELE %s\r\n", filename);
    send(control_sock, cmd, strlen(cmd), 0);
    recv_response(control_sock);
}

void ftp_cwd(int control_sock, const char* dirname) {
    char cmd[256];
    sprintf(cmd, "CWD %s\r\n", dirname);
    send(control_sock, cmd, strlen(cmd), 0);
    recv_response(control_sock);
}

int main() {
    int control_sock;
    struct sockaddr_in server;

    control_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (control_sock < 0) {
        perror("Control socket failed");
        exit(1);
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(FTP_PORT);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(control_sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Connection failed");
        exit(1);
    }

    ftp_login(control_sock, "myuser", "123456");

    // Test các chức năng
    ftp_list(control_sock);
    ftp_cwd(control_sock, "/subfolder");
    ftp_stor(control_sock, "upload.txt");
    ftp_retr(control_sock, "test.txt");
    ftp_delete(control_sock, "deleteme.txt");

    close(control_sock);
    return 0;
}
