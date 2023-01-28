#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include "../include/builtin.h"
#include "../include/command.h"
#include "../include/task.h"
#include "../include/function.h"


const char* ALGORITH;

int help(char **args)
{
	int i;
    printf("--------------------------------------------------\n");
  	printf("My Little Shell!!\n");
	printf("The following are built in:\n");
	for (i = 0; i < num_builtins(); i++) {
    	printf("%d: %s\n", i, builtin_str[i]);
  	}
	printf("%d: replay\n", i);
    printf("--------------------------------------------------\n");
	return 1;
}

int cd(char **args)
{
	if (args[1] == NULL) {
    	fprintf(stderr, "lsh: expected argument to \"cd\"\n");
  	} else {
    	if (chdir(args[1]) != 0)
      		perror("lsh");
	}
	return 1;
}

int echo(char **args)
{
	bool newline = true;
	for (int i = 1; args[i]; ++i) {
		if (i == 1 && strcmp(args[i], "-n") == 0) {
			newline = false;
			continue;
		}
		printf("%s", args[i]);
		if (args[i + 1])
			printf(" ");
	}
	if (newline)
		printf("\n");

	return 1;
}

int exit_shell(char **args)
{
	return 0;
}

int record(char **args)
{
	if (history_count < MAX_RECORD_NUM) {
		for (int i = 0; i < history_count; ++i)
			printf("%2d: %s\n", i + 1, history[i]);
	} else {
		for (int i = history_count % MAX_RECORD_NUM; i < history_count % MAX_RECORD_NUM + MAX_RECORD_NUM; ++i)
			printf("%2d: %s\n", i - history_count % MAX_RECORD_NUM + 1, history[i % MAX_RECORD_NUM]);
	}
	return 1;
}

bool isnum(char *str)
{
	for (int i = 0; i < strlen(str); ++i) {
    	if(str[i] >= 48 && str[i] <= 57)
			continue;
        else
		    return false;
  	}
  	return true;
}

int mypid(char **args)
{
	char fname[BUF_SIZE];
	char buffer[BUF_SIZE];
	if(strcmp(args[1], "-i") == 0) {

	    pid_t pid = getpid();
	    printf("%d\n", pid);
	
	} else if (strcmp(args[1], "-p") == 0) {
	
		if (args[2] == NULL) {
      		printf("mypid -p: too few argument\n");
      		return 1;
    	}

    	sprintf(fname, "/proc/%s/stat", args[2]);
    	int fd = open(fname, O_RDONLY);
    	if(fd == -1) {
      		printf("mypid -p: process id not exist\n");
     		return 1;
    	}

    	read(fd, buffer, BUF_SIZE);
	    strtok(buffer, " ");
    	strtok(NULL, " ");
	    strtok(NULL, " ");
    	char *s_ppid = strtok(NULL, " ");
	    int ppid = strtol(s_ppid, NULL, 10);
    	printf("%d\n", ppid);
	    
		close(fd);

  	} else if (strcmp(args[1], "-c") == 0) {

		if (args[2] == NULL) {
      		printf("mypid -c: too few argument\n");
      		return 1;
    	}

    	DIR *dirp;
    	if ((dirp = opendir("/proc/")) == NULL){
      		printf("open directory error!\n");
      		return 1;
    	}

    	struct dirent *direntp;
    	while ((direntp = readdir(dirp)) != NULL) {
      		if (!isnum(direntp->d_name)) {
        		continue;
      		} else {
        		sprintf(fname, "/proc/%s/stat", direntp->d_name);
		        int fd = open(fname, O_RDONLY);
        		if (fd == -1) {
          			printf("mypid -p: process id not exist\n");
          			return 1;
        		}

        		read(fd, buffer, BUF_SIZE);
        		strtok(buffer, " ");
        		strtok(NULL, " ");
        		strtok(NULL, " ");
		        char *s_ppid = strtok(NULL, " ");
		        if(strcmp(s_ppid, args[2]) == 0)
		            printf("%s\n", direntp->d_name);

        		close(fd);
     		}
	   	}
    	
		closedir(dirp);
	
	} else {
    	printf("wrong type! Please type again!\n");
  	}
	
	return 1;
}

/*********************************** the scheduler part ****************************************/

