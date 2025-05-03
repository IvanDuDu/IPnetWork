#include <stdio.h>
#include <unistd.h>
#include <sys/select.h>

int main() {
    fd_set fdset;
    FD_ZERO(&fdset);

    int k = -1;

    while (1) {
        printf("Nhap cong muon them (hoac -1 de thoat): ");
        scanf("%d", &k);

        if (k == -1) break;

        if (k < 0 || k >= 24) {
            printf("Chi cho phep nhap tu 0 den 23\n");
            continue;
        }

        // Dùng FD_ISSET để kiểm tra bit
        if (!FD_ISSET(k, &fdset)) {
            FD_SET(k, &fdset);
            printf("Da them fd %d\n", k);
        } else {
            printf("Fd %d da tao tu truoc!\n", k);
        }
    }

    printf("fds_bits[0] = %ld\n", fdset.fds_bits[0]);

    // Hiển thị giá trị nhị phân của fds_bits[0]
    printf("Binary value of fds_bits[0]:\n");
    for (int i = sizeof(fdset.fds_bits[0]) * 8 - 1; i >= 0; i--) {
        printf("%ld", (fdset.fds_bits[0] >> i) & 1);
        if (i % 8 == 0) printf(" ");
    }
    printf("\n");

    return 0;
}
