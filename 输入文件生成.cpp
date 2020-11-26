#define _CRT_SECURE_NO_WARNINGS

# include <stdio.h>
# include <string.h>

/*
 * MOV R0, 0x1
 * ADD R0, 0x1
 * NOP
 * B 0xffff8  (-8)
 *
 * 0xea000001,
 * 0xee800001,
 * 0xe0c00000,
 * 0xe62ffff8
*/

int as[] = {
    0xe4600000,  // push r0
    0xe4610000,  // push r1
    0xe4620000,  // push r2
    0xe4630000,  // push r3
    0xe4640000,  // push r4
    0xe4650000,  // push r5
    0xe4660000,  // push r6
    0xe4670000,  // push r7

    0xea00f000,  // mov r0, array=0x0f000
    0xea010004,  // mov r1, 4
    0xea020002,  // mov r2, 2
    0xea030000,  // mov r3, 0
    0xea040001,  // mov r4, 1
    0xe2a40000,  // str r4, [r0, 0]
    0xec850001,  // add r5, r0, r1
    0xe2a45000,  // str r4, [r5, 0]

    0xe6800002,  //SWI #0x2

    //loop:
    0xe2830000,  // ldr r3, [r0, 0]
    0xe2865000,  // ldr r6, [r5, 0]
    0xec833006,  // add r3, r3, r6
    0xe0a35001,  // str r3, [r5, r1]
    0xec800001,  // add r0, r0, r1
    0xec855001,  // add r5, r5, r1
    0xee822001,  // add r2, r2, 1
    0xead20063,  // cmp r2, #0x63
    0xd62fffe0,  // ble loop(-0x20)

    0xe4460000,  // pop r6
    0xe4450000,  // pop r5
    0xe4440000,  // pop r4
    0xe4430000,  // pop r3
    0xe4420000,  // pop r2
    0xe4410000,  // pop r1
    0xe4400000,  // pop r0
	
	
	

};

int main() {
    FILE* fp = fopen("example", "wb");

    for (int i = 0; i < sizeof(as) / sizeof(int); ++i) {
        fwrite(as + i, sizeof(int), 1, fp);
        printf("%x", as[i]);
    }

    fclose(fp);

    return 0;
}