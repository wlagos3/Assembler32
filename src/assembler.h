//
// Created by William James Lagos on 7/26/25.
//
#pragma once
#include "instruction.h"
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    ASSEMBLER_SUCCESS = 0,
    ASSEMBLER_ERROR_NULL_POINTER,
    ASSEMBLER_ERROR_INVALID_INSTRUCTION,
    ASSEMBLER_ERROR_INCOMPLETE_INSTRUCTION,
    ASSEMBLER_ERROR_INVALID_REGISTER,
    ASSEMBLER_ERROR_INVALID_IMMEDIATE,
    ASSEMBLER_ERROR_INVALID_ADDRESS,
    ASSEMBLER_ERROR_INVALID_LABEL,
    ASSEMBLER_ERROR_INVALID_OFFSET,
    ASSEMBLER_ERROR_INVALID_OPCODE,
    ASSEMBLER_ERROR_MEMORY_ALLOCATION,
    ASSEMBLER_ERROR_BUFFER_FULL,
} InstructionValidateResult;

typedef struct {
    char name[256];
    uint32_t instruction_line;
} Label;

typedef struct {
    Instruction *instructions;
    uint32_t instruction_count;
    uint32_t *machine_code;
    uint32_t machine_code_size;
    Label *labels;
    uint32_t label_count;
    InstructionValidateResult last_error;
    char error_message[256];
} Assembler;

Assembler *assembler_create();

void assembler_destroy(Assembler *assembler);

InstructionValidateResult assembler_add_and_validate_instruction(Assembler *assembler, Instruction instruction);

uint32_t *assembler_generate_machine_code(Assembler *assembler);

bool assembler_validate_instruction(const Instruction *instruction);

InstructionValidateResult assembler_validate_r_type(const RTypeInstruction *r_instr);

bool assembler_validate_i_type(const ITypeInstruction *i_instr);

bool assembler_validate_j_type(const JTypeInstruction *j_instr);

uint32_t r_type_to_machine_code(const RTypeInstruction *r_instr);

uint32_t i_type_to_machine_code(const ITypeInstruction *i_instr);

uint32_t j_type_to_machine_code(const JTypeInstruction *j_instr);

const char *assembler_get_error_message(const Assembler *assembler);

void assembler_set_error(Assembler *assembler, InstructionValidateResult error, const char *message);

bool assembler_add_label(Assembler *assembler, const char *name, uint32_t instruction_line);

int32_t assembler_find_label(const Assembler *assembler, const char *name);

bool is_label_line(const char *line);
