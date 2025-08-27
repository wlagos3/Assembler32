//
// Created by William James Lagos on 7/26/25.
//

#include "assembler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 1024

Assembler *assembler_create() {
    Assembler *assembler = malloc(sizeof(Assembler));
    if (!assembler) return NULL;

    assembler->instructions = malloc(sizeof(Instruction) * BUFFER_SIZE);
    assembler->machine_code = malloc(sizeof(uint32_t) * BUFFER_SIZE);
    assembler->labels = malloc(sizeof(Label) * BUFFER_SIZE);
    assembler->instruction_count = 0;
    assembler->machine_code_size = BUFFER_SIZE;
    assembler->label_count = 0;
    assembler->last_error = ASSEMBLER_SUCCESS;
    memset(assembler->error_message, 0, sizeof(assembler->error_message));

    if (!assembler->instructions || !assembler->machine_code) {
        assembler_destroy(assembler);
        return NULL;
    }

    return assembler;
}

void assembler_destroy(Assembler *assembler) {
    if (assembler) {
        free(assembler->instructions);
        free(assembler->machine_code);
        free(assembler->labels);
        free(assembler);
    }
}

InstructionValidateResult assembler_add_and_validate_instruction(Assembler *assembler, Instruction instruction) {
    if (!assembler) {
        return ASSEMBLER_ERROR_NULL_POINTER;
    }

    if (!assembler_validate_instruction(&instruction)) {
        assembler_set_error(assembler, ASSEMBLER_ERROR_INVALID_INSTRUCTION, "Invalid instruction provided");
        return ASSEMBLER_ERROR_INVALID_INSTRUCTION;
    }

    if (assembler->instruction_count >= assembler->machine_code_size) {
        size_t new_size = assembler->machine_code_size * 2;
        void *new_instructions = realloc(assembler->instructions, sizeof(Instruction) * new_size);
        void *new_machine_code = realloc(assembler->machine_code, sizeof(uint32_t) * new_size);

        if (!new_instructions || !new_machine_code) {
            assembler_set_error(assembler, ASSEMBLER_ERROR_MEMORY_ALLOCATION, "Failed to expand instruction buffer");
            return ASSEMBLER_ERROR_MEMORY_ALLOCATION;
        }

        assembler->instructions = (Instruction *) new_instructions;
        assembler->machine_code = (uint32_t *) new_machine_code;
        assembler->machine_code_size = new_size;
    }

    assembler->instructions[assembler->instruction_count++] = instruction;
    return ASSEMBLER_SUCCESS;
}

uint32_t r_type_to_machine_code(const RTypeInstruction *r_instr) {
    uint32_t machine_code = 0;

    machine_code |= (r_instr->opcode & 0x3F) << 26;
    machine_code |= (r_instr->rs & 0x1F) << 21;
    machine_code |= (r_instr->rt & 0x1F) << 16;
    machine_code |= (r_instr->rd & 0x1F) << 11;
    machine_code |= (r_instr->shamt & 0x1F) << 6;
    machine_code |= (r_instr->funct & 0x3F);

    return machine_code;
}

uint32_t i_type_to_machine_code(const ITypeInstruction *i_instr) {
    uint32_t machine_code = 0;

    machine_code |= (i_instr->opcode & 0x3F) << 26;
    machine_code |= (i_instr->rs & 0x1F) << 21;
    machine_code |= (i_instr->rt & 0x1F) << 16;
    machine_code |= (i_instr->immediate & 0xFFFF);

    return machine_code;
}

uint32_t j_type_to_machine_code(const JTypeInstruction *j_instr) {
    uint32_t machine_code = 0;
    machine_code |= (j_instr->opcode & 0x3F) << 26;
    machine_code |= (j_instr->address & 0x3FFFFFF);
    return machine_code;
}

