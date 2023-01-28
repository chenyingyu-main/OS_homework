#ifndef TASK_H
#define TASK_H

#include <ucontext.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

struct Task
{
    char task_name[20];
    char cur_state[20];
    int priority;
    int tid;

    int waiting_time;
    int burst_time;

    // for the resource
    int count_resource;
    int *resource_list;
    int waiting_mode; // = 0 sleep, = 1waiting for resource

    // context
    ucontext_t *context;
    // struct Task *next;
};
typedef struct Task thread;

struct Node
{
    thread *t;
    int got_resource;
    int need_to_wait;
    struct Node *next;
};
typedef struct Node thread_list;

thread_list *thread_Q;     // working thread queue (no terminated thread)
thread_list *print_Q;      // record all thread
thread_list *cur_active;   // current active thread

ucontext_t cpu_idle;      // direct to idle function
ucontext_t init;          // direct to start func(), go back when all terminate
ucontext_t choose;        // direct to task choose func()

void create_context(ucontext_t *, ucontext_t *, void *);
void EnQueTQ(thread_list *);
void EnQuePQ(thread_list *);
thread_list* SortQue(thread_list*);
void test_print_Q(thread_list *);
void create_idle_choose_context();
void task_choose();
void task_scan();

void task_sleep(int);
void task_exit();

#endif
