/*
 *  This is a simple shell program from
 *  rik0.altervista.org/snippetss/csimpleshell.html
 *  It's been modified a bit and comments were added.
 *
 *  It doesn't allow misdirection, e.g., <, >, >>, or |
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#define BUFFER_SIZE 80
#define ARR_SIZE    80
#define PIPE_MAX    9
#define CHILD_MAX   (PIPE_MAX + 1)

// #define DEBUG 1  /* In case you want debug messages */

typedef struct child{
    char *args[ARR_SIZE]; //command to execute
    int fd[2]; //pair of pipes per child
}Child;

void parse_args(char *buffer, char** args, 
                size_t args_size, size_t *nargs)
{
    char *buf_args[args_size]; /* You need C99.  Note that args_size
                                  is normally a constant. */
    char **cp;  /* This is used as a pointer into the string array */
    char *wbuf;  /* String variable that has the command line */
    size_t i, j; 
    
    wbuf=buffer;
    buf_args[0]=buffer; 
    args[0] =buffer;
    
    /* cp is a pointer to buff_args */ 
    for(cp=buf_args; (*cp=strsep(&wbuf, " \n\t")) != NULL ;){
        if ((*cp != '\0') && (++cp >= &buf_args[args_size]))
            break; 
    }

/* 
 * Copy 'buf_args' into 'args'
 */    
    for (j=i=0; buf_args[i]!=NULL; i++){ 
        if(strlen(buf_args[i])>0)  /* Store only non-empty tokens */
            args[j++]=buf_args[i];
    }
    
    *nargs=j;
    args[j]=NULL;
}


int main(int argc, char *argv[], char *envp[]){
    
    //added varaiables
    int temp; //outer for-loop variable
    int pipe_count; //pipe counter
    int ps_count;
    Child child[CHILD_MAX]; //child information

    //child information
    int child_id = -1;

    char buffer[BUFFER_SIZE];
    char *args[ARR_SIZE];

    int *ret_status;
    size_t nargs;
    pid_t pid[PIPE_MAX + 1];

    for(temp = 0; temp < PIPE_MAX; temp++)
    {
    	pipe(child[temp].fd);
    }
    
    while(1){
        printf("ee468>> "); /* Prompt */
        fgets(buffer, BUFFER_SIZE, stdin); /* Read in command line */
        /* Parse the command line into args */
        parse_args(buffer, args, ARR_SIZE, &nargs); 
 
        if (nargs==0) continue; /* Nothing entered so prompt again */
        if (!strcmp(args[0], "exit" )) exit(0);
	
	//initialize pipe counter
	pipe_count = 0;

	//initialize postion counter
	ps_count = 0;

	//test for pipe symbol in argument array
	for(temp = 0; temp < nargs; temp++)
	{
	    child[pipe_count].args[ps_count] = args[temp];

	    //look for pipe symbols in the argument array
	    if(!strcmp(args[temp], "|"))
	    {
	    	child[pipe_count].args[ps_count] = NULL;
		//keep track of the number of pipes found
		pipe_count++;
		ps_count = 0;
	    }
	    else
	    {
	    	ps_count++;
	    }
	}

	child[pipe_count].args[ps_count + 1] = NULL;

	//check for overflow for pipes
	if(pipe_count > PIPE_MAX)
	{
		printf("error: too many pipes\n");
		continue;
	}

	//form the correct number of sub-processes needed
	for(temp = 0; temp < pipe_count + 1; temp++)
	{
		//create child process
		pid[temp] = fork();

		//child process
		if(pid[temp] == 0)
		{
			//give child an unique id
			child_id = temp;

			//leave loop to prevent echo forks
			break;
		}
	}

	

        if(child_id == -1) /* The parent */
	{
	    //wait for every child to exit
	    for(temp = 0; temp < pipe_count + 1; temp++)
	    {
	    	wait() > 0; //ret_statues
	    }
        } 

        else /* The child executing the command */
	{
	    //set up pipe
	    if(pipe_count > 0)
	    {
	    	for(temp = 0; temp < pipe_count; temp++)
		{
			if(child_id == temp)
			{
				close(1);
				close(2);
				dup2(child[temp].fd[1], 1);
				dup2(child[temp].fd[1], 2);
				close(child[temp].fd[0]);
			}
		}
		for(temp = 1; temp < pipe_count + 1; temp++)
		{
			if(child_id == temp)
			{
				close(0);
				dup2(child[temp - 1].fd[0], 0);
				close(child[temp - 1].fd[1]);
			}
		}
	    }

	    //do stuff
	    if(execvp(child[child_id].args[0], child[child_id].args))
	    {
                puts(strerror(errno));
                exit(127);
            }
        }

	printf("I am here! I am process %d\n", child_id);

    }    
    return 0;
}


