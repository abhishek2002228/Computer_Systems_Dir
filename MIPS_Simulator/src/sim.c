#include <stdio.h>
#include "shell.h"

#define special_op 0
#define alu_i 1
#define load 2
#define store 3
#define unknown -1

int instr_type(uint32_t instr);
uint32_t get_field(uint32_t instr, int end_index, int start_index);
void process_special_op(uint32_t instr);
void process_alu_i(uint32_t instr);
void process_load(uint32_t instr);
void process_store(uint32_t instr);

void process_instruction()
{
    /* execute one instruction here. You should use CURRENT_STATE and modify
     * values in NEXT_STATE. You can call mem_read_32() and mem_write_32() to
     * access memory. */
    NEXT_STATE = CURRENT_STATE;
    NEXT_STATE.PC += 4; // if branch, this will be changed again

    uint32_t instr = mem_read_32(CURRENT_STATE.PC);
    int type = instr_type(instr);
    switch (type)
    {
    case unknown:
        printf("Unknown Instruction\n");
        break;
    case special_op:
        process_special_op(instr);
        break;
    case alu_i:
        process_alu_i(instr);
        break;
    case load:
        process_load(instr);
        break;
    case store:
        process_store(instr);
        break;
    default:
        break;
    }
}

uint32_t get_field(uint32_t instr, int end_index, int start_index){
    int width = end_index - start_index + 1;

    return (uint32_t)(instr >> start_index) & (((uint32_t)~0) >> (32 - width));
}

int instr_type(uint32_t instr){
    uint32_t opcode;
    opcode = get_field(instr, 31, 26);
    switch (opcode)
    {
    case(0):
        return special_op;
        break;
    case(8):
    case(9):
    case(12):
    case(13):
    case(14):
    case(15):
    case(10):
    case(11):
        return alu_i;
        break;
    case(32):
    case(33):
    case(35):
    case(36):
    case(37):
        return load;
        break;
    case(40):
    case(41):
    case(43):
        return store;
        break;
    default:
        return unknown;
    }
}

void process_special_op(uint32_t instr){
    uint32_t rs = get_field(instr, 25, 21);
    uint32_t rt = get_field(instr, 20, 16);
    uint32_t rd = get_field(instr, 15, 11);
    uint32_t shamt = get_field(instr, 10, 6);
    uint32_t funct = get_field(instr, 5, 0);

    switch ((int)funct)
    {
    case(32): // ADD
        NEXT_STATE.REGS[rd] = (int)CURRENT_STATE.REGS[rs] + (int)CURRENT_STATE.REGS[rt];
        break;
    case(33): // ADDU
        NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] + CURRENT_STATE.REGS[rt];
        break;
    case(34): // SUB
        NEXT_STATE.REGS[rd] = (int)CURRENT_STATE.REGS[rs] - (int)CURRENT_STATE.REGS[rt];
        break;
    case(35): // SUBU
        NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] - CURRENT_STATE.REGS[rt];
        break;
    case(36): // AND
        NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] & CURRENT_STATE.REGS[rt];
        break;
    case(37): // OR
        NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] | CURRENT_STATE.REGS[rt];
        break;
    case(38): // XOR
        NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] ^ CURRENT_STATE.REGS[rt];
        break;
    case(39):
        NEXT_STATE.REGS[rd] = ~(CURRENT_STATE.REGS[rs] | CURRENT_STATE.REGS[rt]);
        break;
    case(42): // SLT
        NEXT_STATE.REGS[rd] = ((int)(CURRENT_STATE.REGS[rs]) < (int)(CURRENT_STATE.REGS[rt])) ? 1 : 0;
        break;
    case(43): // SLTU
        NEXT_STATE.REGS[rd] = (CURRENT_STATE.REGS[rs] < CURRENT_STATE.REGS[rt]) ? 1 : 0;
        break;
    case(12): //syscall
        if(CURRENT_STATE.REGS[2] == 10){
            RUN_BIT = FALSE;
        }
        break;
    default:
        printf("unknown special_op instruction\n");
        break;
    }
}

void process_alu_i(uint32_t instr){
    uint32_t rs = get_field(instr, 25, 21);
    uint32_t rt = get_field(instr, 20, 16);
    uint32_t u_imm = get_field(instr, 15, 0);
    int s_imm = (((int)u_imm)<<16)>>16;
    uint32_t op_low = get_field(instr, 28, 26);

    switch (op_low)
    {
    case(0): // ADDI
        NEXT_STATE.REGS[rt] = (int)CURRENT_STATE.REGS[rs] + s_imm;
        break;
    case(1): //ADDIU
        NEXT_STATE.REGS[rt] = (int)CURRENT_STATE.REGS[rs] + s_imm;
        break;
    case(2): //SLTI
        NEXT_STATE.REGS[rt] = ((int)CURRENT_STATE.REGS[rs] < s_imm) ? 1 : 0;
        break;
    case(3): //SLTIU
        NEXT_STATE.REGS[rt] = (CURRENT_STATE.REGS[rs] < u_imm) ? 1 : 0;
        break;
    case(4):
        NEXT_STATE.REGS[rt] = CURRENT_STATE.REGS[rs] & u_imm;
        break;
    case(5):
        NEXT_STATE.REGS[rt] = CURRENT_STATE.REGS[rs] | u_imm;
        break;
    case(6):
        NEXT_STATE.REGS[rt] = CURRENT_STATE.REGS[rs] ^ u_imm;
        break;
    case(7):
        NEXT_STATE.REGS[rt] = (u_imm << 16) ;
        break;
    default:
        printf("unknown alu_i instruction\n");
        break;
    }
}

void process_load(uint32_t instr){
    uint32_t base = get_field(instr, 25, 21);
    uint32_t rt = get_field(instr, 20, 16);
    int offset = ((int)get_field(instr, 15, 0) << 16) >> 16;
    uint32_t op_low = get_field(instr, 28, 26);
    uint32_t word = mem_read_32(CURRENT_STATE.REGS[base] + offset);

    switch (op_low)
    {
    case(3): // LW
        NEXT_STATE.REGS[rt] = word;
        break;

    default:
        printf("unknown load instruction");
        break;
    }
}

void process_store(uint32_t instr){
    uint32_t base = get_field(instr, 25, 21);
    uint32_t rt = get_field(instr, 20, 16);
    int offset = ((int)get_field(instr, 15, 0) << 16) >> 16;
    uint32_t op_low = get_field(instr, 28, 26);
    switch (op_low)
    {
    case(3): // SW
        mem_write_32(CURRENT_STATE.REGS[base] + offset, CURRENT_STATE.REGS[rt]);
        break;

    default:
        printf("unknown store instruction");
        break;
    }
}