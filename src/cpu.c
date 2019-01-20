#include <stdio.h>
#include <cpu.h>
#include <HIF.h>
#include <memory.h>
#include <process.h>

// PSA: refer to these two pages while reading this code:
// https://www.masswerk.at/6502/6502_instruction_set.html instructions list and first of the 3 tables at the bottom
// http://nparker.llx.com/a2/opcodes.html more tables to organize instructions

unsigned long cpuThread = 0;

// registers
static unsigned char A = 0;
static unsigned char X = 0;
static unsigned char Y = 0;
static unsigned char S = 0;
static union
{
    struct
    {
        unsigned char PCLO;
        unsigned char PCHI;
    };
    unsigned short PC;
} pc = { 0 };
static union
{
    struct
    {
        unsigned C : 1;     // carry
        unsigned Z : 1;     // zero
        unsigned I : 1;     // interrupt disable
        unsigned D : 1;     // decimal mode
        unsigned B : 1;     // break command
        unsigned unused : 1;
        unsigned V : 1;     // overflow
        unsigned N : 1;     // negative
    };
    unsigned char P;
} status = { 0 };

#define PC pc.PC
#define PCLO pc.PCLO
#define PCHI pc.PCHI
#define C status.C
#define Z status.Z
#define I status.I
#define D status.D
#define B status.B
#define V status.V
#define N status.N
#define P status.P

struct instruction
{
    unsigned c : 2;
    unsigned b : 3;
    unsigned a : 3;
};

// individual bits of part a in branch instructions have special meaning
struct branchInstruction
{
    unsigned unused : 5;
    unsigned y : 1;     // compare flag with
    unsigned x : 2;     // flag selector
};

// special case instructions
#define IS_BRANCH_INSTRUCTION(instr) instr.c == 0b00 && instr.b == 0b100
#define IS_JSR(instr) instr.c == 0b00 && instr.b == 0b000 && instr.a == 0b001
#define IS_JMP_ABS(instr) instr.c == 0b00 && instr.b == 0b011 && instr.a == 0b010
#define IS_JMP_IND(instr) instr.c == 0b00 && instr.b == 0b011 && instr.a == 0b011
#define IS_IMPL(instr) (instr.c == 0b00 && (instr.b == 0b000 && instr.a < 0b100 || instr.b == 0b010 || instr.b == 0b110)) || (instr.c == 0b10 && ((instr.b == 0b010 && instr.a >= 0b100) || instr.b == 0b110))

#define ACC     0
#define ABS     1
#define ABSX    2
#define ABSY    3
#define IMME    4
#define IMPL    5
#define XIND    7
#define INDY    8
#define ZPG     10
#define ZPGX    11
#define ZPGY    12

static int addressMode = ACC;

// look up address mode based on [bbb] and [cc]
/*static const int modeLookup[7][3] =
        {
        [0] = { IMME, XIND, IMME },
        [1] = { ZPG, ZPG, ZPG },
        [2] = { IMPL, IMME, ACC },
        [3] = { ABS, ABS, ABS },
        [4] = { },
        [5] = {},
        [6] = {},
        [7] = {}
        };*/

/**
 * pushes a value onto the stack
 * @param val the value to push
 */
static void push(unsigned char val)
{
    memory[0x100 + S--] = val;
}

/**
 * pulls (pops) a value from the stack
 * @return value pulled from the stack
 */
static unsigned char pull()
{
    return memory[0x100 + ++S];
}

static unsigned char load()
{
    unsigned short address = 0;

    switch(addressMode)
    {
    case ACC:       // operand is accumulator, implied
        return A;
    case ABS:       // operand is absolute address
        address = *((unsigned short *) (memory + PC + 1));
        break;
        //return memory[*((unsigned short *) (memory + PC + 1))];
    case ABSX:      // operand is address; effective address is address incremented by X with carry
        address = (unsigned short) (*((unsigned short *) (memory + PC + 1)) + X + C);
        break;
    case ABSY:      // operand is address; effective address is address incremented by Y with carry
        address = (unsigned short) (*((unsigned short *) (memory + PC + 1)) + Y + C);
        break;
    case IMME:      // operand is byte
        address = (unsigned short) (PC + 1);
        break;
    case XIND:      // operand is zeropage address; effective address is word in (LL + X, LL + X + 1), inc. without carry
        address = *((unsigned short *) (memory + memory[PC+1] + X));
        break;
    case INDY:      // operand is zeropage address; effective address is word in (LL, LL + 1) incremented by Y with carry
        address = (unsigned short) (*((unsigned short *) (memory + memory[PC+1])) + Y + C);
        break;
    case ZPG:       // operand is zeropage address
        address = (unsigned short) (memory[PC+1]);
        break;
    case ZPGX:      // operand is zeropage address; effective address is address incremented by X without carry
        address = (unsigned short) (memory[PC+1] + X);
        break;
    case ZPGY:      // operand is zeropage address; effective address is address incremented by Y without carry
        address = (unsigned short) (memory[PC+1] + Y);
        break;
    default:
        return 0;
    }

    unsigned char byte = readByte(address);
    if(D)
    {
        byte = (unsigned char) (((byte >> 4) * 10) + (byte & 0xF));
    }
    return byte;
}

