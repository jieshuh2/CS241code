# Angrave's 2020 Acme CS 241 Exam Prep		
## A.K.A. Preparing for the Final Exam & Beyond CS 241... 

Some of the questions require research (wikibook; websearch; wikipedia). 
It is accepted to work together and discuss answers, but please make an honest attempt first! 
Be ready to discuss and clear confusions & misconceptions in your last discussion section.
The final will also include pthreads, fork-exec-wait questions and virtual memory address translation. 
Be awesome. Angrave.

## 1. C 


1.	What are the differences between a library call and a system call? Include an example of each.
 System call is a function provided by the kernel to enter kernel mode to access hardware resources whereas, library call is a function provided by programming libraries.

2.	What is the `*` operator in C? What is the `&` operator? Give an example of each.
*gets the value of pointer, & get the address of the value
char a
char * p = &a
a = *p

3.	When is `strlen(s)` != `1+strlen(s+1)` ?
for example strlen(s) = 0, then s + 1 goes out of scope and might fall into another data structure

4.	How are C strings represented in memory? What is the wrong with `malloc(strlen(s))` when copying strings?
a null terminate array. It should include the null byte

5.	Implement a truncation function `void trunc(char*s,size_t max)` to ensure strings are not too long with the following edge cases.
```
if (length < max)
    strcmp(trunc(s, max), s) == 0
else if (s is NULL)
    trunc(s, max) == NULL
else
    strlen(trunc(s, max)) <= max
    // i.e. char s[]="abcdefgh; trunc(s,3); s == "abc". 
```


6.	Complete the following function to create a deep-copy on the heap of the argv array. Set the result pointer to point to your array. The only library calls you may use are malloc and memcpy. You may not use strdup.

    `void duplicate(char **argv, char ***result);` 

7.	Write a program that reads a series of lines from `stdin` and prints them to `stdout` using `fgets` or `getline`. Your program should stop if a read error or end of file occurs. The last text line may not have a newline char.

## 2. Memory 

1.	Explain how a virtual address is converted into a physical address using a multi-level page table. You may use a concrete example e.g. a 64bit machine with 4KB pages. 
One page has 2^12 pages this use the last 12 bits as offset, the 2^52page are used for page number if it is a single-level page. that means we can only have 2^52 page but if we use 2^26 to store the first level page number, the next 2^26 to store the next level, we can look up one by one to find the physical memory address

2.	Explain Knuth's and the Buddy allocation scheme. Discuss internal & external Fragmentation.
Knuth is using boundary tag to show start and end of data segment
Buddy: If there are no free blocks of size 2n, go to the next level and steal that block and split it into two. If two neighboring blocks of the same size become unallocated, they can coalesce together into a single large block of twice the size.
3.	What is the difference between the MMU and TLB? What is the purpose of each?
 Memory Management Unit (MMU) performs the address translation. If the translation succeeds, the page gets pulled from RAM The result is cached in the TLB
4.	Assuming 4KB page tables what is the page number and offset for virtual address 0x12345678  ?

5.	What is a page fault? When is it an error? When is it not an error?
A page fault may happen when a process accesses an address in a frame missing in memory.
If the mapping to the page is exclusively on disk.
6.	What is Spatial and Temporal Locality? Swapping? Swap file? Demand Paging?

## 3. Processes and Threads 

