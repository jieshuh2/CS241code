# Welcome to Homework 0!

For these questions you'll need the mini course and virtual machine (Linux-In-TheBrowser) at -

http://cs-education.github.io/sys/

Let's take a look at some C code (with apologies to a well known song)-
```C
// An array to hold the following bytes. "q" will hold the address of where those bytes are.
// The [] mean set aside some space and copy these bytes into teh array array
char q[] = "Do you wanna build a C99 program?";

// This will be fun if our code has the word 'or' in later...
#define or "go debugging with gdb?"

// sizeof is not the same as strlen. You need to know how to use these correctly, including why you probably want strlen+1

static unsigned int i = sizeof(or) != strlen(or);

// Reading backwards, ptr is a pointer to a character. (It holds the address of the first byte of that string constant)
char* ptr = "lathe"; 

// Print something out
size_t come = fprintf(stdout,"%s door", ptr+2);

// Challenge: Why is the value of away zero?
int away = ! (int) * "";


// Some system programming - ask for some virtual memory

int* shared = mmap(NULL, sizeof(int*), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
munmap(shared,sizeof(int*));

// Now clone our process and run other programs
if(!fork()) { execlp("man","man","-3","ftell", (char*)0); perror("failed"); }
if(!fork()) { execlp("make","make", "snowman", (char*)0); execlp("make","make", (char*)0)); }

// Let's get out of it?
exit(0);
```

## So you want to master System Programming? And get a better grade than B?
```C
int main(int argc, char** argv) {
	puts("Great! We have plenty of useful resources for you, but it's up to you to");
	puts(" be an active learner and learn how to solve problems and debug code.");
	puts("Bring your near-completed answers the problems below");
	puts(" to the first lab to show that you've been working on this.");
	printf("A few \"don't knows\" or \"unsure\" is fine for lab 1.\n"); 
	puts("Warning: you and your peers will work hard in this class.");
	puts("This is not CS225; you will be pushed much harder to");
	puts(" work things out on your own.");
	fprintf(stdout,"This homework is a stepping stone to all future assignments.\n");
	char p[] = "So, you will want to clear up any confusions or misconceptions.\n";
	write(1, p, strlen(p) );
	char buffer[1024];
	sprintf(buffer,"For grading purposes, this homework 0 will be graded as part of your lab %d work.\n", 1);
	write(1, buffer, strlen(buffer));
	printf("Press Return to continue\n");
	read(0, buffer, sizeof(buffer));
	return 0;
}
```
## Watch the videos and write up your answers to the following questions

**Important!**

The virtual machine-in-your-browser and the videos you need for HW0 are here:

http://cs-education.github.io/sys/

The coursebook:

http://cs241.cs.illinois.edu/coursebook/index.html

Questions? Comments? Use Piazza:
https://piazza.com/illinois/fall2020/cs241

The in-browser virtual machine runs entirely in Javascript and is fastest in Chrome. Note the VM and any code you write is reset when you reload the page, *so copy your code to a separate document.* The post-video challenges (e.g. Haiku poem) are not part of homework 0 but you learn the most by doing (rather than just passively watching) - so we suggest you have some fun with each end-of-video challenge.

HW0 questions are below. Copy your answers into a text document because you'll need to submit them later in the course. More information will be in the first lab.

## Chapter 1

In which our intrepid hero battles standard out, standard error, file descriptors and writing to files.

### Hello, World! (system call style)
1. Write a program that uses `write()` to print out "Hi! My name is `<Your Name>`".
	int main() {
	write (1, "Hi! My name is Jessica", 22);
	return 0;
}
### Hello, Standard Error Stream!
2. Write a function to print out a triangle of height `n` to standard error.
   - Your function should have the signature `void write_triangle(int n)` and should use `write()`.
   - The triangle should look like this, for n = 3:
   ```C
   *
   **
   ***
   ```
   void write_triangle(int n) {
	    int i;
	    int j;
		for (i = 1; i < n + 1; i++) {
			for (j = 0; j < i; j++) {
				write(1, "*", 1);
			}
			write(1, "\n", 1);
		}
   }