int add(char **args)
{
	// Command format: add {task_name} {function_name} {priority}
	// initialize the task, connect function, state = "READY"
	// put the task into my working queue and printing queue
	// sort working queue is PP
	// print: "Task {task_name} is ready."

	// printf("%s\n", ALGORITH);

	if(sizeof(args) < 4){
		printf("four argument for add function,\n");
		return 1;
	}
	char *task_name = args[1];
	char *function_name = args[2];
	int priority = atoi(args[3]);

	// initialize my thread
	tid++;
	total_task++;
	thread *t = (thread *)malloc(sizeof(thread));

	strcpy(t->task_name, task_name);
	strcpy(t->cur_state, "READY");
	t->priority = priority;
	t->tid = tid;
	t->burst_time = 0;
	t->waiting_time = 0;
	t->count_resource = 0;
	t->waiting_mode = 0;
	t->resource_list = (int *)malloc(sizeof(int) * 8); // at most 8 resource
	t->context = (ucontext_t *)malloc(sizeof(ucontext_t));

	// connect functions
	if(strcmp(function_name, "task1") == 0) create_context(t->context, NULL, &task1);
	else if(strcmp(function_name, "task2") == 0) create_context(t->context, NULL, &task2);
	else if(strcmp(function_name, "task3") == 0) create_context(t->context, NULL, &task3);
	else if(strcmp(function_name, "task4") == 0) create_context(t->context, NULL, &task4);
	else if(strcmp(function_name, "task5") == 0) create_context(t->context, NULL, &task5);
	else if(strcmp(function_name, "task6") == 0) create_context(t->context, NULL, &task6);
	else if(strcmp(function_name, "task7") == 0) create_context(t->context, NULL, &task7);
	else if(strcmp(function_name, "task8") == 0) create_context(t->context, NULL, &task8);
	else if(strcmp(function_name, "task9") == 0) create_context(t->context, NULL, &task9);
	else if(strcmp(function_name, "test_exit") == 0) create_context(t->context, NULL, &test_exit);
	else if(strcmp(function_name, "test_sleep") == 0) create_context(t->context, NULL, &test_sleep);
	else if(strcmp(function_name, "test_resource1") == 0) create_context(t->context, NULL, &test_resource1);
	else if(strcmp(function_name, "test_resource2") == 0) create_context(t->context, NULL, &test_resource2);
	else{
		free(t->context);
		free(t);
		tid--;
		return -1;
	}

	thread_list *new_thread = (thread_list *)malloc(sizeof(thread_list));
	thread_list *new_thread_copy = (thread_list *)malloc(sizeof(thread_list));
	new_thread->t = t;
	new_thread->got_resource = 0;
	new_thread->need_to_wait = 0;
	new_thread_copy->t = t;
	new_thread_copy->got_resource = 0;
	new_thread_copy->need_to_wait = 0;
	
	// put the task into the thread queue, and print queue
	EnQueTQ(new_thread);
	EnQuePQ(new_thread_copy);   // use for printing, no need to sort

	// sort thread queue if PP
	if(strcmp(ALGORITH, "PP") == 0){
		thread_list *temp = SortQue(thread_Q);
		thread_Q = temp;
	}

	printf("Task %s is ready.\n", task_name);

	fflush(stdout);

	return 1;
}

int del(char **args)
{
	// command format: del {task_name}
	// delete node (thread) from my working queue (no need to remove for print_Q)
	// change the state to TERMINATED
	// print: "Task {task_name} is killed"

	char *task_name = args[1];
	
	thread_list *ptr = thread_Q;
	bool killed = false;

	// deal with head
	if(strcmp(ptr->t->task_name, task_name) == 0){
		strcpy(ptr->t->cur_state, "TERMINATED");
		thread_Q = thread_Q->next;
		free(ptr);	
		killed = true;
	}else{
		while(ptr->next != NULL){
			if(strcmp(ptr->next->t->task_name, task_name) == 0){
				// delete thread (task)
				thread_list *to_delete = ptr->next;
				ptr->next = ptr->next->next;
				strcpy(to_delete->t->cur_state, "TERMINATED");
				free(to_delete);
				killed = true;
				break;
			}
			ptr = ptr->next;
		}
	}	


	if(!killed){
		printf("Task %s does not exist.\n", task_name);
		return 1;
	}

	printf("Task %s is killed.\n", task_name);
	terminated_task++;

	fflush(stdout);

	return 1;
}

int ps(char **args)
{
	// Show the information of all tasks, including TID, task name, task state, running time, waiting time, turnaround time, resources occupied and priority (if any)
	// only terminated task has turnaround time
	// time unit = 10ms;

	printf(" TID|      name|      state| running| waiting| turnaround| resources| priority\n");
	printf("------------------------------------------------------------------------------\n");
	thread_list *ptr = print_Q;
	char resources[10];

	while(ptr != NULL){

		if(ptr->t == NULL){
			ptr = ptr->next;
			continue;
		}

		thread* temp = ptr->t;

		memset(resources, '\0', 10);
		char *turnaround = "none";
		
		// resource list
		if(temp->count_resource == 0) strcat(resources, "none");
		else{
			for(int i = 0; i < temp->count_resource; i++){
				char src_str[3];
				sprintf(src_str, " %d", temp->resource_list[i]);
				strcat(resources, src_str);
			}
		
		}

		// turnaround time
		if(strcmp(temp->cur_state, "TERMINATED") == 0){
			int tat = temp->burst_time + temp->waiting_time;
			char tat_str[10];
			memset(tat_str, '\0', 10);
			sprintf(tat_str, "%d", tat);
			turnaround = tat_str;
		}

		printf("%4d|%10s|%11s|%8d|%8d|%11s|%10s|%9d\n", temp->tid, temp->task_name, temp->cur_state, temp->burst_time, temp->waiting_time, turnaround, resources, temp->priority);


		ptr = ptr->next;
	}
	

	return 1;
}

