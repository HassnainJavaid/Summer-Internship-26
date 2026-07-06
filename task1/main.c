#define _POSIX_C_SOURCE 200112L
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <blis/blis.h>

static double rand_double()
{
    return (double)rand() / (double)RAND_MAX;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage: %s <N> <RunNumber>\n", argv[0]);
        return 1;
    }

    const dim_t N = atoi(argv[1]);
    const int run = atoi(argv[2]);

    srand(run);

    size_t elements = (size_t)N * (size_t)N;

    double *A = (double *)malloc(elements * sizeof(double));
    double *B = (double *)malloc(elements * sizeof(double));
    double *C = (double *)malloc(elements * sizeof(double));

    if (!A || !B || !C)
    {
        printf("Memory allocation failed.\n");
        return 1;
    }

    for (size_t i = 0; i < elements; i++)
    {
        A[i] = rand_double();
        B[i] = rand_double();
        C[i] = 0.0;
    }

    double alpha = 1.0;
    double beta = 0.0;

    struct timespec start, end;

    clock_gettime(CLOCK_MONOTONIC, &start);

    bli_dgemm(
        BLIS_NO_TRANSPOSE,
        BLIS_NO_TRANSPOSE,
        N, N, N,
        &alpha,
        A, N, 1,
        B, N, 1,
        &beta,
        C, N, 1
    );

    clock_gettime(CLOCK_MONOTONIC, &end);

    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) * 1e-9;
    double flops = 2.0 * (double)N * (double)N * (double)N;
    double gflops = flops / (elapsed * 1e9);

    // Master execution tracking file
    FILE *fp = fopen("blis_benchmark_results.csv", "a");
    if (!fp)
    {
        printf("Cannot open results file.\n");
        free(A); free(B); free(C);
        return 1;
    }

    const char *threads = getenv("BLIS_NUM_THREADS");
    if (!threads) threads = "Unknown";

    fprintf(fp, "%d,%d,%s,%.9f,%.0f,%.6f\n", run, (int)N, threads, elapsed, flops, gflops);
    fclose(fp);

    free(A);
    free(B);
    free(C);

    return 0;
}