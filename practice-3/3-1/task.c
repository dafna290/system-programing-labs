#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <ctype.h>

#define BUFFER_SIZE 1024

int main() {
    int pipefd[2];  
    pid_t pid;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid = fork();
    
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    
    if (pid == 0) {
        
        close(pipefd[1]);
        
        bytes_read = read(pipefd[0], buffer, BUFFER_SIZE - 1);
        
        if (bytes_read == -1) {
            perror("read");
            exit(EXIT_FAILURE);
        }
        
        buffer[bytes_read] = '\0';
        
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }
        
        for (int i = 0; buffer[i] != '\0'; i++) {
            if (islower((unsigned char)buffer[i])) {
                buffer[i] = toupper((unsigned char)buffer[i]);
            }
        }
        
        printf("Результат: %s\n", buffer);
        
        close(pipefd[0]);
        
        exit(EXIT_SUCCESS);
        
    } else {
        
        close(pipefd[0]);
        
        printf("Введите строку для преобразования в заглавные буквы: ");
        
        if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) {
            perror("fgets");
            close(pipefd[1]);
            exit(EXIT_FAILURE);
        }
        
        bytes_read = strlen(buffer);
        if (write(pipefd[1], buffer, bytes_read) == -1) {
            perror("write");
            close(pipefd[1]);
            exit(EXIT_FAILURE);
        }
        
        close(pipefd[1]);
        
        int status;
        if (wait(&status) == -1) {
            perror("wait");
            exit(EXIT_FAILURE);
        }
        
    }
    
    return 0;
}