/**
 * stores a byte in memory
 * @param byte
 */
static void store(unsigned char byte)
{
    unsigned short address = 0;

    switch(addressMode)
    {
    case ABS:       // operand is absolute address
        address = *((unsigned short *) (memory + PC + 1));
        break;
        //return memory[*((unsigned short *) (memory + PC + 1))];
    case ABSX:      // operand is address; effective address is address incremented by X with carry
        address = (unsigned short) (*((unsigned short *) (memory + PC + 1)) + X + C);
        break;
    case ABSY:      // operand is address; effective address is address incremented by Y with carry
        address = (unsigned short) (*((unsigned short *) (memory + PC + 1)) + Y + C);
        break;
    case IMME:      // operand is byte
        address = (unsigned short) (PC + 1);
        break;
    case XIND:      // operand is zeropage address; effective address is word in (LL + X, LL + X + 1), inc. without carry
        address = *((unsigned short *) (memory + memory[PC+1] + X));
        break;
    case INDY:      // operand is zeropage address; effective address is word in (LL, LL + 1) incremented by Y with carry
        address = (unsigned short) (*((unsigned short *) (memory + memory[PC+1])) + Y + C);
        break;
    case ZPG:       // operand is zeropage address
        address = (unsigned short) (memory[PC+1]);
        break;
    case ZPGX:      // operand is zeropage address; effective address is address incremented by X without carry
        address = (unsigned short) (memory[PC+1] + X);
        break;
    case ZPGY:      // operand is zeropage address; effective address is address incremented by Y without carry
        address = (unsigned short) (memory[PC+1] + Y);
        break;
    default:
        return;
    }

    writeByte(address, byte);
}

/**
 * executes the instruction pointed to by PC
 * @return 1 for success, 0 for error such as invalid instruction
 */
