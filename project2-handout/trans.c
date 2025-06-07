//523031910732 杨佩尧
//姚明哲 523031910408

/*
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/*
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded.
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N]) {
    // all 12 stack variables
    int i, j, k;
    int temp0, temp1, temp2, temp3, temp4, temp5, temp6, temp7;
    int diag;
    // block_i is temp0
    // block_j is temp1

    if (N == 32) { // 32x32
        for (temp0 = 0; temp0 < N; temp0 += 8) {
            for (temp1 = 0; temp1 < M; temp1 += 8) {
                for (i = temp0; i < temp0 + 8 && i < N; i++) {
                    for (j = temp1; j < temp1 + 8 && j < M; j++) {
                        if (i != j)
                            B[j][i] = A[i][j];
                        else {
                            diag = A[i][j];
                            // Delay writing diagonal to reduce cache conflicts
                        }
                    }
                    if (temp0 == temp1) {
                        B[i][i] = diag;
                    }
                }
            }
        }
    } else if(N == 64) { // 64x64
        for (i = 0; i < N; i += 8) {
            for (j = 0; j < M; j += 8) {
                // upper 4 rows
                for (k = 0; k < 4; k++) {
                    temp0 = A[i + k][j + 0];
                    temp1 = A[i + k][j + 1];
                    temp2 = A[i + k][j + 2];
                    temp3 = A[i + k][j + 3];
                    temp4 = A[i + k][j + 4];
                    temp5 = A[i + k][j + 5];
                    temp6 = A[i + k][j + 6];
                    temp7 = A[i + k][j + 7];
                    // write first half into B directly
                    B[j + 0][i + k] = temp0;
                    B[j + 1][i + k] = temp1;
                    B[j + 2][i + k] = temp2;
                    B[j + 3][i + k] = temp3;
                    // write second half in temporary positions in B
                    B[j + 0][i + k + 4] = temp4;
                    B[j + 1][i + k + 4] = temp5;
                    B[j + 2][i + k + 4] = temp6;
                    B[j + 3][i + k + 4] = temp7;
                }
                // lower 4 rows
                for (k = 0; k < 4; k++) {
                    temp0 = B[j + k][i + 4];
                    temp1 = B[j + k][i + 5];
                    temp2 = B[j + k][i + 6];
                    temp3 = B[j + k][i + 7];
                    // write second half into B directly
                    B[j + k][i + 4] = A[i + 4][j + k];
                    B[j + k][i + 5] = A[i + 5][j + k];
                    B[j + k][i + 6] = A[i + 6][j + k];
                    B[j + k][i + 7] = A[i + 7][j + k];
                    // write first half in temporary positions in B
                    B[j + k + 4][i + 0] = temp0;
                    B[j + k + 4][i + 1] = temp1;
                    B[j + k + 4][i + 2] = temp2;
                    B[j + k + 4][i + 3] = temp3;
                }
                // the lower-right 4x4 block
                for (temp0 = 4; temp0 < 8; temp0++) {
                    for (temp1 = 4; temp1 < 8; temp1++) {
                        B[j + temp1][i + temp0] = A[i + temp0][j + temp1];
                    }
                }
            }
        }
    } else { // 61x67
        for (temp0 = 0; temp0 < N; temp0 += 16) {
            for (temp1 = 0; temp1 < M; temp1 += 16) {
                temp2 = (temp0 == temp1);
                for (i = temp0; i < temp0 + 16 && i < N; i++) {
                    if(temp2 && i < M){
                        for (j = temp1; j < temp1 + 16 && j < M; j++) {
                            if (i != j)
                                B[j][i] = A[i][j];
                            else {
                                diag = A[i][j];
                                // Delay writing diagonal to reduce cache conflicts
                            }
                        }
                        B[i][i] = diag;
                    } else {
                        for (j = temp1; j < temp1 + 16 && j < M; j++) {
                            B[j][i] = A[i][j];
                        }
                    }
                }
            }
        }
    }
}

/*
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started.
 */

 /*
  * trans - A simple baseline transpose function, not optimized for the cache.
  */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N]) {
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }

}

char trans_block_32_desc[] = "8x8 block transpose, submit for 32x32";
void trans_block_32(int M, int N, int A[N][M], int B[M][N]) {
    int block_i, block_j, i, j, diag;
    int block_size = 8;

    for (block_i = 0; block_i < N; block_i += block_size) {
        for (block_j = 0; block_j < M; block_j += block_size) {
            for (i = block_i; i < block_i + block_size && i < N; i++) {
                for (j = block_j; j < block_j + block_size && j < M; j++) {
                    if (i != j)
                        B[j][i] = A[i][j];
                    else {
                        diag = A[i][j];
                        // Delay writing diagonal to reduce cache conflicts
                    }
                }
                if (block_i == block_j) {
                    B[i][i] = diag;
                }
            }
        }
    }
}