1.	What resources are shared between threads in the same process?
Heap, glocal value
2.	Explain the operating system actions required to perform a process context switch
Firstly, the context of the process P1 i.e. the process present in the running state will be saved in the Process Control Block of process P1 i.e. PCB1.
Now, you have to move the PCB1 to the relevant queue i.e. ready queue, I/O queue, waiting queue, etc.
From the ready state, select the new process that is to be executed i.e. the process P2.
Now, update the Process Control Block of process P2 i.e. PCB2 by setting the process state to running. If the process P2 was earlier executed by the CPU, then you can get the position of last executed instruction so that you can resume the execution of P2.
3.	Explain the actions required to perform a thread context switch to a thread in the same process
during a thread switch, the virtual memory space remains the same
4.	How can a process be orphaned? What does the process do about it?
parent terminte when child still running
5.	How do you create a process zombie?
Don't wait for children
6.	Under what conditions will a multi-threaded process exit? (List at least 4)
exit
pthread_exit in last thread
return from main
segfault?
## 4. Scheduling 
1.	Define arrival time, pre-emption, turnaround time, waiting time and response time in the context of scheduling algorithms. What is starvation?  Which scheduling policies have the possibility of resulting in starvation?
arrival time is the time that the process arrive and availabe to be run. pre-emption means drop the current run process when a quantum end. 
turnaround time is the end_time - arrival_time, waiting time = turnarond - runtime. response is start - arrive. starvation means that
starvation earlier processes may never be scheduled to run (assigned a CPU). Like shortest run first
2.	Which scheduling algorithm results the smallest average wait time?
SJF
3.	What scheduling algorithm has the longest average response time? shortest total wait time?
FCFS
4.	Describe Round-Robin scheduling and its performance advantages and disadvantages.
It will arrange process in the sequence they come but they only run for one quantum and are put back to the queue
Large number of processes = Lots of switching
5.	Describe the First Come First Serve (FCFS) scheduling algorithm. Explain how it leads to the convoy effect. 
THe process come first are served first. Long process can block short process
6.	Describe the Pre-emptive and Non-preemptive SJF scheduling algorithms. 

7.	How does the length of the time quantum affect Round-Robin scheduling? What is the problem if the quantum is too small? In the limit of large time slices Round Robin is identical to _____?

8.	What reasons might cause a scheduler switch a process from the running to the ready state?
like in rr, the quantum end
## 5. Synchronization and Deadlock

1.	Define circular wait, mutual exclusion, hold and wait, and no-preemption. How are these related to deadlock?
circular wait is like a wait b, b wait a. mutual exclusion means that one resource can only be hold by one process.
no-preemption means that one cannot force other to give up their resource.  hold and wait means that one can hold a resource without have another so they need to wait.
2.	What problem does the Banker's Algorithm solve?
It tries to solve livelock
3.	What is the difference between Deadlock Prevention, Deadlock Detection and Deadlock Avoidance?

4.	Sketch how to use condition-variable based barrier to ensure your main game loop does not start until the audio and graphic threads have initialized the hardware and are ready.
numthread
numfin
numwait
wait(){
    if numwait < numthread
        wait
    else
        numfin = 0
        broadcast
    numfin ++
    if numfin < numthread
        wait
    else
        numwait = 0
        broadcast
    numwait++
}
5.	Implement a producer-consumer fixed sized array using condition variables and mutex lock.

6.	Create an incorrect solution to the CSP for 2 processes that breaks: i) Mutual exclusion. ii) Bounded wait.

7.	Create a reader-writer implementation that suffers from a subtle problem. Explain your subtle bug.

## 6. IPC and signals

