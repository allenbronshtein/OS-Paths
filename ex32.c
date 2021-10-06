//Details: Allen Bronshtein 206228751
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include "string.h"
#include <dirent.h>
#include <time.h>

#define SIZE 1024
#define OF_ERR "Error opening file"
#define CF_ERR "Error closing file"
#define CD_ERR "Error closing directory"
#define OD_ERR "Not a valid directory"
#define R_ERR "Error reading file"
#define IO_ERR "Input/output File not exist"

void clean(char *buffer) {
    int i;
    for (i = 0; i < SIZE; i++) {
        buffer[i] = '\0';
    }
}

void p_error(char *err_msg) {
    int status;
    status = write(2, err_msg, strlen(err_msg));
    if (status == -1) {
        exit(-1);
    }
    exit(-1);
}

int main(int argc, char *argv[]) {
    int conf_fd, status;
    DIR *main_dir, *student_dir;
    int input_fd, results_fd, correct_output_fd;
    char err_msg[1024], buffer[1024], temp_buffer[1024], program_running_path[1024], results_buffer[1024], results_path[1024];
    char *main_folder_path, *input_path, *correct_output_path;
    clean(err_msg), clean(buffer), clean(temp_buffer), clean(program_running_path), clean(results_buffer), clean(
            results_path);
    getcwd(program_running_path, SIZE);
    strcpy(results_path, program_running_path);
    strcat(results_path, "/results.csv");
    conf_fd = open(argv[1], O_RDONLY);
    if (conf_fd == -1) {
        sprintf(err_msg, "%s %s\n", OF_ERR, argv[1]);
        p_error(err_msg);
    }
    status = read(conf_fd, buffer, SIZE);
    if (status == -1) {
        sprintf(err_msg, "%s %s\n", R_ERR, argv[1]);
        close(conf_fd);
        p_error(err_msg);
    }
    strcpy(temp_buffer, buffer);
    main_folder_path = strtok(temp_buffer, "\n");
    input_path = strtok(NULL, "\n");
    correct_output_path = strtok(NULL, "\n");
    status = close(conf_fd);
    if (status == -1) {
        sprintf(err_msg, "%s %s\n", CF_ERR, argv[1]);
        perror(err_msg);
    }
    main_dir = opendir(main_folder_path);
    if (main_dir == NULL) {
        sprintf(err_msg, "%s\n", OD_ERR);
	close(conf_fd);
        p_error(err_msg);
    }
    input_fd = open(input_path, O_RDONLY);
    if (input_fd == -1) {
        sprintf(err_msg, "%s\n", IO_ERR);
	close(conf_fd);
	closedir(main_dir);
        p_error(err_msg);
    }
    correct_output_fd = open(correct_output_path, O_RDONLY);
    if (correct_output_fd == -1) {
        sprintf(err_msg, "%s\n", IO_ERR);
	close(input_fd);
	close(conf_fd);
	closedir(main_dir);
        p_error(err_msg);
    }
    if (input_fd == -1 || correct_output_fd == -1) {
        status = closedir(main_dir);
        if (status == -1) {
            close(input_fd);
            close(correct_output_fd);
	    close(conf_fd);
            sprintf(err_msg, "%s\n", CD_ERR);
            p_error(err_msg);
        }
        sprintf(err_msg, "%s\n", IO_ERR);
        p_error(err_msg);
    }
    close(correct_output_fd);
    char input[256];
    int stat = read(input_fd, input, 256);
    if (stat == -1) {
        close(input_fd);
        sprintf(err_msg, "%s %s\n", R_ERR, input_path);
        p_error(err_msg);
    }
    close(input_fd);
    char temp_directory[1024];
    clean(temp_directory);
    strcpy(temp_directory, main_folder_path);
    strcat(temp_directory, "/");
    struct dirent *entry;
    while ((entry = readdir(main_dir)) != NULL) {
        char student_directory_path[1024];
        clean(student_directory_path);
        if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            strcpy(student_directory_path, temp_directory);
            strcat(student_directory_path, entry->d_name);
            student_dir = opendir(student_directory_path);
            struct dirent *student_entry;
            int got_c_file = 0;
            while ((student_entry = readdir(student_dir)) != NULL) {
                if (student_entry->d_name[strlen(student_entry->d_name) - 1] == 'c' &&
                    student_entry->d_name[strlen(student_entry->d_name) - 2] == '.') {
                    got_c_file = 1;
                    int time_out = 0;
                    chdir(student_directory_path);
                    char command[256];
                    clean(command);
                    strcpy(command, "gcc "), strcat(command, student_entry->d_name);
                    int exit_status = system(command);
                    // COMPILED
                    if (exit_status % 255 == 0) {
                        clean(command);
                        strcpy(command, "(./a.out < "), strcat(command, input_path), strcat(command, ") > temp.txt");
                        time_t t = time(NULL);
                        struct tm tm = *localtime(&t);
                        int start = tm.tm_sec;
                        system(command);
                        t = time(NULL);
                        tm = *localtime(&t);
                        int finish = tm.tm_sec;
                        if (finish < start){
                            finish+=60;
                        }
                        if(finish-start > 3){
                            time_out = 1;
                        }
                        if (time_out) {
                            char msg[1024];
                            strcpy(msg, entry->d_name);
                            strcat(msg, ",TIMEOUT,20\n");
                            strcat(results_buffer, msg);
                            system("rm temp.txt");
                            break;
                        }
                        char student_output_path[1024];
                        strcpy(student_output_path, student_directory_path);
                        strcat(student_output_path, "/temp.txt");
                        clean(command);
                        strcpy(command, "./comp.out '"), strcat(command, correct_output_path);
                        strcat(command, "' '"), strcat(command, student_output_path);
                        strcat(command, "'");
                        chdir(program_running_path);
                        int exit_val = system(command) % 255;
                        chdir(student_directory_path);
                        system("rm a.out");
                        system("rm temp.txt");
                        char msg[1024];
                        strcpy(msg, entry->d_name);
                        if (exit_val == 1) {
                            strcat(msg, ",EXCELLENT,100\n");
                            strcat(results_buffer, msg);
                        } else if (exit_val == 2) {
                            strcat(msg, ",WRONG,50\n");
                            strcat(results_buffer, msg);
                        } else if (exit_val == 3) {
                            strcat(msg, ",SIMILAR,75\n");
                            strcat(results_buffer, msg);
                        }
                    } else {
                        char msg[1024];
                        strcpy(msg, entry->d_name);
                        strcat(msg, ",COMPILATION_ERROR,10\n");
                        strcat(results_buffer, msg);
                    }
                    break;
                }
            }
            if (!got_c_file) {
                char msg[1024];
                strcpy(msg, entry->d_name);
                strcat(msg, ",NO_C_FILE,0\n");
                strcat(results_buffer, msg);
            }
            closedir(student_dir);
        }
    }
    chdir(program_running_path);
    system("touch results.csv");
    results_fd = open(results_path, O_WRONLY);
    if (results_fd == -1) {
        close(conf_fd);
        sprintf(err_msg, "%s %s\n", OF_ERR, results_path);
        p_error(err_msg);
    }
    int st = write(results_fd, results_buffer, strlen(results_buffer));
    if(st == -1){
        close(conf_fd);
        close(results_fd);
        p_error("Error writing to file results.csv\n");
    }
    close(results_fd);
    close(conf_fd);
}
