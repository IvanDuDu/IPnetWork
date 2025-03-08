// concurrent_server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <time.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define NUM_QUESTIONS 10
#define NUM_OPTIONS 4

typedef struct {
    char question[256]; 
    char options[NUM_OPTIONS][128];  
    int correct_index; 
} Question;

// Danh sách câu hỏi
Question quiz[NUM_QUESTIONS] = {
    {"What is the capital of France?", {"Paris", "London", "Berlin", "Rome"}, 0},
    {"Which planet is known as the Red Planet?", {"Mars", "Venus", "Jupiter", "Saturn"}, 0},
    {"What is the largest mammal?", {"Blue Whale", "Elephant", "Giraffe", "Hippopotamus"}, 0},
    {"How many continents are there on Earth?", {"Seven", "Five", "Six", "Eight"}, 0},
    {"What is the boiling point of water at sea level?", {"100°C", "90°C", "120°C", "80°C"}, 0},
    {"Who developed the theory of relativity?", {"Albert Einstein", "Isaac Newton", "Nikola Tesla", "Galileo Galilei"}, 0},
    {"Which gas do plants use for photosynthesis?", {"Carbon Dioxide", "Oxygen", "Nitrogen", "Hydrogen"}, 0},
    {"What is the chemical symbol for gold?", {"Au", "Ag", "Pb", "Fe"}, 0},
    {"What is the hardest natural substance on Earth?", {"Diamond", "Iron", "Quartz", "Granite"}, 0},
    {"Which ocean is the largest?", {"Pacific Ocean", "Atlantic Ocean", "Indian Ocean", "Arctic Ocean"}, 0}
};

// Gửi câu hỏi và trộn ngẫu nhiên đáp án
int Send_question(int connfd, Question ques) {
    srand(time(NULL));

    // Trộn thứ tự đáp án
    int order[NUM_OPTIONS] = {0, 1, 2, 3};
    for (int i = 3; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = order[i];
        order[i] = order[j];
        order[j] = temp;
    }

    // Gửi câu hỏi
    send(connfd, ques.question, strlen(ques.question), 0);
    sleep(1); // Tạo độ trễ nhẹ để tránh gửi quá nhanh

    // Gửi các đáp án đã trộn
    char option_buffer[BUFFER_SIZE];
    for (int i = 0; i < NUM_OPTIONS; i++) {
        snprintf(option_buffer, sizeof(option_buffer), "%d. %s\n", i + 1, ques.options[order[i]]);
        send(connfd, option_buffer, strlen(option_buffer), 0);
        sleep(1); // Độ trễ nhẹ giữa các lựa chọn
    }

    // Trả về chỉ số của đáp án đúng sau khi trộn
    for (int i = 0; i < NUM_OPTIONS; i++) {
        if (order[i] == ques.correct_index) {
            return i + 1; // Đánh số từ 1 đến 4
        }
    }
    return -1; // Lỗi (không xảy ra)
}

// Kiểm tra đáp án
int check_answer(int connfd, int answer, int correct_ans) {
    if (answer == correct_ans) {
        send(connfd, "Correct\n", 8, 0);
        return 1;
    } else {
        send(connfd, "Wrong\n", 6, 0);
        return 0;
    }
}

// Xử lý client
void handle_client(int connfd) {
    char buffer[BUFFER_SIZE];
    int n, sumPoint = 0;

    for (int j = 0; j < NUM_QUESTIONS; j++) {
        int correct_ans = Send_question(connfd, quiz[j]);

        // Nhận câu trả lời từ client
        n = recv(connfd, buffer, BUFFER_SIZE, 0);
        if (n <= 0) break; // Nếu client đóng kết nối
        buffer[n] = '\0';  // Kết thúc chuỗi

        int ans = atoi(buffer); // Chuyển chuỗi sang số
        sumPoint += check_answer(connfd, ans, correct_ans);
    }

    // Gửi điểm số cuối cùng
    char result[50];
    snprintf(result, sizeof(result), "Final Score: %d/%d\n", sumPoint, NUM_QUESTIONS);
    send(connfd, result, strlen(result), 0);

    close(connfd);
}

// Xử lý tín hiệu SIGCHLD để tránh zombie process
void sigchld_handler(int sig) {
    (void)sig; // Ignore warning
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

int main() {
    int listenfd, connfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    pid_t pid;

    // Tạo socket
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Cấu hình địa chỉ server
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Gán socket với địa chỉ và cổng
    if (bind(listenfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Lắng nghe kết nối
    if (listen(listenfd, 5) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    // Xử lý SIGCHLD
    signal(SIGCHLD, sigchld_handler);

    printf("Server is listening on port %d...\n", PORT);

    while (1) {
        // Chấp nhận kết nối từ client
        connfd = accept(listenfd, (struct sockaddr *)&client_addr, &addr_len);
        if (connfd < 0) continue;

        // Fork process để xử lý client
        pid = fork();
        if (pid == 0) {
            close(listenfd);
            handle_client(connfd);
            exit(0);
        } else {
            close(connfd);
        }
    }

    return 0;
}
