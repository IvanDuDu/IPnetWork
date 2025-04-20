// client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 12345
#define MAX 1024

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[MAX];
    int player_id = -1;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
    connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    while (1) {
        memset(buffer, 0, MAX);
        int valread = read(sock, buffer, MAX);
        if (valread <= 0) break;

        if (strncmp(buffer, "WELCOME", 7) == 0) {
            sscanf(buffer, "WELCOME %d", &player_id);
            printf("You are player %d (%c)\n", player_id, player_id == 0 ? 'O' : 'X');
        } else if (strncmp(buffer, "TURN", 4) == 0) {
            int turn;
            sscanf(buffer, "TURN %d", &turn);
            if (turn == player_id) {
                int row, col;
                printf("Your turn. Enter row and col: ");
                scanf("%d %d", &row, &col);
                char msg[32];
                sprintf(msg, "MOVE %d %d\n", row, col);
                send(sock, msg, strlen(msg), 0);
            }
        } else if (strncmp(buffer, "MOVE", 4) == 0) {
            int p, r, c;
            sscanf(buffer, "MOVE %d %d %d", &p, &r, &c);
            printf("Player %d moved at (%d, %d)\n", p, r, c);
        } else if (strncmp(buffer, "WIN", 3) == 0) {
            int winner;
            sscanf(buffer, "WIN %d", &winner);
            if (winner == player_id) printf("You win!\n");
            else printf("You lose!\n");
            break;
        } else if (strncmp(buffer, "DRAW", 4) == 0) {
            printf("Game is a draw.\n");
            break;
        } else if (strncmp(buffer, "INVALID", 7) == 0) {
            printf("Invalid move. Try again.\n");
            
        }
    }

    close(sock);
    return 0;
}