### Writing to files
3. Take your program from "Hello, World!" modify it write to a file called `hello_world.txt`.
   - Make sure to to use correct flags and a correct mode for `open()` (`man 2 open` is your friend).
	int main() {
		int flide = open("hello_word.txt", O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);
		write(flide, "Hello World\n", 13);
		close(flide);
		return 0;
	}
### Not everything is a system call
4. Take your program from "Writing to files" and replace `write()` with `printf()`.
   - Make sure to print to the file instead of standard out!
   int main() {
	close(1);
	int flide = open("hello_word.txt", O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);
	printf("Hello World\n");
	close(flide);
	return 0;
}
5. What are some differences between `write()` and `printf()`?
  prinf() is more useful when formattng. 
  write() is more useful for decide which file to write to.
## Chapter 2

Sizing up C types and their limits, `int` and `char` arrays, and incrementing pointers

### Not all bytes are 8 bits?
1. How many bits are there in a byte?
	8
2. How many bytes are there in a `char`?
	1
3. How many bytes the following are on your machine?
   - `int`, `double`, `float`, `long`, and `long long`
   4/8/4/8/16
### Follow the int pointer
4. On a machine with 8 byte integers:
```C
int main(){
    int data[8];
} 
```
If the address of data is `0x7fbd9d40`, then what is the address of `data+2`?
0x7fbd9d48
5. What is `data[3]` equivalent to in C?
   - Hint: what does C convert `data[3]` to before dereferencing the address?
data[0] + 3
### `sizeof` character arrays, incrementing pointers
  
Remember, the type of a string constant `"abc"` is an array.

6. Why does this segfault?
```C
char *ptr = "hello";
*ptr = 'J';
```
ptr is not writable. 
7. What does `sizeof("Hello\0World")` return?
12
8. What does `strlen("Hello\0World")` return?
5
9. Give an example of X such that `sizeof(X)` is 3.
"HE"
10. Give an example of Y such that `sizeof(Y)` might be 4 or 8 depending on the machine.
pointer

## Chapter 3

Program arguments, environment variables, and working with character arrays (strings)

### Program arguments, `argc`, `argv`
1. What are two ways to find the length of `argv`?
argc, sizeof(argv)/sizeof(argv[0])
2. What does `argv[0]` represent?
The first input usually program name.
### Environment Variables
3. Where are the pointers to environment variables stored (on the stack, the heap, somewhere else)?
There is a part in memory specially used for store environment variable
### String searching (strings are just char arrays)
4. On a machine where pointers are 8 bytes, and with the following code:
```C
char *ptr = "Hello";
char array[] = "Hello";
```
What are the values of `sizeof(ptr)` and `sizeof(array)`? Why?
### Lifetime of automatic variables
4 (32bit)/8(64bit) it is a pointer
5 (any array of character)
5. What data structure manages the lifetime of automatic variables?
function
## Chapter 4

Heap and stack memory, and working with structs