// 16*8 block try, but failed
char trans_block_16x8_desc[] = "16*8 block transpose";
void trans_block_16x8(int M, int N, int A[N][M], int B[M][N]) {
    int block_i, block_j, i, j, diag;

    for (block_i = 0; block_i < N; block_i += 16) {
        for (block_j = 0; block_j < M; block_j += 8) {
            for (i = block_i; i < block_i + 16 && i < N; i++) {
                for (j = block_j; j < block_j + 8 && j < M; j++) {
                    if (i != j)
                        B[j][i] = A[i][j];
                    else {
                        diag = A[i][j];
                        // Delay writing diagonal to reduce cache conflicts
                    }
                }
                if ((block_i == block_j || block_i + 8 == block_j) && i >= block_j && i < block_j + 8) {
                    printf("blocki %d blockj %d i %d j %d\n", block_i, block_j, i, j);
                    B[i][i] = diag;
                }
            }
        }
    }
}

char trans_block_super_desc[] = "Super optimized transpose with blocking, submit for 64x64";
void trans_block_super(int M, int N, int A[N][M], int B[M][N]) {
    int i, j, k;
    int temp0, temp1, temp2, temp3, temp4, temp5, temp6, temp7;
    for (i = 0; i < N; i += 8) {
        for (j = 0; j < M; j += 8) {
            // upper 4 rows
            for (k = 0; k < 4; k++) {
                temp0 = A[i + k][j + 0];
                temp1 = A[i + k][j + 1];
                temp2 = A[i + k][j + 2];
                temp3 = A[i + k][j + 3];
                temp4 = A[i + k][j + 4];
                temp5 = A[i + k][j + 5];
                temp6 = A[i + k][j + 6];
                temp7 = A[i + k][j + 7];
                // write first half into B directly
                B[j + 0][i + k] = temp0;
                B[j + 1][i + k] = temp1;
                B[j + 2][i + k] = temp2;
                B[j + 3][i + k] = temp3;
                // write second half in temporary positions in B
                B[j + 0][i + k + 4] = temp4;
                B[j + 1][i + k + 4] = temp5;
                B[j + 2][i + k + 4] = temp6;
                B[j + 3][i + k + 4] = temp7;
            }
            // lower 4 rows
            for (k = 0; k < 4; k++) {
                temp0 = B[j + k][i + 4];
                temp1 = B[j + k][i + 5];
                temp2 = B[j + k][i + 6];
                temp3 = B[j + k][i + 7];
                // write second half into B directly
                B[j + k][i + 4] = A[i + 4][j + k];
                B[j + k][i + 5] = A[i + 5][j + k];
                B[j + k][i + 6] = A[i + 6][j + k];
                B[j + k][i + 7] = A[i + 7][j + k];
                // write first half in temporary positions in B
                B[j + k + 4][i + 0] = temp0;
                B[j + k + 4][i + 1] = temp1;
                B[j + k + 4][i + 2] = temp2;
                B[j + k + 4][i + 3] = temp3;
            }
            // the lower-right 4x4 block
            for (k = 4; k < 8; k++) {
                for (int l = 4; l < 8; l++) {
                    B[j + l][i + k] = A[i + k][j + l];
                }
            }
        }
    }
}

char trans_block_super_var_desc[] = "Variant of super optimized transpose with blocking";
void trans_block_super_var(int M, int N, int A[N][M], int B[M][N]) {
    int i, j, k;
    int temp0, temp1, temp2, temp3, temp4, temp5, temp6, temp7;
    for (i = 0; i < N; i += 8) {
        for (j = 0; j < M; j += 8) {
            // upper 4 rows
            for (k = 0; k < 4; k++) {
                temp0 = A[i + k][j + 0];
                temp1 = A[i + k][j + 1];
                temp2 = A[i + k][j + 2];
                temp3 = A[i + k][j + 3];
                temp4 = A[i + k][j + 4];
                temp5 = A[i + k][j + 5];
                temp6 = A[i + k][j + 6];
                temp7 = A[i + k][j + 7];
                // write first half into B directly
                B[j + 0][i + k] = temp0;
                B[j + 1][i + k] = temp1;
                B[j + 2][i + k] = temp2;
                B[j + 3][i + k] = temp3;
                // write second half in temporary positions in B
                B[j + 0][i + k + 4] = temp4;
                B[j + 1][i + k + 4] = temp5;
                B[j + 2][i + k + 4] = temp6;
                B[j + 3][i + k + 4] = temp7;
            }
            // lower 4 rows
            for (k = 0; k < 4; k++) {
                temp0 = A[i + k + 4][j + 0];
                temp1 = A[i + k + 4][j + 1];
                temp2 = A[i + k + 4][j + 2];
                temp3 = A[i + k + 4][j + 3];
                temp4 = A[i + k + 4][j + 4];
                temp5 = A[i + k + 4][j + 5];
                temp6 = A[i + k + 4][j + 6];
                temp7 = A[i + k + 4][j + 7];
                // write second half into B directly
                B[j + 4][i + k + 4] = temp4;
                B[j + 5][i + k + 4] = temp5;
                B[j + 6][i + k + 4] = temp6;
                B[j + 7][i + k + 4] = temp7;
                // write first half in temporary positions in B
                B[j + 4][i + k] = temp0;
                B[j + 5][i + k] = temp1;
                B[j + 6][i + k] = temp2;
                B[j + 7][i + k] = temp3;
            }
            /* Swap */
            for (k = 0; k < 4; k++) {
                temp0 = B[j + k][i + 4];
                temp1 = B[j + k][i + 5];
                temp2 = B[j + k][i + 6];
                temp3 = B[j + k][i + 7];

                B[j + k][i + 4] = B[j + k + 4][i + 0];
                B[j + k][i + 5] = B[j + k + 4][i + 1];
                B[j + k][i + 6] = B[j + k + 4][i + 2];
                B[j + k][i + 7] = B[j + k + 4][i + 3];

                B[j + k + 4][i + 0] = temp0;
                B[j + k + 4][i + 1] = temp1;
                B[j + k + 4][i + 2] = temp2;
                B[j + k + 4][i + 3] = temp3;
            }
        }
    }
}