int start(char **args)
{
	printf("Start simulation.\n");
	// test_print_Q(thread_Q);
	// printf("----\n");
	// test_print_Q(print_Q);

	// printf("total task:%d  ternimated task: %d  timequan: %d\n", total_task, terminated_task, count_quantum);

	if(pause_sig == 1){
		// start from pause
		pause_sig = 0;

		if(strcmp(ALGORITH, "RR") == 0 && count_quantum != 0){
			// keep doing the RR with current active 
			if(cur_active->t != NULL){
				setcontext(cur_active->t->context);
			}
			
		}else{
			thread_list *ptr = thread_Q;

			// find the ready one 
			while(ptr!= NULL){

				// this part is uneeded when using properly
				if(ptr->t == NULL){
					ptr = ptr->next;
					continue;
				}

				if(strcmp(ptr->t->cur_state, "READY") == 0) break;
				else{
					if(ptr->next != NULL) ptr = ptr->next;
					else break;
				}
				// at least, stop at the last
			}
	
			// found the ready one
			// it cannot be "both terminated and the last one"
			if(ptr->next != NULL || strcmp(ptr->t->cur_state, "TERMINATED")!=0){
				printf("Task %s is running.\n", ptr->t->task_name);
				strcpy(ptr->t->cur_state, "RUNNING");
				cur_active = ptr;
				create_idle_choose_context();
				set_signal();
				swapcontext(&init, cur_active->t->context);
			}
		}
		
	}else{
		// start directly

		thread_list *ptr = thread_Q;
		// find the ready one 
			while(ptr!= NULL){
				if(ptr->t == NULL){
					ptr = ptr->next;
					continue;
				}

				if(strcmp(ptr->t->cur_state, "READY") == 0) break;
				else{
					ptr = ptr->next;
				}
			}
	
		// found the ready one
		if(ptr != NULL || strcmp(ptr->t->cur_state, "TERMINATED")!=0){
			printf("Task %s is running.\n", ptr->t->task_name);
			strcpy(ptr->t->cur_state, "RUNNING");
			cur_active = ptr;
			create_idle_choose_context();
			set_signal();
			swapcontext(&init, cur_active->t->context);
		}
	}
	
	if(pause_sig != 1)printf("Simulation over.\n");

	fflush(stdout);

	return 1;
}

#define TIME_QUANTUM 3
// signal handler
void signal_handler(){

	thread_list *temp = cur_active;
	// printf("get sig");

	// deal with RR first
	// if reach time quantum, go to choose task (tur into READY first)
	// if not, just scan and go back to work on current task
	if(strcmp(ALGORITH, "RR") == 0 && strcmp(temp->t->cur_state, "RUNNING") == 0){		
		count_quantum++;
		if(count_quantum == TIME_QUANTUM){
			// reach 30 ms
			count_quantum = 0;
		}else {
			temp->t->burst_time++;
			task_scan();
			if(pause_sig == 1) swapcontext(temp->t->context, &init);
			return;
			// keep doing my task
		}
	}
	
	// if its running, make yourself READY before choose
	if(strcmp(temp->t->cur_state, "RUNNING") == 0){
		temp->t->burst_time++;
		strcpy(temp->t->cur_state, "READY");
	}
	
	// if not currently running => end, then choose new task
	if(strcmp(temp->t->cur_state, "READY") != 0) {
		count_quantum = 0; 
		setcontext(&choose);
	}// currently running, go to choose task
	else swapcontext(temp->t->context, &choose);
	
}

void set_signal(){

	// pause signal 
	signal(SIGTSTP, pause_by_controlz);

	if(signal(SIGVTALRM, signal_handler) == SIG_ERR){
		perror("signal error :(");
		exit(EXIT_FAILURE);
	}

	// set a timer sending a signal (SIGVTALRM) every 10 ms
	struct itimerval value;
	value.it_value.tv_sec = 0;
    value.it_value.tv_usec = 10000;
    value.it_interval.tv_sec = 0;
    value.it_interval.tv_usec = 10000;

	// set if failure
    if(setitimer(ITIMER_VIRTUAL, &value, NULL)){
		perror("set timer failure :(");
		exit(EXIT_FAILURE);
	}
}

void pause_by_controlz(){
	pause_sig = 1;
}

const char *builtin_str[] = {
 	"help",
 	"cd",
	"echo",
 	"exit",
 	"record",
	"mypid",
	"add",
	"del",
	"ps",
	"start"
};

const int (*builtin_func[]) (char **) = {
	&help,
	&cd,
	&echo,
	&exit_shell,
  	&record,
	&mypid,
	&add,
	&del,
	&ps,
	&start
};

int num_builtins() {
	return sizeof(builtin_str) / sizeof(char *);
}
