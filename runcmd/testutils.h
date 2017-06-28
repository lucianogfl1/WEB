/* testutils.h - Program to test runcmd function

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

#ifndef TESTUTILS_H
#define TESTUTILS_H

extern int noio[3]; /* Index 0,1,2 will be file descriptors to /dev/null. */

/* This wrapper function calls 

   pid = run runcmd(command, result, io) ;

   and returns pid. If io is NULL, standard streams are redirected to
   /dev/null before calling runcmd(), in order not to polute the console
   output; streams are restored before returning. This function also 
   outputs some basic information about the subprocess completion. */

int try_runcmd (const char* command, int *result, int *io);

/* Returns 0 if check condition is met; returns 1 otherwise. 
   The function outputs OK or NO, respectively. */

int check (const char* message, int condition);

/* Stringify a macro's integer value. */

#define _strfy(val) # val
#define  strfy(val) _strfy(val)

# define TIMEOUT 1

/* Test case names. */

extern char *test_set;
extern int test_count;

#endif	/* TESTUTILS_H */
