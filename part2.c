#define _CRT_SECURE_NO_WARNINGS

# include <stdio.h>
# include <string.h>
# include <stdlib.h>
# include <windows.h>
# include <stdbool.h>

# include "defs.h"

#define MSB(x) ((x >> 31) & 0x1)

/* registers and memory definition */

word R[16];                     /* general purpose registers */
word CPSR;                      /* Current Program Status Register */

word IR;                        /* Instruction Register */
word AR;
word DR;
byte M[MEMORY_SIZE];            /* main memory */
word EA;                  /* Interruption Allowed */
word ER;                  /* Interruption Response */
word QUERY[5] = { 0x2000, 0x3000, 0x300a, 0x5000, 0x6000 };

/* buffer for CPSR */

static byte N, Z, C, V;

void fetch_instruction() {   /* done */
    AR = PC;
    IR = DR = *(word*)(M + AR);
    PC += 4;
}

/* 是否满足指令执行的条件 */
static bool condition_passed() {  /* done */

    bool ans;
    byte cond       = ((IR >> 28) & 0xf);    /*  4 bit */

    /* 状态寄存器译码 */
    byte N = ((CPSR >> 31) & 0x1);
    byte Z = ((CPSR >> 30) & 0x1);
    byte C = ((CPSR >> 29) & 0x1);
    byte V = ((CPSR >> 28) & 0x1);
    
    /* 判断执行条件 */
    switch(cond) {
        case EQ: ans = (Z == 1); break;
        case NE: ans = (Z == 0); break;
        case CS: ans = (C == 1); break;
        case CC: ans = (C == 0); break;
        case MI: ans = (N == 1); break;
        case PL: ans = (N == 0); break;
        case VS: ans = (V == 1); break;
        case VC: ans = (V == 0); break;
        case HI: ans = ((C == 1) && (Z == 0)); break;
        case LS: ans = ((C == 0) || (Z == 1)); break;
        case GE: ans = (N == V); break;
        case LT: ans = (N != V); break;
        case GT: ans = ((N == V) && (Z == 0)); break;
        case LE: ans = ((N != V) || (Z == 1)); break;
        case AL: ans = true;  break;
        case NV: ans = false; break;
    }

    return ans;
}

