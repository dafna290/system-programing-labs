#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <time.h>

#define LOG_FILE "/home/dafna/daemon.log"
#define CONFIG_FILE "/home/dafna/config.txt"

volatile sig_atomic_t reload_config = 0;

void log_message(const char *msg) {
    FILE *log = fopen(LOG_FILE, "a");
    if (log) {
        time_t now = time(NULL);
        fprintf(log, "[%s] %s\n", ctime(&now), msg);
        fclose(log);
    }
}

void read_config() {
    FILE *cfg = fopen(CONFIG_FILE, "r");
    if (cfg) {
        char buffer[256];
        log_message("Re-reading config.txt");
        while (fgets(buffer, sizeof(buffer), cfg)) {
            // Process config lines here
        }
        fclose(cfg);
    } else {
        log_message("Failed to open config.txt");
    }
}

void signal_handler(int sig) {
    if (sig == SIGHUP) {
        reload_config = 1;
    } else if (sig == SIGTERM || sig == SIGINT) {
        log_message("Daemon terminated");
        exit(0);
    }
}

void daemonize() {
    pid_t pid = fork();
    if (pid < 0) {
        exit(1);
    }
    if (pid > 0) {
        exit(0);
    }
    
    setsid();
    
    pid = fork();
    if (pid < 0) {
        exit(1);
    }
    if (pid > 0) {
        exit(0);
    }
    
    chdir("/");
    
    umask(0);
    
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}

int main() {
    daemonize();
    
    signal(SIGHUP, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);
    
    log_message("Daemon started");
    read_config();
    
    while (1) {
        if (reload_config) {
            reload_config = 0;
            read_config();
        }
        sleep(1);
    }
    
    return 0;
}
