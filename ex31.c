//Details: Allen Bronshtein 206228751
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include "string.h"

#define O_ERR "Error opening file"
#define R_ERR "Error reading file"
#define MALLOC_ERR "Error allocating memory"
#define C_ERR "Error closing file"
#define SIZE 1024

void clean(char *buffer) {
    int i;
    for (i = 0; i < SIZE; i++) {
        buffer[i] = '\0';
    }
}

char *buffMax(char *buffer1, char *buffer2) {
    if (strlen(buffer1) < strlen(buffer2))
        return buffer2;
    return buffer1;
}

char *buffMin(char *buffer1, char *buffer2, int *min_fd, int fd1, int fd2) {
    if (strlen(buffer1) < strlen(buffer2)) {
        *min_fd = fd1;
        return buffer1;
    }
    *min_fd = fd2;
    return buffer2;
}

void p_error(char *err_msg, int num, int fd1, int fd2) {
    int status;
    if (num > 0) {
        close(fd1);
    }
    if (num > 1) {
        close(fd2);
    }
    status = write(2, err_msg, strlen(err_msg));
    if (status == -1) {
        exit(-1);
    }
    exit(-1);
}

char *substr(char *p, unsigned long length, int fd1, int fd2) {
    char err_msg[1024];
    clean(err_msg);
    unsigned long i;
    char *word = malloc(sizeof(char) * length);
    if (word == NULL) {
        sprintf(err_msg, "%s", MALLOC_ERR);
        p_error(err_msg, 2, fd1, fd2);
    } else {
        for (i = 0; i < length; i++) {
            word[i] = *p++;
        }
    }
    return word;
}

void cut_end(char *buffer) {
    unsigned long size = strlen(buffer);
    buffer[size - 1] = '\0';
}

int is_odd(unsigned long num) {
    if (num % 2 == 1) {
        return 1;
    }
    return 0;
}

void read_to_buffer(int fd, char *buffer, int fd1, char *argv[], int fd2) {
    int status;
    char err_msg[1024];
    clean(err_msg);
    status = read(fd, buffer, SIZE);
    if (status == -1) {
        if (fd == fd1) {
            sprintf(err_msg, "%s %s", R_ERR, argv[1]);
            p_error(err_msg, 2, fd1, fd2);
        } else {
            sprintf(err_msg, "%s %s", R_ERR, argv[2]);
            p_error(err_msg, 2, fd1, fd2);
        }
    }
}

int is_Similar(char *buffer1, char *buffer2, int fd1, int fd2, char *argv[]) {
    int min_fd = -1;
    char *buffer_max = buffMax(buffer1, buffer2);
    char *buffer_min = buffMin(buffer1, buffer2, &min_fd, fd1, fd2);
    int offset = 1;
    unsigned long size_min = strlen(buffer_min), half;
    half = size_min / 2;
    if (is_odd(size_min))
        half++;
    char *p = buffer_min;
    while (1) {
        if (strlen(buffer_min) < half) {
            break;
        }
        char *substring = substr(p, half, fd1, fd2);
        if (strstr(buffer_max, substring) != NULL) {
            return 1;
        }
        free(substring);
        lseek(min_fd, offset++, SEEK_SET);
        read_to_buffer(min_fd, buffer_min, fd1, argv, fd2);
        cut_end(buffer_min);
        p = buffer_min;
    }

    return 0;
}

void load(char *buffer1, char *buffer2, int *fd1, int *fd2, char *argv[]) {
    char err_msg[1024];
    clean(buffer1), clean(buffer2), clean(err_msg);
    *fd1 = open(argv[1], O_RDONLY), *fd2 = open(argv[2], O_RDONLY);
    if (*fd1 == -1) {
        sprintf(err_msg, "%s %s", O_ERR, argv[1]);
        p_error(err_msg, 1, *fd1, *fd2);
    }
    if (*fd2 == -1) {
        sprintf(err_msg, "%s %s", O_ERR, argv[2]);
        p_error(err_msg, 2, *fd1, *fd2);
    }
}

int finish(int status, int fd1, int fd2, char *argv[]) {
    char err_msg[1024];
    clean(err_msg);
    int close_status;
    close_status = close(fd1);
    if (close_status == -1) {
        sprintf(err_msg, "%s %s", C_ERR, argv[1]);
        p_error(err_msg, 0, fd1, fd2);
    }
    close(fd2);
    if (close_status == -1) {
        sprintf(err_msg, "%s %s", C_ERR, argv[2]);
        p_error(err_msg, 0, fd1, fd2);
    }
    return status;
}

int main(int argc, char *argv[]) {
    int fd1 = -1, fd2 = -1;
    char buffer1[SIZE], buffer2[SIZE];
    load(buffer1, buffer2, &fd1, &fd2, argv);
    read_to_buffer(fd1, buffer1, fd1, argv, fd2);
    read_to_buffer(fd2, buffer2, fd1, argv, fd2);
    if (strcmp(buffer1, buffer2) == 0) {
        return finish(1, fd1, fd2, argv);
    } else {
        if (is_Similar(buffer1, buffer2, fd1, fd2, argv)) {
            return finish(3, fd1, fd2, argv);
        } else {
            return finish(2, fd1, fd2, argv);
        }
    }
}
