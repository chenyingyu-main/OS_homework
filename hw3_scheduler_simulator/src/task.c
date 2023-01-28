#include <ucontext.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/task.h"
#include "../include/resource.h"
#include "../include/function.h"
#include "../include/builtin.h"

#define STACK_SIZE 65536

void create_context(ucontext_t *context, ucontext_t *next_context, void *func){
    getcontext(context);
    context->uc_stack.ss_sp = malloc(STACK_SIZE);
    context->uc_stack.ss_size = STACK_SIZE;
    context->uc_stack.ss_flags = 0;
    context->uc_link = next_context;
    makecontext(context, (void (*)(void))func, 0);
}

void EnQueTQ(thread_list *t){
    t->next = NULL;

    if(thread_Q == NULL){
        thread_Q = t;
        return;
    }

    thread_list *ptr = thread_Q;

    while (ptr->next)
    {
        ptr = ptr->next;
    }
    ptr->next = t;
    
}

void EnQuePQ(thread_list *t){
    t->next = NULL;

    if(print_Q == NULL){
        print_Q = t;
        return;
    }

    thread_list *ptr = print_Q;

    while (ptr->next)
    {
        ptr = ptr->next;
    }
    ptr->next = t;
    
}


thread_list* SortQue(thread_list* head){
    // bubble sort the thread Q with priority
    thread_list* tmp;
    thread_list* curr = head;
    thread_list* prev = head;
    thread_list* tail = NULL;
    
    while(head!=tail) {
        curr = head;
        prev = head;
        while(curr && curr->next && curr->next!=tail) {
            if((curr->t != NULL) && (curr->t->priority > curr->next->t->priority)) {
                tmp = curr->next;
                curr->next = tmp->next;
                tmp->next = curr;
                if(curr==head) {
                    prev = tmp;
                    head = tmp;
                } else {
                    prev->next = tmp;
                    prev = prev->next;
                }
            } else {
                if(curr!=head) {
                    prev = prev->next;
                }
                curr = curr->next;
            }
        }
        // In each iteration, we need to adjust tail. And we know curr->next = tail, so we let tail = curr here.  
        tail = curr;
    }
    return head;
}

// used to test if the queue is right
void test_print_Q(thread_list *queue){
    thread_list *ptr = queue;
    while (ptr != NULL)
    {
        if(ptr->t != NULL){
            printf("%s\n", ptr->t->task_name);
        }
        
        ptr = ptr->next;
    }
    
}

void create_idle_choose_context(){

    getcontext(&cpu_idle);
	cpu_idle.uc_stack.ss_sp = malloc(STACK_SIZE);
	cpu_idle.uc_stack.ss_size = STACK_SIZE;
	cpu_idle.uc_link = NULL;
	makecontext(&cpu_idle, idle, 0);  // function is in function.h
	
	getcontext(&choose);
	choose.uc_stack.ss_sp = malloc(STACK_SIZE);
	choose.uc_stack.ss_size = STACK_SIZE;
	choose.uc_link = NULL;
	makecontext(&choose, task_choose, 0); // function bellow
}

void task_scan(){
    // dont have to do anything with READY and TERMINATED
    // only check WAITING, if they are qualified to become READY

    thread_list *temp = thread_Q;
	while(temp != NULL){

        // head of threadQ, t is NULL
        if(temp->t == NULL){
			temp = temp->next;
			continue;
		}
        if(strcmp(temp->t->cur_state, "READY") == 0 && temp != cur_active){
            temp->need_to_wait++;
            temp->t->waiting_time ++;
        }

		if(strcmp(temp->t->cur_state, "WAITING") == 0){
			temp->t->waiting_time ++;

            // printf("task = %s t= %d, limit = %d\n",temp->t->task_name, temp->t->waiting_time, temp->need_to_wait);
			
			if(temp->t->waiting_mode == 0){
                // waiting mode = 0: sleeping
				if(temp->t->waiting_time == temp->need_to_wait) 
                    strcpy(temp->t->cur_state, "READY");
			}else{
                // waiting mode = 1: sb took my resource :(
				temp->need_to_wait++;
				int get_src_fail = 0;
				for(int i = temp->got_resource; i < temp->t->count_resource; i++){
					if(resource_taken[temp->t->resource_list[i]] == true){
                        // taken == true fail to get resource
						get_src_fail = 1;
						break;
					} 
				}
				
				if(get_src_fail == 0){ // successfully get resource
                    strcpy(temp->t->cur_state, "READY");
                }
			}	
		}
		
		temp = temp->next;
	}	
}

