#ifndef MAILBOX_H_
#define MAILBOX_H_

#include <semaphore.h>
#include <pthread.h>

#define RANGE 1
#define ALLDONE 2
#define MAXTHREAD 10

typedef struct {
	int iSender; /* sender of the message (0 .. number-of-threads) */
	int type;    /* its type */
	int value1;  /* first value */
	int value2;  /* second value */
} msg;

extern msg** mailboxes;
extern msg** readyMail;
extern sem_t** pSems;
extern sem_t** cSems;
extern pthread_t** threads;

void SendMsg(int iTo, msg* pMsg);
void RecvMsg(int iFrom, msg* pMsg);

#endif