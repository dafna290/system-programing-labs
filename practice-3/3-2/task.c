#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>

#define FIFO_NAME "/tmp/time_fifo"
#define BUFFER_SIZE 64
#define LOG_FILE "time.log"

void sender_process() {
    int fd;
    char buffer[BUFFER_SIZE];
    time_t rawtime;
    struct tm *timeinfo;
    
    sleep(1);
    
    fd = open(FIFO_NAME, O_WRONLY);
    if (fd == -1) {
        perror("open for writing");
        exit(EXIT_FAILURE);
    }
    
    printf("Отправитель (PID: %d): начал отправку времени\n", getpid());
    
    for (int i = 0; i < 10; i++) {
        time(&rawtime);
        timeinfo = localtime(&rawtime);
        strftime(buffer, BUFFER_SIZE, "%Y-%m-%d %H:%M:%S", timeinfo);
        
        if (write(fd, buffer, strlen(buffer)) == -1) {
            perror("write");
            break;
        }
        
        if (write(fd, "\n", 1) == -1) {
            perror("write newline");
            break;
        }
        
        sleep(1);
    }
    
    close(fd);
    exit(EXIT_SUCCESS);
}

void receiver_process() {
    int fd_fifo, fd_log;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    
    mkfifo(FIFO_NAME, 0666);
    
    fd_fifo = open(FIFO_NAME, O_RDONLY);
    if (fd_fifo == -1) {
        perror("open for reading");
        exit(EXIT_FAILURE);
    }
    
    fd_log = open(LOG_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd_log == -1) {
        perror("open log");
        close(fd_fifo);
        exit(EXIT_FAILURE);
    }
    
    printf("Получатель (PID: %d): записывает время в %s\n", getpid(), LOG_FILE);
    
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        bytes_read = read(fd_fifo, buffer, BUFFER_SIZE - 1);
        
        if (bytes_read <= 0) {
            break;
        }
        
        if (buffer[bytes_read - 1] == '\n') {
            buffer[bytes_read - 1] = '\0';
        }
        
        write(fd_log, buffer, strlen(buffer));
        write(fd_log, "\n", 1);
    }
    
    close(fd_fifo);
    close(fd_log);
    exit(EXIT_SUCCESS);
}

int main() {
    pid_t pid;
    
    unlink(FIFO_NAME);
    
    if (mkfifo(FIFO_NAME, 0666) == -1) {
        perror("mkfifo");
        #define FALLBACK_FIFO "time_fifo"
        unlink(FALLBACK_FIFO);
        if (mkfifo(FALLBACK_FIFO, 0666) == -1) {
            perror("mkfifo fallback");
            exit(EXIT_FAILURE);
        }
        exit(EXIT_FAILURE);
    }
    
    pid = fork();
    
    if (pid == -1) {
        perror("fork");
        unlink(FIFO_NAME);
        exit(EXIT_FAILURE);
    }
    
    if (pid == 0) {
        receiver_process();
    } else {
        sender_process();
        wait(NULL);
        unlink(FIFO_NAME);
    }
    
    return 0;
}
