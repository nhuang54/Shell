//
//  main.c
//  CS410Shell
//
//  Created by Nilesh Khaitan on 4/1/14.
//  Copyright (c) 2014 Nilesh Khaitan. All rights reserved.
//


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include "parser.h"
#include "execute.h"

#define BUF_LIM 120

//Used to show current directory as shell prompt
static char *currentDirectory;

void shellPrompt()
{
    printf("myshell>");
}

typedef void (*sighandler_t)(int);

char c = '\0';

//Used for exiting on ctrl + c
void handle_signal(int signo)
{
    printf(" Please use ctrl + Z instead to exit");
    fflush(stdout);
}

int main(int argc, char **argv, char **envp)
{
    //Buffer to hold our input commands
    char buffer[BUF_LIM];
    
    //Index for cycling through buffer
    int index;
    
    //Stores pointers to token strings
    char *tokens[BUF_LIM];
    
    //Process ID
    pid_t pid;
    
    //Status of process
    int status;
    
    //Used for deciding file descriptors
    int again, in, out, inout, back, err_redir;
    
    //File descriptors
    int fd[2], fdin, fdout;
    
    char returnChar;
    
    //Will display the entire current working directory as a prompt
    shellPrompt();
    
    //used to exit by pressing ctrl+c
    signal(SIGINT, SIG_IGN);
    signal(SIGINT, handle_signal);

    //Get buffer from user and loop the shell
    while(fgets(buffer,BUF_LIM,stdin) != NULL)
    {
        //used to exit by pressing ctrl+c
        signal(SIGINT, SIG_IGN);
        signal(SIGINT, handle_signal);

        //If user hit return, reprint shell prompt and get user input again
        switch(returnChar = buffer[0])
        {
                
            case '\n':
                shellPrompt();
                break;
            default:
                //Parse our buffer command
                Parser(buffer, tokens, &index, &again, &in, &out, &inout, &back, &err_redir);
                
                //If
                if((in && inout) || (out && inout))
                {
                    fprintf(stderr, "Not implemented\n");
                    continue;
                }
                
                //Call fork
                pid = fork();
                
                //Fork doesn't work
                if(pid < 0)
                {
                    perror("fork error");
                    exit(1);
                }
                
                //child process
                else if(!pid)
                {
                    //Execute the command
                    execute(tokens, fd, fdin, fdout, again, in, out, inout, back, err_redir, pid, status);
                    bzero(buffer, BUF_LIM);
                }
                waitpid(pid, &status, 0);
                shellPrompt();
                break;
        }
    }
    return 0;
}