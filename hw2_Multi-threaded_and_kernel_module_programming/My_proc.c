#include<linux/module.h>
#include<linux/init.h>
#include<linux/proc_fs.h>
#include<linux/sched.h>
#include<linux/uaccess.h>
#include<linux/fs.h>
#include<linux/seq_file.h>
#include<linux/slab.h>

#define PROC_FILE_NAME "thread_info"
#define DIV_TO_MSEC 1000000
#define STR_MAX 128
#define MESSAGE_MAX 4096
static char str[MESSAGE_MAX]; // this is the content in the proc file

static int my_proc_show(struct seq_file *m, void *v){
	seq_printf(m,"%s\n",str);
	return 0;
}

ssize_t my_proc_read(struct file* file, char __user *buffer, size_t count, loff_t *f_pos){

	ssize_t RETVAL = 0;

	int n = strlen(str);

	// see if str is valid
	if(*f_pos >= n) return RETVAL;
	if(*f_pos + count > n){
		count = n - *f_pos;
	}

	// return 0 success
	if(copy_to_user(buffer, str, count)){
		return EFAULT;
	}
	printk(KERN_INFO "Successfully push message to user.");
	
	*f_pos += count;
	memset(str, 0, MESSAGE_MAX);
	return count;
}

// size_t count: length, loff_t *f_pos: offset
void count_time(char *tid_str);
ssize_t my_proc_write(struct file* file, const char __user *buffer, size_t count, loff_t *f_pos){

	char* temp = kzalloc((count + 1), GFP_KERNEL);

	if(!temp) return -EINVAL;

	// return 0 success
	if(copy_from_user(temp, buffer, count)){
		kfree(temp);
		return EFAULT;
	}
	// modify the "str" and count time
	count_time(temp);

	printk(KERN_INFO "Received %s characters from the user\n", temp);
	// printk(KERN_INFO "%s", str);
	
    return count;
}

void count_time(char *tid_str){

	long int_id; // convert str to int (tid)
	kstrtol(tid_str, 10, &int_id); 

	struct pid *thread_pid;
	struct task_struct *time_task;

	thread_pid = find_get_pid(int_id);
	time_task = pid_task(thread_pid, PIDTYPE_PID); 

	u64 ut = (time_task->utime) / DIV_TO_MSEC;
	unsigned long nv = time_task->nvcsw;
	unsigned long inv = time_task->nivcsw; 
	unsigned long switch_count = nv + inv;

	char tmp[STR_MAX];
	sprintf(tmp, "\tThreadID:%s Time:%lld(ms) context switch time:%ld\n", tid_str, ut, switch_count);

	strcat(str, tmp);
}

static int my_proc_open(struct inode *inode, struct file *file){
	return single_open(file, my_proc_show, NULL);
}

static struct proc_ops my_fops={
    .proc_open = my_proc_open,
    .proc_read = my_proc_read,
    .proc_write = my_proc_write,
	.proc_lseek = seq_lseek
};

static int __init hello_init(void){
	struct proc_dir_entry *entry;

	entry = proc_create(PROC_FILE_NAME, 0666, NULL, &my_fops);
    // mode_t: 0666(read and write), parent: NULL(proc)
    
	if(!entry){
        printk(KERN_INFO "proc file creation fail.\n");
		return -1;	
	}else{
		printk(KERN_INFO "create proc file successfully\n");
	}
	return 0;
}

static void __exit hello_exit(void){
	remove_proc_entry(PROC_FILE_NAME, NULL);
	printk(KERN_INFO "Goodbye world!\n");
}

module_init(hello_init);
module_exit(hello_exit);
MODULE_LICENSE("GPL");