static int executeInstruction()
{
    unsigned char instr = readByte(PC);
    struct instruction currentInstruction = *(struct instruction *) &instr;

    //printf("PC: %04X\t%02X %02X %02X\n\nA\tX\tY\tS\tN V B D I Z C\n%02X\t%02X\t%02X\t%02X\t%u %u %u %u %u %u %u\n\n", PC, memory[PC], memory[PC+1], memory[PC+2], A, X, Y, S, N, V, B, D, I, Z, C);

    // only halt instruction supported
    if(currentInstruction.a == 0b000 && currentInstruction.b == 0b000 && currentInstruction.c == 0b10)
    {
        WaitForSingleObject(runningMutex, INFINITE);
        running = 0;
        ReleaseMutex(runningMutex);
        return 0;
    }

    // special case instructions first
    if(IS_JSR(currentInstruction))
    {
        // push address of instruction following JSR to return to
        PC += 2;
        push(PCHI);
        push(PCLO);
        // jump to address in argument
        //printf("JSR to %04X\n", *((unsigned short *) (memory + PC - 1)));
        PC = *((unsigned short *) (memory + PC - 1));
    }
    else if(IS_JMP_ABS(currentInstruction))
    {
        PC = *((unsigned short *) (memory + PC + 1));
    }
    else if(IS_JMP_IND(currentInstruction))
    {
        PC = *((unsigned short *) (memory + *((unsigned short *) (memory + PC + 1))));
    }
    else if(IS_BRANCH_INSTRUCTION(currentInstruction))
    {
        struct branchInstruction branch = *((struct branchInstruction *) &currentInstruction);

        unsigned int flag;

        switch(branch.x)
        {
        case 0:
            flag = N;
            break;
        case 1:
            flag = V;
            break;
        case 2:
            flag = C;
            break;
        default:
            flag = Z;
            break;
        }

        if(flag == branch.y)
        {
            // PC would be address of branch + 2 by the time the offset is added
            PC += (signed char) memory[PC+1] + (unsigned short) 2;
        }
        else
        {
            PC += 2;
        }

    }
    else if(IS_IMPL(currentInstruction))
    {
        if(currentInstruction.c == 0)
        {
            switch(currentInstruction.b)
            {
            case 0:
                switch(currentInstruction.a)
                {
                case 0:     // BRK
                    if(!I)  // ignored if interrupt disable is set
                    {
                        B = 1;
                        PC++;
                        push(PCHI);
                        push(PCLO);
                        push(P);

                        PC = *((unsigned short *) (memory + 0xFFFE));
                    }
                    break;
                case 2:     // RTI
                    P = pull();
                case 3:     // RTS
                    PCLO = pull();
                    PCHI = pull();
                    //printf("returning to %02X\n", memory[PC]);
                    break;
                default:
                    return 0;
                }
                break;
            case 2:
                switch(currentInstruction.a)
                {
                case 0:     // PHP
                    push(P);
                    break;
                case 1:     // PLP
                    P = pull();
                    break;
                case 2:     // PHA
                    push(A);
                    break;
                case 3:     // PLA
                    A = pull();
                    break;
                case 4:     // DEY
                    Y--;
                    break;
                case 5:     // TAY
                    Y = A;
                    break;
                case 6:     // INY
                    Y++;
                    break;
                default:    // INX
                    X++;
                    break;
                }
                break;
            case 6:
                switch(currentInstruction.a)
                {
                    case 0:     // CLC
                        C = 0;
                        break;
                    case 1:     // SEC
                        C = 1;
                        break;
                    case 2:     // CLI
                        I = 0;
                        break;
                    case 3:     // SEI
                        I = 1;
                        break;
                    case 4:     // TYA
                        A = Y;
                        break;
                    case 5:     // CLV
                        V = 0;
                        break;
                    case 6:     // CLD
                        D = 0;
                        break;
                    default:    // SED
                        D = 1;
                        break;
                }
                break;
            default:
                return 0;
            }
        }
        else
        {
            switch(currentInstruction.b)
            {
            case 2:
                switch(currentInstruction.a)
                {
                case 4:      // TXA
                    A = X;
                    break;
                case 5:      // TAX
                    X = A;
                    break;
                case 6:      // DEX
                    X--;
                    break;
                case 7:      // NOP
                    // NOP
                    break;
                default:
                    return 0;
                }
                break;
            case 6:
                            // TXS
                if(currentInstruction.a == 4)
                {
                    S = X;
                }
                else        // TSX
                {
                    X = S;
                }
                break;
            default:
                return 0;
            }
        }
        PC++;
    }
    else
    {
        // size in bytes of the instruction (1-3) for incrementing PC
        unsigned char size = 0;

        switch(currentInstruction.c)
        {
        case 1:
            switch(currentInstruction.b)
            {
            case 0:
                addressMode = XIND;
                size = 2;
                break;
            case 1:
                addressMode = ZPG;
                size = 2;
                break;
            case 2:
                addressMode = IMME;
                size = 2;
                break;
            case 3:
                addressMode = ABS;
                size = 3;
                break;
            case 4:
                addressMode = INDY;
                size = 2;
                break;
            case 5:
                addressMode = ZPGX;
                size = 2;
                break;
            case 6:
                addressMode = ABSY;
                size = 3;
                break;
            case 7:
                addressMode = ABSX;
                size = 3;
                break;
            default:
                size = 1;
                break;
            }
            break;
        default:
            switch(currentInstruction.b)
            {
            case 0:     // some instructions in this group are implied or JSR abs, which are handled above
                addressMode = IMME;
                size = 2;
                break;
            case 1:
                addressMode = ZPG;
                size = 2;
                break;
            case 2:     // most in this group are implied, which are handled above
                addressMode = ACC;
                size = 1;
                break;
            case 3:
                addressMode = ABS;
                size = 3;
                break;
            // case 4 is only branch, handled above
            case 5:
                if(currentInstruction.c == 2 && (currentInstruction.a == 4 || currentInstruction.a == 5))
                {
                    addressMode = ZPGY;
                }
                else
                {
                    addressMode = ZPGX;
                }
                size = 2;
                break;
            case 6:
                addressMode = IMPL;
                size = 1;
                break;
            case 7:
                if(currentInstruction.c == 2 && currentInstruction.a == 5)
                {
                    addressMode = ABSY;
                }
                else
                {
                    addressMode = ABSX;
                }
                size = 3;
                break;
            default:
                break;
            }
        }

        // try store instructions first
        if(currentInstruction.a == 4)
        {
            // all b-values are STA for c = 1
            // odd b-values are STY and STX for c = 0 and 2,
            if(currentInstruction.c == 1)
            {
                store(A);
                PC += size;
                return 1;
            }
            if(currentInstruction.b % 2 == 1)
            {
                store(currentInstruction.c == 0 ? Y : X);
                PC += size;
                return 1;
            }
        }

        unsigned char data = load();

        // to keep resultant value while updating flags
        unsigned char hold;

        switch(currentInstruction.c)
        {
        case 1:
            switch(currentInstruction.a)
            {
            case 0:     // ORA
                A |= data;
                N = (signed char) A < 0 ? 1 : 0;
                Z = A == 0 ? 1 : 0;
                break;
            case 1:     // AND
                A &= data;
                N = (signed char) A < 0 ? 1 : 0;
                Z = A == 0 ? 1 : 0;
                break;
            case 2:     // EOR
                A ^= data;
                N = (signed char) A < 0 ? 1 : 0;
                Z = A == 0 ? 1 : 0;
                break;
            case 3:     // ADC
                hold = A;
                A += data + C;
                // TODO figure out if this sets the V flag properly
                V = ((signed char) hold >= 0 && (signed char) data >= 0 && (signed char) A < 0)
                        || ((signed char) hold < 0 && (signed char) data < 0 && (signed char) A >= 0) ? 1 : 0;

                C = (int) hold + data + C > 0xFF ? 1 : 0;
                N = (signed char) A < 0 ? 1 : 0;
                Z = A == 0 ? 1 : 0;
                break;
            case 5:     // LDA
                A = data;
                N = (signed char) A < 0 ? 1 : 0;
                Z = A == 0 ? 1 : 0;
                break;
            case 6:     // CMP
                C =  A >= data ? 1 : 0;
                data = A - data;
                N = (signed char) data < 0 ? 1 : 0;
                Z = data == 0 ? 1 : 0;
                break;
            case 7:     // SBC
                hold = A;
                A -= data - C;

                V = ((signed char) hold >= 0 && (signed char) data >= 0 && (signed char) A < 0)
                    || ((signed char) hold < 0 && (signed char) data < 0 && (signed char) A >= 0) ? 1 : 0;

                C = (int) hold + data + C > 0xFF ? 1 : 0;
                N = (signed char) A < 0 ? 1 : 0;
                Z = A == 0 ? 1 : 0;
                break;
            default:
                return 0;
            }
            break;
        case 2:
            switch(currentInstruction.a)
            {
            case 0:     // ASL
                if(addressMode == ACC)
                {
                    // sign bit goes into carry
                    C = (signed char) A < 0 ? 1 : 0;
                    A <<= 1;

                    N = (signed char) A < 0 ? 1 : 0;
                    Z = A == 0 ? 1 : 0;
                }
                else
                {
                    C = (signed char) data < 0 ? 1 : 0;
                    store(data << 1);

                    N = (signed char) data < 0 ? 1 : 0;
                    Z = data == 0 ? 1 : 0;
                }
                break;
            case 1:     // ROL
                if(addressMode == ACC)
                {
                    C = (signed char) A < 0 ? 1 : 0;
                    A = (unsigned char) ((A << 1) + C);

                    N = (signed char) A < 0 ? 1 : 0;
                    Z = A == 0 ? 1 : 0;
                }
                else
                {
                    C = (signed char) data < 0 ? 1 : 0;
                    data = (unsigned char) ((data << 1) + C);
                    store(data);

                    N = (signed char) data < 0 ? 1 : 0;
                    Z = data == 0 ? 1 : 0;
                }
                break;
            case 2:     // LSR
                if(addressMode == ACC)
                {
                    // bit 0 goes into carry
                    C = A & 1 ? 1 : 0;
                    A >>= 1;

                    Z = A == 0 ? 1 : 0;
                }
                else
                {
                    C = data & 1 ? 1 : 0;
                    store(data >> 1);

                    Z = data == 0 ? 1 : 0;
                }
                break;
            case 3:     // ROR
                if(addressMode == ACC)
                {
                    C = A & 1 ? 1 : 0;
                    A = (unsigned char) ((A >> 1) | (C ? 0x80 : 0));

                    N = (signed char) A < 0 ? 1 : 0;
                    Z = A == 0 ? 1 : 0;
                }
                else
                {
                    C = data & 1 ? 1 : 0;
                    data = (unsigned char) ((data >> 1) | (C ? 0x80 : 0));
                    store(data);

                    N = (signed char) data < 0 ? 1 : 0;
                    Z = data == 0 ? 1 : 0;
                }
                break;
            case 5:     // LDX
                X = data;
                N = (signed char) X < 0 ? 1 : 0;
                Z = X == 0 ? 1 : 0;
                break;
            case 6:     // DEC
                data--;
                store(data);

                N = (signed char) data < 0 ? 1 : 0;
                Z = data == 0 ? 1 : 0;
                break;
            case 7:     // INC
                data++;
                store(data);

                N = (signed char) data < 0 ? 1 : 0;
                Z = data == 0 ? 1 : 0;
                break;
            default:
                return 0;
            }
            break;
        case 0:
            switch(currentInstruction.a)
            {
            case 1:     // BIT
                // bits 7 and 6 of the operand are transferred to bits 7 and 6 of the status register (N and V)
                P = (unsigned char) ((data & 0b11000000) | (P & 0b00111111));
                Z = (data & A) == 0 ? 1 : 0;
                break;
            case 5:     // LDY
                Y = data;
                N = (signed char) X < 0 ? 1 : 0;
                Z = X == 0 ? 1 : 0;
                break;
            case 6:     // CPY
                C = Y >= data ? 1 : 0;
                data = Y - data;
                N = (signed char) data < 0 ? 1 : 0;
                Z = data == 0 ? 1 : 0;
                break;
            case 7:     // CPX
                C =  X >= data ? 1 : 0;
                data = X - data;
                N = (signed char) data < 0 ? 1 : 0;
                Z = data == 0 ? 1 : 0;
                break;
            default:
                return 0;
            }
            break;
        default:
            return 0;
        }
        PC += size;
    }

    return 1;
}

