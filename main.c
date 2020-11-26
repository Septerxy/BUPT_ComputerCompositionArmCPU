#define _CRT_SECURE_NO_WARNINGS

# include <stdio.h>
# include <string.h>
# include <stdlib.h>

# include "defs.h"
# include "part1.h"



int main(int argc, char ** argv) {  /* done */


    int exit_code;

    if(argc == 1 || argc == 2) {
        const char* exec_file;
        word start_address;

        exec_file = (argc == 1 ? "example" : argv[1]);
        start_address = load_program(exec_file);
        exit_code = run_program(start_address);
    }
    else {
        printf("error: multiple input files not supported\n");
        exit(1);
    }

    return exit_code;
}
