// matmult_p.c:
// Multiplies two input matrixes using multiple child processes
// Author: Nilesh Khaitan, YongAn Lee
// CS410 - Spring 2014

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>

#define ROW 100
#define COL 100

int to_num(char *token) {
    int i, val = 0, len = strlen(token);
    for (i = 0; i < len; i++) {
        val += (token[i] - 48) * pow(10, (len - i - 1));
    }
    return val;
}

int n_token(char *row_cpy) {
    int n_tok = 0;
    char *token = strtok(row_cpy, " ");
    while (token) {
        n_tok++;
        token = strtok(NULL, " ");
    }
    return n_tok;
}

int main(int argc, char *argv[]) {
    // memory aligned result, mat_a, mat_b with 900 * 4 bytes gaps
    int *mem_shared, *mem_shared_cpy;
    int mat_b[ROW][COL], mat_a[ROW][COL];
    int cnt = 0, a_row = 0, a_col = 0, b_row = 0, b_col = 0;
    int i, j, shmid, c_row, c_col;
    char *row = (char *) malloc(sizeof(char) * 128);
    char *row_cpy = (char *) malloc(sizeof(char) * 128);

    while (cnt != 2) {
        fgets(row, 128, stdin);
        row[strlen(row) - 1] = 0;
        strcpy(row_cpy, row);

        if (strlen(row) == 0) {                  // empty line
            cnt++;
        } else if (cnt == 0) {                   // mat_a
            // sets the number of columns of mat_a
            if (a_col == 0) {
                a_col = n_token(row_cpy);
                strcpy(row_cpy, row);

                int val = 0, tok_n = 0;
                char *token = strtok(row_cpy, " ");
                while (token) {
                    val = to_num(token);
                    mat_a[a_row][tok_n] = val;
                    tok_n++;
                    token = strtok(NULL, " ");
                }
            } else if (n_token(row_cpy) != a_col) {
                printf("inconsistent number of columns\n");
                exit(0);
            } else {
                strcpy(row_cpy, row);

                int val = 0, tok_n = 0;
                char *token = strtok(row_cpy, " ");
                while (token) {
                    val = to_num(token);
                    mat_a[a_row][tok_n] = val;
                    tok_n++;
                    token = strtok(NULL, " ");
                }
            }

            a_row++;
        } else if (cnt == 1) {                   // mat_b
            // sets the number of columns of mat_b
            if (b_col == 0) {
                b_col = n_token(row_cpy);
                strcpy(row_cpy, row);

                int val = 0, tok_n = 0;
                char *token = strtok(row_cpy, " ");
                while (token) {
                    val = to_num(token);
                    mat_b[b_row][tok_n] = val;
                    tok_n++;
                    token = strtok(NULL, " ");
                }
            } else if (n_token(row_cpy) != b_col) {
                printf("inconsistent number of columns\n");
                exit(0);
            } else {
                strcpy(row_cpy, row);

                int val = 0, tok_n = 0;
                char *token = strtok(row_cpy, " ");
                while (token) {
                    val = to_num(token);
                    mat_b[b_row][tok_n] = val;
                    tok_n++;
                    token = strtok(NULL, " ");
                }
            }

            b_row++;
        }
    }

    if (a_col != b_row) {
        printf("the two matrixes cannot be multiplied\n");
        exit(0);
    }

    int a_size = a_row * a_col, b_size = b_row * b_col, c_size = a_row * b_col;
    if ((shmid = shmget(IPC_PRIVATE, (a_size + b_size + c_size) * sizeof(int), 0600)) < 0) {
        printf("shmid return failed\n");
        exit(0);
    }

    if ((mem_shared = shmat(shmid, 0, 0)) == (void *) - 1) {
        printf("shmat error\n");
        shmctl(shmid, IPC_RMID, NULL);
        exit(0);
    }

    mem_shared_cpy = mem_shared;
    for (i = 0; i < a_row; i++) {
        for (j = 0; j < a_col; j++) {
            *mem_shared_cpy = mat_a[i][j];
            mem_shared_cpy++;
        }
    }
    for (i = 0; i < b_row; i++) {
        for (j = 0; j < b_col; j++) {
            *mem_shared_cpy = mat_b[i][j];
            mem_shared_cpy++;
        }
    }
    
    char str_i[8], str_j[8], str_shmid[32];
    char str_a_row[8], str_a_col[8], str_b_row[8], str_b_col[8];
    c_row = a_row;
    c_col = b_col;
    for (i = 0; i < c_row; i++) {
        for (j = 0; j < c_col; j++) { 
            pid_t pid;
            if ((pid = fork()) < 0) {
                printf("fork failed\n");
                error(0);
            }
            
            if (pid == 0) {
                snprintf(str_i, 8, "%d", i);
                snprintf(str_j, 8, "%d", j);
                snprintf(str_shmid, 32, "%d", shmid);
                snprintf(str_a_row, 8, "%d", a_row);
                snprintf(str_a_col, 8, "%d", a_col);
                snprintf(str_b_row, 8, "%d", b_row);
                snprintf(str_b_col, 8, "%d", b_col);
                if (execlp("./multiply", "multiply", str_i, str_j, str_shmid, 
                    str_a_row, str_a_col, str_b_row, str_b_col, (char *) NULL) < 0) {
                    printf("exec family error\n");
                    exit(0);
                }
                exit(0);                         // kill the child
            }
        }
    }
    
    for (i = 0; i < c_row * c_col; i++) {
        waitpid(-1, NULL, 0);                    // waits for all of its children
    }
 
    mem_shared_cpy = mem_shared;
    mem_shared_cpy += (a_row * a_col) + (b_row * b_col);
    // printf("The product of the two input matrixes is:\n");
    for (i = 0; i < a_row; i++) {
        for (j = 0; j < b_col; j++) {
            printf("%d ", *mem_shared_cpy);
            mem_shared_cpy++;
        }
        printf("\n");
    }
    printf("\n");
    shmctl(shmid, IPC_RMID, NULL);               // get rid of the shmid at the end

    return 0;
}
