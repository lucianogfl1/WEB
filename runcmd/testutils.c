/* testutils.c - Program to test runcmd function

   Copyright (c) 2014, Francisco José Monaco <moanco@icmc.usp.br>

   This file is part of POSIXeg

   POSIXeg is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUTfile descriptor buffer ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
    
*/

#include <stdio.h>
#include <unistd.h>
#include <runcmd.h>

int noio[3]; 

char *test_set = "Test";
int test_count = 1;

/* This a runcmd() wrapper. */ 

int try_runcmd (const char* command, int *result, int *io)
{
  int pid, tio[3], tmp_result, nbytes;

  printf ("\nCommand %s%n", command, &nbytes);
  printf ("%*s", 20-nbytes, " ");

  /* Supress output if not redirecting. */
  if (!io)
    {
      tio[0]=dup(0); tio[1]=dup(1); tio[2]=dup(2);
      close (0); close(1); close (2);
      dup(noio[0]); dup(noio[1]); dup(noio[2]);
    }

  /* Run the command.*/

  pid = runcmd (command,  &tmp_result, io);

  /* Restore output if not redirecting. */
  if (!io)
    {
      close (0); close (1); close (2);
      dup(tio[0]); dup(tio[1]); dup(tio[2]);
      close (tio[0]); close(tio[1]); close (tio[2]);
    }

  /* Output basic information on subprocess completion. */

  printf ("(%5d, %8s, %3d, %s, exec %s)\n",
  	  pid,
  	  IS_NORMTERM(tmp_result) ? "normal" : "abnormal",
  	  EXITSTATUS(tmp_result),
  	  IS_NONBLOCK(tmp_result) ? "nonblock" : "block",
  	  IS_EXECOK(tmp_result) ? "yes" : "no ");

  if (result)
    *result = tmp_result;

  return pid;

}

/* Returns 0 if check condition is met; returns 1 otherwise. 
   The function outputs OK or NO, respectively. */

int check (const char* message, int condition)
{
  int nbytes;
  printf ("   %s %03d whether %s%n", test_set, test_count++, message, &nbytes);
  printf ("%*s\n", 70-nbytes, condition ? "OK":"NO");
  return condition ? 0 : 1;
}
