#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/file.h>
#include <errno.h>
#include <string.h>

#define LOCK_FILE "/tmp/dmald.lock"
#define DATA_FILE "/tmp/dmald.data"

static int fd_lock = -1;
static int fd_latency = -1;
static volatile sig_atomic_t running = 1;
static volatile sig_atomic_t need_update = 0;

static pid_t read_daemon_pid() {
    char pid_str[32] = {0};
    int fd = open(LOCK_FILE, O_RDONLY);
    if (fd == -1) return -1;
    
    ssize_t len = read(fd, pid_str, sizeof(pid_str)-1);
    close(fd);
    return (len > 0) ? atoi(pid_str) : -1;
}

static int set_latency(int32_t target) {
    if (fd_latency == -1) {
        fd_latency = open("/dev/cpu_dma_latency", O_WRONLY);
        if (fd_latency == -1) {
            perror("open /dev/cpu_dma_latency");
            return -1;
        }
    }
    return write(fd_latency, &target, sizeof(target));
}

static void handle_sigterm(int sig) { running = 0; }
static void handle_sigusr1(int sig) { need_update = 1; }

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <latency_value> (or -1 to terminate daemon)\n", argv[0]);
        return 1;
    }

    int32_t target = atoi(argv[1]);

    fd_lock = open(LOCK_FILE, O_CREAT | O_RDWR, 0644);
    if (fd_lock == -1) {
        perror("open lock file");
        return 1;
    }

    if (flock(fd_lock, LOCK_EX | LOCK_NB) == -1) {
        if (errno == EWOULDBLOCK) {
            pid_t daemon_pid = read_daemon_pid();
            if (daemon_pid <= 0) {
                fprintf(stderr, "Error: Can't read daemon PID\n");
                return 1;
            }

            if (target == -1) {
                kill(daemon_pid, SIGTERM);
                printf("Terminated daemon (PID %d)\n", daemon_pid);
            } else {
                // Set target and signal to daemon
                int fd = open(DATA_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd != -1) {
                    write(fd, &target, sizeof(target));
                    close(fd);
                }
                kill(daemon_pid, SIGUSR1);
                printf("Updated target to %d (PID %d)\n", target, daemon_pid);
            }
            return 0;
        }
        perror("flock");
        return 1;
    }
    if (target < 0) return 1;

    // --- Deamon ----
    printf("Daemon started (PID %d)\n", getpid());
    
    // PID to Lock
    char pid_str[32];
    snprintf(pid_str, sizeof(pid_str), "%d", getpid());
    ftruncate(fd_lock, 0);
    write(fd_lock, pid_str, strlen(pid_str));

    // SIGNAL
    signal(SIGTERM, handle_sigterm);
    signal(SIGUSR1, handle_sigusr1);

    // Set DMA
    if (set_latency(target) == -1) {
        flock(fd_lock, LOCK_UN);
        close(fd_lock);
        return 1;
    }

    // Main Loop
    while (running) {
        if (need_update) {
            int fd = open(DATA_FILE, O_RDONLY);
            if (fd != -1) {
                int32_t new_target;
                read(fd, &new_target, sizeof(new_target));
                close(fd);
                set_latency(new_target);
                printf("Updated latency to %d\n", new_target);
            }
            need_update = 0;
        }
        pause();
    }

    // Clean
    flock(fd_lock, LOCK_UN);
    close(fd_lock);
    printf("Daemon stopped\n");
    return 0;
}
