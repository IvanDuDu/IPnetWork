#include <stdio.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
int main(){
    // bai 7
    int sockfd1, sockfd2;
    fd_set read_fds;
    sockfd1 =socket(AF_INET, SOCK_STREAM, 0);
    sockfd2 =socket(AF_INET, SOCK_STREAM, 0);
    struct timeval timeout;
    timeout.tv_sec=0;
    timeout.tv_usec=5;

    FD_ZERO(&read_fds);
    FD_SET(sockfd1, &read_fds);
    FD_SET(sockfd2, &read_fds);

    int maxfd = (sockfd1 > sockfd2) ? sockfd1 : sockfd2;

    int result = select(maxfd + 1, &read_fds, NULL, NULL, &timeout);
            //bai10
        if(result > 0){ 
            //bai 8
            for (int i = 0; i <= maxfd; i++) {
                if (FD_ISSET(i, &read_fds)) {
                    // Xử lý socket i
                    printf("Socket %d da ket noi\n",i );
                }
                }

        }else if(result==0) {
            //bai 9
            printf("Timeout xay ra \n");
        }else {
                perror("select");
            }
        


    }
