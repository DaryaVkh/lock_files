#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<signal.h>
#include<errno.h>

#define true 1
#define false 0
#define buff_size 1024
#define mode S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
#define opening_error "Can't open file\n"
#define writing_error "Error while writing to file\n"
#define reading_error "Error while reading file\n"
#define file_error "Invalid file\n"
#define creating_error "Error while creating file\n"
#define invalid_pid "Process PID and PID from .lck file aren't same\n"

int is_lock_owner = false;
int is_error = false;
int locks = 0;
int unlocks = 0;

void lockFile(char *file, int *is_own_lock, int is_fatal) {
    char lock_file_name_buff[buff_size];
    char pid_buff[buff_size];
    sprintf(lock_file_name_buff, "%s.lck", file);
    while (true) {
        struct stat file_info;
        if (stat(lock_file_name_buff, &file_info) == -1) {
            if (errno == ENOENT) {
                break;
            }
        }
        if (!S_ISREG(file_info.st_mode)) {
            break;
        }
    }
    int fd_lock = open(lock_file_name_buff, O_WRONLY | O_CREAT | O_TRUNC, mode);
    if (fd_lock == -1) {
        perror(creating_error);
        exit(EXIT_FAILURE);
    }
    *is_own_lock = true;
    pid_t pid = getpid();
    sprintf(pid_buff, "%d\n", pid);
    if (write(fd_lock, pid_buff, strlen(pid_buff)) != strlen(pid_buff)) {
        is_error = true;
        fprintf(stderr, writing_error);
        is_fatal ? exit(EXIT_FAILURE) : kill(getpid(), SIGINT);
    }

    close(fd_lock);
}

void unlock(char *file, int *is_own_lock, int is_fatal) {
    char lock_file_name_buff[buff_size];
    char lock_file_buff[buff_size];
    sprintf(lock_file_name_buff, "%s.lck", file);

    struct stat file_info;
    if ((stat(lock_file_name_buff, &file_info) == -1 && errno == ENOENT) || !S_ISREG(file_info.st_mode)) {
        is_error = true;
        fprintf(stderr, file_error);
        if (!is_fatal) {
            kill(getpid(), SIGINT);
        }
        exit(EXIT_FAILURE);
    }

    int lock_fd = open(lock_file_name_buff, O_RDONLY);
    if (lock_fd == -1) {
        is_error = true;
        fprintf(stderr, opening_error);
        is_fatal ? exit(EXIT_FAILURE) : kill(getpid(), SIGINT);
    }
    ssize_t len = read(lock_fd, lock_file_buff, buff_size - 1);
    if (len == -1) {
        is_error = true;
        fprintf(stderr, reading_error);
        is_fatal ? exit(EXIT_FAILURE) : kill(getpid(), SIGINT);
    }
    lock_file_buff[len] = '\0';

    pid_t pid = getpid();
    pid_t lockPid;

    if (sscanf(lock_file_buff, "%d", &lockPid) != 1) {
        is_error = true;
        fprintf(stderr, file_error);
        is_fatal ? exit(EXIT_FAILURE) : kill(getpid(), SIGINT);
    }

    if (pid != lockPid) {
        is_error = true;
        fprintf(stderr, invalid_pid);
        is_fatal ? exit(EXIT_FAILURE) : kill(getpid(), SIGINT);
    }

    close(lock_fd);
    remove(lock_file_name_buff);
    *is_own_lock = false;
}

void handler(int sig) {
    char statistic_buff[buff_size];
    int is_own_lock = false;
    pid_t pid = getpid();

    lockFile("statistic.txt", &is_own_lock, true);

    if (is_error) {
        sprintf(statistic_buff, "Process PID: %d (%d locks, %d unlocks). ERROR OCCURRED\n", pid, locks, unlocks);
    } else {
        sprintf(statistic_buff, "Process PID: %d (%d locks, %d unlocks).\n", pid, locks, unlocks);
    }

    int statistic_fd = open("statistic.txt", O_WRONLY | O_APPEND | O_CREAT, mode);
    if (write(statistic_fd, statistic_buff, strlen(statistic_buff)) != strlen(statistic_buff)) {
        fprintf(stderr, writing_error);
        exit(EXIT_FAILURE);
    }

    close(statistic_fd);
    unlock("statistic.txt", &is_own_lock, true);

    exit(EXIT_SUCCESS);
}

void lock() {
    signal(SIGINT, handler);
    while (true) {
        lockFile("file_for_lock.txt", &is_lock_owner, false);
        locks++;
        sleep(1);
        unlock("file_for_lock.txt", &is_lock_owner, false);
        unlocks++;
    }
}

int main() {
    lock();
    return 0;
}