static void execute_instruction3() {
    /* decode instruction */
    byte I          = ((IR >> 25) & 0x1);    /*  1 bit */
    byte op         = ((IR >> 21) & 0xf);    /*  4 bit */
    byte Rd         = ((IR >> 16) & 0xf);    /*  4 bit */
    byte Rn         = ((IR >> 12) & 0xf);    /*  4 bit */
    hword shifter_operand = (IR & 0xfff);    /* 12 bit */
    
    /* for and, add */
    word first_operand  = R[Rn];
    word second_operand = (I ? shifter_operand : R[shifter_operand & 0xf]);    /* imm_12 or register */

    /* for lsl */
    byte shift = shifter_operand & 0x1f;

    /* execute instruction */
    switch(op) {
        case AND: {
            R[Rd] = first_operand & second_operand;    
            N = MSB(R[Rd]);
            Z = !R[Rd];
            C = 0;
            /* V unchanged */
            break;
        }
        case ORR: {
            R[Rd] = first_operand | second_operand; 
            N = MSB(R[Rd]);
            Z = !R[Rd];
            C = 0;
            /* V unchanged */
            break;
        }
        case EOR: {
            R[Rd] = first_operand ^ second_operand; 
            N = MSB(R[Rd]);
            Z = !R[Rd];
            C = 0;
            /* V unchanged */
            break;
        }
        case BIC: {
            R[Rd] = first_operand & (~ second_operand); 
            N = MSB(R[Rd]);
            Z = !R[Rd];
            C = 0;
            /* V unchanged */
            break;
        }

        case ADD: {
            R[Rd] = first_operand + second_operand;
            N = MSB(R[Rd]);
            Z = !R[Rd];
            C = !(((uint64_t)first_operand + (uint64_t)second_operand)==(uint64_t)(R[Rd]));
            V = !(((int64_t)first_operand + (int64_t)second_operand)==((int64_t)R[Rd]));
            break;
        }
        case ADC: {
            R[Rd] = first_operand + second_operand + C;
            N = MSB(R[Rd]);
            Z = !R[Rd];
            C = !(((uint64_t)first_operand + (uint64_t)second_operand + (uint64_t)C)==(uint64_t)(R[Rd]));
            V = !(((int64_t)first_operand + (int64_t)second_operand + (uint64_t)C)==((int64_t)R[Rd]));
            break;
        }
        case SUB: {
            R[Rd] = first_operand - second_operand; 
            N = MSB(R[Rd]);
            Z = !R[Rd];
            C = !(((uint64_t)first_operand - (uint64_t)second_operand)==(uint64_t)(R[Rd]));
            V = !(((int64_t)first_operand - (int64_t)second_operand)==((int64_t)R[Rd]));
            break;
        }
        case SBC: {
            R[Rd] = first_operand - second_operand + C - 1; 
            N = MSB(R[Rd]);
            Z = !R[Rd];
            C = !(((uint64_t)first_operand - (uint64_t)second_operand + (uint64_t)C - 1)==(uint64_t)(R[Rd]));
            V = !(((int64_t)first_operand - (int64_t)second_operand + (uint64_t)C - 1)==((int64_t)R[Rd]));
            break;
        }
        case MUL: {
            R[Rd] = first_operand * second_operand; 
            N = MSB(R[Rd]);
            Z = !R[Rd];
            /* C V unchanged */
            break;
        }
        case UDIV: {
            if( second_operand == 0x0)
            {
                fprintf(stderr, "error: divisor cannot be zero\n");
                R[Rd] = 0;
            }
            else
                R[Rd] = first_operand / second_operand; 
            /* N Z C V unchanged */
            break;
        }
        case SDIV: {
            if( second_operand == 0x0)
            {
                fprintf(stderr, "error: divisor cannot be zero\n");
                R[Rd] = 0;
            }
            else
                R[Rd] = (int32_t)(first_operand) / (int32_t)(second_operand);
            /* N Z C V unchanged */
            break;
        }

        case LSL: {
            R[Rd] = first_operand << shift; 
            N = MSB(R[Rd]);
            Z = !R[Rd];
            C = MSB(first_operand << (shift - 1));    /* Last digit removed */
            /* V unchanged */
            break;
        }
        case LSR: {
            R[Rd] = first_operand >> shift;        
            N = MSB(R[Rd]);
            Z = !R[Rd];
            C = (first_operand >> (shift - 1)) & 0x1;    /* Last digit removed */
            /* V unchanged */
            break;
        }
        case ASR: {
            R[Rd] = ((int32_t)(first_operand)) >> shift;
            N = MSB(R[Rd]);
            Z = !R[Rd];
            C = (first_operand >> (shift - 1)) & 0x1;    /* Last digit removed */
            /* V unchanged */
            break;
        }
        case ROR: {
            R[Rd] = (first_operand >> shift) | (first_operand << (32 - shift)); 
            N = MSB(R[Rd]);
            Z = !R[Rd];
            C = ((first_operand >> (shift - 1)) | (first_operand << (32 - (shift - 1)))) & 0x1;    /* Last digit removed */
            /* V unchanged */
            break;
        }

        default: exit(2); break;

    }
}

static void execute_instruction0() {
    /* decode instruction */
    byte I          = ((IR >> 25) & 0x1);    /*  1 bit */
    byte op         = ((IR >> 21) & 0xf);    /*  4 bit */
    byte Rd         = ((IR >> 16) & 0xf);    /*  4 bit */
    byte Rn         = ((IR >> 12) & 0xf);    /*  4 bit */
    hword shifter_operand = (IR & 0xfff);    /* 12 bit */
    
    /* for ldr */
    word base  = R[Rn];
    word offset = (I ? shifter_operand : R[shifter_operand & 0xf]);    /* imm_12 or register */

    switch(op) {
        case LDRB: {
            AR = base + offset;
            R[Rd] = DR = *(byte*)(M + AR);
            break;
        }
        case STRB: {
            AR = base + offset;
            *(byte*)(M + AR) = DR = (byte)R[Rd];
            break;
        }
        case LDRH: {
            AR = base + offset;
            R[Rd] = DR = *(hword*)(M + AR);
            break;
        }
        case STRH: {
            AR = base + offset;
            *(hword*)(M + AR) = DR = (hword)R[Rd];
            break;
        }
        case LDR: {
            AR = base + offset;
            R[Rd] = DR = *(word*)(M + AR);
            break;
        }
        case STR: {
            AR = base + offset;
            *(word*)(M + AR) = DR = (word)R[Rd];
            break;
        }

        case NOP: break;

        default: exit(2); break;
    }

}

