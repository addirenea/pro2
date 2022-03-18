#include <assert.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>

#define MAX 3

int buffer[MAX];
// char *buffer[MAX];

int fill = 0;
int use = 0;

// figure out how to use while instead of for loops
int loops = 20;

// use to keep track of total wc
int acc = 0;

sem_t empty;
sem_t full;
sem_t mutex;

void put(int value)
{
    buffer[fill] = value;
    fill = (fill + 1) % MAX;
}

int get()
{
    int tmp = buffer[use];
    use = (use + 1) % MAX;
    return tmp;
}

void *producer(void *arg)
{
    // purpose: read next line from stdin
    // & add line to buffer

    int val = (long)arg;
    for (int i = 0; i < loops; i++) {

        assert(sem_wait(&empty) == 0);
        assert(sem_wait(&mutex) == 0);

        put(i + val);

        assert(sem_post(&mutex) == 0);
        assert(sem_post(&full) == 0);

    }
    return NULL;
}

void *consumer(void *arg)
{
    // purpose: print task num & line
    // get word count of line
    // & increament total wc acc for file

    for (int i = 0; i < (loops*2); i++) {

        assert(sem_wait(&full) == 0);
        assert(sem_wait(&mutex) == 0);

        int tmp = get();

        assert(sem_post(&mutex) == 0);
        assert(sem_post(&empty) == 0);

        printf("%d\n", tmp);
    }
    return NULL;
}

int main()
{
    // get num of consumers

    pthread_t p1;
    pthread_t p2;

    // init consumers array
    pthread_t c1;

    sem_init(&mutex, 0, 1);
    sem_init(&empty, 0, MAX);
    sem_init(&full, 0, 0);

    assert(pthread_create(&p1, NULL, producer, (void*)100) == 0);
    assert(pthread_create(&p2, NULL, producer, (void*)200) == 0);

    // create consumers via loop
    assert(pthread_create(&c1, NULL, consumer, NULL) == 0);

    assert(pthread_join(p1, NULL) == 0);
    assert(pthread_join(p2, NULL) == 0);

    // join consumers via loop
    assert(pthread_join(c1, NULL) == 0);

    // print final wc acc for file

    return 0;

}