char trans_block_61x67_desc[] = "61*67 block transpose";
void trans_block_61x67(int M, int N, int A[N][M], int B[M][N]) {
    int i, j, k;
    int temp0, temp1, temp2, temp3, temp4, temp5, temp6, temp7;

    for (i = 0; i < 64; i += 8) {
        for (j = 0; j < 56; j += 8) {
            // upper 4 rows
            for (k = 0; k < 4; k++) {
                temp0 = A[i + k][j + 0];
                temp1 = A[i + k][j + 1];
                temp2 = A[i + k][j + 2];
                temp3 = A[i + k][j + 3];
                temp4 = A[i + k][j + 4];
                temp5 = A[i + k][j + 5];
                temp6 = A[i + k][j + 6];
                temp7 = A[i + k][j + 7];
                // write first half into B directly
                B[j + 0][i + k] = temp0;
                B[j + 1][i + k] = temp1;
                B[j + 2][i + k] = temp2;
                B[j + 3][i + k] = temp3;
                // write second half in temporary positions in B
                B[j + 0][i + k + 4] = temp4;
                B[j + 1][i + k + 4] = temp5;
                B[j + 2][i + k + 4] = temp6;
                B[j + 3][i + k + 4] = temp7;
            }
            // lower 4 rows
            for (k = 0; k < 4; k++) {
                temp0 = B[j + k][i + 4];
                temp1 = B[j + k][i + 5];
                temp2 = B[j + k][i + 6];
                temp3 = B[j + k][i + 7];
                // write second half into B directly
                B[j + k][i + 4] = A[i + 4][j + k];
                B[j + k][i + 5] = A[i + 5][j + k];
                B[j + k][i + 6] = A[i + 6][j + k];
                B[j + k][i + 7] = A[i + 7][j + k];
                // write first half in temporary positions in B
                B[j + k + 4][i + 0] = temp0;
                B[j + k + 4][i + 1] = temp1;
                B[j + k + 4][i + 2] = temp2;
                B[j + k + 4][i + 3] = temp3;
            }
            // the lower-right 4x4 block
            for (temp0 = 4; temp0 < 8; temp0++) {
                for (temp1 = 4; temp1 < 8; temp1++) {
                    B[j + temp1][i + temp0] = A[i + temp0][j + temp1];
                }
            }
        }
    }
    // i = 64, j = 56
    // 0 <= i < 67, 56 <= j < 61
    for(k = 0; k < N; k++){
        temp0 = A[k][j + 0];
        temp1 = A[k][j + 1];
        temp2 = A[k][j + 2];
        temp3 = A[k][j + 3];
        temp4 = A[k][j + 4];

        B[j + 0][k] = temp0;
        B[j + 1][k] = temp1;
        B[j + 2][k] = temp2;
        B[j + 3][k] = temp3;
        B[j + 4][k] = temp4;
    }
    // 64 <= i < 67, 0 <= j < 56
    for(k = 0; k < j; k++){
        temp0 = A[i + 0][k];
        temp1 = A[i + 1][k];
        temp2 = A[i + 2][k];

        B[k][i + 0] = temp0;
        B[k][i + 1] = temp1;
        B[k][i + 2] = temp2;
    }
}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions() {
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc);

    /* Register any additional transpose functions */
    // registerTransFunction(trans, trans_desc);

    // registerTransFunction(trans_block_32, trans_block_32_desc);

    // registerTransFunction(trans_block_16x8, trans_block_16x8_desc);

    // registerTransFunction(trans_block_super, trans_block_super_desc);

    // registerTransFunction(trans_block_super_var, trans_block_super_var_desc);

    // registerTransFunction(trans_block_61x67, trans_block_61x67_desc);

}

/*
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N]) {
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

