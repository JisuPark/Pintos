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
#include "threads/palloc.h"

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
      //if(argv[1]==NULL)f->eax = -1;
      //else 
      f->eax = Exec((const char*)argv[1]);
      break;
    case SYS_WAIT :
      f->eax = Wait((pid_t)argv[1]);
      break;
    case SYS_READ :
      f->eax = Read((int)argv[1],(void *)argv[2],(unsigned)argv[3]);
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
  if ( ptr == NULL ||  						//Null pointer
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
  int cnt = 0;
  struct thread *cur = thread_current();
  struct thread *parent = cur->parent;

  if(status<0)status = -1;

  /* Check child list until child id equal to current thread id. */
  for(;cnt<MAX_CHILD;cnt++)
  {
    /* If we find exact child which want to be exited */
    if(cur->tid == parent->child_manage.id[cnt])
    {
      /* Process termination message */
      printf("%s: exit(%d)\n",cur->name,status);
      
      /* Deattache child thread */
     // parent->child_manage.child[cnt] = 0;
      
      /* Change status */
      //parent->child_manage.status[cnt] = THREAD_DYING; 
      parent->child_manage.status[cnt] = status; 
      
      /* Decrease number of child */
      //parent->child_manage.tot_child-=1;
      
      thread_exit();
      return;
    }
  }
}

pid_t
Exec(const char* cmd_line)
{

  /* Owned by /threads/palloc.c */
  //uint32_t check = palloc_get_page(PAL_USER);
  //if(!check) return -1;

  /* Owned by /userprog/process.c */
  return process_execute(cmd_line);
}

int
Wait(pid_t pid)
{
  /* Owned by /userprog/process.c */
 // if(process_wait(pid)<0)
 // {
 //   return -1;
 // }
 // else 
  return process_wait(pid);
}

int 
Read(int fd, void *buffer, unsigned size)
{
  
  int cnt;

  if( buffer == NULL ||
      pagedir_get_page(thread_current()->pagedir,buffer) == NULL ||
      buffer+size >= (int*)PHYS_BASE )
  {
    Exit(-1);
  }
  
  else if( fd == 0 )
  {
    for(cnt = 0 ; cnt < size ; cnt++)
    {
      buffer = input_getc();
      buffer++;
    }
  }

  return size;
  
}

int
Write(int fd, const void *buffer, unsigned size)
{
  if( buffer == NULL ||
      pagedir_get_page(thread_current()->pagedir,buffer) == NULL ||
      buffer+size >= (int*)PHYS_BASE )
  {
    Exit(-1);
  }
  
  else if(fd == 1)
  {
    putbuf(buffer,size);
    return 0;
  }

  return size;
}

