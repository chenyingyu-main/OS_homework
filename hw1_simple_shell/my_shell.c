#ifndef _POSIX_C_SOURCE
	#define  _POSIX_C_SOURCE 200809L
#endif

#include <sys/wait.h>   /* waitpid() and associated macros */
#include <unistd.h>     /* chdir(), fork() etc */
#include <stdlib.h>     /* malloc, free, exit, execvp,  EXIT_SUCCESS*/
#include <stdio.h>      /* we have getline() in it */
#include <string.h>
# include <fcntl.h>
# include <dirent.h>
#include <stdbool.h> 

/* from TA: https://brennan.io/2015/01/16/write-a-shell-in-c/ */
/* a simple shell without pipline (simple command only) */

#define MAX_ARGS_NUM 50
#define MAX_ARGS_LENGTH 100
/* struct for command: split for several demands */
struct command{
  struct command *next;         /* point to the next command */
  int start, end;				        /* pointer: record the start and end position of command */
  int argc;			                /* argument count */
  bool in_redir, out_redir;			/* < (in_redir), > (out_redir)*/
  char readfile[MAX_ARGS_LENGTH];
  char writefile[MAX_ARGS_LENGTH];	            /* the path of redirection file */
  char *args[MAX_ARGS_NUM];                  /* echo 123, argc = 2, args[0] = echo, args[1] = 123 */
  bool bg_exec; 			          /* 0: no need back ground execution */
};

/* function used */
void lsh_loop(void);
int main(int argc, char **argv)
{
  // print hello-messege
  printf("==================================================\n");
  printf("* Welcome to my little shell:)                   *\n");
  printf("*                                                *\n");
  printf("* Type \"help\" to see builtin founction.          *\n");
  printf("*                                                *\n");
  printf("* If you want to do things below:                *\n");
  printf("* + redirection: \">\" or \"<\"                      *\n");
  printf("* + pipe: \"|\"                                    *\n");
  printf("* + background: \"&\"                              *\n");
  printf("* Make sure they are seperated by \"(space)\".     *\n");
  printf("*                                                *\n");
  printf("* Have fun!! AND score high ;)                   *\n");
  printf("==================================================\n");

  // Run command loop.
  lsh_loop();
  
  return 0;
}

/***************************** command loop *********************************/
char *read_line(void);
bool check_replay(char* line);
void manage_record(char* line);
void browse_line(char* line, struct command* cmd);
void parse_command(char* line, struct command* cmd);
int lsh_execute(struct command* cmd);
void initialize(struct command* cmd);

int record_pos = 0;
char *record_cmd[16];

char *line;   // command string
struct command cmd;
void lsh_loop(void)
{
  int status;

  // basic body: read, parse, execute
  do {
    printf(">>> $ ");
    fflush(stdin);               // Empty the input buffer, to ensure that following data reads are not affected
    cmd.start = 0;

    line = read_line();          /* read line  */

    // change line if replay
    bool replay_err = check_replay(line);
    if(replay_err) continue;

    // manage the record_command
    manage_record(line);

    browse_line(line, &cmd);     
    // '&' or not, get the end of line
    parse_command(line, &cmd);   /* parse command  */ 

    struct command* pcmd = &cmd;
    status = lsh_execute(pcmd);  /* execute command  */ 

    // if not background
    if(cmd.bg_exec == false)     
      wait(NULL);

    struct command* temp;
		pcmd = pcmd->next;
		while(pcmd){
			temp = pcmd->next;
			free(pcmd);
			pcmd = temp;
		}

    free(line);
    initialize(&cmd);
  } while (status);
}

// SUPER IMPORTANT 
void initialize(struct command* cmd){
	cmd->next = NULL;
	cmd->start = -1;
  cmd->end = -1;
	cmd->argc = 0;
	cmd-> in_redir = cmd->out_redir = false;
	cmd->bg_exec = false;

  memset(cmd->readfile, 0, MAX_ARGS_LENGTH);
  memset(cmd->writefile, 0, MAX_ARGS_LENGTH);
  for(int i = 0; i < MAX_ARGS_NUM; i++) cmd->args[i] = NULL;
  // these are a must do !!!!!!!!!!
  // or you will get a error !!!!! (lsh: bad address) :(
  // REMEMBER dont let the previous address affect you!!!
}

