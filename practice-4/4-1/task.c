#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 1024

int main() {
    char **lines = NULL;  
    int count = 0; 
    char buffer[MAX_LINE_LENGTH];
    
    printf("Введите строки (для завершения введите пустую строку):\n");
    
    while (1) {
        printf("Строка %d: ", count + 1);
        
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            break;
        }
        
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
            len--;
        }
        
        if (len == 0) {
            break;
        }
        
        char *new_line = (char*)malloc((len + 1) * sizeof(char));
        if (new_line == NULL) {
            fprintf(stderr, "Ошибка: не удалось выделить память для строки\n");
            for (int i = 0; i < count; i++) {
                free(lines[i]);
            }
            free(lines);
            return 1;
        }
        
        strcpy(new_line, buffer);
        
        char **temp = (char**)realloc(lines, (count + 1) * sizeof(char*));
        if (temp == NULL) {
            fprintf(stderr, "Ошибка: не удалось перераспределить память\n");
            free(new_line);
            for (int i = 0; i < count; i++) {
                free(lines[i]);
            }
            free(lines);
            return 1;
        }
        
        lines = temp;
        lines[count] = new_line;
        count++;
    }
    
    printf("\nСтроки в обратном порядке:\n");
    if (count == 0) {
        printf("Нет введенных строк.\n");
    } else {
        for (int i = count - 1; i >= 0; i--) {
            printf("%d: %s\n", count - i, lines[i]);
        }
    }
    
    for (int i = 0; i < count; i++) {
        free(lines[i]);
    }
    free(lines);
    
    return 0;
}
