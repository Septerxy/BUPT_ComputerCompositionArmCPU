# ifndef __DEFS_H__
# define __DEFS_H__

# include <stdint.h>

/* type definition */

typedef uint32_t word;
typedef uint16_t hword;
typedef uint8_t  byte;

/* registers and memory definition */

# define SP R[13]                /* Stack Pointer */
# define LR R[14]                /* Link Register */
# define PC R[15]                /* Program Counter */

# define MEMORY_SIZE (1<<20)     /* 1MB */

extern word R[16];               /* general purpose registers */
extern word CPSR;                /* Current Program Status Register */
extern word IR;                  /* Instruction Register */
extern word AR;
extern word DR;

extern word EA;                  /* Interruption Allowed */
extern word ER;                  /* Interruption Response */
extern word QUERY[5];

extern byte M[MEMORY_SIZE];      /* main memory */
                                 /* 0x00000 ~ 0x0ffff belongs to operating system 
                                  * 0x10000 ~ 0xfffff belongs to users */


//#define M[100] = 0x0100 00F0     /* push ac */
/* mov ac, 0 */
/* disp 中断了，成功 */
/* pop ac */
/* ei = 0 */
/* pop pc */


/* opcode and cond for decoding machine code */

enum CONDITION {     /* arm cond */
    EQ, NE, CS, CC,  /* 0000 ~ 0011 */
    MI, PL, VS, VC,  /* 0100 ~ 0111 */
    HI, LS, GE, LT,  /* 1000 ~ 1011 */
    GT, LE, AL, NV   /* 1100 ~ 1111 */
};

enum OP3 {    /* triple operand instructions */
    AND, ORR,  EOR,  BIC,
    ADD, ADC,  SUB,  SBC,
    MUL, UDIV, SDIV,
    LSL, LSR,  ASR,  ROR
};

enum OP0 {    /* zero and some triple operand instructions */
    LDRB, STRB,
    LDRH, STRH,
    LDR,  STR,
    NOP
};

enum OP2 {    /* single operand instructions */
    MOV,  MOVW, MOVT, MVN,
    SXTB, SXTH,
    CMP,  CMN,  TST,  TEQ
};

enum OP1 {    /* double operand instructions */
    BL,  B,
    POP, PUSH,
    SWI
};

# endif
