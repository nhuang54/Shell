// multiply.c:
// Program used by matmult_p to multiply a row and a column of matrixes
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

int to_num(char *token) {
    int i, val = 0, len = strlen(token);
    for (i = 0; i < len; i++) {
        val += (token[i] - 48) * pow(10, (len - i - 1));
    }
    return val;
}

int main(int argc, char *argv[]) {
    int *mem_shared, *mem_shared_cpy;
    int i, j, row, col, shmid, a_row, a_col, b_row, b_col, prod;
    char *str_row, *str_col, *str_shmid;
    char *str_a_row, *str_a_col, *str_b_row, *str_b_col;

    str_row = argv[1];
    str_col = argv[2];
    str_shmid = argv[3];
    str_a_row = argv[4];
    str_a_col = argv[5];
    str_b_row = argv[6];
    str_b_col = argv[7];
    row = to_num(str_row);
    col = to_num(str_col);
    shmid = to_num(str_shmid);
    a_row = to_num(str_a_row);
    a_col = to_num(str_a_col);
    b_row = to_num(str_b_row);
    b_col = to_num(str_b_col);

    if ((mem_shared = shmat(shmid, 0, 0)) == (void *) - 1) {
        printf("shmat error\n");
        exit(0);
    }

    int mult_row[a_col], mult_col[b_row];

    // fetch the elements of the row to be multiplied
    mem_shared_cpy = mem_shared;
    mem_shared_cpy += (row * a_col);
    for (i = 0; i < a_col; i++) {
        mult_row[i] = *mem_shared_cpy;
        mem_shared_cpy++;
    }

    // fetch the elements of the col to be multiplied
    mem_shared_cpy = mem_shared;
    mem_shared_cpy += (a_row * a_col) + col;
    for (i = 0; i < b_row; i++) {
        mult_col[i] = *mem_shared_cpy;
        mem_shared_cpy += b_col;
    }

    // dot product
    prod = 0;
    for (i = 0; i < a_col; i++) {
        prod += (mult_row[i] * mult_col[i]);
    }

    // place it at the right offset
    mem_shared_cpy = mem_shared;
    mem_shared_cpy += ((a_row * a_col) + (b_row * b_col));
    mem_shared_cpy += (col + (row * b_col));
    *mem_shared_cpy = prod;

    return 0;
}
