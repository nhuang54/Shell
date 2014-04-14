// matmult_t.c
// Multiplies two input matrixes via multithreading approach.
// Aurhots: Nilesh Khaitan, YongAn Lee
// CS410 - Spring 2014

// thread implementation using setjmp/longjmp is codumented in
// R.Engleschall's "Portable Multithreading: the Signal Stack Trick
// for User-Space Thread Creation" in Proceedings of the USENIX
// Annual Technical Conference, 2000. The macros and some of the
// source codes used in the program is a direct implementation of
// the document acknowledged above.

#include <math.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// the cited document above says the UNIX platform require at lesat 16K
#define STACK_SZ 16777216
#define ROW 100
#define COL 100

#define mctx_save(mctx) \
setjmp((mctx)->jb)

#define mctx_restore(mctx) \
longjmp((mctx)->jb, 1)

#define mctx_switch(mctx_old,mctx_new) \
if (setjmp((mctx_old)->jb) == 0 ) \
longjmp((mctx_new)->jb, 1)

typedef struct mctx_st {
    jmp_buf jb;
} mctx_t;

static jmp_buf end;
static int mat_a[ROW][COL], mat_b[ROW][COL], mat_c[ROW][COL];
static int a_row, a_col, b_row, b_col, c_row, c_col;
static int n_thr = 0, max_thr = 0;
static mctx_t thread[ROW * COL];
static char *stack[STACK_SZ];

static mctx_t mctx_caller;
static sig_atomic_t mctx_called;

static mctx_t *mctx_creat;
static void (*mctx_creat_func)(int);
static int mctx_creat_arg1;

static sigset_t mctx_creat_sigs;

void mctx_create_boot() {
    void (*mctx_start_func)(int);
    int mctx_start_arg1;

    sigprocmask(SIG_SETMASK, &mctx_creat_sigs, NULL);
  
    mctx_start_func = mctx_creat_func;
    mctx_start_arg1 = mctx_creat_arg1;
  
    mctx_switch(mctx_creat, &mctx_caller);
  
    mctx_start_func(mctx_start_arg1);
}

void mctx_create_trampoline(int sig) {
    if (mctx_save(mctx_creat) == 0) {
        mctx_called = 1;
        return;
    }
  
    mctx_create_boot();
}

void mctx_create (mctx_t *mctx, void (*sf_addr) (int),  
        int sf_arg1, void *sk_addr, size_t sk_size) {
    struct sigaction sa;
    struct sigaction osa;
    struct sigaltstack ss;
    struct sigaltstack oss;
    sigset_t osigs;
    sigset_t sigs;
  
    sigemptyset(&sigs);
    sigaddset(&sigs, SIGUSR1);
    sigprocmask(SIG_BLOCK, &sigs, &osigs);
  
    memset((void *) &sa, 0, sizeof(struct sigaction));
    sa.sa_handler = mctx_create_trampoline;
    sa.sa_flags = SA_ONSTACK;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGUSR1, &sa, &osa);
  
    ss.ss_sp = sk_addr;
    ss.ss_size = sk_size;
    ss.ss_flags = 0;
    sigaltstack(&ss, &oss);
  
    mctx_creat = mctx;
    mctx_creat_func = sf_addr;
    mctx_creat_arg1 = sf_arg1;
    mctx_creat_sigs = osigs;
    mctx_called = 0;
    kill(getpid(), SIGUSR1);
    sigfillset(&sigs);
    sigdelset(&sigs, SIGUSR1);
    while(!mctx_called)
        sigsuspend(&sigs);
  
    sigaltstack(NULL, &ss);
    ss.ss_flags = SS_DISABLE;
    sigaltstack(&ss, NULL);
    if (!(oss.ss_flags & SS_DISABLE))
        sigaltstack(&oss, NULL);
    sigaction(SIGUSR1, &osa, NULL);
    sigprocmask(SIG_SETMASK, &osigs, NULL);
  
    mctx_switch(&mctx_caller, mctx);
  
    return;
}

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

void multiply(int thr_id) {
    int i, row = (thr_id / b_col), col = (thr_id % b_col);
    int mult_row[a_row], mult_col[b_col];

    for (i = 0; i < a_col; i++) {
        mult_row[i] = mat_a[row][i];
    }

    for (i = 0; i < b_row; i++) {
        mult_col[i] = mat_b[i][col];
    }

    int prod = 0;
    for (i = 0; i < a_col; i++) {
        prod += (mult_row[i] * mult_col[i]);
    }
    mat_c[row][col] = prod;
    n_thr++;

    // printf("n_thr: %d, thr_id: %d\n", n_thr, thr_id);

    // if there are more elements to be multiplied,
    // it switches before jumping to the end
    if (n_thr < max_thr) {
        mctx_switch(&thread[thr_id], &thread[thr_id + 1]);
    }

    longjmp(end, 1);                             // jump to the end
}

void my_thr_create(void (* func) (int), int thr_id) {
    int size = STACK_SZ / 2024;
    stack[thr_id] = (char *) malloc(STACK_SZ / 262144);

    mctx_create(&thread[thr_id], func, thr_id, 
                (void *) stack[thr_id] + size, size);
    return;
}

int main(int argc, char *argv[]) {
    int i, j, cnt = 0;
    char *row = (char *) malloc(sizeof(char) * 128);
    char *row_cpy = (char *) malloc(sizeof(char) * 128);

    a_row = 0, a_col = 0, b_row = 0, b_col = 0;
    
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
 
    int thr_id, c_row = a_row, c_col = b_col;
    for (i = 0; i < c_row; i++) {
        for (j = 0; j < c_col; j++) {
            thr_id = ((c_col * i) + j);
            my_thr_create(multiply, thr_id);
        }
    }
    max_thr = c_row * c_col;

    if (setjmp(end) == 0) {
        mctx_switch(&mctx_caller, &thread[0]);
    }

    // printf("The product of the two input matrixees:\n");
    for (i = 0; i < c_row; i++) {
        for (j = 0; j < c_col; j++) {
            printf("%d ", mat_c[i][j]);
        }
        printf("\n");
    }
    printf("\n");
    return 0;
}

