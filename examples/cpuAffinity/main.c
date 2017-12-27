#include <stdio.h>
#include <math.h>
#include <unistd.h>
#define __USE_GNU  
#include <sched.h>  
#include <pthread.h>


void WasteTime()
{
    int abc = 10000000;
    while(abc--)
    {
        int tmp = 10000*10000;
    }
    sleep(1);

}

void *thread_func(void *param)
{
    cpu_set_t mask;
    while(1)
    {
         CPU_ZERO(&mask);
        CPU_SET(1, &mask); 

        if (pthread_setaffinity_np(pthread_self(), sizeof(mask),
            &mask) < 0) {
            perror("pthread_setaffinity_np");
        }
 
        WasteTime();

         CPU_ZERO(&mask);
        CPU_SET(2, &mask); 
        if (pthread_setaffinity_np(pthread_self(), sizeof(mask),
            &mask) < 0) {
            perror("pthread_setaffinity_np");
        }

        WasteTime();
    }
}
 
void *thread_func1(void *param)
{
    cpu_set_t mask;
    while(1)
    {
         CPU_ZERO(&mask);
        CPU_SET(3, &mask); 

        if (pthread_setaffinity_np(pthread_self(), sizeof(mask),
            &mask) < 0) {
            perror("pthread_setaffinity_np");
        }
 
        WasteTime();

         CPU_ZERO(&mask);
        CPU_SET(4, &mask); 
        if (pthread_setaffinity_np(pthread_self(), sizeof(mask),
            &mask) < 0) {
            perror("pthread_setaffinity_np");
        }

        WasteTime();
    }
}
 
int main(int argc, char *argv[])
{
    cpu_set_t mask;
     CPU_ZERO(&mask);
    CPU_SET(0, &mask); 
    if (sched_setaffinity(0, sizeof(mask), &mask) < 0) {
        perror("sched_setaffinity");
    }

    pthread_t my_thread;
 
    if (pthread_create(&my_thread, NULL, thread_func,
        NULL) != 0) {
        perror("pthread_create");
    }
    if (pthread_create(&my_thread, NULL, thread_func1,
        NULL) != 0) {
        perror("pthread_create");
    }
    while(1) { WasteTime(); }
    pthread_exit(NULL);

}
