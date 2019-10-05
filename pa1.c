/**********************************************************************
 * Copyright (c) 2019
 *  Sang-Hoon Kim <sanghoonkim@ajou.ac.kr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTIABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 **********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>
#include <signal.h>

#include "types.h"
#include "parser.h"

/*====================================================================*/
/*          ****** DO NOT MODIFY ANYTHING FROM THIS LINE ******       */
/**
 * String used as the prompt (see @main()). You may change this to
 * change the prompt */
static char __prompt[MAX_TOKEN_LEN] = "$";

/**
 * Time out value. It's OK to read this value, but ** DO NOT CHANGE
 * DIRECTLY **. Instead, use @set_timeout() function below.
 */
static unsigned int __timeout = 2;

static void set_timeout(unsigned int timeout)
{
	__timeout = timeout;

	if (__timeout == 0) {
		fprintf(stderr, "Timeout is disabled\n");
	} else {
		fprintf(stderr, "Timeout is set to %d second%s\n",
				__timeout,
				__timeout >= 2 ? "s" : "");
	}
}
/*          ****** DO NOT MODIFY ANYTHING UP TO THIS LINE ******      */
/*====================================================================*/
int change_dir(int nr_tokens, char *tokens[])
{
        char buf[255];
 //       printf("token : %s\n", tokens[1]);
        if(strncmp(tokens[1], "~", sizeof("~")) == 0)
            chdir(getenv("HOME"));
        else
            if(chdir(tokens[1]))
            {
//                printf("Error");
                _exit(0);
                return 0;
            }

//        getcwd(buf, 255);
//        printf("now : %s\n", buf);

        return 1;
       
}

void signal_handler(int signal_number){
    printf("%d signaled!\n", signal_number);
    alarm(__timeout);
}

struct sigaction sa = {
    .sa_handler = signal_handler,
    .sa_flags = 0,
}, old_sa;


/***********************************************************************
 * run_command()
 *
 * DESCRIPTION
 *   Implement the specified shell features here using the parsed
 *   command tokens.
 *
 * RETURN VALUE
 *   Return 1 on successful command execution
 *   Return 0 when user inputs "exit"
 *   Return <0 on error
 */
static int run_command(int nr_tokens, char *tokens[])
{
	/* This function is all yours. Good luck! */
    int sum;
    int * status;
    int pid;
   int j = 0;
   sigaction(SIGALRM, &sa, 0);
//    alarm(__timeout);

	if (strncmp(tokens[0], "exit", strlen("exit")) == 0) {
		return 0;
	}
    else if(!strncmp(tokens[0], "for", strlen("for")))
    {
        int i = 2;
        int f = 1;
        int cd = 0;
        sum = atoi(tokens[1]);
    
        while(tokens[i] != NULL)
        {
            if(strncmp(tokens[i], "cd", strlen("cd")) == 0)
            {
                cd = 1;
                f = i;
            }
            if(strncmp(tokens[i], "for", strlen("for")) == 0)
            {
              //  printf("i : %d, nr : %d\n", i, nr_tokens);
                if(i>=nr_tokens-1)
                {
                    printf("break");
                   break;
                }
                sum *= atoi(tokens[++i]);
                f = i;
           }
            i++;
        }
           
        for(int i = 0; i<sum; i++)
        {  
            if(cd == 1)
                change_dir(nr_tokens, &tokens[f]);
            else
            {
                if((pid = fork())==0)
                {  
                    execvp(tokens[f+1], &tokens[f+1]);
                    _exit(0);
                }
                if(pid>0)
                {
                     wait(0);
                }     
            }
         }
    }
    else if(!strncmp(tokens[0], "cd", strlen("cd")))
    {
        change_dir(nr_tokens, tokens);

    }
    else if(!strncmp(tokens[0], "prompt", strlen("prompt")))
    {
        
        memset(&__prompt, '\0',MAX_TOKEN_LEN);
        strncpy(&__prompt[j], tokens[1], strlen(tokens[1]));
    
        printf("%s\n", __prompt);

    }
    else if(!strncmp(tokens[0], "timeout", strlen("timeout")))
    {
        if(nr_tokens == 1)
            printf("Current timeout is %d second\n", __timeout);
        else
            set_timeout(atoi(tokens[1]));
        

        pid = fork();
    
       if(pid == 0)
       {
           printf("id : %d, time : %d\n", pid, __timeout);
            sigaction(SIGALRM, &sa, 0);
           alarm(__timeout);

            _exit(0);
       }
       else
       {
           wait(0);
       }
    }
    else
    {
        if((pid = fork()) == 0)
        {
            if(execvp(tokens[0], tokens)<0)
            {
                fprintf(stderr, "No such file or directory\n");
               // close(fd1);
                _exit(0);
            }
        }
        else
        {
            wait( 0);
        }
    }

	return 1;
}


/***********************************************************************
 * initialize()
 *
 * DESCRIPTION
 *   Call-back function for your own initialization code. It is OK to
 *   leave blank if you don't need any initialization.
 *
 * RETURN VALUE
 *   Return 0 on successful initialization.
 *   Return other value on error, which leads the program to exit.
 */
static int initialize(int argc, char * const argv[])
{
	return 0;
}


/***********************************************************************
 * finalize()
 *
 * DESCRIPTION
 *   Callback function for finalizing your code. Like @initialize(),
 *   you may leave this function blank.
 */
static void finalize(int argc, char * const argv[])
{

}


/*====================================================================*/
/*          ****** DO NOT MODIFY ANYTHING BELOW THIS LINE ******      */

static bool __verbose = true;
static char *__color_start = "[0;31;40m";
static char *__color_end = "[0m";

/***********************************************************************
 * main() of this program.
 */
int main(int argc, char * const argv[])
{
	char command[MAX_COMMAND_LEN] = { '\0' };
	int ret = 0;
	int opt;

	while ((opt = getopt(argc, argv, "qm")) != -1) {
		switch (opt) {
		case 'q':
			__verbose = false;
			break;
		case 'm':
			__color_start = __color_end = "\0";
			break;
		}
	}

	if ((ret = initialize(argc, argv))) return EXIT_FAILURE;

	if (__verbose)
		fprintf(stderr, "%s%s%s ", __color_start, __prompt, __color_end);

	while (fgets(command, sizeof(command), stdin)) {	
		char *tokens[MAX_NR_TOKENS] = { NULL };
		int nr_tokens = 0;

		if (parse_command(command, &nr_tokens, tokens) == 0)
			goto more; /* You may use nested if-than-else, however .. */

		ret = run_command(nr_tokens, tokens);
		if (ret == 0) {
			break;
		} else if (ret < 0) {
			fprintf(stderr, "Error in run_command: %d\n", ret);
		}

more:
		if (__verbose)
			fprintf(stderr, "%s%s%s ", __color_start, __prompt, __color_end);
	}

	finalize(argc, argv);

	return EXIT_SUCCESS;
}
