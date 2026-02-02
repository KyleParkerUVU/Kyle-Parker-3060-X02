#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

/*
  Thread returns this struct via pthread_exit().
  Parent frees it after printing.
*/
typedef struct {
    int original;
    int count;
    int *factors;   // dynamically allocated array of factors
} factor_result_t;

typedef struct {
    int value;
} thread_arg_t;

/*
  Trial division factorization.
  Produces factors in nondecreasing order naturally.
*/
static factor_result_t *factor_number(int n) {
    factor_result_t *res = (factor_result_t *)malloc(sizeof(factor_result_t));
    if (!res) return NULL;

    res->original = n;
    res->count = 0;

    // Max prime factors is stated as 10, but allocate a bit more safely.
    int capacity = 16;
    res->factors = (int *)malloc(sizeof(int) * capacity);
    if (!res->factors) {
        free(res);
        return NULL;
    }

    int x = n;

    // Factor out 2s
    while (x % 2 == 0) {
        if (res->count == capacity) {
            capacity *= 2;
            int *tmp = (int *)realloc(res->factors, sizeof(int) * capacity);
            if (!tmp) {
                free(res->factors);
                free(res);
                return NULL;
            }
            res->factors = tmp;
        }
        res->factors[res->count++] = 2;
        x /= 2;
    }

    // Factor odd divisors
    for (int d = 3; (long long)d * d <= x; d += 2) {
        while (x % d == 0) {
            if (res->count == capacity) {
                capacity *= 2;
                int *tmp = (int *)realloc(res->factors, sizeof(int) * capacity);
                if (!tmp) {
                    free(res->factors);
                    free(res);
                    return NULL;
                }
                res->factors = tmp;
            }
            res->factors[res->count++] = d;
            x /= d;
        }
    }

    // If remainder is > 1, it's a prime factor
    if (x > 1) {
        if (res->count == capacity) {
            capacity *= 2;
            int *tmp = (int *)realloc(res->factors, sizeof(int) * capacity);
            if (!tmp) {
                free(res->factors);
                free(res);
                return NULL;
            }
            res->factors = tmp;
        }
        res->factors[res->count++] = x;
    }

    return res;
}

static void *thread_main(void *arg) {
    thread_arg_t *targ = (thread_arg_t *)arg;
    int n = targ->value;

    factor_result_t *res = factor_number(n);

    // Free the argument struct allocated by parent
    free(targ);

    // Return result struct pointer to parent
    pthread_exit((void *)res);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage:%s <number to factor>...\n", argv[0]);
        return 0;
    }

    int num_threads = argc - 1;

    // Per assignment: no more than 25 numbers
    if (num_threads > 25) {
        fprintf(stderr, "Error: too many numbers (max 25).\n");
        return 1;
    }

    pthread_t *threads = (pthread_t *)malloc(sizeof(pthread_t) * num_threads);

    if (!threads) {
        perror("malloc");
        return 1;
    }

    // 1) Create ALL threads first
    for (int i = 0; i < num_threads; i++) {
        int n = atoi(argv[i + 1]);

        // Simplification says smallest is 2, but guard anyway
        if (n < 2) {
            fprintf(stderr, "Error: invalid input '%s' (must be >= 2)\n", argv[i + 1]);
            free(threads);
            return 1;
        }

        thread_arg_t *targ = (thread_arg_t *)malloc(sizeof(thread_arg_t));
        if (!targ) {
            perror("malloc");
            free(threads);
            return 1;
        }
        targ->value = n;

        int rc = pthread_create(&threads[i], NULL, thread_main, (void *)targ);
        if (rc != 0) {
            fprintf(stderr, "Error: pthread_create failed (code %d)\n", rc);
            free(targ);
            free(threads);
            return 1;
        }
    }

    // 2) Join in the same order as arguments and print from main thread only
    for (int i = 0; i < num_threads; i++) {
        void *retval = NULL;

        int rc = pthread_join(threads[i], &retval);
        if (rc != 0) {
            fprintf(stderr, "Error: pthread_join failed (code %d)\n", rc);
            free(threads);
            return 1;
        }

        factor_result_t *res = (factor_result_t *)retval;
        if (!res) {
            fprintf(stderr, "Error: factorization failed (out of memory)\n");
            free(threads);
            return 1;
        }

        printf("%d:", res->original);
        for (int k = 0; k < res->count; k++) {
            printf(" %d", res->factors[k]);
        }
        printf("\n");

        free(res->factors);
        free(res);
    }

    free(threads);
    return 0;
}

