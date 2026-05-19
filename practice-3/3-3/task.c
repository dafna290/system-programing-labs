#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define FILE_BUFFER 4096

void handle_client(int client_socket) {
    char filename[BUFFER_SIZE];
    char file_buffer[FILE_BUFFER];
    ssize_t bytes_read;
    int file_fd;
    struct stat file_stat;
    
    memset(filename, 0, BUFFER_SIZE);
    if (recv(client_socket, filename, BUFFER_SIZE - 1, 0) <= 0) {
        perror("recv filename");
        return;
    }
    
    filename[strcspn(filename, "\n")] = '\0';
   
    if (stat(filename, &file_stat) == -1) {
        char *error_msg = "ERROR: File not found\n";
        send(client_socket, error_msg, strlen(error_msg), 0);
        printf("[СЕРВЕР] Файл не найден: %s\n", filename);
        return;
    }
    
    file_fd = open(filename, O_RDONLY);
    if (file_fd == -1) {
        perror("open");
        char *error_msg = "ERROR: Cannot open file\n";
        send(client_socket, error_msg, strlen(error_msg), 0);
        return;
    }

    while ((bytes_read = read(file_fd, file_buffer, FILE_BUFFER)) > 0) {
        if (send(client_socket, file_buffer, bytes_read, 0) == -1) {
            perror("send");
            break;
        }
    }
    
    send(client_socket, "EOF", 3, 0);
    close(file_fd);
}

void run_server() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    
    if (listen(server_fd, 5) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    
    printf("[СЕРВЕР] Запущен на порту %d, PID: %d\n", PORT, getpid());
    
    while (1) {
        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd == -1) {
            perror("accept");
            continue;
        }
        
        printf("[СЕРВЕР] Клиент подключен: %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        handle_client(client_fd);
        close(client_fd);
    }
    
    close(server_fd);
}

void run_client() {
    int sock;
    struct sockaddr_in server_addr;
    char filename[BUFFER_SIZE];
    char buffer[FILE_BUFFER];
    ssize_t bytes_received;

    
    sleep(1);
    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        exit(EXIT_FAILURE);
    }
    
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect");
        exit(EXIT_FAILURE);
    }
    
    printf("Введите имя файла: ");
    
    if (fgets(filename, BUFFER_SIZE, stdin) == NULL) {
        perror("fgets");
        close(sock);
        exit(EXIT_FAILURE);
    }
    
    filename[strcspn(filename, "\n")] = '\0';
    
    if (send(sock, filename, strlen(filename), 0) == -1) {
        perror("send");
        close(sock);
        exit(EXIT_FAILURE);
    }
    
    while (1) {
        memset(buffer, 0, FILE_BUFFER);
        bytes_received = recv(sock, buffer, FILE_BUFFER, 0);
        
        if (bytes_received <= 0) break;
        if (bytes_received == 3 && strncmp(buffer, "EOF", 3) == 0) break;
        
        if (strncmp(buffer, "ERROR:", 6) == 0) {
            close(sock);
            exit(EXIT_FAILURE);
        }
        
        printf("[КЛИЕНТ] Получено: %.*s\n", (int)bytes_received, buffer);
    }
    
    close(sock);
}

int main() {
    pid_t pid;
    
    pid = fork();
    
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    
    if (pid == 0) {
        run_server();
    } else {
        run_client();
        wait(NULL);
        printf("\nПрограмма завершена\n");
    }
    
    return 0;
}
