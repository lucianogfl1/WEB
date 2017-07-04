/*Copyright (c) 2017,

	Guilherme Rodrigues 	     guilhermerodrigues10@gmail.com
	Ilgner Lino Vieira           ilgner_lv@hotmail.com
	Lucas Françoso Bataglia      lukao350@gmail.com
 	Luciano Gabriel Francisco    lucianofl1@gmail.com
 
   This file is part of Shellgas project.
    
   Shellgas is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

   ================================================================================
   Shellgas source code may be obtained from (https://github.com/SO-II-2017/shellgas/)*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#include "foosh.h"
#include "debug.h"

#define MAX_SIZE 1024
#define PROMPT "shellgas$ "

/* Structs */

struct command
{
    char **argv;
};

typedef struct process_t
{
    pipeline_t process;
    int pid;
    int status; 		/* (0) running; (1) stopped/not running; */
    int foreground; 		/* (0) background; (1) foreground; (-1) if it is not running */
    char *processName;
} process_t;

/* Function's prototypes */

int internal_commands(pipeline_t *internalPipeline); /*run all internal commands*/
void execute_commands(pipeline_t *pipeline);
int external_commands(pipeline_t *currentCommand);
int external_pipes_commands(pipeline_t *currentCommand);
void delFunc (void *func); /*function to delete nodes*/
void childHandler(int sig); /*common child handler*/
void childHandlerCtrlC(int sig);
void childHandlerCtrlZ(int sig);
void childHandlerEnd(int sig);
int spawn_proc(int in, int out, struct command *cmd); /*functions to spawn processes [pipes]*/
int fork_pipes(int n, struct command *cmd, pipeline_t *currentPipeline);


typedef void (*sighandler_t) (int);

int go_on = 1;			/* This variable controls the main loop. */
list_t *jobsList;

int main (int argc, char **argv)
{
    buffer_t *command_line;
    int aux;

    pipeline_t *pipeline;
    jobsList = new_list(delFunc);

    command_line = new_command_line(); 		/* Read a Line */

    pipeline = new_pipeline();

    /* This is the main loop */
    while (go_on)
    {
        char diretorio[3000];

    printf("%s", PROMPT);
    
    if (getcwd(diretorio, sizeof(diretorio)) != NULL) 
         
        printf("/%s$ ", diretorio); 
        fflush(stdout);
        fflush(stdin);

        aux = read_command_line(command_line);

        if (aux < 0)
	{
            getchar();
        }
        else
	{
            /* Parse (see the tparse.h file) */
            if (!parse_command_line(command_line, pipeline) || 1)
	    {
                execute_commands(pipeline); 		/* Execute commands in a pipeline */
            }
        }
    }

    release_command_line (command_line);
    release_pipeline (pipeline);

    return EXIT_SUCCESS;
}


/* Execute Commands Function */
/* This function checks and separates the arguments and commands to send*/

void execute_commands(pipeline_t *pipeline)
{
    int returning;
	// if there is more than one command on the command line
    if (pipeline->ncommands > 1)
    {
        /* It is necessary to start execution of commands behind forward, that is, the last command to the end */
        // Call the external command function
        external_pipes_commands(pipeline);
    }
    else if (pipeline->ncommands == 0)
	{
	         /* If there is no any command, that is, only one was enter , nothing to do */
	}
    else
    {
        //if there is a single command on the command line
        if ((returning = internal_commands(pipeline)) == 1)
	{
            returning = external_commands(pipeline);
        }
    }
}

/* External Pipes Commands Function */

