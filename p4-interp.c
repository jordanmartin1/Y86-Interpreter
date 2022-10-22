/*
 * CS 261 PA4: Mini-ELF interpreter
 *
 * Name: Jordan Martin
 */

#include "p4-interp.h"

/**********************************************************************
 *                         REQUIRED FUNCTIONS
 *********************************************************************/

//decode and execute the instruction
y86_reg_t decode_execute (y86_t *cpu, y86_inst_t inst, bool *cnd, y86_reg_t *valA)
{
    //check if the values are null
    if (valA == NULL || cnd == NULL) {
        cpu->stat = INS;
        return 0;
    }


    //initialize variables used in decode and execute y86
    y86_reg_t valE = 0;
    y86_reg_t valB;


    //switch statement for the instruction
    switch (inst.icode) {
    case 0:
        //halt
        cpu->stat = HLT;
        break;
    case 1:
        //nop
        break;
    case 2:
        //cmov
        *valA = cpu->reg[inst.ra];
        valE = *valA;
        break;
    case 3:
        //irmov
        valE = inst.valC.v;
        break;
    case 4:
        //rmmov
        *valA = cpu->reg[inst.ra];
        valB = cpu->reg[inst.rb];
        valE = valB + inst.valC.d;
        break;
    case 5:
        //mrmov
        valB = cpu->reg[inst.rb];
        valE = valB + inst.valC.d;
        break;
    case 6:
        //op
        *valA = cpu->reg[inst.ra];
        valB = cpu->reg[inst.rb];

        //switch statement for additional details of op
        switch(inst.ifun.op) {
        case 0:
            //add
            valE = *valA + valB;
            break;
        case 1:
            //sub
            valE = valB - *valA;
            break;
        case 2:
            //and
            valE = valB & *valA;
            break;
        case 3:
            //xor
            valE = *valA ^ valB;
            break;
        default:
            cpu->stat = INS;
            return valE;
        }

        break;

    case 7:
        //jmp

        break;
    case 8:
        //call
        valB = cpu->reg[inst.rb];
        valE = valB - 8;
        break;
    case 9:
        //ret
        *valA = cpu->reg[inst.ra];
        valB = cpu->reg[inst.rb];
        valE = valB + 8;
        break;
    case 10:
        //push
        *valA = cpu->reg[inst.ra];
        valB = cpu->reg[RSP];
        valE = valB - 8;
        break;
    case 11:
        //pop
        *valA = cpu->reg[RSP];
        valB = cpu->reg[RSP];
        valE = valB + 8;
        break;
    default:
        cpu->stat = INS;
        break;


    }

    return valE;
}


//memory, write back, and PC functions from y86
void memory_wb_pc (y86_t *cpu, y86_inst_t inst, byte_t *memory,
                   bool cnd, y86_reg_t valA, y86_reg_t valE)
{
    y86_reg_t valM;
    uint64_t *x;


    //switch statement for the instruction
    switch (inst.icode) {
    case 0:
        //halt
        cpu->stat = HLT;
        cpu->pc = inst.valP;
        break;
    case 1:
        //nop
        cpu->pc = inst.valP;
        break;
    case 2:
        //cmov
        //conditional
        cpu->pc = inst.valP;
        break;
    case 3:
        //irmov
        cpu->reg[inst.rb] = valE;
        cpu->pc = inst.valP;
        break;
    case 4:
        //rmmov
        x = (uint64_t*) &memory[valE];
        valM = *x;
        cpu->pc = inst.valP;
        break;
    case 5:
        //mrmov
        x = (uint64_t*) &memory[valE];
        valM = *x;
        inst.ra = valM;
        cpu->pc = inst.valP;
        break;
    case 6:
        //op
        cpu->reg[inst.rb] = valE;
        cpu->pc = inst.valP;
        break;
    case 7:
        //jump
        break;
    case 8:
        //call
        break;
    case 9:
        x = (uint64_t*) &memory[valA];
        valM = *x;
        cpu->reg[RSP] = valE;
        cpu->pc = valM;
        break;
    case 10:
        //ret
        cpu->reg[RSP] = valE;
        cpu->pc = inst.valP;
        break;
    case 11:
        //push
        x = (uint64_t*) &memory[valE];
        valA = *x;
        cpu->reg[RSP] = valE;
        cpu->pc = inst.valP;
        break;
    case 12:
        //pop
        x = (uint64_t*) &memory[valA];
        valM = *x;
        cpu->reg[inst.ra] = valM;
        cpu->reg[RSP] = valE;
        cpu->pc = inst.valP;
        break;
    default:
        cpu->stat = INS;
        break;

    }




}

