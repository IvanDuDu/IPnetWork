#include <stdio.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
int main(){
    // bai 7
    int listener, client;
    fd_set read_fds;

    struct timeval timeout;
    timeout.tv_sec=0;
    timeout.tv_usec=5;

    FD_ZERO(&read_fds);
    FD_SET(listener, &read_fds);
    FD_SET(client, &read_fds);

    int maxfd = (listener > client) ? listener : client;

    int result = select(maxfd + 1, &read_fds, NULL, NULL, &timeout);

            //bai 10
        if(result > 0){ 
            //bai 8
            for (int i = 0; i <= maxfd; i++) {
                if (FD_ISSET(i, &read_fds)) {
                    // Xử lý socket i
                    printf("Socket %d da ket noi",i );
                }
                }

        }else if(result==0) {
            //bai 9
            printf("Timeout xay ra \n");
        }else {
                perror("select");
            }
        


    }
