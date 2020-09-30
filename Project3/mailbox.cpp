#include <iostream>
using namespace std;
#include "mailbox.h"

msg** mailboxes;
sem_t** pSems;
sem_t** cSems;
pthread_t** threads;

void SendMsg(int iTo, msg* pMsg)
{
    sem_wait(pSems[iTo]);
    mailboxes[iTo] = pMsg;
    sem_post(cSems[iTo]);
}

void RecvMsg(int iFrom, msg* pMsg)
{
    sem_wait(cSems[iFrom]);
    *pMsg = *mailboxes[iFrom];
    sem_post(pSems[iFrom]);
}