void task_choose(){

    task_scan();
	
    // deal with RR first
	if(strcmp(ALGORITH, "RR") == 0){
	
		thread_list *temp = cur_active;
		int all_w = 0; // all tasks are waiting
		
        // not end
		if(terminated_task != total_task){
			while(temp != NULL){
				if(temp->next == NULL){ // tail
                    temp = thread_Q;   // head
                }else{
                    temp = temp->next;
                }

                // find the ready one
				if(strcmp(temp->t->cur_state, "READY") == 0) break;
				
                // a cycle, back to myself  and im not ready => means all waiting
				if(temp == cur_active && strcmp(temp->t->cur_state, "READY") != 0){
					all_w = 1;
					break;
				}else if(temp == cur_active) {
					break;
				}
			}
			
			if(pause_sig == 1) setcontext(&init);
			
            // if all waiting => idle
			if(all_w == 1){
				printf("CPU idle\n");
				setcontext(&cpu_idle);
			}else{
                // adjust the running one
				if(cur_active != temp) printf("Task %s is running.\n", temp->t->task_name);
				strcpy(temp->t->cur_state, "RUNNING");
                cur_active = temp;
				setcontext(cur_active->t->context);
			}
		}else setcontext(&init);
		
    // FCFS and PP    
	}else{
		int not_end = 0; // record the number of waiting
		thread_list *ptr = thread_Q;
				
		while(ptr!= NULL){

            if(ptr->t == NULL){
                ptr = ptr->next;
                continue;
            }

			if(strcmp(ptr->t->cur_state, "READY") == 0) break;
			else{
				if(strcmp(ptr->t->cur_state, "WAITING") == 0) not_end++;
			}
			if(ptr->next == NULL) break;
			ptr = ptr->next;
		}
		
		if(pause_sig == 1) setcontext(&init);
		
        // if it comes to the last one, no one ready => idle
		if((ptr->next == NULL) && (strcmp(ptr->t->cur_state, "READY") != 0) && (not_end != 0)){
			printf("CPU idle\n");
			setcontext(&cpu_idle);
		}else if((ptr->next == NULL) && (strcmp(ptr->t->cur_state, "TERMINATED") == 0)  && (not_end == 0)){
            // comes to the last one, its terminated, all end
			setcontext(&init);
		}else{
			// adjust the running one
            if(cur_active != ptr) printf("Task %s is running.\n", ptr->t->task_name);
            strcpy(ptr->t->cur_state, "RUNNING");
            cur_active = ptr;
            setcontext(cur_active->t->context);
    } 
		
	}	
	
}
void task_sleep(int ms)
{
    // Print a message in the format: Task {task_name} goes to sleep.
    // his task will be switched to WAITING state
    // fter 10 * msec_10 ms, this task will be switched to READY state

    thread_list *temp = cur_active;
    temp->t->waiting_mode = 0;
    strcpy(temp->t->cur_state, "WAITING");
	temp->t->burst_time++;
    temp->t->waiting_time--;
	temp->need_to_wait += ms;
    // printf("time to wait = %d\n", temp->need_to_wait);
    printf("Task %s goes to sleep.\n", temp->t->task_name);

	swapcontext(temp->t->context, &cpu_idle);
}

void task_exit()
{
    //Task {task_name} has terminated.
    terminated_task++;
    thread_list *temp = cur_active;
    strcpy(temp->t->cur_state, "TERMINATED");
    temp->t->burst_time++;
    
    printf("Task %s has terminated.\n", temp->t->task_name);
    
    swapcontext(temp->t->context, &cpu_idle);
}
