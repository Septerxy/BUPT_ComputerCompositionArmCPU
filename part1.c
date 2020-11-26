#define _CRT_SECURE_NO_WARNINGS

# include <stdio.h>
# include <string.h>
# include <stdlib.h>
# include <stdbool.h>

# include "defs.h"
# include "print.h"
# include "part2.h"


word load_program(const char* exec_file) {  /* done */

    FILE* fp = fopen(exec_file, "rb");
    if(!fp) {
        printf("error: executable file \"%s\" does not exist\n", exec_file);
        exit(1);
    }

    /* move the exec_file to vm memory */
    word start_address = 0x20000;
    
    for( word address = start_address; !feof(fp) ; address += sizeof(word)) {
        fread(M + address, sizeof(word), 1, fp);
    }

    fclose(fp);
    
    return start_address;
}

int run_program(word start_address) { /* done */
    
    PC = start_address;
    SP = 0x01000;
    EA = 0;
    ER = 0;

    while(1) {  /* 指令周期 */

        print();
        input();

        EA = 0;             //开中断
        ER = 0;
        fetch_instruction();
        execute_instruction(); 
    }

    return 0;
}
