#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 9090
#define BUFFER_SIZE 1024
#define MAX_CONTACTS 100

struct contact {
    char username[50];
    char phone[15];
};
typedef struct contact Contact;

Contact contactList[MAX_CONTACTS];
int contactCount = 0;

// Hàm kiểm tra số điện thoại hợp lệ
int isValidPhone(const char* phone) {
    int len = strlen(phone);
    if (len < 10 || len > 11) return 0;
    for (int i = 0; i < len; i++) {
        if (phone[i] < '0' || phone[i] > '9') return 0;
    }
    return 1;
}

// Đọc danh bạ từ file vào mảng contactList
int readContacts(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Cannot open contacts file");
        return -1;
    }

    char line[100];
    contactCount = 0;
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;
        char* token = strtok(line, ":");
        if (token) {
            strcpy(contactList[contactCount].username, token);
            token = strtok(NULL, ":");
            if (token) {
                strcpy(contactList[contactCount].phone, token);
                contactCount++;
            }
        }
    }

    fclose(file);
    return 1;
}

// Ghi lại contactList vào file
int writeContacts(const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        perror("Cannot open contacts file for writing");
        return -1;
    }

    for (int i = 0; i < contactCount; i++) {
        fprintf(file, "%s:%s\n", contactList[i].username, contactList[i].phone);
    }

    fclose(file);
    return 1;
}

// Xử lý yêu cầu client
void handle_client(int connfd) {
    char buffer[BUFFER_SIZE];
    int n;

    // Nhận dữ liệu
    n = recv(connfd, buffer, BUFFER_SIZE, 0);
    if (n <= 0) {
        perror("recv failed");
        close(connfd);
        return;
    }
    buffer[n] = '\0';

    printf("Client: %s\n", buffer);

    // Tách dữ liệu username|old_phone|new_phone
    char* username = strtok(buffer, "|");
    char* old_phone = strtok(NULL, "|");
    char* new_phone = strtok(NULL, "|");

    // Kiểm tra cú pháp
    if (!username || !old_phone || !new_phone || strtok(NULL, "|") != NULL ||
        !isValidPhone(old_phone) || !isValidPhone(new_phone)) {
        strcpy(buffer, "ERROR Cú pháp không hợp lệ");
        send(connfd, buffer, strlen(buffer), 0);
        close(connfd);
        return;
    }

    // Tìm username
    int found = 0;
    for (int i = 0; i < contactCount; i++) {
        if (strcmp(contactList[i].username, username) == 0) {
            found = 1;
            if (strcmp(contactList[i].phone, old_phone) == 0) {
                // Cập nhật số mới
                strcpy(contactList[i].phone, new_phone);
                writeContacts("contacts.txt");
                strcpy(buffer, "OK Cập nhật thành công");
            } else {
                strcpy(buffer, "ERROR Số điện thoại cũ không đúng");
            }
            break;
        }
    }

    if (!found) {
        strcpy(buffer, "ERROR Tài khoản không tồn tại");
    }

    send(connfd, buffer, strlen(buffer), 0);
    close(connfd);
}

int main() {
    if (readContacts("contacts.txt") < 0) {
        return 1;
    }

    int listenfd, connfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    // Tạo socket
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Thiết lập địa chỉ server
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind socket
    if (bind(listenfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(listenfd);
        exit(EXIT_FAILURE);
    }

    // Lắng nghe kết nối
    if (listen(listenfd, 5) < 0) {
        perror("Listen failed");
        close(listenfd);
        exit(EXIT_FAILURE);
    }

    printf("Phone update server is listening on port %d...\n", PORT);

    // Vòng lặp nhận client tuần tự
    while (1) {
        connfd = accept(listenfd, (struct sockaddr*)&client_addr, &addr_len);
        if (connfd < 0) {
            perror("Accept failed");
            continue;
        }

        // Xử lý client
        handle_client(connfd);
    }

    close(listenfd);
    return 0;
}
