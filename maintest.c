#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#define SIZE 9
#define MAX_THREADS 27

typedef enum { CHECK_ROW, CHECK_COLUMN, CHECK_ALL_COLUMNS, CHECK_SUBGRID } CheckKind;
typedef struct { CheckKind kind; int index; int result_index; } ThreadData;

static int board[SIZE][SIZE];
static int results[MAX_THREADS];

static int valid_values(const int values[SIZE]) {
    int seen[SIZE + 1] = {0};
    for (int i = 0; i < SIZE; ++i) {
        int value = values[i];
        if (value < 1 || value > SIZE || seen[value]) return 0;
        seen[value] = 1;
    }
    return 1;
}

static void *check_worker(void *arg) {
    ThreadData *d = (ThreadData *)arg;
    int values[SIZE];
    int valid = 1;

    if (d->kind == CHECK_ROW) {
        for (int c = 0; c < SIZE; ++c) values[c] = board[d->index][c];
        valid = valid_values(values);
    } else if (d->kind == CHECK_COLUMN) {
        for (int r = 0; r < SIZE; ++r) values[r] = board[r][d->index];
        valid = valid_values(values);
    } else if (d->kind == CHECK_ALL_COLUMNS) {
        for (int c = 0; c < SIZE && valid; ++c) {
            for (int r = 0; r < SIZE; ++r) values[r] = board[r][c];
            valid = valid_values(values);
        }
    } else { /* one 3x3 subgrid */
        int k = 0;
        int start_row = (d->index / 3) * 3;
        int start_col = (d->index % 3) * 3;
        for (int r = start_row; r < start_row + 3; ++r)
            for (int c = start_col; c < start_col + 3; ++c)
                values[k++] = board[r][c];
        valid = valid_values(values);
    }

    results[d->result_index] = valid;
    return NULL;
}

static int read_board(void) {
    FILE *file = fopen("input.txt", "r");
    if (!file) return 0;
    for (int r = 0; r < SIZE; ++r) {
        for (int c = 0; c < SIZE; ++c) {
            if (fscanf(file, "%d", &board[r][c]) != 1) {
                fclose(file);
                return 0;
            }
        }
    }
    /* Require exactly 81 integers; trailing whitespace is fine. */
    int extra;
    if (fscanf(file, "%d", &extra) == 1) {
        fclose(file);
        return 0;
    }
    fclose(file);
    return 1;
}

int main(int argc, char *argv[]) {
    if (argc != 2 || (strcmp(argv[1], "1") != 0 && strcmp(argv[1], "2") != 0))
        return 1;
    if (!read_board()) return 1;

    printf("BOARD STATE IN input.txt:\n");
    for (int r = 0; r < SIZE; ++r) {
        for (int c = 0; c < SIZE; ++c)
            printf("%s%d", c ? " " : "", board[r][c]);
        putchar('\n');
    }

    pthread_t threads[MAX_THREADS];
    ThreadData data[MAX_THREADS];
    int thread_count = 0;
    const int mode = argv[1][0] - '0';
    clock_t start, end;
    for (int i = 0; i < MAX_THREADS; ++i) results[i] = 0;

    start = clock();
    /* Both modes use 9 row checks and 9 subgrid checks. */
    for (int i = 0; i < SIZE; ++i) {
        data[thread_count] = (ThreadData){CHECK_ROW, i, i};
        if (pthread_create(&threads[thread_count], NULL, check_worker, &data[thread_count]) != 0) return 1;
        ++thread_count;
    }
    if (mode == 1) {
        /* Option 1: one thread validates all nine columns (19 threads total). */
        data[thread_count] = (ThreadData){CHECK_ALL_COLUMNS, 0, 9};
        if (pthread_create(&threads[thread_count], NULL, check_worker, &data[thread_count]) != 0) return 1;
        ++thread_count;
    } else {
        /* Option 2: nine separate column threads (27 threads total). */
        for (int i = 0; i < SIZE; ++i) {
            data[thread_count] = (ThreadData){CHECK_COLUMN, i, 9 + i};
            if (pthread_create(&threads[thread_count], NULL, check_worker, &data[thread_count]) != 0) return 1;
            ++thread_count;
        }
    }
    int subgrid_result_start = mode == 1 ? 10 : 18;
    for (int i = 0; i < SIZE; ++i) {
        data[thread_count] = (ThreadData){CHECK_SUBGRID, i, subgrid_result_start + i};
        if (pthread_create(&threads[thread_count], NULL, check_worker, &data[thread_count]) != 0) return 1;
        ++thread_count;
    }
    for (int i = 0; i < thread_count; ++i) pthread_join(threads[i], NULL);
    end = clock();

    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    int solution = 1;
    for (int i = 0; i < thread_count; ++i) if (!results[i]) { solution = 0; break; }
    printf("SOLUTION: %s (%.4f seconds)\n", solution ? "YES" : "NO", elapsed);
    return 0;
}