int external_pipes_commands(pipeline_t *currentCommand) {

    pid_t pid;
    int status;

    struct command *cmd = malloc(sizeof(struct command) * currentCommand->ncommands);

    /*command line array*/
    int j;
    for (j = 0; j < currentCommand->ncommands; j++) {
        char **command_line = malloc(sizeof(char) * currentCommand->narguments[j] * 10);
        /*organize command line array*/
        int i;
		//put each argument in a vector, or makes a parse all arguments
        for (i = 0; i < currentCommand->narguments[j]; i++) {
            command_line[i] = currentCommand->command[j][i];
        }
        
        /*last null*/
        command_line[currentCommand->narguments[0]] = NULL;
        
        cmd[j].argv = command_line;
    }
    
    pid = fork ();
    fatal (pid < 0, "Fork failed...");
    
    if (pid == 0) {
        fork_pipes(currentCommand->ncommands, cmd, currentCommand);
    }
    else {
        wait(&status);
    }
    
    return 0;
}

/* Spawn Process [Pipes] Function 
 function that loads and executes a new child process. 
 The current process may wait for the child to terminate or may continue to execute asynchronously. 
 Creating a new subprocess requires enough memory in which both the child process and the current program can execute.
*/

int spawn_proc(int in, int out, struct command *cmd)
{
    pid_t pid;
    
    if ((pid = fork()) == 0)
    {
        if (in != 0)
        {
            dup2 (in, 0);
            close (in);
        }
        
        if (out != 1)
        {
            dup2 (out, 1);
            close (out);
        }
        return execvp(cmd->argv [0], (char * const *)cmd->argv);
    }
    return pid;
}

/* Fork Pipes Function */
/* This function taking all the arguments and separate commands of external commands
* function and treat each of them according to the type of command it is (background, foreground, redirect stdout, in etc.)*/