uint32_t *assembler_generate_machine_code(Assembler *assembler) {
    if (!assembler || !assembler->instructions) {
        return NULL;
    }

    for (size_t i = 0; i < assembler->instruction_count; i++) {
        Instruction *instr = &assembler->instructions[i];
        uint32_t machine_code = 0;

        if (strlen(instr->label_ref) > 0) {
            int32_t label_line = assembler_find_label(assembler, instr->label_ref);
            printf("Found label at: %d\n", label_line);
            if (label_line < 0) {
                return NULL;
            }

            if (instr->type == I_TYPE) {
                int32_t offset = label_line - (int32_t) i - 1;
                instr->data.i.immediate = (int16_t) offset;
            } else if (instr->type == J_TYPE) {
                instr->data.j.address = (uint32_t) label_line;
            }
        }

        switch (instr->type) {
            case R_TYPE:
                machine_code = r_type_to_machine_code(&instr->data.r);
                break;
            case I_TYPE:
                machine_code = i_type_to_machine_code(&instr->data.i);
                break;
            case J_TYPE:
                machine_code = j_type_to_machine_code(&instr->data.j);
                break;
            default:
                machine_code = 0;
                break;
        }

        assembler->machine_code[i] = machine_code;
    }

    return assembler->machine_code;
}

bool assembler_validate_instruction(const Instruction *instruction) {
    if (!instruction) {
        return false;
    }

    switch (instruction->type) {
        case R_TYPE:
            return assembler_validate_r_type(&instruction->data.r);
        case I_TYPE:
            return assembler_validate_i_type(&instruction->data.i);
        case J_TYPE:
            return assembler_validate_j_type(&instruction->data.j);
        default:
            return false;
    }
}

bool assembler_validate_r_type(const RTypeInstruction *r_instr) {
    if (!r_instr) {
        return false;
    }

    if (r_instr->rs > 31 || r_instr->rt > 31 || r_instr->rd > 31) {
        return false;
    }

    if (r_instr->shamt > 31) {
        return false;
    }

    if (r_instr->opcode > 63 || r_instr->funct > 63) {
        return false;
    }

    return true;
}

bool assembler_validate_i_type(const ITypeInstruction *i_instr) {
    if (!i_instr) {
        return false;
    }

    if (i_instr->rs > 31 || i_instr->rt > 31) {
        return false;
    }

    if (i_instr->opcode > 63) {
        return false;
    }

    return true;
}

bool assembler_validate_j_type(const JTypeInstruction *j_instr) {
    if (!j_instr) {
        return false;
    }

    if (j_instr->opcode > 63) {
        return false;
    }

    if (j_instr->address > 0x3FFFFFF) {
        return false;
    }

    return true;
}

const char *assembler_get_error_message(const Assembler *assembler) {
    if (!assembler) {
        return "Assembler is NULL";
    }
    return assembler->error_message;
}

void assembler_set_error(Assembler *assembler, InstructionValidateResult error, const char *message) {
    if (!assembler) {
        return;
    }

    assembler->last_error = error;
    if (message) {
        strncpy(assembler->error_message, message, sizeof(assembler->error_message) - 1);
        assembler->error_message[sizeof(assembler->error_message) - 1] = '\0';
    } else {
        assembler->error_message[0] = '\0';
    }
}

bool assembler_add_label(Assembler *assembler, const char *name, uint32_t instruction_line) {
    if (!assembler || !name) {
        return false;
    }

    if (assembler->label_count >= BUFFER_SIZE) {
        return false;
    }

    strncpy(assembler->labels[assembler->label_count].name, name,
            sizeof(assembler->labels[assembler->label_count].name) - 1);
    assembler->labels[assembler->label_count].name[sizeof(assembler->labels[assembler->label_count].name) - 1] = '\0';
    assembler->labels[assembler->label_count].instruction_line = instruction_line;
    assembler->label_count++;

    return true;
}

int32_t assembler_find_label(const Assembler *assembler, const char *name) {
    if (!assembler || !name) {
        return -1;
    }

    for (uint32_t i = 0; i < assembler->label_count; i++) {
        if (strcmp(assembler->labels[i].name, name) == 0) {
            return (int32_t) assembler->labels[i].instruction_line;
        }
    }

    return -1;
}

bool is_label_line(const char *line) {
    if (!line) return false;
    char *trimmed = (char *) line;
    while (*trimmed == ' ' || *trimmed == '\t') trimmed++;
    if (*trimmed == '\0' || *trimmed == '\n' || *trimmed == '#' || *trimmed == ';') {
        return false;
    }
    char *colon = strchr(trimmed, ':');
    if (!colon) return false;

    for (char *p = trimmed; p < colon; p++) {
        if (*p == ' ' || *p == '\t') return false;
    }

    return true;
}