/**********************************************************************
 *                         OPTIONAL FUNCTIONS
 *********************************************************************/

//print available options
void usage_p4 (char **argv)
{
    printf("Usage: %s <option(s)> mini-elf-file\n", argv[0]);
    printf(" Options are:\n");
    printf("  -h      Display usage\n");
    printf("  -H      Show the Mini-ELF header\n");
    printf("  -a      Show all with brief memory\n");
    printf("  -f      Show all with full memory\n");
    printf("  -s      Show the program headers\n");
    printf("  -m      Show the memory contents (brief)\n");
    printf("  -M      Show the memory contents (full)\n");
    printf("  -d      Disassemble code contents\n");
    printf("  -D      Disassemble data contents\n");
    printf("  -e      Execute program\n");
    printf("  -E      Execute program (trace mode)\n");
}

//check arguments through the available options
bool parse_command_line_p4 (int argc, char **argv,
                            bool *header, bool *segments, bool *membrief, bool *memfull,
                            bool *disas_code, bool *disas_data,
                            bool *exec_normal, bool *exec_trace, char **filename)
{
    int opt;
    while ((opt = getopt(argc, argv, "HhafsmMdDeE")) != -1) {
        switch(opt) {
        case 'h':
            usage_p4(argv);
            return true;
        case 'H':
            *header = true;
            break;
        case 'a':
            *header = true;
            *segments = true;
            *membrief = true;
            break;
        case 'f':
            *header = true;
            *segments = true;
            *memfull = true;
            break;
        case 's':
            *segments = true;
            break;
        case 'm':
            *membrief = true;
            break;
        case 'M':
            *memfull = true;
            break;
        case 'd':
            *disas_code = true;
            break;
        case 'D':
            *disas_data = true;
            break;
        case 'e':
            *exec_normal = true;
            break;
        case 'E':
            *exec_trace = true;
            break;
        default:
            usage_p4(argv);
            return false;
        }
    }

    if (optind != argc-1) {
        usage_p4(argv);
        return false;
    }

    *filename = argv[optind];




    return true;
}

//print out the current cpu state
void dump_cpu_state (y86_t cpu)
{
    printf("Y86 CPU state:\n");

    printf("  %%rip: %016lx   flags: Z%d S%d O%d     ", cpu.pc, cpu.zf, cpu.sf, cpu.of);

    switch (cpu.stat) {
    case 1:
        printf("AOK\n");
        break;
    case 2:
        printf("HLT\n");
        break;
    case 3:
        printf("ADR\n");
        break;
    case 4:
        printf("INS\n");
        break;
    }

    printf("  %%rax: %016lx    %%rcx: %016lx\n",  cpu.reg[0],  cpu.reg[1]);
    printf("  %%rdx: %016lx    %%rbx: %016lx\n", cpu.reg[2], cpu.reg[3]);
    printf("  %%rsp: %016lx    %%rbp: %016lx\n",   cpu.reg[4],  cpu.reg[5]);
    printf("  %%rsi: %016lx    %%rdi: %016lx\n",  cpu.reg[6], cpu.reg[7]);
    printf("   %%r8: %016lx     %%r9: %016lx\n",  cpu.reg[8], cpu.reg[9]);
    printf("  %%r10: %016lx    %%r11: %016lx\n",  cpu.reg[10], cpu.reg[11]);
    printf("  %%r12: %016lx    %%r13: %016lx\n",  cpu.reg[12], cpu.reg[13]);
    printf("  %%r14: %016lx\n",  cpu.reg[14]);




}