int fork_pipes (int n, struct command *cmd, pipeline_t *currentPipeline)
{
    int i;
    int in;
    int fd [2];
    int fdopen;
    
    /* The first process should get its input from the original file descriptor 0.  */
    in = 0;
    
    for (i = 0; i < n - 1; ++i)
    {
        pipe (fd);
        
        /* f [1] is the write end of the pipe*/
        spawn_proc (in, fd [1], cmd + i);
        
        /*the child will write here.  */
        close (fd [1]);
        
        /*the next child will read from there.  */
        in = fd [0];
    }
    
    /* Last stage of the pipeline*/
    if (in != 0)
        dup2 (in, 0);
    
    if (REDIRECT_STDOUT(currentPipeline)) 
    {
        fdopen = open(currentPipeline->file_out, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        
        dup2(fdopen, STDOUT_FILENO);   /* make stdout go to file*/
        dup2(fdopen, STDERR_FILENO); 
        close(fdopen);
        
    }
    else if (REDIRECT_STDIN(currentPipeline)) 
    {
        fdopen = open(currentPipeline->file_in, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        
        dup2(fdopen, STDIN_FILENO);   /* make stdin go to file */
        dup2(fdopen, STDERR_FILENO);  
        close(fdopen);
    }

    if (RUN_BACKGROUND(currentPipeline)) 
    {
        int pid;
        pid = fork();
        
        if (pid == 0) 
        {
            execvp (cmd[i].argv [0], (char * const *)cmd [i].argv); /*running in child*/
        }
        else 
        {
            list_node_t *node;
            process_t *process;
            char processName[100];
            
            node = append_node(jobsList);
            
            process = malloc(sizeof(process_t));
            process->pid = pid;
            process->process = *currentPipeline;
            
            process->processName = malloc(sizeof(char) * strlen(*cmd->argv));
            
            memcpy(processName, *cmd->argv, sizeof(cmd->argv));
            processName[99] = '\0';
            
            strcpy(process->processName, processName);
            
            process->status = 0;
            process->foreground = 0;
            
            node->value = (void *)process;
            
            printf("[%d] %d\n", jobsList->size, process->pid);
        }
        
        return pid;
    }
    else 
    {
        return execvp (cmd[i].argv [0], (char * const *)cmd [i].argv);
    }

}

/* Internal Commands Function */
/* This function makes the signal processing such as killing processes zombies, running in foreground and background, job control, etc.)
, ie, internal commands from a shell*/
/*
*   Internal commands are something which is built into the shell. For the shell built in commands, the execution speed is really high. 
    It is because no process needs to be spawned for executing it.
    For example, when using the "cd" command, no process is created. The current directory simply gets changed on executing it.
	fonte: http://www.theunixschool.com/2012/03/internal-vs-external-commands.html
*/

int internal_commands(pipeline_t *internalPipeline) {
    /*sanity test*/
    if (internalPipeline->narguments[0] > 0) {
        
        /*check for each feature*/
        if (strcmp(internalPipeline->command[0][0], "exit") == 0) {
            go_on = 0;
            kill(getppid(), SIGTERM);
            return 0;
        }
        else if (strcmp(internalPipeline->command[0][0], "fg") == 0) {
            if (internalPipeline->narguments[0] > 1) {
                
                list_node_t *node = jobsList->first;
                
                if (node != NULL) {
                    int i = 0;
                    for (i = 0; i < jobsList->size; i++) {
                        
                        process_t process = *((process_t*)node->value);
                        
                        if (strcmp(process.processName, internalPipeline->command[0][1]) == 0) {
                            printf("%s\n", process.processName);
                            
                            process_t processAfter;
                            list_node_t *newNode;
                            int status;

                            process_t *currentProcess = malloc(sizeof(process_t)); /*setting as the last!*/
                            currentProcess->pid = process.pid;
                            currentProcess->process = process.process;
                            currentProcess->processName = process.processName;
                            currentProcess->status = 0;
                            currentProcess->foreground = 1;
                            
                            del_node(jobsList, node);
                            
                            newNode = append_node(jobsList);
                            newNode->value = (void *)currentProcess;
                            
                            kill(process.pid, SIGCONT);
                            
                            waitpid(process.pid, &status, WUNTRACED);
                            
                            processAfter = *((process_t*)jobsList->last->value);
                            
                            if (WIFEXITED(status) == 1 && processAfter.status == 0) {
                                del_node(jobsList, jobsList->last);  /*may not be the last one!*/
                            }

                            i = jobsList->size;
                        }
                        else {
                            node = node->next;
                        }
                    }
                }
            }
            else {
                if (jobsList->last != NULL) {
                    process_t *processAfter;
                    
                    process_t *process = ((process_t*)jobsList->last->value);
                    process->status = 0;
                    process->foreground = 1;
                    
                    printf("%s\n", process->processName);

                    kill(process->pid, SIGCONT);
                    
                    int status;
                    
                    pid_t pid = process->pid;
                    waitpid(pid, &status, WUNTRACED);
                    
                    if (jobsList->size > 0) {
                        processAfter = ((process_t*)jobsList->last->value);
                        
                        /* status == 0 and foreground == 0 can be stopped on background, so we can't remove from jobs list*/
                        if (WIFEXITED(status) == 1 && processAfter->status == 0 && processAfter->foreground == 1) {
                            del_node(jobsList, jobsList->last);  /*may not be the last one!*/
                        }
                    }

                }
            }
            
            return 0;
        }
        else if (strcmp(internalPipeline->command[0][0], "bg") == 0) {
            if (internalPipeline->narguments[0] > 1) {
                list_node_t *node = jobsList->first;
                
                if (node != NULL) {
                    int i;
                    for (i = 0; i < jobsList->size; i++) {
                        process_t *process = ((process_t*)node->value);
                        
                        /*continuing the process but not waiting (bg)*/
                        if (strcmp(process->processName, internalPipeline->command[0][1]) == 0) {
                            process->status = 0;
                            process->foreground = 0;
                            
                            kill(process->pid, SIGCONT);
                            
                            signal(SIGCHLD, childHandler);
                            
                            printf("[%d] %d\n", jobsList->size, process->pid);
                        }
                        
                        node = node->next;
                    }
                }
            }
            else {
                if (jobsList->last != NULL) {
                    process_t *process = ((process_t*)jobsList->last->value);
                    process->status = 0;
                    process->foreground = 0;
                    
                    kill(process->pid, SIGCONT);
                    
                    signal(SIGCHLD, childHandler);
                }
            }
            
            return 0;
        }
        else if (strcmp(internalPipeline->command[0][0], "jobs") == 0) {
            if (jobsList->size > 0) {
                list_node_t *node = jobsList->first;
                
                int i;
                i = 1;
                
                while (node != NULL) {
                    process_t process = *((process_t*)node->value);
                    
                    char *sign;
                    sign = "+";
                    
                    if (i == jobsList->size - 1) {
                        sign = "-";
                    }
                    else if (i != jobsList->size) {
                        sign = " ";
                    }
                    
                    printf("\n[%d]%s %s %d \t %s", i, sign, (process.status == 0) ? "Running":"Stopped", process.pid, process.processName);
                    
                    node = node->next;
                    i++;
                }
                
                printf("\n");
            }
            
            
            return 0;

        }
        else if (strcmp(internalPipeline->command[0][0], "cd") == 0) {
            /*argument line array*/
            int result;
	    result = chdir(internalPipeline->command[0][1]);
            
            if (result == 0) {
                return 0;
            }
            else {
                return -1;
            }
        }
        
        
        return 1;
    }
    else {
        return -1; /*error code*/
    }
}


/* External Commands Function */
/*
	 External commands are not built into the shell. 
	 These are executables present in a separate file. 
	 When an external command has to be executed, a new process has to be spawned and the command gets executed.
	 For example, when you execute the "cat" command, which usually is at /usr/bin, the executable /usr/bin/cat gets executed.
	 fonte: http://www.theunixschool.com/2012/03/internal-vs-external-commands.html
*/
int external_commands(pipeline_t *currentCommand) {
    int returning;
    returning = -1;
    
    /*sanity test*/
    if (currentCommand->narguments[0] > 0) {
        int i;
        pid_t pid;
        int status;

        /*command line array*/
        char **command_line;
        command_line = malloc(sizeof(char) * currentCommand->narguments[0] * 100);
	
        /*organize command line array*/
       
        for (i = 0; i < currentCommand->narguments[0]; i++) {
            command_line[i] = currentCommand->command[0][i];
        }
        
        /*last null*/
        command_line[currentCommand->narguments[0]] = NULL;
        
        /*create fork*/
       
        pid = fork ();
        fatal (pid < 0, "Fork failed...");
        
        if (pid > 0) {
            list_node_t *node = append_node(jobsList);
            
            struct sigaction actC;
            struct sigaction actZ;
            struct sigaction act;

            void *temp;
            void *temp2;
            void *temp3;

            process_t *process;

            temp = memset(&actC, 0, sizeof(struct sigaction));
            fatal (!temp, "memset failed");
            actC.sa_handler = childHandlerCtrlC;
            sigaction(SIGINT, &actC, NULL);
            
            temp2 = memset(&actZ, 0, sizeof(struct sigaction));
            fatal (!temp2, "memset failed");
            actZ.sa_handler = childHandlerCtrlZ;
            sigaction(SIGTSTP, &actZ, NULL);
            
            temp3 = memset(&act, 0, sizeof(struct sigaction));
            fatal (!temp3, "memset failed");
            act.sa_handler = childHandlerEnd;
            sigaction(SIGTERM, &act, NULL);
            
            struct sigaction actBG;
            void *tempBG;
            tempBG = memset(&actBG, 0, sizeof(struct sigaction));

            process = malloc(sizeof(process_t));
            process->pid = pid;
            process->process = *currentCommand;
            
            process->processName = malloc(sizeof(char) * strlen(command_line[0]));            
            strcpy(process->processName, command_line[0]);

            process->status = 0;
            
            node->value = (void *)process;
            
            if (RUN_FOREGROUND(currentCommand)) {
                process->foreground = 1;
                
                setpgid(pid, pid);
                
                waitpid(pid, &status, WUNTRACED);
                
                /*status == 0 and foreground == 0 can be stopped on background*/
                if (WEXITSTATUS(status) == 0 && process->status == 0 && process->foreground == 1) {
                    del_node(jobsList, jobsList->last);
                }
            }
            else {
                process->foreground = 0;
                
                if (process->status == 0 && process->foreground == 0) {
                    printf("[%d] %d\n", jobsList->size, pid);
                }
                
                fatal (!tempBG, "memset failed");
                actBG.sa_handler = childHandler;
                sigaction(SIGCHLD, &actBG, NULL);
            }
        }
        else {
            int returningCode;
            setpgid(getpid(), pid);

            if (REDIRECT_STDOUT(currentCommand)) {
                int fd;
                fd = open(currentCommand->file_out, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
                
                dup2(fd, STDOUT_FILENO);   /* make stdout go to file*/
                dup2(fd, STDERR_FILENO);   
                
                close(fd);
            }
            else if (REDIRECT_STDIN(currentCommand)) {
                int fd;
                fd = open(currentCommand->file_in, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
                
                dup2(fd, STDIN_FILENO);   /* make stdout go to file*/
                dup2(fd, STDERR_FILENO);  
                
                close(fd);
            }

            if ((returningCode = execvp(command_line[0], command_line)) < 0) {
                kill(getpid(), SIGSYS);
            }
        }
    }
    
    return returning;
}

/* this function is the case where the user press ctrl-c*/
void childHandlerCtrlC(int sig) {
    printf("ctrlc %s\n", ((process_t*)jobsList->last->value)->processName);
    process_t *processValue = ((process_t*)jobsList->last->value);
    
    if (processValue != NULL) {
        kill(processValue->pid, SIGINT);
    }
}

/* this function is the case where the user closes the child process*/
void childHandlerEnd(int sig) {
    printf("end %d (%d) %s", __LINE__, sig, ((process_t*)jobsList->last->value)->processName);
    
    process_t *processValue = ((process_t*)jobsList->last->value);
    
    if (processValue != NULL) {
        kill(processValue->pid, SIGTERM);
        
        del_node(jobsList, jobsList->last);
        printf("end %d", __LINE__);
    }
}
/* this function is the case where the user press ctrl-z*/
void childHandlerCtrlZ(int sig) {
    process_t *processValue = ((process_t*)jobsList->last->value);
    char *sign;

    processValue->status = 1;
    processValue->foreground = 1; /*set as foreground because we don't know what the user may want*/
    
    kill(processValue->pid, SIGTSTP);
    
    sign = "+";
    printf("\n[%d]%s\t Stopped \t %s\n", jobsList->size, sign, processValue->processName);
}

void childHandler(int sig) {
    char *sign;
    int i;
    int sanity = -1;
    list_node_t *node;
    process_t *process = NULL;

    pid_t pid;
    pid = wait(NULL);
    
    node = jobsList->first;    
    
    
    /*loop to get the job in the jobs list*/
    for (i = 0; i < jobsList->size; i++) {
        process = ((process_t*)node->value);
        
        if (process->pid == pid) {
            sanity = i;
            i = jobsList->size;
        }
        else {
            node = node->next;
        }
    }

    
    /*prepares the printf and deletenode if needed*/
    if (process->foreground == 0) {
        sign = "+";
        
        if (sanity == jobsList->size - 2) {
            sign = "-";
        }
        else if (sanity != jobsList->size - 1) {
            sign = " ";
        }
        
        if (WEXITSTATUS(sig) == 0) {
            ((process_t*)node->value)->status = -1;
            printf("[%d]%s\t Done \t %s\n", sanity+1, sign, process->processName);
            del_node(jobsList, node);

        }
        else {
            printf("[%d]%s\t Stopped \t %s\n", sanity+1, sign, process->processName);
        }
    }
}

/* Delete Node Function */

void delFunc (void *func)
{
    free (func);
}
