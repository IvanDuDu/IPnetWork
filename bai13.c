#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#define FD_SETSIZE 1024
#include <sys/select.h>
#include <unistd.h>



//bai 20
int my_select(int maxfd, fd_set *readfds, struct timeval *timeout) {
    return select(maxfd + 1, readfds, NULL, NULL, timeout);
    }


int main() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr = {0};
    int listener;
    listener= socket(AF_INET, SOCK_STREAM, 0);


    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    bind(listener, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    listen(listener, 5);


        //bai 12
    fd_set readfds,exceptfds;
    FD_SET(listener, &readfds);

    int maxfd = (listener > 0) ? listener : 0;

    my_select(FD_SETSIZE,&readfds, NULL);
    
    struct timeval timeout;
    while (1) {
        //bai 13
        FD_ZERO(&readfds);
        FD_ZERO(&exceptfds);

        FD_SET(sockfd, &readfds);
        FD_SET(sockfd, &exceptfds);

        FD_SET(0,&readfds); //stdin
        maxfd= (sockfd>0)? sockfd :0;

        timeout.tv_sec = 5; // Chờ 5 giây
        timeout.tv_usec = 0;

        int result = select(sockfd + 1, &readfds, NULL, &exceptfds, &timeout);
        if (result > 0) {
            //bai 15
            printf("So socket san sang la %d", result);
            if (FD_ISSET(sockfd, &readfds)) {
            // Có kết nối mới
            printf("Có kết nối mới!\n");
                }
            //bai 16
                for (int i = 0; i <= maxfd; i++) {
                if (!FD_ISSET(i, &readfds)) {
                    close(i);
                    printf("Da dong ket noi cua %d", i);
                }
                }
            //bai 17
            if(FD_ISSET(sockfd, &exceptfds)){
                printf("Socket gap ngoai le");
            }
        } else if (result == 0) {
            //bai 14
            printf("Hết thời gian chờ, tang timeout them 3s\n");
            timeout.tv_sec +=3;
        } else {
            perror("select error");
            break;
        }
    }
    close(sockfd);
    return 0;
    }