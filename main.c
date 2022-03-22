#include <assert.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>


#define MAX 5

volatile int acc = 0;

char * endmsg = "END OF FILE. TERMINATE PROGRAM NOW.";

// char buffer[MAX][256];
char buffer[MAX][256];

int fill = 0;
int use = 0;
sem_t empty;
sem_t full; // these 2 used to check track of count in buffer
sem_t mutex; // locks critical sections
int complete = 0; // alerts when EOF is reached



void put(char *value, size_t size) {

    if (!feof(stdin)) {
        memset(buffer[fill], 0, 256);
        memcpy(buffer[fill], (void *)value, size);
    }
    fill = (fill + 1) % MAX;

}



char* get() {

    char *tmp = buffer[use];
    // if (*temper != -1) {
    //     memset(buffer[use], 0, 256);
    //
    // }
    use = (use + 1) % MAX;
    return tmp;
}



int getWordCount(char *str) {

    int prev_in_word = 0;
    int wc = 0;

    // loops through all charaters in str
    while (*str)
    {
        // checks if character is whitespace
        if (*str == ' ' || *str == '\t' || *str == '\n')
            prev_in_word = 0;

        // otherwise checks if prev character was whitespace
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

    // int consumers = (uintptr_t)arg;

    // continues until reaching end of stdin file
    while(!feof(stdin)) {

        // gets next line
        size_t bufsize = 256;
        char *line = (char *)malloc(bufsize * sizeof(char));
        size_t size = getline(&line, &bufsize, stdin);

        // CRITCAL SECTION: adds line to buffer
        assert(sem_wait(&empty) == 0);
        assert(sem_wait(&mutex) == 0);

        put(line, size);

        assert(sem_post(&mutex) == 0);
        assert(sem_post(&full) == 0);

        free(line);


    }

    // waits until consumers have run on all remaining lines
    assert(sem_wait(&empty) == 0);

    // FINAL CALL! marks program as complete and posts to full to signal consumers to end
    // complete = 1;
    // for (int i = 0; i < consumers; i++) {
    //     assert(sem_post(&full) == 0);
    //     // assert(sem_post(&empty) == 0);
    // }



    assert(sem_wait(&empty) == 0);
    assert(sem_wait(&mutex) == 0);

    for (int i = 0; i < MAX; i++) {
        memset(buffer[i], -1, 256);
    }
    assert(sem_post(&mutex) == 0);
    assert(sem_post(&full) == 0);

    return NULL;
}



void *consumer(void *arg) {
    // purpose: print task num & line
    // get word count of line
    // & increament total wc acc for file

    int task_num = (uintptr_t)arg;

    while(1) {
        printf("thread %d waiting \n", task_num);

        // CRITICAL SECTION: gets next line
        assert(sem_wait(&full) == 0);
        assert(sem_wait(&mutex) == 0);

        char *line = get();

        // checks if program is complete
        printf("thread %d at if statement \n", task_num);
        if (*line == -1) {
            printf("inside if statement \n");
            assert(sem_post(&mutex) == 0);
            assert(sem_post(&full) == 0);
            break;
        }

        // assert(sem_post(&mutex) == 0);
        // assert(sem_post(&empty) == 0);

        // prints task and line
        printf("%d: %s", task_num, line);

        // gets word count for current line
        int wc = getWordCount(line);
        printf("word count: %d \n", wc);

        // CRITICAL SECTION: updates acc
        // assert(sem_wait(&mutex) == 0);
        acc = acc + wc;
        // assert(sem_post(&mutex) == 0);

        assert(sem_post(&mutex) == 0);
        assert(sem_post(&empty) == 0);
        printf("thread %d posted \n", task_num);

    }

    return NULL;
}



int main(int argc, char **argv) {

    // checks for valid command line arguments
    if (argc != 2) {
        printf("Invalid Parameters.");
        return -1;
    }
    int consumers = atoi(argv[1]);

    // initialize global variables
    sem_init(&mutex, 0, 1);
    sem_init(&empty, 0, MAX);
    sem_init(&full, 0, 0);
    // buffer = (char **) malloc(MAX * (sizeof(char) * 256) + 1);


    // create producer
    pthread_t pro;
    assert(pthread_create(&pro, NULL, producer, (void*)(uintptr_t)consumers) == 0); // does producer need an arg?


    // create consumers
    pthread_t con_threads[consumers];

    for (int i = 0; i < consumers; i++) {
        if (pthread_create(&con_threads[i], NULL, consumer, (void*)(uintptr_t)i) != 0) {
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
    printf("\n Final Word Count: %d\n", acc);


    // free(buffer);
    return 0;

}