/************************ replay and record ***********************/
bool check_replay(char* line){

  char *s = strstr(line, "replay");  // to see if there's "replay" contained
  if(s == NULL) return 0; 

  // else : replay certain command
  char *token;
  char new_line[500];
  memset(new_line, 0, 500);
  
  token = strtok(line, " "); // split the line with " "
  while(token != NULL){
    if(strcmp(token, "replay") != 0){
      // token is not "replay"
      strcat(new_line, token);

    }else{
      token = strtok(NULL, " "); // replay | the rest
      int number = atoi(token);
      if(number < 1 || number > 16 || number > record_pos) {
        printf("replay: wrong args\n");
        return 1; // error

      }else{
        // no error
        strcat(new_line, record_cmd[number - 1]);
      }
    }
    token = strtok(NULL, " ");		
  }

  // change line to new_line    
  strcpy(line, new_line); // make the command line the line
  strcat(line, "\n");

  return 0; // no error
}

void manage_record(char* line){

  if((strcmp(line, "\n") == 0) || (strcmp(line, "  \n") == 0) || (strcmp(line, " \n") == 0)){
    return;
  }

  // space left
  if(record_pos < 16){
    char *s = malloc(sizeof(char)* strlen(line));
    strncpy(s, line, strlen(line) - 1); // length - 1
    record_cmd[record_pos] = s; 
    record_pos++;

  // no space left: pos = 16
  }else{
    char *bye = record_cmd[0];    // bye bye first command
    free(bye);
    for(int i = 0; i < 15;i++){   // move forward
      record_cmd[i] = record_cmd[i+1];
    }
    char *s = malloc(sizeof(char)* strlen(line));
    strncpy(s, line, strlen(line) - 1); 
    record_cmd[record_pos-1] = s; // refresh the command to record_cmd[15] 
  }
}

/************************ read line and parse line ***********************/
/****** read line *****/
char *read_line(void)
{
  char *line = NULL;
  size_t bufsize = 0; // have getline allocate a buffer for us

  // if unsuccessful return 
  if (getline(&line, &bufsize, stdin) == -1){
    if (feof(stdin)) {
      exit(EXIT_SUCCESS);  // We recieved an EndOfFile
    } else  {
      perror("readline");
      exit(EXIT_FAILURE);
    }
  }

  return line;
}

/****** parse line *****/
void browse_line(char* line, struct command* cmd)
{
  for(int i = 0; i < strlen(line); i++){
		if(line[i] == '&'){
			line[i] = ' ';
			cmd->bg_exec = 1;
		}
		
		if(line[i] == '\n'){
			line[i] = '\0';  // end mark
			cmd->end = i;    
		}
	}
}

int get_path(char *dst, char *line, int p);   // and return the end_pos of dst in line
void parse_command(char* line, struct command* cmd)
{
  int start = cmd->start, end = cmd->end;  // that is 0 and strlen(line)
  bool with_bg = cmd->bg_exec;

  struct command *pipe_cmd = cmd;  // pipe_cmd point to the same address of cmd
  initialize(pipe_cmd); // initialize piped command

  char c; 
  bool c_is_letter = 0;
  // browse the entire line
  for(int i = start; i < end; i++){
    c = line[i];

		if(c == '<'){
      line[i] = '\0'; // end point: used to cut 
			pipe_cmd->in_redir = true;

			int j = get_path(pipe_cmd->readfile, line, i + 2); 
      // i + 2 => < filePath（readfile = filePath）
      // return the pos of (end of filePath)
			
			// if found the name of the file, change the i to j
			if(j > 0) i = j;

      c_is_letter = 0;

		}else if( c == '>'){
      line[i] = '\0'; // end point: used to cut 
			pipe_cmd->out_redir = true;

			int j = get_path(pipe_cmd->writefile, line, i + 2);
			if(j > 0) i = j;

      c_is_letter = 0;

		}else if(c == '|'){ // pipline 
      c_is_letter = 0;
      line[i] = '\0';

			pipe_cmd->end = i;
			pipe_cmd->bg_exec = with_bg;
			pipe_cmd->next = (struct command*) malloc(sizeof(struct command)); 
      // point to a new empty command
			pipe_cmd = pipe_cmd->next; // move on to the new command
			initialize(pipe_cmd);      // initialize, make a new command start = -1, end = -1

		}else if(c == ' ' || c == '\0'){
      c_is_letter = 0;
      line[i] = '\0';

		}else{  
      // there's a letter or a number

			// if pipline happened, a new pcmd is initialized start == -1
			if(pipe_cmd->start == -1) 
        pipe_cmd->start = i;

			if(!c_is_letter){
				c_is_letter = 1;
				pipe_cmd->args[pipe_cmd->argc++] = line + i; 	
        // get the string start at line[i]
			}
		}
  }

  pipe_cmd->end = end;
	pipe_cmd->bg_exec = with_bg;
	pipe_cmd->next = NULL;
}

