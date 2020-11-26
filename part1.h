# ifndef __PART1_H__
# define __PART1_H__

# include "defs.h"

word load_program (const char*);    /* return the start_address */
int  run_program  (word);           /* return 0 if no error or error code */

# endif