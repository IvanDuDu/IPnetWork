#include <stdio.h>
#include <poll.h>
#include <unistd.h>
#include <sys/socket.h>

#define MAX_CLIENT 1024

int main() {
    struct pollfd fds[MAX_CLIENT];

    int timeout = 5000; // 5 giây
    fds[0].fd = 0;     // stdin

    fds[0].events = POLLIN; // Theo dõi đọc
    int sockfd1= socket(AF_INET, SOCK_STREAM, 0);
    int sockfd2= socket(AF_INET, SOCK_STREAM, 0);
    fds[1].fd = sockfd1;
    fds[1].events = POLLIN;
    fds[2].fd = sockfd2;
    fds[2].events = POLLIN | POLLOUT;

    
    fds[3].fd = -1; // FD không hợp lệ
    fds[3].events = POLLIN;

    int ret= poll(fds, 4, 10000); 

    //int ret = poll(fds, 4, 0); // Không chờ


    if (ret == -1) {
        perror("poll error");
    } else if (ret == 0) {
        printf("Timeout sau 5 giây.\n");
    } else {
        if (fds[0].revents & POLLIN) {
            printf("Có dữ liệu từ bàn phím.\n");
            }
        // if (fds[1].revents & POLLIN) {
        //     printf("Socket có dữ liệu để đọc.\n");
        //     }
        // if (fds[2].revents & POLLIN) {
        //     printf("Socket có dữ liệu để đọc.\n");
        //     }        
        for (int i = 0; i < 2; i++) {
            if (fds[i].revents & POLLIN) {
            printf("Socket %d có dữ liệu\n", fds[i].fd);
            }else if(fds[i].revents & POLLNVAL){
                printf("FD không hợp lệ\n");
            }else if(fds[i].revents & POLLOUT){
                pprintf("Socket %d san sang de ghi\n", fds[i].fd);
            }else if(fds[i].revents & POLLHUP) {
                printf("Client đóng kết nối\n");
                }
            }

        }

}