#define  _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#include <unistd.h>
#include <pthread.h>
#include <sys/mman.h>

#include IMPL

#ifndef OPT
#define OUTPUT_FILE "orig.txt"

#else
#include "text_align.h"
#include "debug.h"
#include <fcntl.h>
#define ALIGN_FILE "align.txt"
#define OUTPUT_FILE "opt.txt"

#ifndef THREAD_NUM
#define THREAD_NUM 2
#endif

#endif

#define DICT_FILE "./dictionary/words.txt"



static inline void get_real_clk(struct timespec *moment)
{
    clock_gettime(CLOCK_REALTIME, moment);
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

int main(int argc, char *argv[])
{
#ifndef OPT
    FILE *fp;
    int i = 0;
    char line[MAX_LAST_NAME_SIZE];
#else
    struct timespec mid;
#endif
    struct timespec start, end;
    double cpu_time1, cpu_time2;
    /* Build the entry */
    entry *pHead, *e;
    printf("size of entry : %lu bytes\n", sizeof(entry));

#if defined(OPT)
    char *map, *data_end;
    pthread_t threads[THREAD_NUM];
    thread_arg thread_args[THREAD_NUM];
    /* Start timing */
    get_real_clk(&start);
    int fd = open(DICT_FILE, O_RDONLY | O_NONBLOCK);
    off_t file_size = fsize(DICT_FILE);
    map = mmap(NULL, file_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    assert(map && "mmap error");
    data_end = map;
    size_t sizeForthread = file_size/THREAD_NUM;
    for (int i=0; i<THREAD_NUM; i++) {
        thread_args[i].data_start = data_end;
        thread_args[i].id  = i+1;
        if (i!=(THREAD_NUM-1)) {
            data_end = map + (i+1)*sizeForthread;
            while (*data_end!='\n')
                data_end++;
            thread_args[i].data_end = data_end;
            data_end++;
        } else
            thread_args[i].data_end = map + file_size;
        thread_args[i].entry_list_head = NULL;
        thread_args[i].entry_list_tail = NULL;
    }
    pthread_setconcurrency(THREAD_NUM + 1);
    /* Deliver the jobs to all threads and wait for completing */
    for (int i = 0; i < THREAD_NUM; i++)
        pthread_create(&threads[i], NULL, (void *)&append, (void *)(&thread_args[i]));

    for (int i = 0; i < THREAD_NUM; i++)
        pthread_join(threads[i], NULL);

    /* Connect the linked list of each thread */
    pHead = thread_args[0].entry_list_head;
    e = thread_args[0].entry_list_tail;
    for (int i = 1; i < THREAD_NUM; i++) {
        e->pNext = thread_args[i].entry_list_head;
        e = thread_args[i].entry_list_tail;
    }
    /* Stop timing */
    get_real_clk(&end);
#else /* ! OPT */
    fp = fopen(DICT_FILE, "r");
    if (!fp) {
        printf("cannot open the file\n");
        return -1;
    }
    pHead = (entry *) malloc(sizeof(entry));
    e = pHead;
    e->pNext = NULL;

#if defined(__GNUC__)
    __builtin___clear_cache((char *) pHead, (char *) pHead + sizeof(entry));
#endif
    /* Start timing */
    clock_gettime(CLOCK_REALTIME, &start);
    while (fgets(line, sizeof(line), fp)) {
        while (line[i] != '\0')
            i++;
        line[i - 1] = '\0';
        i = 0;
        e = append(line, e);
    }
    /* Stop timing */
    clock_gettime(CLOCK_REALTIME, &end);

    /* close file as soon as possible */
    fclose(fp);
#endif

    cpu_time1 = diff_in_second(start, end);

    /* Find the given entry */
    /* the givn last name to find */
    char input[MAX_LAST_NAME_SIZE] = "zyxel";
    e = pHead;

    assert(findName(input, e) &&
           "Did you implement findName() in " IMPL "?");
    assert(0 == strcmp(findName(input, e)->lastName, "zyxel"));

#if defined(__GNUC__)
    __builtin___clear_cache((char *) pHead, (char *) pHead + sizeof(entry));
#endif
    /* Compute the execution time */
    clock_gettime(CLOCK_REALTIME, &start);
    findName(input, e);
    clock_gettime(CLOCK_REALTIME, &end);
    cpu_time2 = diff_in_second(start, end);

    /* Write the execution time to file. */
    FILE *output;
    output = fopen(OUTPUT_FILE, "a");
    fprintf(output, "append() findName() %lf %lf\n", cpu_time1, cpu_time2);
    fclose(output);

    printf("execution time of append() : %lf sec\n", cpu_time1);
    printf("execution time of findName() : %lf sec\n", cpu_time2);

    /* Release memory */
#ifndef OPT
    while (pHead) {
        e = pHead;
        pHead = pHead->pNext;
        free(e);
    }
#else
    /* Free the allocated detail entry */
    e = pHead;
    while (e) {
        free(e->dtl);
        e = e->pNext;
    }

    //free(entry_pool);
    //for (int i = 0; i < THREAD_NUM; ++i)
    //  free(thread_args[i]);

    munmap(map, file_size);
    close(fd);
#endif
    return 0;
}