1.	Write brief code to redirect future standard output to a file.
cat a.txt > b.txt
2.	Write a brief code example that uses dup2 and fork to redirect a child process output to a pipe
pipe[2]
pipe(pipe)
fork
    if child == 0
        dup2((pipe_fds[1], 1);
    else
        ...
3.	Give an example of kernel generated signal. List 2 calls that can a process can use to generate a SIGUSR1.
kill(pid, SIGUSR1);
killall -SIGUSR1 dd
4.	What signals can be caught or ignored?
almost all except sigkill. Something like sigfault can be ignore but you should not do that
5.	What signals cannot be caught? What is signal disposition?
A signal disposition is a per-process attribute that determines how a signal is handled after it is delivered
6.	Write code that uses sigaction and a signal set to create a SIGALRM handler.
struct sigaction sa;
sa.sa_handler = myhandler;
sigemptyset(&sa.sa_mask);
sa.sa_flags = 0;
sigaction(SIGALRM, &sa, NULL)
7.	Why is it unsafe to call printf, and malloc inside a signal handler?
it use buffer and is not signal safe
## 7. Networking 

1.	Explain the purpose of `socket`, `bind`, `listen`, and `accept` functions
socket set up socket. 
bind bind the socket to port
listen put client in the back queue
accept accept the client's request
2.	Write brief (single-threaded) code using `getaddrinfo` to create a UDP IPv4 server. Your server should print the contents of the packet or stream to standard out until an exclamation point "!" is read.

3.	Write brief (single-threaded) code using `getaddrinfo` to create a TCP IPv4 server. Your server should print the contents of the packet or stream to standard out until an exclamation point "!" is read.

4.	Explain the main differences between using `select` and `epoll`. What are edge- and level-triggered epoll modes?
select will wait for any of those file descriptors to become ‘ready’.
It is a more efficient way to wait for many file descriptors. It will tell you exactly which descriptors are ready. 
edge force you to finish reading all the data but level can return without finish
5.	Describe the services provided by TCP but not UDP. 
TCP will wait for your data to arrive and make sure no data is lost
6.	How does TCP connection establishment work? And how does it affect latency in HTTP1.0 vs HTTP1.1?

7.	Wrap a version of read in a loop to read up to 16KB into a buffer from a pipe or socket. Handle restarts (`EINTR`), and socket-closed events (return 0).
while(1) {
    if (read = -1 and EINTR) {
        countinue
    }
    if (read == 0) {
        return bytes
    }
    if (read == -1) {
        return -1
    }
    bytes += read
}

8.	How is Domain Name System (DNS) related to IP and UDP? When does host resolution not cause traffic?
DNS maps hostnames to addresses
9.	What is NAT and where and why is it used? 
It is a tool to create server and client to easily connect

## 8. Files 

1.	Write code that uses `fseek`, `ftell`, `read` and `write` to copy the second half of the contents of a file to a `pipe`.
fseek(fd,0,SET_END)
offset = ftell(fd)
fseek(fd, offset/2, SET_SET)
while(read != 0) {
    write(buf, pipe[1])
}
2.	Write code that uses `open`, `fstat`, `mmap` to print in reverse the contents of a file to `stderr`.
int fd = open(...); //File is 2 Pages
char* addr = mmap(..fd..);

3.	Write brief code to create a symbolic link and hard link to the file /etc/password
symlink("/etc/password", "link");
ln -s file1.txt file2.txt
4.	"Creating a symlink in my home directory to the file /secret.txt succeeds but creating a hard link fails" Why? 

5.	Briefly explain permission bits (including sticky and setuid bits) for files and directories.
permission bits includes read write and execute permission for different users
setuid bits indicated that when run, the program will set the uid of the user to that of the owner of the file
Sticky bits were a bit that could be set on an executable file that would allow a program’s text segment to remain in swap even after the end of the program’s execution
6.	Write brief code to create a function that returns true (1) only if a path is a directory.
struct stat s;
if (0 == stat(name, &s)) {
printf("%s ", name);
if (S_ISDIR( s.st_mode)) puts("is a directory");
7.	Write brief code to recursive search user's home directory and sub-directories (use `getenv`) for a file named "xkcd-functional.png' If the file is found, print the full path to stdout.

8.	The file 'installmeplz' can't be run (it's owned by root and is not executable). Explain how to use sudo, chown and chmod shell commands, to change the ownership to you and ensure that it is executable.

## 9. File system 
Assume 10 direct blocks, a pointer to an indirect block, double-indirect, and triple indirect block, and block size 4KB.

1.	A file uses 10 direct blocks, a completely full indirect block and one double-indirect block. The latter has just one entry to a half-full indirect block. How many disk blocks does the file use, including its content, and all indirect, double-indirect blocks, but not the inode itself? A sketch would be useful.
4kb * (10 + 4kb + 4kb + 2kb)
2.	How many i-node reads are required to fetch the file access time at /var/log/dmesg ? Assume the inode of (/) is cached in memory. Would your answer change if the file was created as a symbolic link? Hard link?
3
symblolic might change but hard link would not
3.	What information is stored in an i-node?  What file system information is not? 
inode contains the data of a file
exclude filename
4.	Using a version of stat, write code to determine a file's size and return -1 if the file does not exist, return -2 if the file is a directory or -3 if it is a symbolic link.

5.	If an i-node based file uses 10 direct and n single-indirect blocks (1 <= n <= 1024), what is the smallest and largest that the file contents can be in bytes? You can leave your answer as an expression.

6.	When would `fstat(open(path,O_RDONLY),&s)` return different information in s than `lstat(path,&s)`?
lstat() is identical to stat(), except that if pathname is a symbolic link, then it returns information about the link itself, not the file that it refers to.
## 10. "I know the answer to one exam question because I helped write it"

Create a hard but fair 'spot the lie/mistake' multiple choice or short-answer question. Ideally, 50% can get it correct.
Given a symbloic link, write code to read the content from it
