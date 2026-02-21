// a1.c — Assignment 1 Step 1: CLI + allocation + init
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <limits.h>
#include <omp.h>

//print for us the error message
static void die(const char *msg) {
    fprintf(stderr, "Error: %s\n", msg);
    exit(EXIT_FAILURE);
}

//convert the input to long
static long parse_long(const char *s, const char *name) {
    errno = 0;
    char *end = NULL;
    long v = strtol(s, &end, 10);
    if (errno != 0 || end == s || *end != '\0') {
        fprintf(stderr, "Error: invalid %s '%s'\n", name, s);
        exit(EXIT_FAILURE);
    }
    return v;
}

//format the 2D array to 1D row-column major
static size_t idx(size_t i, size_t j, size_t N) {
    return i * N + j;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "structure is : %s N mode\n", argv[0]);
        fprintf(stderr, "  N: matrix dimension (e.g: 512, 1024, 2048, …)\n");
        fprintf(stderr, "  mode: (0 to 6) \n");
        return EXIT_FAILURE;
    }

    long N_in = parse_long(argv[1], "N");
    long mode_in = parse_long(argv[2], "mode");

    if (N_in <= 0) die("N must be > 0");
    if (mode_in < 0 || mode_in > 6) die("mode must be between 0 and 6");

    const size_t N = (size_t)N_in;
    const int mode = (int)mode_in;

    // Overflow-safe element count check: N*N must fit in size_t
    if (N != 0 && N > (SIZE_MAX / N)) die("N is too large (N*N overflows)");
    const size_t elems = N * N;

    // Allocate contiguous matrices
    double *A = (double*)malloc(elems * sizeof(double));
    double *B = (double*)malloc(elems * sizeof(double));
    double *C = (double*)malloc(elems * sizeof(double));
    if (!A || !B || !C) die("malloc failed (not enough memory)");

    // Deterministic initialization with the same formula
    for (size_t i = 0; i < N; i++) {
        for (size_t j = 0; j < N; j++) {
            A[idx(i,j,N)] = sin((double)i) * cos((double)j) + sqrt((double)(i + j + 1));
            B[idx(i,j,N)] = cos((double)i) * sin((double)j) + sqrt((double)(i + j + 2));
            C[idx(i,j,N)] = 0.0;
        }
    }

    double serial_multiplication, serial_multiplication_end, start_s,end_s = 0.0;

    //start with mode 0 (serial)
    if (mode ==0){
        //start serial time
        start_s = omp_get_wtime();
        //implement the multiplication and adding it to c
        serial_multiplication= omp_get_wtime();
        for (size_t i = 0; i < N; i++) {
            for (size_t j = 0; j < N; j++) {
                double sum = 0.0;
                for (size_t k = 0; k < N; k++) {
                    sum += A[idx(i,k,N)] * B[idx(k,j,N)];
                }
                C[idx(i,j,N)] = sum;
            }
        }
        serial_multiplication_end = omp_get_wtime();

        double final_m= serial_multiplication_end - serial_multiplication;
        
        //calcylate the sum, max and the checksum formula
        double sumC = 0.0;
        double maxC = -INFINITY;
        long long checksum = 0;
        double serial_sum_max_check, serial_sum_max_check_end = 0.0;
        serial_sum_max_check = omp_get_wtime();
        for (size_t i = 0; i < N; i++) {
            for (size_t j = 0; j < N; j++) {
                double v = C[idx(i,j,N)];
                sumC += v;
                if (v > maxC) maxC = v;

                // cast it from float to integers
                long long term = ((long long)(v * 1000.0)) % 100000;
                checksum += term;
            }
        }

        serial_sum_max_check_end = omp_get_wtime();
        end_s = omp_get_wtime();
        //time to run the multiplocation,sum andb checksum
        double final_sum_max= serial_sum_max_check_end-serial_sum_max_check;
        //whole time to run serial
        double final_s=end_s - start_s ;

        printf("N=%zu mode=%d omp_max_threads=%d\n", N, mode, omp_get_max_threads());
        printf("Matrix Multiplication and C serial time =%.6f\n", final_m);
        printf("SUM, Max, checkSum time =%.6f\n", final_sum_max);
        printf("Total serial time=%.6f\n", final_s);
        printf("sumC=%.6f maxC=%.6f checksum=%lld\n", sumC, maxC, checksum);

    }

    free(A);
    free(B);
    free(C);
    return EXIT_SUCCESS;
}
