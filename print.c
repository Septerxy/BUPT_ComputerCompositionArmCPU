#define _CRT_SECURE_NO_WARNINGS

# include <stdio.h>
# include <string.h>
# include <stdint.h>
# include <stdbool.h>
# include <stdlib.h>

# include "defs.h"

void input() {

    printf("Press Enter to execute next instruction.\n");
    getchar();

}

static char* AS_CONDITION [] = {
    "EQ", "NE", "CS", "CC",
    "MI", "PL", "VS", "VC",
    "HI", "LS", "GE", "LT",
    "GT", "LE", "",   "NV"
};

static char* AS_OP3 [] = {
    "AND", "ORR",  "EOR",  "BIC",
    "ADD", "ADC",  "SUB",  "SBC",
    "MUL", "UDIV", "SDIV",
    "LSL", "LSR",  "ASR",  "ROR"
};

static char* AS_OP0 [] = {
    "LDRB", "STRB",
    "LDRH", "STRH",
    "LDR",  "STR",
    "NOP"
};

static char* AS_OP2 [] = {
    "MOV",  "MOVW", "MOVT", "MVN",
    "SXTB", "SXTH",
    "CMP",  "CMN",  "TST",  "TEQ"
};

static char* AS_OP1 [] = {
    "BL",  "B",
    "POP", "PUSH",
    "SWI"
};

static char** AS_OP [] = {
    AS_OP0, AS_OP1, AS_OP2, AS_OP3
};

static void disas_instruction(char as[], word ir) {

    int len = 0;
    
    byte cond   = ((ir >> 28) & 0xf);
    byte optype = ((ir >> 26) & 0x3);
    byte I      = ((ir >> 25) & 0x1);
    byte op     = ((ir >> 21) & 0xf);
    byte S      = ((ir >> 20) & 0x1);

    bool suffix_S = (S && (optype == 3));

    len = sprintf(as, "%s%s%s ", 
            AS_OP[optype][op],       /* ADD */
            AS_CONDITION[cond],      /* EQ  */
            (suffix_S ? "S" : "") ); /* S   */

    byte  Rd = ((ir >> 16) & 0xf);
    byte  Rn = ((ir >> 12) & 0xf);
    byte  Rm = (ir & 0xf);

    word imm_5  = (ir & 0x1f);
    word imm_12 = (ir & 0xfff);
    word imm_16 = (ir & 0xffff);
    word imm_20 = (ir & 0xfffff);

    switch(optype) {
        case 0: {
            if(op == NOP)       break;

            if(!I)  len += sprintf(as+len, "R%-2d, [R%-2d, R%-2d]", Rd, Rn, Rm);
            else    len += sprintf(as+len, "R%-2d, [R%-2d, #0x%x]", Rd, Rn, imm_12);
            break;
        }
        case 3: {
            if(!I)  len += sprintf(as+len, "R%-2d, R%-2d, R%-2d", Rd, Rn, Rm);
            else    len += sprintf(as+len, "R%-2d, R%-2d, #0x%x", Rd, Rn, imm_12);
            break;
        }
        case 2: {
            if(!I)  len += sprintf(as+len, "R%-2d, R%-2d", Rd, Rn);
            else    len += sprintf(as+len, "R%-2d, #0x%x", Rd, imm_16);
            break;
        }
        case 1: {
            if(!I)  len += sprintf(as+len, "R%-2d", Rd);
            else    len += sprintf(as+len, "#0x%x", imm_20);
            break;
        }
    }

}

void print() {

    char as[3][64];

    /* windows */
    system("cls");

    disas_instruction(as[0], *(word*)(M+PC-4));    /* the instruction just executed */
    disas_instruction(as[1], *(word*)(M+PC  ));    /* the instruction to be executed */
    disas_instruction(as[2], *(word*)(M+PC+4));

    /* show registers */
    printf("Registers\n");
    printf("R0 :0x%08x    R1 :0x%08x    R2 :0x%08x    R3 :0x%08x\n", R[0],  R[1],  R[2],  R[3]);
    printf("R4 :0x%08x    R5 :0x%08x    R6 :0x%08x    R7 :0x%08x\n", R[4],  R[5],  R[6],  R[7]);
    printf("R8 :0x%08x    R9 :0x%08x    R10:0x%08x    R11:0x%08x\n", R[8],  R[9],  R[10], R[11]);
    printf("R12:0x%08x    SP :0x%08x    LR :0x%08x    PC :0x%08x\n", R[12], SP ,   LR,    PC   );
    printf("CPSR: 0x%08x\n", CPSR);
    printf("\n");

    /* show memory recently visited */
    int32_t center_address = AR / 4 * 4;
    int32_t first_address = center_address - 32;

    printf("Memory\n");
    for(int i = 0; i < 4; ++i) {
        for(int j = 0; j < 4; ++j) {
            int32_t ad = first_address + (i*4 + j) * 4;
            if(ad > 0)
                printf("0x%05x: 0x%08x%s", ad, *(word*)(M + ad), ((j==3) ? "\n" : "    " ));
        }
    }
    printf("\n");

    /* show stack */
    printf("Stack\n");
    printf("      0x%05x 0x%08x\n", SP+12, *(word*)(M+SP+12));
    printf("      0x%05x 0x%08x\n", SP+8 , *(word*)(M+SP+8 ));
    printf("      0x%05x 0x%08x\n", SP+4 , *(word*)(M+SP+4 ));
    printf("SP -> 0x%05x 0x%08x\n", SP   , *(word*)(M+SP   ));
    printf("\n");

    /* show instructions */
    printf("Instructions\n");
    printf("      0x%05x 0x%08x %s\n", PC-4, *(word*)(M+PC-4), as[0]);
    printf("PC -> 0x%05x 0x%08x %s\n", PC,   *(word*)(M+PC  ), as[1]);
    printf("      0x%05x 0x%08x %s\n", PC+4, *(word*)(M+PC+4), as[2]);
    printf("\n");

}
