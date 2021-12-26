#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

void *read(void *arg);
void *accu(void *arg);
static sem_t sem_one;
static sem_t sem_two;
static int num;

int main(int argc, char *argv[])
{
    pthread_t id_t1, id_t2;
    sem_init(&sem_one, 0, 0);
    sem_init(&sem_two, 0, 1);

    pthread_create(&id_t1, NULL, read, NULL);
    pthread_create(&id_t2, NULL, accu, NULL);

    pthread_join(id_t1, NULL);
    pthread_join(id_t2, NULL);

    sem_destroy(&sem_one);
    sem_destroy(&sem_two);

    sem_close(&sem_one);
    sem_close(&sem_two);

    return 0;
}

void *read(void *arg)
{
    int i;
    for(i=0; i<5; i++)
    {        
        sem_wait(&sem_two); // sem_two의 count을 0으로 설정 
        fputs("Input num: ", stdout);
        scanf("%d", &num);
        sem_post(&sem_one); // sem_one의 count을 1로 설정 
    }
    return NULL;
}

void *accu(void *arg)
{
    int sum=0, i;
    for(i=0; i<5; i++)
    {
        sem_wait(&sem_one); //sem_one count 0
        sum += num;
        printf("sum: %d\n", sum); 
        sem_post(&sem_two); //sem_two count 1 
        
    }
    printf("Result: %d\n", sum);
    return NULL;
}