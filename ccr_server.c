// server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 12345
#define MAX 1024

int board[3][3]; // 0: O, 1: X, -1: empty

void init_board() {
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            board[i][j] = -1;
}

void print_board() {
    printf("\nBoard:\n");
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (board[i][j] == -1) printf(".");
            else if (board[i][j] == 0) printf("O");
            else printf("X");
            printf(" ");
        }
        printf("\n");
    }
}

int check_winner() {
    for (int i = 0; i < 3; i++) {
        if (board[i][0] != -1 && board[i][0] == board[i][1] && board[i][1] == board[i][2])
            return board[i][0];
        if (board[0][i] != -1 && board[0][i] == board[1][i] && board[1][i] == board[2][i])
            return board[0][i];
    }
    if (board[0][0] != -1 && board[0][0] == board[1][1] && board[1][1] == board[2][2])
        return board[0][0];
    if (board[0][2] != -1 && board[0][2] == board[1][1] && board[1][1] == board[2][0])
        return board[0][2];
    return -1;
}

int is_draw() {
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            if (board[i][j] == -1)
                return 0;
    return 1;
}

void send_to_all(int clients[], char *msg) {
    send(clients[0], msg, strlen(msg), 0);
    send(clients[1], msg, strlen(msg), 0);
}

int main() {
    int server_fd, new_socket[2];
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[MAX];
    int current_player = 0;

    init_board();

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 2);

    printf("Waiting for players...\n");
    for (int i = 0; i < 2; i++) {
        new_socket[i] = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
        printf("Player %d connected.\n", i);
        char msg[32];
        sprintf(msg, "WELCOME %d\n", i);
        send(new_socket[i], msg, strlen(msg), 0);
    }

    while (1) {
        char turn_msg[32];
        sprintf(turn_msg, "TURN %d\n", current_player);
        send(new_socket[current_player], turn_msg, strlen(turn_msg), 0);

        memset(buffer, 0, MAX);
        int valread = read(new_socket[current_player], buffer, MAX);
        if (valread <= 0) break;

        int row, col;
        if (sscanf(buffer, "MOVE %d %d", &row, &col) != 2) {
            send(new_socket[current_player], "INVALID\n", 8, 0);
            continue;
        }

        if (row < 0 || row > 2 || col < 0 || col > 2 || board[row][col] != -1) {
            send(new_socket[current_player], "INVALID\n", 8, 0);
            continue;
        }

        board[row][col] = current_player;
        print_board();

        char move_msg[32];
        sprintf(move_msg, "MOVE %d %d %d\n", current_player, row, col);
        send_to_all(new_socket, move_msg);

        int winner = check_winner();
        if (winner != -1) {
            char win_msg[32];
            sprintf(win_msg, "WIN %d\n", winner);
            send_to_all(new_socket, win_msg);
            break;
        } else if (is_draw()) {
            send_to_all(new_socket, "DRAW\n");
            break;
        }

        current_player = 1 - current_player;
    }

    close(new_socket[0]);
    close(new_socket[1]);
    close(server_fd);
    return 0;
}
