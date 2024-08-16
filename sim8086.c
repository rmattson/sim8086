#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MOD_MASK 0b11000000
#define REG_MASK 0b00111000
#define RM_MASK 0b00000111

#define true 1
#define false 0

const char RegistersByte[8][3] = {"al", "cl", "dl", "bl",
                                  "ah", "ch", "dh", "bh"};
const char RegistersWord[8][3] = {"ax", "cx", "dx", "bx",
                                  "sp", "bp", "si", "di"};

typedef struct
{
    char *opcode; // instruction opcode
    int d;        // 0: instruction src is REG; 1: instruction src is R/M
    int w;        // operating on 0: byte; 1: word
} Instruction;

const Instruction GetInstruction(unsigned int byte);

int main(int argc, char **argv)
{
    FILE *fptr;

    if ((fopen_s(&fptr, argv[1], "rb")) != 0)
    {
        printf("Error reading %s", argv[1]);
        exit(1);
    }

    printf("; %s disassembly:\n", argv[1]);
    printf("bits 16\n\n");

    unsigned char ReadBuffer;
    int bHasInstruction = false;
    Instruction instruction;
    while (fread(&ReadBuffer, sizeof(ReadBuffer), 1, fptr) == 1)
    {
        if (bHasInstruction)
        {
            // We have an instruction, so parse the extra data that comes with it
            unsigned int ModField = (ReadBuffer & MOD_MASK) >> 6; // Currently unused
            unsigned int RegField = (ReadBuffer & REG_MASK) >> 3; // Either src or dest
            unsigned int RmField = ReadBuffer & RM_MASK; // src or dest, opposite of RegField

            char src[3];
            char dest[3];
            
            // The `d` field on the instruction tells us which register is our
            // source and which is our destination. If `d` is 1, then that means
            // the REG field is the destination, and the R/M is the source.
            // If it's 0, then it's the reverse.
            if (instruction.d)
            {
                strcpy_s(dest, sizeof(dest), instruction.w ? RegistersWord[RegField] : RegistersByte[RegField]);
                strcpy_s(src, sizeof(src), instruction.w ? RegistersWord[RmField] : RegistersByte[RmField]);
            }
            else
            {
                strcpy_s(src, sizeof(src), instruction.w ? RegistersWord[RegField] : RegistersByte[RegField]);
                strcpy_s(dest, sizeof(dest), instruction.w ? RegistersWord[RmField] : RegistersByte[RmField]);
            }
            printf("%s, %s\n", dest, src);
            bHasInstruction = false;
        }
        else
        {
            // We do not yet have an instruction, so try to parse one on this byte
            instruction = GetInstruction(ReadBuffer);
            bHasInstruction = true;
            printf("%s ", instruction.opcode);
        }
    }

    return 0;
}

const Instruction GetInstruction(unsigned int byte)
{
    Instruction instruction;
    switch (byte >> 2) // The high 6 bits tell us what the instruction is
    {
    case 0b100010:
        instruction.opcode = "mov";
        break;
    }
    instruction.d = (byte >> 1) & 1;
    instruction.w = (byte & 1);
    return instruction;
}