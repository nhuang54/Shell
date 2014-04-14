//
//  main.c
//  CS410Shell
//
//  Created by Nilesh Khaitan on 4/1/14.
//  Copyright (c) 2014 Nilesh Khaitan. All rights reserved.
//


#ifndef Shell_parser_h
#define Shell_parser_h


void Parser(char buffer[], char* tokens[], int * index, int * again, int * in, int * out, int * inout, int * back, int * err_redir)
{
    //Makes sure the end of our buffer has NULL
    buffer[(strlen(buffer) - 1)] = 0;
    
    // Check if the command is exit
    if (strcmp("exit", buffer) == 0)
    {
        exit(EXIT_SUCCESS);
    }
    
    //Reset from previous buffer values
    *index = 0;
    *again = 0;
    *in = 0;
    *out = 0;
    *inout = 0;
    *back = 0;
    *err_redir = 0;
    
    //Writes pointer of token to our token array
    tokens[(*index)++] = strtok(buffer, " ");
    
    
    while((tokens[*index] = strtok(NULL, " ")))
    {
        switch(*tokens[*index])
        {
            case ';':
                *again = *index;
                tokens[*index] = NULL;
                break;

            case '<':
                *in = *index;
                tokens[*index] = NULL;
                break;
                
            case '>':
                *out = *index;
                tokens[*index] = NULL;
                break;

            case '1>':
                *out = *index;
                tokens[*index] = NULL;
                break;

            case '2>':
                *out = *index;
                tokens[*index] = NULL;
                break;
                
            case '&>':
                *out = *index;
                tokens[*index] = NULL;
                break;
                
            case '|':
                *inout = *index;
                tokens[*index] = NULL;
                break;
                
            case '&':
                *back = *index;
                tokens[*index] = NULL;
                break;
        }
        (*index)++;
    }
    int i = 0;
    for (i = 0; i < *index; i++)
    {
        printf("Tokens[%d] = %s\n",i,tokens[i]);
    }
}



#endif
