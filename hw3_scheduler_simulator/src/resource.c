#include "../include/resource.h"
#include "../include/task.h"
#include <stdlib.h>
#include <ucontext.h>
#include <stdio.h>

bool resource_taken[8] = {false,false,false,false,false,false,false,false};

void get_resources(int count, int *resources)
{
    // Task {task_name} gets resource {resource_id}.
    // Check if all resources in the list are available
    bool get_all = true;
	
	// count_resource: count should get resource number
	// got resource: number of gotten resource
    thread_list *temp = cur_active;
    temp->t->count_resource = temp->got_resource;
	temp->t->waiting_mode = 0; // == 1 means sb took my resource
	
	for(int i = temp->got_resource; i < (temp->t->count_resource + count); i++){
		if(resource_taken[resources[i]] == true) get_all = false;
		temp->t->resource_list[i] = resources[i - temp->got_resource];
	}
	
    // get all resource
    // Get the resource(s)
    // Print a message for each resource in the list in the format:
    // Task {task_name} gets resource {resource_id}.
	if(get_all == true){
		for(int i = 0; i < count ; i++){
			printf("Task %s gets resource %d.\n", temp->t->task_name, resources[i]);
			resource_taken[resources[i]] = true;
		}
		
		temp->t->count_resource += count;
		temp->got_resource = temp->t->count_resource;
		
	}else{
    // This task will be switched to WAITING state
    // Print a message in the format: Task {task_name} is waiting resource.
    // When all resources in the list are available, this task will be switched to READY state
    // Check again when CPU is dispatched to this task
		temp->t->count_resource += count;
        strcpy(temp->t->cur_state, "WAITING");
		temp->t->waiting_mode = 1;
        // adjust the time counting
		temp->t->burst_time++;
        temp->t->waiting_time--;
        temp->need_to_wait--;
        printf("Task %s is waiting resource.\n", temp->t->task_name);
		setcontext(&cpu_idle);
	} 
}

void release_resources(int count, int *resources)
{
    // Release the resource(s)
    // Print a message for each resource in the list in the format: Task {task_name} releases resource {resource_id}. 
    thread_list *temp = cur_active;
	temp->t->count_resource = 0;
    for(int i = 0; i < count; i++){
		resource_taken[resources[i]] = false; // return the resource
		temp->t->resource_list[i] = resources[i];
	}
	
	for(int i = 0; i < count; i++){
		printf("Task %s releases resource %d.\n", temp->t->task_name, resources[i]);
	}
	printf("\n"); 
}