static void execute_instruction2() {

    /* decode instruction */
    byte I          = ((IR >> 25) & 0x1);    /*  1 bit */
    byte op         = ((IR >> 21) & 0xf);    /*  4 bit */
    byte Rd         = ((IR >> 16) & 0xf);    /*  4 bit */
    
    /* for mov and sxt */
    word src_operand = ( I ? (IR & 0xffff) : R[IR & 0xf]);    /* imm_16 or register */

    /* for cmp */
    word result;
    word first_operand  = R[Rd];
    word second_operand = src_operand;

    switch(op) {
        case MOV:  /* the same as movw */
        case MOVW: R[Rd]  = src_operand;           break;
        case MOVT: R[Rd] |= (src_operand << 0x10); break;
        case MVN:  R[Rd] = ~src_operand;           break;

        case SXTB: R[Rd] = (int32_t)(int8_t )src_operand; break;
        case SXTH: R[Rd] = (int32_t)(int16_t)src_operand; break;

        case CMP: {
            result = first_operand - second_operand;
            N = MSB(result);
            Z = !result;
            C = (first_operand >= second_operand);
            V = (~(MSB(result) ^ MSB(second_operand))) & (MSB(first_operand) ^ MSB(second_operand));
            break;
        }
        case CMN: {
            result = first_operand + second_operand;
            N = MSB(result);
            Z = !result;
            C = (result < first_operand) || (result < second_operand);
            V = (MSB(result) ^ MSB(first_operand)) & (MSB(result) ^ MSB(second_operand));
            break;
        }
        case TST: {
            result = first_operand & second_operand;
            N = MSB(result);
            Z = !result;
            C = 0;
            /* V unchanged */
            break;
        }
        case TEQ: {
            result = first_operand ^ second_operand;
            N = MSB(result);
            Z = !result;
            C = 0;
            /* V unchanged */
            break;
        }
        
        
        default: exit(2); break;
    }
}

static void execute_instruction1() {

    /* decode instruction */
    byte op = ((IR >> 21) & 0xf);    /*  4 bit */
    byte Rd = ((IR >> 16) & 0xf);    /*  4 bit */

    /* for b */
    word imm_20 = (IR & 0xfffff);    /* 20 bit */
    int32_t offset = ((imm_20 & 0x80000) ? (imm_20 | 0xfff00000) : imm_20 ); /* sign extension */

    switch(op) {
        case BL: LR = PC;    /* no break; */ /* pc +=4 when fetching instruction */
        case B:  PC = PC - 4 + offset; break;    /* offset is relative to current instruction */

        case POP: {
            AR = SP;
            DR = *(word*)(M + AR);
            R[Rd] = DR;
            SP = SP + 4;
            break;
        }            
        case PUSH: {
            SP = SP - 4;
            AR = SP;
            DR = R[Rd];
            *(word*)(M + AR) = (word)DR;
            break;
        }  
        case SWI: {
            *(word*)(M + 0x300a) = (word)0xe46c0000;//push R12;保护现场
            *(word*)(M + 0x300e) = (word)0xea0c0000;//mov R12, #0；是设备服务功能，假设让ac=0
                                    //disp 中断了，成功；模型机机器指令disp
            *(word*)(M + 0x3012) = (word)0xe44c0000;//pop R12；恢复现场
            //printf("准备写入pop pc\n");
            *(word*)(M + 0x3016) = (word)0xe44f0000;//pop pc；回复现场，返回原程序断点
            //printf("已经写入pop pc，机器码为%x\n", M[0x3016]);
            system("cls");
            printf("An interrupt requestn had been made, interrupt number is %x\n", imm_20);
            ER = 1;
            printf("Interruption response is in progress\n");
            getchar();
            //push pc
            SP = SP - 4;
            AR = SP;
            DR = PC;
            *(word*)(M + AR) = (word)DR;
            PC = QUERY[imm_20];
            printf("The interrupt response ends. Access to interrupted service\n");
            printf("The interrupted service ends. Go back\n");
            printf("The interrupt is successful\n");
            getchar();
            break;
        }

        default: exit(2); break;
    }
}

void execute_instruction() {

    if(!condition_passed())
        return;

    /* buffer for CPSR */
    N = ((CPSR >> 31) & 0x1);
    Z = ((CPSR >> 30) & 0x1);
    C = ((CPSR >> 29) & 0x1);
    V = ((CPSR >> 28) & 0x1);

    byte optype = ((IR >> 26) & 0x3);    /*  2 bit */
    byte S      = ((IR >> 20) & 0x1);    /*  1 bit */

    /* switch */
    switch(optype) {
        case 3: execute_instruction3(); break;    /* triple operand instructions */
        case 2: execute_instruction2(); break;    /* double operand instructions */
        case 1: execute_instruction1(); break;    /* single operand instructions */
        case 0: execute_instruction0(); break;    /* zero operand and some triple operand instructoins */
    }
    
    /* renew CPSR */
    if(S) {
        CPSR = ((N << 31) | (Z << 30) | (C << 29) | (V << 28));
    }
}
