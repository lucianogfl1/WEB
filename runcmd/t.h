/* t1.h - Auxiliary program to test runcmd.

   Copyright (c) 2014, Francisco José Monaco <moanco@icmc.usp.br>

   This file is part of POSIXeg

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

#ifndef T_H
#define T_H

#include "testutils.h"

/* Arguments. */

# define T_SOMEVALUE 11	  /* Reserved to test return atoi(argv[1]). */
# define T_MAKEIO    12	  /* Reads from stdin and writes to stdout. */
# define T_WRITEFIFO 13	  /* Write to a named pipe (FIFO). */
# define T_CHECKFDS  14	  /* Check for extra descriptors.  */
# define T_SEGFAULT  15	  /* Cause program to segfault.    */


/* Exit status. */

# define T_NOARG      10    /* If no argument is given, return this value. */
# define T_IO          9    /* If can read from stdin, write this to stdout. */
# define T_NEVER       8    /* Never return this value. */
# define T_EXTRAFDS    7    /* Process has extra file descriptors. */

/* Tokens. */

# define T_TOKENSIZE 3
# define T_WRITETHIS "abc"	/* Write this token to stdout. */
# define T_READTHIS  "edf"	/* Read this token from stdin.*/

#define T_FIFONAME   "t1fifo"
#define T_TEMPFILE   "temp"	/* Generic file name. */

#endif	/* T_H */
