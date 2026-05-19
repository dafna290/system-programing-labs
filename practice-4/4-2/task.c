#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <limits.h>

int main(int argc, char *argv[]) {
    const char *filename;
    int fd = -1;
    struct stat st;
    int *data = NULL;
    size_t num_elements = 0;
    long long sum = 0;
    double average = 0.0;
    
    if (argc != 2) {
        printf("Введите название файла как аргумент\n");
        return 1;
    }
    filename = argv[1];
    
    fd = open(filename, O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "Ошибка: не удалось открыть файл '%s': %s\n", 
                filename, strerror(errno));
        return 1;
    }
    
    if (fstat(fd, &st) == -1) {
        fprintf(stderr, "Ошибка: не удалось получить информацию о файле '%s': %s\n", 
                filename, strerror(errno));
        close(fd);
        return 1;
    }
    
    if (st.st_size == 0) {
        fprintf(stderr, "Ошибка: файл '%s' пуст\n", filename);
        close(fd);
        return 1;
    }
    
    if (st.st_size % sizeof(int) != 0) {
        fprintf(stderr, "Предупреждение: размер файла (%ld) не кратен размеру int (%lu)\n", 
                st.st_size, sizeof(int));
    }
    
    num_elements = st.st_size / sizeof(int);
    printf("Файл '%s' содержит %zu целых чисел (%ld байт)\n", 
           filename, num_elements, st.st_size);
    
    data = (int*)mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (data == MAP_FAILED) {
        fprintf(stderr, "Ошибка: не удалось отобразить файл в память: %s\n", 
                strerror(errno));
        close(fd);
        return 1;
    }
    
    close(fd);
    fd = -1;
    
    printf("Числа в файле:\n");
    
    for (size_t i = 0; i < num_elements; i++) {
        printf("  [%zu] = %d\n", i, data[i]);
        sum += data[i];
        
        if (i > 0 && (sum > 0 && data[i] > 0 && sum > LLONG_MAX - data[i])) {
            fprintf(stderr, "Предупреждение: возможное переполнение суммы\n");
        }
    }
    
    if (num_elements > 0) {
        average = (double)sum / num_elements;
    }
    
    printf("\nРезультаты:\n");
    printf("  Количество элементов: %zu\n", num_elements);
    printf("  Среднее значение: %.2f\n", average);
    
    if (munmap(data, st.st_size) == -1) {
        fprintf(stderr, "Ошибка: не удалось освободить отображение памяти: %s\n", 
                strerror(errno));
        return 1;
    }
    
    return 0;
}
