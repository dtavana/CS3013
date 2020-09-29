#include <iostream>
using namespace std;
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
#include "mailbox.h"

void* threadInit(void*);

void initSemaphores(int threadNumber)
{
    pSems = (sem_t**) malloc((threadNumber + 1) * sizeof(sem_t*));
    cSems = (sem_t**) malloc((threadNumber + 1) * sizeof(sem_t*));
    for(int i = 0; i <= threadNumber; i++) {
        pSems[i] = (sem_t*) malloc(sizeof(sem_t));
        cSems[i] = (sem_t*) malloc(sizeof(sem_t));
        sem_init(pSems[i], 0, 1);
        sem_init(cSems[i], 0, 0);
    }
}

void initThreads(int threadNumber)
{
    threads = (pthread_t**) malloc((threadNumber) * sizeof(pthread_t*));
    for(int i = 0; i < threadNumber; i++) {
        int* index = (int *) malloc(sizeof(*index));
        *index = i + 1;
        threads[i] = (pthread_t*) malloc(sizeof(pthread_t));
        pthread_create(threads[i], NULL, &threadInit, index);
    }
}

void initMailboxes(int threadNumber, int target)
{
    mailboxes = (msg**) malloc((threadNumber + 1) * sizeof(msg*));
    readyMail = (msg**) malloc(threadNumber * sizeof(msg*));
    // Share workload equally among threads
    int shared = target / threadNumber;
    int current = 1;
    for(int i = 0; i < threadNumber; i++) {
        readyMail[i] = (msg*) malloc(sizeof(msg));
        readyMail[i]->iSender = 0;
        readyMail[i]->type = RANGE;
        readyMail[i]->value1 = current;
        if(i != threadNumber - 1) {
            readyMail[i]->value2 = shared * (i + 1);
        }
        else {
            readyMail[i]->value2 = target;
        }
        current = shared * (i + 1) + 1; 
    }
}

void startSend(int threadNumber)
{
    for(int i = 0; i < threadNumber; i++) {
        SendMsg(i + 1, readyMail[i]);
    }
}

int startReceive(int threadNumber)
{
    int total = 0;
    msg* result = (msg*) malloc(sizeof(msg));
    for(int i = 1; i <= threadNumber; i++) {
        RecvMsg(0, result);
        total += result->value1;
    }
    return total;
}

void cleanup(int threadNumber)
{
    for(int i = 0; i < threadNumber; i++) {
        free(mailboxes[i]);
        free(threads[i]);
        sem_destroy(pSems[i]);
        sem_destroy(cSems[i]);
    }
    sem_destroy(pSems[threadNumber]);
    sem_destroy(cSems[threadNumber]);
    free(pSems);
    free(cSems);
    free(mailboxes);
    free(threads);
}

void* threadInit(void* passedIndex)
{
    int index = *((int*) passedIndex);
    free(passedIndex);
    
    msg* message = (msg*) malloc(sizeof(msg));
    RecvMsg(index, message);
    message->iSender = index;
    message->type = ALLDONE;

    int total = 0;
    for(int i = message->value1; i <= message->value2; i++) {
        total += i;
    }
    message->value1 = total;
    
    SendMsg(0, message);

    return (void*) 0;
}

int main(int argc, char* argv[])
{
    int threadNumber, target;
    threadNumber = atoi(argv[1]);
    target = atoi(argv[2]);

    if(threadNumber > MAXTHREAD) {
        cout << "Too many threads, defaulting to " << MAXTHREAD << " threads" << endl;
        threadNumber = MAXTHREAD;
    }

    initSemaphores(threadNumber);
    initMailboxes(threadNumber, target);

    startSend(threadNumber);
    initThreads(threadNumber);
    int total = startReceive(threadNumber);
    cout << "The total for 1 to " << target << " using " << threadNumber << " threads is " << total << "." << endl;

    cleanup(threadNumber);
}
