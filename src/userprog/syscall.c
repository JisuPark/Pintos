#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

#include "threads/vaddr.h"
#include "threads/init.h"
#include "devices/shutdown.h"
#include "userprog/process.h"
#include "devices/input.h" 
#include <user/syscall.h>
#include "userprog/pagedir.h"

static void syscall_handler (struct intr_frame *);
bool is_valid_syscall(void *ptr);
void Halt(void);
void Exit(int status);
pid_t Exec(const char* cmd_line);
int Wait(pid_t pid);
int Read(int fd,void *buffer,unsigned size);
int Write(int fd,const void *buffer, unsigned size);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  int SYS_NUM = *(int*)(f->esp);
  int *argv = f->esp;

  /* Reject if syscall is not valid pointers. */
  if(!is_valid_syscall(argv))
  {
    Exit(-1);
  }
 
  switch(SYS_NUM)
  {
    case SYS_HALT :
      Halt();
      break;
    case SYS_EXIT :
      Exit((int)argv[1]);
      break;
    case SYS_EXEC :
      f->eax = Exec((const char*)argv[1]);
      break;
    case SYS_WAIT :
      f->eax = Wait((pid_t)argv[1]);
      break;
    case SYS_READ :
      f->eax = Read((int)argv[1],(void*)argv[2],(unsigned)argv[3]);
      break;
    case SYS_WRITE :
      f->eax = Write((int)argv[1],(void*)argv[2],(unsigned)argv[3]);
      break;
    
    default:
      Exit(-1);
      break;
  }  
}

bool
is_valid_syscall(void *ptr)
{
  if ( ptr == NULL ||  							//Null pointer
       !pagedir_get_page(thread_current()->pagedir,ptr)	||	//Pointer to unmapped virtual memory      
       ptr >= PHYS_BASE						//Pointer to kernel virtual address space (above PHYS_BASE)
     )
  {
    return false;  
  }
  return true;
}

void
Halt(void)
{
  /* Owned by ../devices/shutdown.c. */
  shutdown_power_off();
}

void
Exit(int status)
{
  struct thread *thread_cur = thread_current();
  
  struct list_elem *e;
  struct child_thread *c;

  /* Process termination message */
  printf("%s: exit(%d)\n",thread_cur->name,status);

  /* Check child list until child id equal to current thread id. */
  for( e = list_front(&thread_cur->parent->child);
       e!= list_end(&thread_cur->parent->child);
       e = list_next(e))
  {
    c = list_entry(e, struct child_thread, elem);
    if(c->tid == thread_cur->tid)
    {
      c->status = status;
      break;
    }
  }

  /* Owned by ../threads/thread.c. */
  thread_exit();
}

pid_t
Exec(const char* cmd_line)
{
  /* Owned by /userprog/process.c */
  return process_execute(cmd_line);
}

int
Wait(pid_t pid)
{
  /* Owned by /userprog/process.c */
  return process_wait(pid);
}

int 
Read(int fd, void *buffer, unsigned size)
{
  int cnt;
  
  if( fd == 0 )
  {
    for(cnt = 0 ; cnt < size ; cnt++)
    {
      buffer = input_getc();
      buffer++;
    }

    return cnt;
  }

  return 0;
}

int
Write(int fd, const void *buffer, unsigned size)
{
  if(fd == 1){
    putbuf(buffer,size);
    return size;
  }
  return 0;
}

