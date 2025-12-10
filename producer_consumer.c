#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/semaphore.h>
#include <linux/delay.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("JUAN NUNEZ AIMA");

struct task_struct **producer;
struct task_struct **consumer;
struct task_struct **taskbuffer;
struct semaphore empty;
struct semaphore full;



static int prod  = 0;
static int cons = 0;
static int uid=0;
static int size = 1;
module_param(cons,int , 0);
module_param(prod, int, 0);
module_param(size,int,0);
module_param(uid, int,0);

static int in =0;
static int out =0;

static int consumerfn(void *arg) {
  struct task_struct *ts = (struct task_struct *) arg;
  char thread_name[TASK_COMM_LEN] = { };
  get_task_comm(thread_name,ts);
  struct task_struct *task;
  
  while (!kthread_should_stop()){
    if (down_interruptible(&full))
      continue;

    task = taskbuffer[out];
    pid_t parentid = task->parent->pid;
    pid_t pid = task->pid;
    printk(KERN_INFO "[%s] has consumed a zombie process with pid %d and parent pid %d", thread_name, pid, parentid);
    out = (out + 1) % size;
    up(&empty);
    msleep(50);
  }
  return 0;
}


static int producerfn(void *arg){
  struct task_struct *ts = (struct task_struct *) arg;
  char thread_name[TASK_COMM_LEN] = { };
  get_task_comm(thread_name, ts);
  struct task_struct *task;
  while (!kthread_should_stop()) {
  
    for_each_process(task){
        if (task->exit_state & EXIT_ZOMBIE){
          if(task_uid(task).val!=uid){
            continue;
            }
            if (down_interruptible(&empty))
              continue;
            taskbuffer[in]=task;
            pid_t parentpid= task->parent->pid;
            pid_t pid = task->pid;
            in = (in+1) %size;
            up(&full);
            printk(KERN_INFO "[%s] has produced a zombie process with pid %d and parent pid %d", thread_name, pid, parentpid );
            }
    }
    msleep(1000);
  }
  return 0;
}


static int __init producer_consumer_init(void){
  sema_init(&full,0);
  sema_init(&empty,size);
  taskbuffer = kmalloc(sizeof(struct task_struct *) * size, GFP_KERNEL);
  
  if (prod !=0){
  producer = kmalloc(sizeof(struct task_struct *) * prod, GFP_KERNEL);
  for (int i=0; i<prod ; i++){
    producer[i]= kthread_run(producerfn,(void *)&producer[i], "Producer-%d", i+1);
  }
  }
  
  if (cons!=0){
  consumer = kmalloc(sizeof(struct task_struct *)* cons, GFP_KERNEL);
  for (int i=0; i<cons;i++){
    consumer[i]= kthread_run(consumerfn,(void *)&consumer[i], "Consumer-%d", i+1);
  }
  }
  
  return 0;
  
}

static void __exit producer_consumer_exit(void){
    if (producer) {
        for (int i=0; i<prod; i++)
            if (producer[i])
                kthread_stop(producer[i]);
        kfree(producer);
    }
    
    if (consumer) {
        for (int i=0; i<cons; i++)
            if (consumer[i])
                kthread_stop(consumer[i]);
        kfree(consumer);
    }
    
    kfree(taskbuffer);
}


module_init(producer_consumer_init);
module_exit(producer_consumer_exit);



