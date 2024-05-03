#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <math.h>
#include <fcntl.h>
#include <time.h> 

// Function to convert number string to integer
int string_to_int(char *str) {
    if (strcmp(str, "nol") == 0) return 0;
    else if (strcmp(str, "satu") == 0) return 1;
    else if (strcmp(str, "dua") == 0) return 2;
    else if (strcmp(str, "tiga") == 0) return 3;
    else if (strcmp(str, "empat") == 0) return 4;
    else if (strcmp(str, "lima") == 0) return 5;
    else if (strcmp(str, "enam") == 0) return 6;
    else if (strcmp(str, "tujuh") == 0) return 7;
    else if (strcmp(str, "delapan") == 0) return 8;
    else if (strcmp(str, "sembilan") == 0) return 9;
    else return -1; // Error
}

// Function to convert number to words
void int_to_words(int num, char *words) {
    const char *ones[] = {"", "satu", "dua", "tiga", "empat", "lima", "enam", "tujuh", "delapan", "sembilan"};
    const char *tens[] = {"", "sepuluh", "dua puluh", "tiga puluh", "empat puluh", "lima puluh", "enam puluh", "tujuh puluh", "delapan puluh", "sembilan puluh"};
    const char *teens[] = {"sepuluh", "sebelas", "dua belas", "tiga belas", "empat belas", "lima belas", "enam belas", "tujuh belas", "delapan belas", "sembilan belas"};

    if (num < 10) {
        strcpy(words, ones[num]);
    } else if (num < 20) {
        strcpy(words, teens[num - 10]);
    } else {
        int digit1 = num % 10;
        int digit2 = num / 10;
        if (digit1 == 0) {
            strcpy(words, tens[digit2]);
        } else {
            sprintf(words, "%s %s", tens[digit2], ones[digit1]);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2 || (strcmp(argv[1], "-kali") != 0 && strcmp(argv[1], "-tambah") != 0 && strcmp(argv[1], "-kurang") != 0 && strcmp(argv[1], "-bagi") != 0)) {
        
        return 1;
    }

    int fd1[2], fd2[2];
    if (pipe(fd1) == -1 || pipe(fd2) == -1) {
        perror("Pipe failed");
        return 1;
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("Fork failed");
        return 1;
    }

    if (pid > 0) { // Parent process
        close(fd1[0]); // Close reading end of first pipe
        close(fd2[1]); // Close writing end of second pipe

        char input_str1[20], input_str2[20];
        fflush(stdout); // Flush stdout buffer to ensure prompt is displayed
        scanf("%s %s", input_str1, input_str2);

        int num1 = string_to_int(input_str1);
        int num2 = string_to_int(input_str2);

        if (num1 == -1 || num2 == -1) {
            printf("Invalid input\n");
            return 1;
        }

        int result;
        if (strcmp(argv[1], "-kali") == 0) {
            result = num1 * num2;
        } else if (strcmp(argv[1], "-tambah") == 0) {
            result = num1 + num2;
        } else if (strcmp(argv[1], "-kurang") == 0) {
            result = num1 - num2;
        } else { // "-bagi"
            if (num2 == 0) {
                printf("ERROR: Division by zero\n");
                return 1;
            }
            result = num1 / num2;
        }

        write(fd1[1], &result, sizeof(result));
        close(fd1[1]);

        wait(NULL);

        char result_str[100];
        read(fd2[0], result_str, sizeof(result_str));
        printf("%s\n", result_str);
        close(fd2[0]);

        // Writing to history.log
        FILE *history = fopen("histori.log", "a");
        if (history != NULL) {
            
            char log_message[256];
            char operation_type[10];
            
            // Get current date and time
            time_t current_time;
            struct tm *local_time;
            char time_str[20];

            time(&current_time);
            local_time = localtime(&current_time);
            strftime(time_str, sizeof(time_str), "%d/%m/%y %H:%M:%S", local_time);
            
            if (strcmp(argv[1], "-kali") == 0) {
                 sprintf(log_message, "[%s] [KALI] %s kali %s sama dengan %s\n", time_str, input_str1, input_str2, result_str);
            } else if (strcmp(argv[1], "-tambah") == 0) {
                sprintf(log_message, "[%s] [TAMBAH] %s tambah %s sama dengan %s\n", time_str, input_str1, input_str2, result_str);
            } else if (strcmp(argv[1], "-kurang") == 0) {
            	if(result < 0) {
            	sprintf(log_message, "[%s] [KURANG] ERROR pada pengurangan\n", time_str);
            	}
            	else {
                sprintf(log_message, "[%s] [KURANG] %s kurang %s sama dengan %s\n", time_str, input_str1, input_str2, result_str);
                }
            } else { // "-bagi"
                sprintf(log_message, "[%s] [BAGI] %s bagi %s sama dengan %s\n", time_str, input_str1, input_str2, result_str);
            }

            

            // Write log message to file
            
            fputs(log_message, history);
            fclose(history);
        } else {
            printf("Error writing to history.log\n");
        }
    } else { // Child process
        close(fd1[1]); // Close writing end of first pipe
        close(fd2[0]); // Close reading end of second pipe

        int result;
        read(fd1[0], &result, sizeof(result));

        char result_str[100];
        if (result == 0) {
            strcpy(result_str, "nol");
        } else if (result < 0) {
            strcpy(result_str, "ERROR");
        } else {
            int_to_words(result, result_str);
        }

        write(fd2[1], result_str, sizeof(result_str));
        close(fd2[1]);

        exit(0);
    }

    return 0;
}
