#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>

#define MAX_N 5000
#define MESSAGE_MAX 4096 // same as My_proc

int matA[MAX_N][MAX_N];
int matB[MAX_N][MAX_N];
long long int matC[MAX_N][MAX_N];
int R1 = 0, C1 = 0;
int R2 = 0, C2 = 0; // c1 == r2
int THREAD_NUM = 0;

void* multi(void* arg);
void send_threadID(int thread_now);
void read_info();

int main(int argc, char **argv){

    memset(matA, 0, sizeof(matA)); 
    memset(matB, 0, sizeof(matB)); 
    memset(matC, 0, sizeof(matC)); 

    /***** get input thread_num, two matrix *****/
    char *str = argv[1];
    THREAD_NUM = atoi(str);

    // matrix a
    FILE *fp = fopen(argv[2], "r");
    if(fp == NULL) exit(1);

    if(fscanf(fp, "%d %d", &R1, &C1) != 2)  exit(1);
    for(int i = 0; i < R1; i++)
        for(int j = 0; j < C1; j++)
            fscanf(fp, "%d", &matA[i][j]);
    fclose(fp);

    // matrix b
    fp = fopen(argv[3], "r");
    if(fp == NULL) exit(1);

    if(fscanf(fp, "%d %d", &R2, &C2) != 2)  exit(1);
    for(int i = 0; i < R2; i++)
        for(int j = 0; j < C2; j++)
            fscanf(fp, "%d", &matB[i][j]);
    fclose(fp);

    /***** multithread matrix multiplication *****/

    // declaring threads
    pthread_t threads[THREAD_NUM];
    printf("PID: %d\n", getpid());
    
    // Creating four threads, each evaluating its own part
    for (int i = 0; i < THREAD_NUM; i++) {
        pthread_create(&threads[i], NULL, multi, (void*)(i));
    }
 
    // joining and waiting for all threads to complete
    for (int i = 0; i < THREAD_NUM; i++){
        pthread_join(threads[i], NULL); 
    }  

    read_info();

    /***** write output to a file *****/
    FILE *fpOut = fopen("result.txt", "w");
    fprintf(fpOut, "%d %d\n", R1, C2);
    for (int i = 0; i < R1; i++) {
        for (int j = 0; j < C2; j++)
            fprintf(fpOut, "%lld ", matC[i][j]);      
        fprintf(fpOut, "\n");
    }
    fclose(fpOut);    

    return 0;
}

void read_info(){
    /* read the string from My_proc */
    int fd = open("/proc/thread_info", O_RDWR);
    // try to open the thread_info
    if(fd < 0){
        perror("Fail to open the proc file thread_info.");
        return errno;
    }

    char *StringToShow = malloc(4096 + 1);
    memset(StringToShow, 0, strlen(StringToShow));
    int fd_read = read(fd, StringToShow, MESSAGE_MAX);
    if(fd_read < 0){
        perror("Fail to read from the proc file thread_info.");
        return errno;
    }
    printf("%s\n", StringToShow);
}

void send_threadID(int thread_now){

    int fd = open("/proc/thread_info", O_RDWR);
    // try to open the thread_info
    if(fd < 0){
        perror("Fail to open the proc file thread_info.");
        return errno;
    }

    /* write my ID to My_proc */
    // convert int to string
    int length = snprintf(NULL, 0, "%d", thread_now);
    char *StringToSend = malloc(length + 1);
    sprintf(StringToSend, "%d", thread_now);
    // printf("\tThreadID: %s\n", StringToSend);

    int fd_write = write(fd, StringToSend, length);
    if(fd_write < 0){
        perror("Fail to write ID to the proc file thread_info.");
        return errno;
    }
    
    close(fd);
}

 
void* multi(void* arg)
{
    
    long long int row_c = (int) arg;
    int from = (row_c * R1) / THREAD_NUM;
    int to = (row_c + 1) * R1 / THREAD_NUM;

    for(int i = from; i < to; i++){
        for(int j = 0; j < C2; j++){
            matC[i][j] = 0;
            for(int k = 0; k < C1; k++){
                matC[i][j] += matA[i][k] * matB[k][j];
            }
        }
    }

    // write the id to the proc file right before the termination
    int thread_now = (int)gettid();
    send_threadID(thread_now);
}
