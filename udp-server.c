// udp_server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define PORT 8080
#define MAXLINE 1024
#define XOR_KEY 0x5A

void xor_cipher( char *data){
    for( int i=0 ; data[i] != '\0';  i++){
        data[i] ^= XOR_KEY;
    }   
}

int main() {
    int sockfd;
    char buffer[MAXLINE];
    struct sockaddr_in servaddr, cliaddr;

    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    // Filling server information
    servaddr.sin_family = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    // Bind the socket with the server address
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    socklen_t len = sizeof(cliaddr); // len is value/result

    printf("Server is running...\n");

while(1){
    fd_set readfds;
    struct  timeval timeout;
    FD_ZERO(&readfds);
    FD_SET(sockfd, &readfds);

    timeout.tv_sec=15;
    timeout.tv_usec=0;
    int activity= select( sockfd+1, &readfds, NULL ,NULL, &timeout);
    if(activity==-1){
         perror("Loi select");
         break;
    }else if (activity==0){
        printf("Khong nhan duoc du lieu trong 5 giay....\n");
        continue;
    }
    

    int n = recvfrom( sockfd,buffer, MAXLINE,0,(struct sockaddr *)&cliaddr, &len);
    if(n<0){
        perror("recvfrom failed");
        continue;
    }
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &cliaddr.sin_addr,client_ip,INET_ADDRSTRLEN);
    buffer[n] = '\0';
    printf("client %d: %s \n", ntohs(cliaddr.sin_port), buffer);
    

    xor_cipher(buffer);
    printf("Giai ma : %s \n" , buffer);
    sendto(sockfd, buffer, strlen(buffer),0, (struct sockaddr*)&cliaddr, len);
}

    close(sockfd);
    return 0;
}
