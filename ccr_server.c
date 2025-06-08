// concurrent_server.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <sys/wait.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define USER_COUNT 800

struct users
{
    char  username[50];
    char  password[50];
};
typedef struct users user;
user allUser[100];
int count;



int readAccount(const char* filename, user allUser[], int * count){
    FILE* file= fopen(filename, "r");
    if(!file){
        printf(" Cannot open file");
        return -1;
    }
    char line[256];
    *count=0;
    while(fgets( line, sizeof(line),file)){
        line[strcspn(line,"\n")] =0;

        char *token = strtok (line,":");
        if(token){
            strcpy(allUser[*count].username, token);
        
        }
        token =strtok(NULL,":");
        if(token){
            strcpy(allUser[*count].password,token);
        }
        (*count)++;
        
    }
    fclose(file);
    return 1;
}

int writeAccount(const char* filename, user allUser[], int count){
    FILE* file= fopen(filename, "w");
    if(!file){
        printf(" Cannot open file");
        return -1;
    }
 
    for(int i =0 ; i< count; i++){
        fprintf( file, "%s:%s\n",allUser[i].username, allUser[i].password);
    }
    fclose(file);
    return 1;
}

int endWith( const char *str, const char * suffix){
     if( ! str || ! suffix){
        return 0;
     }

    size_t strLen = strlen( str);
    size_t suffixLen = strlen( suffix);

    if(strLen > suffixLen) return 0;

    return (strcmp(str+strLen - suffixLen,suffix)==0);

}

int isValid( const char* username, const char* password, user allUser[]){

    if(endWith(username, "@example.com")){
        for( int i=0 ; i < sizeof(allUser)/sizeof(allUser[0]); i++){
            if(strcmp(allUser[i].username,username)==0) return -1;
        }
        return 1;

    }else{
        return 0;
    }
}




// Signal handler to prevent zombie processes
void sigchld_handler(int sig) {
    (void)sig; // Ignore unused parameter warning
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void handle_client(int connfd) {
    char buffer[BUFFER_SIZE];
    int n;

    // Communication with client
    while ((n = recv(connfd, buffer, BUFFER_SIZE, 0)) > 0) {
        buffer[n] = '\0'; // Null-terminate the received string
        printf("Client: %s\n", buffer);

        char *username = strtok (buffer,":");
        char *password=  strtok (NULL,":");
        switch (isValid(username,password,allUser))
        {
        case -1:
            strcpy("ERROR email da ton tai ", buffer);
            break;
        case 0:
            strcpy("ERROR cu phap khong hop le ", buffer);
            break;
        case 1:
           strcpy( allUser[count].username,username);
           strcpy( allUser[count].password,password);
           strcpy("OK dang ky thanh cong ", buffer);
            break;
        default:
            break;
        }




        // Echo back the message to the client
        send(connfd, buffer, n, 0);
    }

    if (n == 0) {
        printf("Client disconnected.\n");
    } else {
        perror("recv failed");
    }

    // Close the client socket
    close(connfd);
}

int main() {
    
    
    if(readAccount("/home/sigmaduck/Downloads/Source_codes_Ch4/users.txt", allUser , &count)){
        printf("read file successfully, number of account : %d", count);
        
    }else{
        printf("read file failed");
    };

    int listenfd, connfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    pid_t pid;

    // Create the listening socket
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Setup the server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind the listening socket to the specified port
    if (bind(listenfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(listenfd);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(listenfd, 5) < 0) {
        perror("Listen failed");
        close(listenfd);
        exit(EXIT_FAILURE);
    }

    // Handle SIGCHLD to prevent zombie processes
    signal(SIGCHLD, sigchld_handler);

    printf("Server is listening on port %d...\n", PORT);

    // Server loop to accept multiple clients
    while (1) {
        // Accept an incoming connection
        connfd = accept(listenfd, (struct sockaddr *)&client_addr, &addr_len);
        if (connfd < 0) {
            perror("Accept failed");
            continue;
        }

        // Fork a child process to handle the client
        pid = fork();
        if (pid < 0) {
            perror("Fork failed");
            close(connfd);
        } else if (pid == 0) {
            // Child process: handle the client
            close(listenfd);  // Close the listening socket in the child process
            handle_client(connfd);
            close(connfd);
            exit(0);
        } else {
            // Parent process: continue accepting new clients
            close(connfd);  // Close the client socket in the parent process
        }
    }

    return 0;
}