### Memory allocation using `malloc`, the heap, and time
1. If I want to use data after the lifetime of the function it was created in ends, where should I put it? How do I put it there?
you can put it onto heap use malloc.
2. What are the differences between heap and stack memory?
stack is usually where variable are store. heap is a memory that you can declare to store variable that can use out of scope.  
3. Are there other kinds of memory in a process?
yes
4. Fill in the blank: "In a good C program, for every malloc, there is a _free__".
### Heap allocation gotchas
5. What is one reason `malloc` can fail?
not enough space to malloc.
6. What are some differences between `time()` and `ctime()`?
time return the hours since January 1, 1970;
ctime can return the exact time base on the result of time();
7. What is wrong with this code snippet?
```C
free(ptr);
free(ptr);
```
double free. 
8. What is wrong with this code snippet?
```C
free(ptr);
printf("%s\n", ptr);
```
trying to access the memory which are already released.
9. How can one avoid the previous two mistakes? 
after free(ptr), always set ptr to NULL;
### `struct`, `typedef`s, and a linked list
10. Create a `struct` that represents a `Person`. Then make a `typedef`, so that `struct Person` can be replaced with a single word. A person should contain the following information: their name (a string), their age (an integer), and a list of their friends (stored as a pointer to an array of pointers to `Person`s).
#include <stdio.h>
struct Person{
	char* name;
	int age;
	struct Person *(*friends)[5];
};
typedef struct Person person_t;
int main() {
	person_t * person1 = (person_t *) malloc(sizeof(person_t));
	person_t * person2 = (person_t *) malloc(sizeof(person_t));
	person1->name = "Agent Smith";
	person2->name = "Sonny Moore";
	person1->age = 128;
	person2->age = 256;
	(*person1->friends)[0] = person2;
	(*person2->friends)[0] = person1;
	return 0;
}
11. Now, make two persons on the heap, "Agent Smith" and "Sonny Moore", who are 128 and 256 years old respectively and are friends with each other.
above
### Duplicating strings, memory allocation and deallocation of structures
Create functions to create and destroy a Person (Person's and their names should live on the heap).
12. `create()` should take a name and age. The name should be copied onto the heap. Use malloc to reserve sufficient memory for everyone having up to ten friends. Be sure initialize all fields (why?).
person_t* create(char* name, int age){
	person_t* person = (person_t *)malloc(sizeof(person_t));
	person->name = strup(name);
	person->age = age;
	return person;
}
13. `destroy()` should free up not only the memory of the person struct, but also free all of its attributes that are stored on the heap. Destroying one person should not destroy any others.
void destroy(person_t* ptr) {
	free(ptr->name);
	memset(ptr, 0, sizeof(person_t));
	free(ptr);
}
## Chapter 5 

Text input and output and parsing using `getchar`, `gets`, and `getline`.

### Reading characters, trouble with gets
1. What functions can be used for getting characters from `stdin` and writing them to `stdout`?
gest(), puts()
2. Name one issue with `gets()`.
Don't know if input is too long or not.
### Introducing `sscanf` and friends
3. Write code that parses the string "Hello 5 World" and initializes 3 variables to "Hello", 5, and "World".
int main() {
	char* input= "Hello 5 World";
	char hello[10];
	char world[10];
	int number;
	sscanf(input, "%s %d %s", hello, &number, world);
	printf("%s", hello);
	return 0;
}
### `getline` is useful
4. What does one need to define before including `getline()`?
#define _GNU_SOURCE
5. Write a C program to print out the content of a file line-by-line using `getline()`.
int main() {
	File* flide = *fopen("hello_word.txt", r);
	char* buffer= NULL;
	size_t capacity;
	ssize_t result = 1;
	while (result > 0) {
		result = getline(&buffer, &capacity, flide);
		puts(buffer);
	}
	free(buffer);
	close(flide);
	return 0;
}

## C Development

These are general tips for compiling and developing using a compiler and git. Some web searches will be useful here

1. What compiler flag is used to generate a debug build?
-g
2. You modify the Makefile to generate debug builds and type `make` again. Explain why this is insufficient to generate a new build.
change flag
3. Are tabs or spaces used to indent the commands after the rule in a Makefile?
NO
4. What does `git commit` do? What's a `sha` in the context of git?
git commit kind store you change in the local area. Sha is something like a key.
5. What does `git log` show you?
git log command displays a record of the commits in a Git repository
6. What does `git status` tell you and how would the contents of `.gitignore` change its output?
Git status command is used in Git to know the status of the working tree.
7. What does `git push` do? Why is it not just sufficient to commit with `git commit -m 'fixed all bugs' `?
You should push the local chanage to the remote repository.
8. What does a non-fast-forward error `git push` reject mean? What is the most common way of dealing with this?
the remote is changed and there is a difference with yours. Maybe git pull or fetch first before push.

## Optional (Just for fun)
- Convert your a song lyrics into System Programming and C code and share on Piazza.
- Find, in your opinion, the best and worst C code on the web and post the link to Piazza.
- Write a short C program with a deliberate subtle C bug and post it on Piazza to see if others can spot your bug.
- Do you have any cool/disastrous system programming bugs you've heard about? Feel free to share with your peers and the course staff on piazza.
