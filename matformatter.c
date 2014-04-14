#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ROW 30
#define COL 30

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
    int i, j, mat_a[ROW][COL], mat_b[ROW][COL];
    int cnt = 0, a_row = 0, a_col = 0, b_row = 0, b_col = 0;
    char *row = (char *) malloc(sizeof(char) * 128);
    char *row_cpy = (char *) malloc(sizeof(char) * 128);
int number_token;
    while (cnt != 1) {
        fgets(row, 128, stdin);
        row[strlen(row) - 1] = 0;
        strcpy(row_cpy, row);

        if (strlen(row) == 0) {
            cnt++;
        } else if (cnt == 0) {
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
        } else if ((number_token = n_token(row_cpy)) != a_col) {
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
        }
    }

    for (i = 0; i < a_row; i++) {
        for (j = 0; j < a_col; j++) {
            mat_b[j][i] = mat_a[i][j];
        }
    }
    b_row = a_col;
    b_col = a_row;

    // printf("The transpose of the input matrix:\n");
    for (i = 0; i < b_row; i++) {
        for (j = 0; j < b_col; j++) {
            printf("%d ", mat_b[i][j]);
        }
        printf("\n");
    }

    return 0;
}

