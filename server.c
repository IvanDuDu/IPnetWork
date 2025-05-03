#include <stdio.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

int main() {
    int listener, client;
    fd_set read_fds;
    struct timeval timeout;

    struct sockaddr_in server_addr, client_addr;
    socklen_t addrlen = sizeof(client_addr);

    // Tạo socket listener
    listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener < 0) {
        perror("Listener socket failed");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(8888);

    if (bind(listener, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        return 1;
    }

    listen(listener, 5);
    printf("Dang lang nghe tren cong 8888...\n");

    // Tạo client socket và kết nối tới listener
    client = socket(AF_INET, SOCK_STREAM, 0);
    if (client < 0) {
        perror("Client socket failed");
        return 1;
    }

    if (connect(client, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Client connect failed");
        return 1;
    }
    printf("Client da ket noi\n");

    // Server accept kết nối từ client
    int connfd = accept(listener, (struct sockaddr*)&client_addr, &addrlen);
    if (connfd < 0) {
        perror("Accept failed");
        return 1;
    }
    printf("Server da chap nhan ket noi\n");

    // Đặt thời gian timeout cho select
    timeout.tv_sec = 3;
    timeout.tv_usec = 0;

    // Khởi tạo fd_set
    FD_ZERO(&read_fds);
    FD_SET(connfd, &read_fds);
    FD_SET(client, &read_fds);

    int maxfd = (connfd > client) ? connfd : client;

    int result = select(maxfd + 1, &read_fds, NULL, NULL, &timeout);

    if (result > 0) {
        for (int i = 0; i <= maxfd; i++) {
            if (FD_ISSET(i, &read_fds)) {
                printf("Socket %d san sang nhan du lieu!\n", i);
            }
        }
    } else if (result == 0) {
        printf("Timeout xay ra\n");
    } else {
        perror("select loi");
    }

    // Đóng socket
    close(client);
    close(connfd);
    close(listener);

    return 0;
}
