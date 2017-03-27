#ifndef _PHONEBOOK_H
#define _PHONEBOOK_H

#include <pthread.h>
#include <time.h>

#define MAX_LAST_NAME_SIZE 16

#define OPT 1

typedef struct _detail {
    char firstName[16];
    char email[16];
    char phone[10];
    char cell[10];
    char addr1[16];
    char addr2[16];
    char city[16];
    char state[2];
    char zip[5];
} detail;

typedef detail *pdetail;

typedef struct __PHONE_BOOK_ENTRY {
    char *lastName;
    struct __PHONE_BOOK_ENTRY *pNext;
    pdetail dtl;
} entry;

entry *findName(char lastname[], entry *pHead);


typedef struct _thread_arg {
    char *data_start;
    char *data_end;
    entry *entry_list_head;
    entry *entry_list_tail;
    int count;
} thread_arg;

void append(void *arg);

void show_entry(entry *pHead);

static double diff_in_second(struct timespec t1, struct timespec t2);

#endif
