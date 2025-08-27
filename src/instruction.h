#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    R_TYPE,
    I_TYPE,
    J_TYPE
} InstructionType;

typedef struct {
    uint8_t opcode;
    uint8_t rs;
    uint8_t rt;
    uint8_t rd;
    uint8_t shamt;
    uint8_t funct;
} RTypeInstruction;

typedef struct {
    uint8_t opcode;
    uint8_t rs;
    uint8_t rt;
    int16_t immediate;
} ITypeInstruction;

typedef struct {
    uint8_t opcode;
    uint32_t address;
} JTypeInstruction;

typedef struct {
    InstructionType type;
    char label_ref[256];

    union {
        RTypeInstruction r;
        ITypeInstruction i;
        JTypeInstruction j;
    } data;
} Instruction;

typedef struct {
    const char *name;
    InstructionType type;
    uint8_t opcode;
    uint8_t funct;
} InstructionDef;

Instruction parse_instruction(const char *line);

int is_valid_register(const char *reg);

int parse_register(const char *reg);

int16_t parse_immediate(const char *imm);

uint32_t parse_address(const char *addr);

bool is_label_reference(const char *token);
