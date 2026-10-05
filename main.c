#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>


#define SIZE 9
#define NUM_THREADS 27


int board[SIZE][SIZE];
int results[NUM_THREADS];


typedef struct {
    int index;
} ThreadData;


/* Check one row */
void *check_row(void *arg)
{
    ThreadData *data = (ThreadData *)arg;
    int row = data->index;
    int seen[SIZE + 1] = {0};


    for (int col = 0; col < SIZE; col++) {
        int value = board[row][col];


        if (value < 1 || value > 9 || seen[value]) {
            results[row] = 0;
            return NULL;
        }


        seen[value] = 1;
    }


    results[row] = 1;
    return NULL;
}


/* Check one column */
void *check_column(void *arg)
{
    ThreadData *data = (ThreadData *)arg;
    int col = data->index;
    int seen[SIZE + 1] = {0};


    for (int row = 0; row < SIZE; row++) {
        int value = board[row][col];


        if (value < 1 || value > 9 || seen[value]) {
            results[9 + col] = 0;
            return NULL;
        }


        seen[value] = 1;
    }


    results[9 + col] = 1;
    return NULL;
}


/* Check one 3x3 subgrid */
void *check_subgrid(void *arg)
{
    ThreadData *data = (ThreadData *)arg;
    int grid = data->index;


    int start_row = (grid / 3) * 3;
    int start_col = (grid % 3) * 3;


    int seen[SIZE + 1] = {0};


    for (int row = start_row; row < start_row + 3; row++) {
        for (int col = start_col; col < start_col + 3; col++) {
            int value = board[row][col];


            if (value < 1 || value > 9 || seen[value]) {
                results[18 + grid] = 0;
                return NULL;
            }


            seen[value] = 1;
        }
    }


    results[18 + grid] = 1;
    return NULL;
}


int main(int argc, char *argv[])
{
    if (argc != 2) {
        return 1;
    }


    /* The assignment requires option 1. */
    if (argv[1][0] != '1' || argv[1][1] != '\0') {
        return 1;
    }


    FILE *file = fopen("input.txt", "r");


    if (file == NULL) {
        return 1;
    }


    /* Read the 9x9 board */
    for (int row = 0; row < SIZE; row++) {
        for (int col = 0; col < SIZE; col++) {
            if (fscanf(file, "%d", &board[row][col]) != 1) {
                fclose(file);
                return 1;
            }
        }
    }


    fclose(file);


    printf("BOARD STATE IN input.txt:\n");


    for (int row = 0; row < SIZE; row++) {
        for (int col = 0; col < SIZE; col++) {
            if (col > 0) {
                printf(" ");
            }
            printf("%d", board[row][col]);
        }
        printf("\n");
    }


    pthread_t threads[NUM_THREADS];
    ThreadData data[NUM_THREADS];


    for (int i = 0; i < NUM_THREADS; i++) {
        results[i] = 0;
    }


    struct timespec start, end;


    clock_gettime(CLOCK_MONOTONIC, &start);


    int thread_count = 0;


    /* Create 9 row threads */
    for (int i = 0; i < 9; i++) {
        data[thread_count].index = i;


        if (pthread_create(&threads[thread_count], NULL,
                           check_row, &data[thread_count]) != 0) {
            return 1;
        }


        thread_count++;
    }


    /* Create 9 column threads */
    for (int i = 0; i < 9; i++) {
        data[thread_count].index = i;


        if (pthread_create(&threads[thread_count], NULL,
                           check_column, &data[thread_count]) != 0) {
            return 1;
        }


        thread_count++;
    }


    /* Create 9 subgrid threads */
    for (int i = 0; i < 9; i++) {
        data[thread_count].index = i;


        if (pthread_create(&threads[thread_count], NULL,
                           check_subgrid, &data[thread_count]) != 0) {
            return 1;
        }


        thread_count++;
    }


    /* Wait for all 27 threads */
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }


    clock_gettime(CLOCK_MONOTONIC, &end);


    double elapsed =
        (end.tv_sec - start.tv_sec) +
        (end.tv_nsec - start.tv_nsec) / 1000000000.0;


    int solution = 1;


    for (int i = 0; i < NUM_THREADS; i++) {
        if (results[i] == 0) {
            solution = 0;
            break;
        }
    }


    printf("SOLUTION: %s (%.4f seconds)\n",
           solution ? "YES" : "NO",
           elapsed);


    return 0;
}
