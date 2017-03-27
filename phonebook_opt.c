#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "phonebook_opt.h"
#include "debug.h"


entry *findName(char lastname[], entry *pHead)
{
    size_t len = strlen(lastname);
    while (pHead) {
        if (strncasecmp(lastname, pHead->lastName, len) == 0
                && (pHead->lastName[len] == '\n' ||
                    pHead->lastName[len] == '\0')) {
            pHead->lastName[len] = '\0';
            if (!pHead->dtl)
                pHead->dtl = (pdetail) malloc(sizeof(detail));
            return pHead;
        }
        pHead = pHead->pNext;
    }
    return NULL;
}

/**
 * Generate a local linked list in thread.
 */
void append(void *arg)
{
    thread_arg *t_arg = (thread_arg *) arg;
    char *data = t_arg->data_start;
    int w = 0;
    int count = 0;
    entry *e = NULL;
    while (data < t_arg->data_end) {
        if (*(data+w) == '\n') {
            count++;
            e = (entry *)malloc(sizeof(entry));
            e->lastName = data;
            *(data+w) = '\0';
            data+=(w+1);
            w = 0;
            if (!t_arg->entry_list_tail)
                t_arg->entry_list_tail = e;
            e->pNext = t_arg->entry_list_head;
            t_arg->entry_list_head = e;
        }
        w++;
    }
    t_arg->count = count;
    pthread_exit(NULL);
}

void show_entry(entry *pHead)
{
    while (pHead) {
        printf("%s", pHead->lastName);
        pHead = pHead->pNext;
    }
}

static double diff_in_second(struct timespec t1, struct timespec t2)
{
    struct timespec diff;
    if (t2.tv_nsec-t1.tv_nsec < 0) {
        diff.tv_sec  = t2.tv_sec - t1.tv_sec - 1;
        diff.tv_nsec = t2.tv_nsec - t1.tv_nsec + 1000000000;
    } else {
        diff.tv_sec  = t2.tv_sec - t1.tv_sec;
        diff.tv_nsec = t2.tv_nsec - t1.tv_nsec;
    }
    return (diff.tv_sec + diff.tv_nsec / 1000000000.0);
}
