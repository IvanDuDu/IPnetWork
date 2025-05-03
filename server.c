#include <stdio.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main() {
    fd_set fdset;
    FD_ZERO(&fdset);
    // Tạo socket mới
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket failed");
        return 1;
    }

    // Thêm sockfd vào fd_set
    FD_SET(sockfd, &fdset);
    printf("Da them sockfd %d vao fd_set\n", sockfd);

    int k = -1, choice;

    while (1) {
        printf("\nChon thao tac:\n");
        printf("1. Them fd vao fd_set\n");
        printf("2. Xoa fd khoi fd_set\n");
        printf("3. Hien thi fds_bits[0]\n");
        printf("4. Thuc hien select()\n");
        printf("0. Thoat\n");
        printf("Lua chon: ");
        scanf("%d", &choice);

        if (choice == 0) break;

        switch (choice) {
            case 1:
                printf("Nhap fd muon them: ");
                scanf("%d", &k);

                if (k < 0 || k >= FD_SETSIZE) {
                    printf("Chi cho phep nhap tu 0 den %d\n", FD_SETSIZE - 1);
                    break;
                }

                if (!FD_ISSET(k, &fdset)) {
                    FD_SET(k, &fdset);
                    printf("Da them fd %d\n", k);
                } else {
                    printf("Fd %d da ton tai!\n", k);
                }
                break;

            case 2:
                printf("Nhap fd muon xoa: ");
                scanf("%d", &k);

                if (k < 0 || k >= FD_SETSIZE) {
                    printf("Chi cho phep nhap tu 0 den %d\n", FD_SETSIZE - 1);
                    break;
                }

                if (FD_ISSET(k, &fdset)) {
                    FD_CLR(k, &fdset);
                    printf("Da xoa fd %d\n", k);
                } else {
                    printf("Fd %d chua ton tai!\n", k);
                }
                break;

            case 3:
                printf("fds_bits[0] = %ld\n", fdset.fds_bits[0]);
                printf("Binary value of fds_bits[0]:\n");
                for (int i = sizeof(fdset.fds_bits[0]) * 8 - 1; i >= 0; i--) {
                    printf("%ld", (fdset.fds_bits[0] >> i) & 1);
                    if (i % 8 == 0) printf(" ");
                }
                printf("\n");
                break;

            case 4: {
                // Tạo bản sao fd_set vì select() sẽ thay đổi nó
                fd_set read_fds;
                FD_ZERO(&read_fds);
                read_fds = fdset;

                printf("Dang goi select()...\n");
                int maxfd = sockfd;  // hoặc tìm max trong fdset nếu cần
                
                struct timeval timeout;
                timeout.tv_sec = 5;
                timeout.tv_usec = 0;
                //int ready =select(sockfd + 1, &read_fds, NULL, NULL, NULL  );
                int ready =select(sockfd + 1, &read_fds, NULL, NULL, &timeout);
                if (ready < 0) {
                    perror("select error");
                } else {
                    printf("So luong fd san sang: %d\n", ready);
                    for (int i = 0; i <= maxfd; i++) {
                        if (FD_ISSET(i, &read_fds)) {
                            printf("Fd %d san sang doc!\n", i);
                        }
                    }
                }
                break;
            }

            default:
                printf("Lua chon khong hop le!\n");
        }
    }

    close(sockfd); // Đóng socket khi kết thúc

    return 0;
}
