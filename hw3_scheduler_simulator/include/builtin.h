#ifndef BUILTIN_H
#define BUILTIN_H

int help(char **args);
int cd(char **args);
int echo(char **args);
int exit_shell(char **args);
int record(char **args);
int mypid(char **args);

int add(char **args);
int del(char **args);
int ps(char **args);
int start(char **args);

extern const char* ALGORITH;

int tid;
int total_task;
int terminated_task;
int count_quantum;   // quantum = 30ms
int pause_sig;

void signal_handler();
void pause_by_controlz();
void set_signal();

extern const char *builtin_str[];

extern const int (*builtin_func[]) (char **);

extern int num_builtins();

#endif
