#include <assert.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>


#define MAX 5

char *buffer[MAX];

int fill = 0;
int use = 0;

// use to keep track of total wc
volatile int acc = 0;

sem_t empty;
sem_t full;
sem_t mutex;

void put(char* value) {

    buffer[fill] = value;
    fill = (fill + 1) % MAX;
}


char* get() {

    char* tmp = buffer[use];
    use = (use + 1) % MAX;
    return tmp;
}


int getWordCount(char *str) {

    int prev_in_word = 0;
    int wc = 0;

    while (*str)
    {
        if (*str == ' ' || *str == '\t' || *str == '\n')
            prev_in_word = 0;

        else if (prev_in_word == 0)
        {
            prev_in_word = 1;
            wc++;
        }

         str++;
    }

    return wc;
}


void *producer(void *arg) {
    // purpose: read next line from stdin
    // & add line to buffer

    char* line = arg;
    // idea: instead of passing the line as arg,
    // set up a while loop that runs while there are more lines in stdin
    // then lines 69-75 go inside the while loop

    assert(sem_wait(&empty) == 0);
    assert(sem_wait(&mutex) == 0);

    put(line);

    assert(sem_post(&mutex) == 0);
    assert(sem_post(&full) == 0);

    return NULL;
}

void *consumer(void *arg) {
    // purpose: print task num & line
    // get word count of line
    // & increament total wc acc for file
    int *task_num = arg;

    assert(sem_wait(&full) == 0);
    assert(sem_wait(&mutex) == 0);

    char *line = get();

    printf("%d: '%s'\n", *task_num, line);

    int wc = getWordCount(line);

    acc = acc + wc;

    assert(sem_post(&mutex) == 0);
    assert(sem_post(&empty) == 0);

    return NULL;
}

int main(int argc, char **argv) {
    if (argc != 2) {

        printf("Invalid Parameters.");
        return -1;

    }

    int consumers = atoi(argv[1]);

    sem_init(&mutex, 0, 1);
    sem_init(&empty, 0, MAX);
    sem_init(&full, 0, 0);

    // what is the arg for producer? anything?
    pthread_t pro;
    assert(pthread_create(&pro, NULL, producer, (void*)100) == 0);


    // create block of consumer threads
    pthread_t con_threads[consumers];
    int *arg = malloc(sizeof(*arg));

    // loop through all consumer threads and create, passing task number as arg
    for (int i = 0; i < consumers; i++) {
        *arg = i;

        if (pthread_create(&con_threads[i], NULL, consumer, arg) != 0) {
            printf("Unable to create thread.\n");
            exit(1);
        }
    }


    // join producer and consumers
    assert(pthread_join(pro, NULL) == 0);

    for (int i = 0; i < consumers; i++) {
        assert(pthread_join(con_threads[i], NULL) == 0);
    }


    // print final wc acc for file
    printf("Final Word Count: %d\n", acc);


    // free allocated memory
    free(arg);

    return 0;

}
