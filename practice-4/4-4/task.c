#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main() {
    size_t size = 100 * 1024 * 1024;
    char *large_array = (char*)malloc(size);
    
    if (large_array == NULL) {
        printf("Memory allocation failed\n");
        return 1;
    }
    
    memset(large_array, 'A', size);
    printf("Allocated %zu bytes (100 MB)\n", size);
    printf("PID: %d\n", getpid());
    printf("Press Enter to continue...\n");
    getchar();
    
    free(large_array);
    printf("Memory freed\n");
    printf("Press Enter to exit...\n");
    getchar();
    
    return 0;
}
