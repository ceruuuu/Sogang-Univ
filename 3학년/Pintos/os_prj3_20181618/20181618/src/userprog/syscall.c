#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

#include "userprog/process.h"
#include "threads/vaddr.h"
#include "devices/shutdown.h"
#include "userprog/pagedir.h"
#include "devices/input.h"
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "filesys/off_t.h"
#include "threads/synch.h"

typedef int pid_t;

static void syscall_handler (struct intr_frame *);
bool check_address(void *p, struct thread *t);
int write(int fd, const void *buf, unsigned size);
int read(int , void*, unsigned);
pid_t exec(const char* cmd_line);

struct file{
	struct inode *inode;
	off_t pos;
	bool deny_write;
};

struct lock lk;

void
syscall_init (void) 
{
  lock_init(&lk);
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

bool check_address(void *p, struct thread *t){
	if(!is_user_vaddr(p))
		return 1;
	if(p>=PHYS_BASE || pagedir_get_page(t->pagedir, p) == NULL)
		return 1;
	else return 0;
}

int read(int fd, void* buffer, unsigned size){
	unsigned int len = 0;
	char c = 0;
	int temp;

	if(check_address(buffer, thread_current())) exit(-1);

	lock_acquire(&lk);

	if(fd == 0){
		while(1){
			c = input_getc();
			((char*)buffer)[len] = c;
			len++;
			if(len >= size) break;
		}
		temp = len;
	}
	else if(fd >= 3 && fd < 128){
		if(thread_current()->file_info[fd] == NULL){
			exit(-1);
		}
	
		temp = file_read(thread_current()->file_info[fd], buffer, size);
	}
	else temp = -1;
	
	lock_release(&lk);
	return temp;
}

int write(int fd, const void *buffer, unsigned size){
	int temp;

	if(check_address(buffer, thread_current())) exit(-1);

	lock_acquire(&lk);
	
	if(fd == 1){
		putbuf(buffer, size);
		
		temp = size;
	}
	else if(fd >= 3 && fd < 128){
	
		if(thread_current()->file_info[fd] == NULL) {
			lock_release(&lk);
			exit(-1);
		}

		if(thread_current()->file_info[fd]->deny_write)
			file_deny_write(thread_current()->file_info[fd]);

		temp = file_write(thread_current()->file_info[fd], buffer, size);
	}
	else temp = -1;
	
	lock_release(&lk);
	return temp;
}

void exit(int state){
	if(state<0) state = -1;

	thread_current()->exit_status = state;
	thread_current()->child_signal = 1;
	
	for(int i=3;i<128;i++){
		if(thread_current()->file_info[i] != NULL) close(i);
	}
	
	printf("%s: exit(%d)\n", thread_current()->name, thread_current()->exit_status);

	thread_exit();
}

pid_t exec(const char* cmd_line){

	tid_t tid = process_execute(cmd_line);
	struct thread *t = tid_to_thread(tid);

	if(t==NULL) return -1;

	return (pid_t)tid;
}

int fibonacci(int n){
	if(n<0) exit(-1);
	if(n==0) return 0;
	if(n==1) return 1;
	return fibonacci(n-2) + fibonacci(n-1);
}

int max_of_four_int(int a, int b, int c, int d){
	
	int n[5];
	int t, i, j;
	n[0] =a; n[1]=b; n[2]=c; n[3]=d;

	for(i=0;i<4;i++){
		for(j=0;j<3;j++){
			if(n[j] < n[j+1]){
				t= n[j];
				n[j] = n[j+1];
				n[j+1] = t;
			}
		}
	}
	return n[0];
}

bool create(const char *file, unsigned initial_size){
	if(file == NULL) exit(-1);
	if(check_address(file, thread_current())) exit(-1);

	return filesys_create(file, initial_size);
}

bool remove(const char *file){
	if(file == NULL) exit(-1);
	if(check_address(file, thread_current())) exit(-1);

	return filesys_remove(file);
}	

int open(const char *file){
	struct file *cur_file;
	struct thread *cur_thread;
	int temp;

	if(file == NULL) exit(-1);
	if(check_address(file, thread_current())) exit(-1);

	lock_acquire(&lk);

	cur_file = filesys_open(file);
	cur_thread = thread_current();

	if(cur_file == NULL){
		temp = -1;		
	}
	else{
		if((cur_thread->fd) >= 128) {
			lock_release(&lk);
			return -1;
		}
		
		if(strcmp(cur_thread->name, file) == 0) file_deny_write(cur_file);

		cur_thread->file_info[cur_thread->fd++] = cur_file;	
		temp = (cur_thread->fd)-1;		
	}
	lock_release(&lk);
	return temp;
}

void close(int fd){
	struct thread *cur_thread;

	cur_thread = thread_current();

	if(fd < 3 || fd >= cur_thread->fd) exit(-1);
	if(cur_thread->file_info[fd] == NULL) exit(-1);

	file_close(cur_thread->file_info[fd]);
	cur_thread->file_info[fd] = NULL;
}

int filesize(int fd){
	struct thread *cur_thread;
	cur_thread = thread_current();

	if(fd < 3 || fd >= cur_thread->fd) exit(-1);
	if(cur_thread->file_info[fd] == NULL) exit(-1);

	return file_length(cur_thread->file_info[fd]);
}

void seek(int fd, unsigned position){
	struct thread *cur_thread;
	cur_thread = thread_current();

	if(fd < 3 || fd >= cur_thread->fd) exit(-1);
	if(cur_thread->file_info[fd] == NULL) exit(-1);

	file_seek(cur_thread->file_info[fd], position);
}

unsigned tell(int fd){
	struct thread *cur_thread;
	cur_thread = thread_current();

	if(fd < 3 || fd >= cur_thread->fd) exit(-1);
	if(cur_thread->file_info[fd] == NULL) exit(-1);

	return file_tell(cur_thread->file_info[fd]);
}

int wait(pid_t pid){
	return process_wait(pid);
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  int key;

  void* esp = f->esp;
  void* arg[5];

  struct thread *t = thread_current();

  
  if(check_address(esp,t)) exit(-1);

  key = *(int*)esp;

  switch(key){
	case SYS_HALT://halt
		shutdown_power_off();
		break;
	case SYS_EXIT:
		arg[0] = (void*)(esp+4);
		if(check_address(arg[0], t)) exit(-1);
		exit(*(int*)arg[0]);

		break;
	case SYS_EXEC:
		arg[0] = (void*)(esp+4);
		if(check_address(arg[0], t)) exit(-1);
		if(check_address(*(char**)arg[0],t)) exit(-1);

		f->eax = exec(*(char**)arg[0]);
		break;
	case SYS_WAIT:
		arg[0] = (void*)(esp+4);
		if(check_address(arg[0], t)) exit(-1);
		f->eax = wait(*(int*)arg[0]);
		break;
	case SYS_READ:
		arg[0] = (void*)((uintptr_t)esp+4);
		arg[1] = (void*)((uintptr_t)esp+8);
		arg[2] = (void*)((uintptr_t)esp+12);
		if(check_address(arg[0],t)) exit(-1);
		if(check_address(arg[1],t)) exit(-1);
		if(check_address(*(char**)arg[1], t)) exit(-1);
		if(check_address(arg[2],t)) exit(-1);
		f->eax = read(*(int*)arg[0], *(char**)arg[1], *(size_t*)arg[2]);
		//lock_release(lk);
		break;
	case SYS_WRITE:	
		arg[0] = (void*)((uintptr_t)esp+4);
		arg[1] = (void*)((uintptr_t)esp+8);
		arg[2] = (void*)((uintptr_t)esp+12);
		if(check_address(arg[0],t)) exit(-1);
		if(check_address(arg[1],t)) exit(-1);
		if(check_address(*(char**)arg[1], t)) exit(-1);
		if(check_address(arg[2],t)) exit(-1);
		f->eax = write(*(int*)arg[0], *(char**)arg[1], *(size_t*)arg[2]);
		break;
	case SYS_FIBO:
		arg[0] = (void*)(esp+4);
		if(check_address(arg[0], t)) exit(-1);
		f->eax = fibonacci(*(int*)arg[0]);
		break;
	case SYS_MOFI:
		arg[0] = (void*)((uintptr_t)esp+4);
		arg[1] = (void*)((uintptr_t)esp+8);
		arg[2] = (void*)((uintptr_t)esp+12);
		arg[3] = (void*)((uintptr_t)esp+16);
		if(check_address(arg[0],t)) exit(-1);
		if(check_address(arg[1],t)) exit(-1);
		if(check_address(arg[2],t)) exit(-1);
		if(check_address(arg[3],t)) exit(-1);
		f->eax = max_of_four_int(*(int*)arg[0],*(int*)arg[1],*(int*)arg[2],*(int*)arg[3]);
		break;
	case SYS_CREATE:
		arg[0] = (void*)((uintptr_t)esp+4);
		arg[1] = (void*)((uintptr_t)esp+8);
		if(check_address(arg[0], t)) exit(-1);
		if(check_address(*(char**)arg[0], t)) exit(-1);
		if(check_address(arg[1], t)) exit(-1);
		f->eax = create(*(char**)arg[0], *(size_t*)arg[1]);
		break;
	case SYS_REMOVE:
		arg[0] = (void*)((uintptr_t)esp+4);
		if(check_address(arg[0],t)) exit(-1);
		if(check_address(*(char**)arg[0], t)) exit(-1);
		f->eax = remove(*(char**)arg[0]);
		break;
	case SYS_OPEN:
		arg[0] = (void*)((uintptr_t)esp+4);
		if(check_address(arg[0],t)) exit(-1);
		if(check_address(*(char**)arg[0], t)) exit(-1);
	
		f->eax = open(*(char**)arg[0]);
		break;
	case SYS_CLOSE:
		arg[0] = (void*)((uintptr_t)esp+4);
		if(check_address(arg[0], t)) exit(-1);

		close(*(int*)arg[0]);
		break;
	case SYS_FILESIZE:
		arg[0] = (void*)((uintptr_t)esp+4);
		if(check_address(arg[0], t)) exit(-1);

		f->eax = filesize(*(int*)arg[0]);
		break;
	case SYS_SEEK:
		arg[0] = (void*)((uintptr_t)esp+4);
		arg[1] = (void*)((uintptr_t)esp+8);
		if(check_address(arg[0], t)) exit(-1);
		if(check_address(arg[1], t)) exit(-1);

		seek(*(int*)arg[0], *(size_t*)arg[1]);
		break;
	case SYS_TELL:
		arg[0] = (void*)((uintptr_t)esp+4);
		if(check_address(arg[0], t)) exit(-1);

		f->eax = tell(*(int*)arg[0]);
		break;
	default:
		break;


  }

}