// get path, and return the end_pos of dst in line
int get_path(char *dst, char *line, int p){
  // p = begin pos
  // line = input command string
  // dst = file path

  if(line[p] == ' ' || line[p] == '\0') return -1;		/* do not find name of file */

	int c_dst = 0;
	char c = line[p];

	// copy my src string to dst
	while( c !=' ' && c !='|' && c != '<' && c != '>' && c != '\n' && c != '\0'){
	
		dst[c_dst++] = c;
		c = line[++p];
	}

	// return the pos of the end of the dst (in line)
	return p-1;
}

/**************************** execute command: launch and execute ***************************/

/*** Function Declarations for builtin shell commands: ***/
int lsh_help(struct command* cmd);
int lsh_cd(struct command* cmd);
int lsh_echo(struct command* cmd);
int lsh_record(struct command* cmd);
int lsh_mypid(struct command* cmd);
int lsh_exit(struct command* cmd);

/*** List of builtin commands, followed by their corresponding functions. ***/
char *builtin_str[] = {
  "help",
  "cd",
  "echo", 
	"record", 
	"mypid", 
  "exit"
};

int (*builtin_func[]) (struct command*) = {
  &lsh_help,
  &lsh_cd,
  &lsh_echo,
  &lsh_record,
  &lsh_mypid,
  &lsh_exit
};

int lsh_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

/***************** Builtin function implementations.****************/
int lsh_cd(struct command* cmd)
{
  if (cmd->args[1] == NULL) {
    fprintf(stderr, "lsh: expected argument to \"cd\"\n");
  } else {
    // chdir: change the directory to path args[1]
    // successful return value = 0
    if (chdir(cmd->args[1]) != 0) {
      perror("lsh");
    }
  }
  return 1;
}

int lsh_help(struct command* cmd)
{
  printf("----------------------------------------------------\n");
	printf("my little shell                                     \n");
	printf("type program names and arguments, and hit enter.    \n\n");
	printf("the following are bulid in:                         \n");
	printf("1: help:   show all bulid-in function info         \n");
	printf("2: cd:     change directory                        \n");
	printf("3: echo:   echo the strings to standard output     \n");
	printf("4: record: show last-16 cmds you typed in          \n");
	printf("5: replay: re-execute the cmd showed in record     \n");
	printf("6: mypid:  find and print process-ids              \n");
	printf("7: exit:   exit shell                              \n\n");
	printf("use the \"man\" command for information on other programs.\n");
	printf("----------------------------------------------------\n");
  return 1;
}

int lsh_echo(struct command* cmd){

  int i=1; // start from args[1]

  if(cmd->argc <= 1){
    perror("echo: please enter argument.");
  }

	if(strcmp(cmd->args[1], "-n") == 0){
		i++;

    do{
      fprintf(stdout, "%s ", cmd->args[i]);
      i++;
    }while (cmd->args[i] != NULL);
	}
  else{
		do{
			fprintf(stdout, "%s ", cmd->args[i]);
      i++;
		}while(cmd->args[i] != NULL);
		fprintf(stdout, "\n");
	}

  return 1;
}

// for linux: CANNOT text on mac :(
int lsh_mypid(struct command* cmd){

  // gentle reminder: use fopen() with “r”

  if(strcmp(cmd->args[1], "-i") == 0){
    // simply get and show
    int pid = getpid();
		printf("%d\n", pid);

  }else if(strcmp(cmd->args[1], "-p") == 0){

    /* reference: https://www.linuxquestions.org/questions/linux-enterprise-47/ppid-from-proc-file-system-594775/ */
    // /proc/<pid>/stat: pid(int) + a command(string) + status(char) + ppid(int)

    // file_address = "/proc/<pid>/stat"
    int pid = atoi(cmd->args[2]);
    char file_ad[200];           // long enough 
    sprintf(file_ad, "/proc/%d/stat", pid); // connect file string (address)


    if(access(file_ad, F_OK) == 0){
			// if we can acess the file, open the file with read only
			FILE *file = fopen(file_ad, "r");
			
			int pid, ppid;
			char comm[1000], state;
			fscanf(file, "%d %s %c %d", &pid, comm, &state, &ppid);
			printf("%d\n", ppid);
			
			fclose(file);
		}else printf("mypid -p: process id not exist\n");
    
  }else if(strcmp(cmd->args[1], "-c") == 0){

    // You can get the pids of all child processes of a
    // given parent process <pid> by reading the /proc/<pid>/task/<tid>/children entry.

    int pid = atoi(cmd->args[2]);
    char file_ad[100];
		sprintf(file_ad, "/proc/%d/task/%d/children", pid, pid);
		
		if(access(file_ad, F_OK) == 0){
			FILE *fd = fopen(file_ad, "r");
			
			char cpid[1000];
			while(fgets(cpid, 1000, fd) != NULL){
        printf("%s\n", cpid);
      }
			
			fclose(fd);
		}else printf("mypid -c: process id not exist\n");
  }

  return 1;
}