/**
 * continuously runs instructions
 */
static void run()
{
    WaitForSingleObject(runningMutex, INFINITE);
    running = 1;
    ReleaseMutex(runningMutex);
    unsigned int instructionsRun = 0;


    while(executeInstruction())
    {
        WaitForSingleObject(runningMutex, INFINITE);
        if(!running)
        {
            ReleaseMutex(runningMutex);
            break;
        }
        ReleaseMutex(runningMutex);
        //Sleep(200);
        instructionsRun++;
        //WaitForSingleObject(runningMutex, INFINITE);
    }

    // release again because the loop didn't get to release it when the condition wasn't met
    //ReleaseMutex(runningMutex);

    printf("Done after %d instructions\n\n", instructionsRun);
    printf("PC: %04X\t%02X %02X %02X\n\nA\tX\tY\tS\tN V B D I Z C\n%02X\t%02X\t%02X\t%02X\t%u %u %u %u %u %u %u\n\n", PC, memory[PC], memory[PC+1], memory[PC+2], A, X, Y, S, N, V, B, D, I, Z, C);
/*    printf("Contents of $0400:\n");
    WaitForSingleObject(memMutex, INFINITE);
    for(unsigned short base = 0x400; base < 0x800; base += 0x40)
    {
        printf("%04X: ", base);
        for(unsigned short ch = 0; ch < 0x28; ch++)
        {
            printf("%c ", memory[base + ch]);
        }
        printf("\n");
    }
    ReleaseMutex(memMutex);*/

    WaitForSingleObject(runningMutex, INFINITE);
    cpuThread = 0;
    running = 0; // in case !executeInstruction() was the reason the loop exited
    ReleaseMutex(runningMutex);
    printf("done running\n");
}

/**
 * performs the 6502 reset routine
 */
void reset()
{
    Sleep(1); // give previous run thread time to end if F1 was pressed
    PC = *((unsigned short *) (memory + 0xFFFC));
    S = 0xFF;
    I = 1;

    cpuThread = _beginthread(run, 0, NULL);
}