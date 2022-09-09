#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

//to split string using given separator
char **stringSplitter(char line[], char sep[])
{
    char **output;
    output = malloc(strlen(line) * sizeof(char *));
    char *token = strtok(line, sep);

    for (int i = 0; token != NULL; i++)
    {
        output[i] = malloc((strlen(token) + 1) * sizeof(char));
        strcpy(output[i], token);
        token = strtok(NULL, sep);
    }
    return output;
}

// simple exec without pipe
void simpleExec(char command[],int background)
{
    pid_t pid;
    int status;
    char **argu = stringSplitter(command," ");

    if ((pid = fork()) == 0) // child
        if (execvp(argu[0], argu) == -1)
            exit(55);
        else
            exit(0);
    else if(!background)  // parent - if not background then wait for child
    {
        wait(&status);
        if (WEXITSTATUS(status) == 55)
            printf("Command not found\n");
    }
}

// exec with pipe
void pipeExec(char line[], int pipeCount)
{
    pid_t pid;
    int status;
    int childCount = pipeCount + 1;
    int fp[childCount][2];

    for (int i = 0; i < childCount; i++)
        if (pipe(fp[i]) == -1)
            exit(1);

    char **pipedCmdLines;
    pipedCmdLines = stringSplitter(line, "|");

    for (int i = 0; i < childCount; i++)
    {
        char **arguCmd = stringSplitter(pipedCmdLines[i], " ");
        if ((pid = fork()) == 0)
        {
            for (int j = 0; j < childCount; j++)
            {
                if (j == i)
                    close(fp[j][1]);
                else if (j == (i + 1))
                    close(fp[j][0]);
                else
                {
                    close(fp[j][0]);
                    close(fp[j][1]);
                }
            }
            dup2(fp[i][0], 0);
            if (i != (childCount - 1))
                dup2(fp[i + 1][1], 1);
            if (execvp(arguCmd[0], arguCmd) == -1)
                exit(55);

            close(fp[i][0]);
            close(fp[i + 1][1]);
            exit(0);
        }
    }
    for (int j = 0; j < childCount; j++)
    {
        close(fp[j][0]);
        close(fp[j][1]);
    }

    while (childCount > 0)
    {
        pid = wait(&status);
        if (WEXITSTATUS(status) == 55)
            printf("Command not found\n");
        childCount--;
    }
}

// handling input redirection to take input from a file
void inputRedirectHandler(char command[], int pipeCount, int outputRedirectionCount, int background)
{
    char **argu=stringSplitter(command, "<");
    char *fileName=stringSplitter(argu[1]," ")[0];
    int in = open(fileName, O_RDONLY);
    
    int save_in = dup(fileno(stdin));
    if (-1 == dup2(in, fileno(stdin))) 
        perror("cannot redirect stdin"); 

    if(pipeCount>0)
        pipeExec(argu[0], pipeCount);
    else
        simpleExec(argu[0],background);

    fflush(stdin); close(in);
    dup2(save_in, fileno(stdin));
    close(save_in);
}

// handling output redirection to send output to a file
void outputRedirectHandler(char command[], int pipeCount, int inputRedirection, int outputRedirectionCount, int background)
{
    char **argu=stringSplitter(command, outputRedirectionCount==1 ? ">":">>");
    char *fileName=stringSplitter(argu[1]," ")[0];
    int out = outputRedirectionCount==1 ? open(fileName,O_CREAT|O_TRUNC|O_WRONLY,0666) : open(fileName,O_CREAT|O_APPEND|O_WRONLY, 0666);

    int save_out = dup(fileno(stdout));
    if (-1 == dup2(out, fileno(stdout))) 
        perror("cannot redirect stdout");

    if(inputRedirection!=0)
        inputRedirectHandler(argu[0], pipeCount, 0, background);
    else
    {
        if(pipeCount>0)
            pipeExec(argu[0], pipeCount);
        else
            simpleExec(argu[0],background);
    }
    
    fflush(stdout); close(out);
    dup2(save_out, fileno(stdout));
    close(save_out);
}

void commandHandler(char command[])
{
    if(command[strlen(command)-1]=='\n')
        command[strlen(command)-1]='\0';

    int i=0, inputRedirection=0, outputRedirectionCount=0, background=0, pipeCount=0;
    
    while(command[i]!='\0')    
        switch (command[i++])
        {
            case '&':
                background=1;
                break;
            case '>':
                outputRedirectionCount++;
                break;
            case '|':
                pipeCount++;
                break;
            case '<':
                inputRedirection=1;
                break;
        }

    if(outputRedirectionCount!=0)
        outputRedirectHandler(command, pipeCount, inputRedirection, outputRedirectionCount, background);
    else if(inputRedirection!=0)
        inputRedirectHandler(command, pipeCount, outputRedirectionCount, background);
    else if(pipeCount>0)
        pipeExec(command, pipeCount);
    else
        simpleExec(command,background); 
}

int main(int argc, char *argv[])
{
    char input[100];
    int i;
    while(1){
        printf("miniBash> ");
        fgets(input, sizeof(input),stdin);
 
        char **commands = stringSplitter(input, ";");
        i=0;
        while(commands[i]!=NULL)
            commandHandler(commands[i++]);                    
    }
}