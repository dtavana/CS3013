#ifndef MAILBOX_H_
#define MAILBOX_H_

#include <semaphore.h>
#include <pthread.h>

#define RANGE 1
#define ALLDONE 2

typedef struct {
	int iSender; /* sender of the message (0 .. number-of-threads) */
	int type;    /* its type */
	int value1;  /* first value */
	int value2;  /* second value */
} msg;

extern msg** mailboxes; // Mailboxes for all threads including parent thread
extern sem_t** pSems; // Producer semaphores for all mailboxes including parent thread mailbox
extern sem_t** cSems; // Consumer semaphores for all mailboxes including parent thread mailbox
extern pthread_t** threads; // All CHILD threads (0 index is threadId 1)

void SendMsg(int iTo, msg* pMsg);
void RecvMsg(int iFrom, msg* pMsg);

#endif