/* runcmd.c - Execute a command as a subprocess.

   Copyright (c) 2014, Francisco José Monaco <moanco@icmc.usp.br>
   This file is part of POSIXeg

   This file has been modified by
	Guilherme Rodrigues

	Ilgner Lino Vieira
	ilgnerlv@gmail.com

	Lucas Françozo Bataglia		
	lukao350@gmail.com

	Luciano Gabriel Francisco 
	lucianogfl1@gmail.com

    as part of "Shellgas" project.

   POSIXeg is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <string.h>

#include "runcmd.h"
#include "debug.h"
#define MAX_PROCESS (1 << 15)

/* Executes 'command' in a subprocess. Information on the subprocess execution
   is stored in 'result' after its completion, and can be inspected with the
   aid of macros made available for this purpose. Argument 'io' is a pointer
   to an integer vector where the first, second and third positions store
   file descriptors to where standard input, output and error, respective,
   shall be redirected; if NULL, no redirection is performed. On
   success, returns subprocess' pid; on error, returns 0. */
static int processBack[MAX_PROCESS];

void child_end(int sig, siginfo_t *info, void *ucontext);
int runcmd (const char *command, int *result, const int *io) /* ToDO: const char* */
{
  int pid, status,aux, i, tmp_result, pipefd[2], isBackground=0, tamanhoPalavra;
  char *args[RCMD_MAXARGS], *cmd, ultimoCaracter;
  struct sigaction act;
  void *temp;

  tmp_result = 0;

  /* Parse arguments to obtain an argv vector. */

  cmd = malloc ((strlen (command)+1) * sizeof(char));
  sysfail (!cmd, -1);
  strcpy (cmd, command);

  tamanhoPalavra= strlen(cmd);

  ultimoCaracter = cmd[tamanhoPalavra-1];



  if(ultimoCaracter=='&'){
    isBackground=1;
  }


  i=0;
  args[i++] = strtok (cmd, RCMD_DELIM);
  while ((i<RCMD_MAXARGS) && (args[i++] = strtok (NULL, RCMD_DELIM)));
  i--;


  temp = memset(&act, 0, sizeof(struct sigaction));
  sysfatal (!temp);

  act.sa_flags |= SA_SIGINFO;
  act.sa_sigaction = child_end;
  sysfail(sigaction(SIGCHLD, &act, NULL)< 0, -1);
  sysfail(pipe(pipefd) < 0, -1);
  /* Create a subprocess. */

  pid = fork();
  sysfail (pid<0, -1);

  /* Caller process (parent). */
  if (pid>0)
    {


      if(isBackground){
        if(result!=NULL){
            tmp_result= NONBLOCK;
        }
        processBack[pid]=1;
      }else{
        close(pipefd[1]);
        aux = wait(&status);
        sysfail(aux<0, -1);


            if(WIFEXITED(status)){
                tmp_result |= NORMTERM;
                tmp_result |= WEXITSTATUS(status);

                if(read(pipefd[0], NULL, 1) == 0){
                    tmp_result |= EXECOK;
                }
            }

      }


 }


  /* Subprocess (child) */
  else
    {
      if(io!=NULL){


	    int i=0;
        for(i=0; i<3; i++){
            close(i);
            dup(io[i]);
            close(io[i]);
        }
      }

      close(pipefd[0]);
      aux=0;
      aux = execvp(args[0], args);

      write(pipefd[1], &aux, 4);
      exit(EXECFAILSTATUS);

    }

    if(result){
        *result = tmp_result;
    }


  free (cmd);
  return pid;			/* Only parent reaches this point. */
}

/* If function runcmd is called in non-blocking mode, then function
   pointed by rcmd_onexit is asynchronously evoked upon the subprocess
   termination. If this variable points to NULL, no action is performed.
*/


void child_end(int sig, siginfo_t *info, void *ucontext){
   int status=0;
   pid_t pid = info->si_pid;
   waitpid(pid,&status,0);
   if(processBack[pid]){
        processBack[pid]=0;
        if(runcmd_onexit){
            runcmd_onexit();
        }
   }
}


void (*runcmd_onexit)(void) = NULL;