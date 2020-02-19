#include <stdio.h> /* standard I/O routines header */
#include <stdlib.h> /* handles the exit function */
#include <unistd.h> /* header to access POSIX OS API */
#include <sys/wait.h> /* handles the wait function */

int main()
{
    int g_pid; /* grandparent pid */
    if (fork() == 0) /* fork and check for child (parent) */
    {
        g_pid = getppid(); /* set g_pid to originating pid */
        if (fork() == 0) /* fork again and check for child */
        {
            printf("I am the child process C and my pid is %d. My parent P has pid %d. My grandparent has pid %d.\n", getpid(), getppid(), g_pid);
        }
        else /* if second fork() is non-zero i.e. parent */
        {
            wait(NULL); /* allow time to execute child process */
            printf("I am the parent process P and my pid is %d. My parent G has pid %d.\n", getpid(), g_pid);
        }
    }
    else /* if first fork() is non-zero i.e. grandparent */
    {
        wait(NULL); /* allow time to execute parent process */
        printf("I am the grandparent process G and my pid is %d.\n", getpid());
    }
    exit(0);        
}
    
