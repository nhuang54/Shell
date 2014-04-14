//
//  main.c
//  CS410Shell
//
//  Created by Nilesh Khaitan on 4/1/14.
//  Copyright (c) 2014 Nilesh Khaitan. All rights reserved.
//

#ifndef __Shell__execute__
#define __Shell__execute__


void execute(char* tokens[], int fd[], int fdin, int fdout, int again, int in, int out, int inout, int back, int err_redir, pid_t pid, int status)
{
    // For ;
    if(again)
    {
        
        if((pid = fork()) < 0)
        {
            perror("fork error");
            exit(1);
        }
        
        else if(pid)
        {
            // child process
            if(execvp(tokens[again + 1], &tokens[again + 1]))
            {
                perror("execvp failed");
                exit(1);
            }
        }
        
        // parent process
        waitpid(pid, &status, 0);
        if(execvp(*tokens,tokens))
        {
            perror("execvp failed");
            exit(1);
        }
        
    }
    
    
    //For <
    if(in)
    {
        //Open file
        fdin = open(tokens[in + 1], O_RDONLY);
        if(fdin == -1)
        {
            perror("open failed");
            exit(1);
        }
        
        //set it as standard input
        dup2(fdin, STDIN_FILENO);
        
        if(!out)
        {
            //execute
            if(execvp(*tokens, tokens))
            {
                perror("execvp failed");
                exit(1);
            }
        }
    }
    
    // For >
    if(out)
    {
        if(-1 == (fdout = open(tokens[out +1], O_WRONLY | O_CREAT, 0600)))
        {
            perror("open failed");
            exit(1);
        }
        
        //set it as standard output
        dup2(fdout, STDOUT_FILENO); // 1
        
        if(execvp(*tokens,tokens))
        {
            perror("execvp failed");
            exit(1);
        }
        
    }
    
    // For 2>
    if(err_redir)
    {
        if(-1 == (fdout = open(tokens[out +1], O_WRONLY | O_CREAT, 0600)))
        {
            perror("open failed");
            exit(1);
        }
        
        //set it as
        dup2(fdout, 2);   //2
        if(execvp(*tokens,tokens))
        {
            perror("execvp failed");
            exit(1);
        }
        
    }
    
    // For |
    if(inout)
    {
        if(-1 == pipe(fd))
        {
            perror("pipe error");
            exit(1);
        }
        
        if((pid = fork()) < 0)
        {
            perror("fork error");
            exit(1);
        }
        
        else if(pid)
        {
            //make stdout the pipe
            if(-1 == dup2(fd[1], STDOUT_FILENO))
            {
                perror("dup2 error");
                exit(1);
            }
            
            close(fd[0]);
            if(execvp(*tokens,tokens))
            {
                perror("execvp failed");
                exit(1);
            }
        }
        
        else
        {
            if(-1 == dup2(fd[0], STDIN_FILENO))
            {
                perror("dup2 error");
                exit(1);
            }
            
            close(fd[1]);
            if(execvp(tokens[inout + 1], &tokens[inout + 1]))
            {
                perror("execvp failed");
                exit(1);
            }
        }
    }
    
    if(!in && !out && !inout)
    {
        if(execvp(*tokens, tokens))
        {
            perror("execvp failed");
            exit(1);
        }
    }
    
    // Not background job -> wait.
    // Else if it is a background job -> dont wait
    else if(pid && !back)
    {
        waitpid(pid, &status, 0);
    }
    
    free(tokens);
    
}
#endif /* defined(__Shell__execute__) */