int lsh_record(struct command* cmd){

  printf("history command:\n");
	for(int i = 0; i < record_pos ; i++){
		printf(" %d: %s\n", i + 1, record_cmd[i]);
	}
	printf("\n");

  return 1;
}

int lsh_exit(struct command* cmd)
{
  printf("my little shell: see you next time.\n");
  return 0;
}

/************* execute our command ************/


/* reference :
*  open(), close(): https://blog.jaycetyle.com/2018/12/linux-fd-open-close/
*  O_TRUNC: https://manpages.ubuntu.com/manpages/impish/zh_TW/man2/open.2.html */
/* adjust input source and output destination */
void adjust_in_out(struct command* cmd, int readfile, int writefile){

  // if cmd with "<" : input redirection
	if(cmd->in_redir == 1){
		int read_src = open(cmd->readfile, O_RDONLY);  // open readfile, read_only
		dup2(read_src, STDIN_FILENO);                  // point stdin_fileno to the read port (opened readfile)
    // STDIN_FILENO is the default standard input file descriptor number which is 0
		close(read_src);                               // after 
	}

  // if cmd with ">" : output redirection
	if(cmd->out_redir == 1){
		int flag = O_WRONLY|O_TRUNC|O_CREAT;                  // write_only, refresh the content, create new file
		int write_dst = open(cmd->writefile, flag, S_IRWXU);  // can write, read, exec
		dup2(write_dst, STDOUT_FILENO);
		close(write_dst);
	}

	if(readfile != STDIN_FILENO){
		dup2(readfile,STDIN_FILENO);   // make stdin_fileno point to the readfile
		close(readfile);
	}

	if(writefile != STDOUT_FILENO){
		dup2(writefile, STDOUT_FILENO); // so as the stdout_fileno
		close(writefile);
	}
}

int lsh_launch(struct command* cmd)
{
  // the last command: set the in and out back to stdin stdout
	if(cmd->next == NULL){

    adjust_in_out(cmd, STDIN_FILENO, STDOUT_FILENO);
    // this is ESSENTIAL when ">" or "<"
    // but no use for other command
		
    // execute the corresponding command
		for (int i = 0; i < lsh_num_builtins(); i++) {
      if (strcmp(cmd->args[0], builtin_str[i]) == 0) {
        return (*builtin_func[i])(cmd);
      }
    }
    // execution error
    if(execvp(cmd->args[0], cmd->args) == -1) perror("lsh");
		exit(EXIT_FAILURE);
		
		return 1;
	} 

  /*********** multipipe ************/

  int pipefd[2];
	// 1 is read end, 0 is write end
  // fd[0] fd[1]

  if (pipe(pipefd) < 0) {
		perror("lsh: pipe could not be initialized");
		return 0; 
	}
	pid_t p1 = fork();

	if(p1 < 0){			/* error forking */
		perror("lsh: couldn't fork");
    return 0;
	}
	else if(p1 == 0){		
    // child processing 
		close(pipefd[0]);          // It only needs to read at the read end
		adjust_in_out(cmd, STDIN_FILENO, pipefd[1]);  // set in to pipefd[1]
		
    // just as above 
		for (int i = 0; i < lsh_num_builtins(); i++) {
      if (strcmp(cmd->args[0], builtin_str[i]) == 0) {
        return (*builtin_func[i])(cmd);
      }
    }if(execvp(cmd->args[0], cmd->args) == -1) perror("lsh");

		exit(EXIT_FAILURE);
		return 1;
	}
	else{	
    // parent processing 
		wait(NULL);                   // parent executing, waiting for child
		cmd = cmd->next;              // move on to the next piping command
		close(pipefd[1]);             // only needs to write at the write end
		adjust_in_out(cmd, pipefd[0], STDOUT_FILENO); // set out to pipefd[0]
		return lsh_launch(cmd);       // multi pipe 
	}
}

int lsh_execute(struct command* cmd)
{
  if (cmd->args[0] == NULL) return 1;  // An empty command was entered.

  // the last command & no background
	if(cmd->next == NULL && cmd->bg_exec == false){

    // do the corresponding  build in functions 
		for(int i = 0; i < lsh_num_builtins(); i++){
			if(strcmp(cmd->args[0], builtin_str[i]) == 0){
				return (*builtin_func[i])(cmd);
			}
		}
	}

  // with background or pipeline *************

	// let's fork a process that execute our command !!! 
	pid_t pid = fork();
	
	if(pid < 0) 
    perror("lsh");        // error forking
	else if (pid == 0) 
    lsh_launch(cmd);      //child process
	
	// if background / no background
	if(cmd->bg_exec) printf("[Pid]: %d\n", pid);
	else {
    wait(NULL); // wait(NULL) will block the parent process until any of its children has finished.
  } 
	
  return 1;
}
