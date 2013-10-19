#include "userprog/syscall.h"
#include "userprog/process.h" 
#include "threads/init.h" 
#include <user/syscall.h> 
#include "devices/input.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/vaddr.h"  
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "userprog/pagedir.h"

static void syscall_handler (struct intr_frame *);

/* Implementation start */
bool is_valid_syscall(void *ptr);
void Halt(void);
void Exit(int status);
pid_t Exec(const char *cmd_line);
int Wait(pid_t pid);
int Read (int fd, void *buffer, unsigned size);
int Write(int fd, const void *buffer, unsigned size);

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

  switch(SYS_NUM){
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
  if ( ptr == NULL ||                                           //Null pointer
      !pagedir_get_page(thread_current()->pagedir,ptr) ||      //Pointer to unmapped virtual memory
      ptr >= PHYS_BASE                                         //Pointer to kernel virtual address space (above PHYS_BASE)
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
    return ;
}

void 
Exit(int status)
{
  int cnt = 0;
  struct thread *cur = thread_current();
  struct thread *parent = cur->child_memo.parent;

  if(status<0)status = -1;

  /* Check child list until child id equal to current thread id. */
  for(; cnt<CH_MAX; cnt++){
    /* If we find exact child which want to be exited */
    if(cur->tid == parent->child_memo.child_pid[cnt]){
      /* Change status */
      parent->child_memo.child_status[cnt] = status;
      /* Process termination message */
      printf("%s: exit(%d)\n", cur->name, status);
      thread_exit();
      return;
    }
  }
}

pid_t 
Exec(const char *cmd_line)
{

  /* Owned by /userprog/process.c */
  return process_execute(cmd_line);         
}

int 
Wait(pid_t pid)
{
  return process_wait(pid);
}

int 
Read (int fd, void *buffer, unsigned size)
{
  unsigned cnt = 0;

  /* Reject if syscall is not valid pointers. */  
  if(!is_valid_syscall((void*)buffer))
  {
    Exit(-1);
  }

  else if(fd == 0){
    for(; cnt<size; cnt++){
      buffer = input_getc();
      buffer++;
    }
  }
  return size;
}

int 
Write(int fd, const void *buffer, unsigned size)
{
  /* Reject if syscall is not valid pointers. */  
  if(!is_valid_syscall(buffer))
  {
    Exit(-1);
  } 

  else if(fd ==1){
    putbuf(buffer, size);
    return 0;
  }

  return size;
}
