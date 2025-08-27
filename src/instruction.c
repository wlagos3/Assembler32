#include "instruction.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

static const InstructionDef instruction_table[] = {
    {"add", R_TYPE, 0x00, 0x01},
    {"sub", R_TYPE, 0x00, 0x02},
    {"and", R_TYPE, 0x00, 0x03},
    {"or", R_TYPE, 0x00, 0x04},
    {"xor", R_TYPE, 0x00, 0x05},
    {"sll", R_TYPE, 0x00, 0x06},
    {"srl", R_TYPE, 0x00, 0x07},
    {"sra", R_TYPE, 0x00, 0x08},
    {"jr", R_TYPE, 0x00, 0x09},
    {"addi", I_TYPE, 0x01, 0x00},
    {"beq", I_TYPE, 0x02, 0x00},
    {"bneq", I_TYPE, 0x03, 0x00},
    {"bltz", I_TYPE, 0x04, 0x00},
    {"bgtz", I_TYPE, 0x05, 0x00},
    {"blt", I_TYPE, 0x06, 0x00},
    {"bgt", I_TYPE, 0x07, 0x00},
    {"lw", I_TYPE, 0x08, 0x00},
    {"sw", I_TYPE, 0x09, 0x00},
    {"lh", I_TYPE, 0x0A, 0x00},
    {"sh", I_TYPE, 0x0B, 0x00},
    {"j", J_TYPE, 0x0C, 0x00},
    {"jal", J_TYPE, 0x0D, 0x00},
};

static const InstructionDef *find_instruction(const char *name) {
    for (int i = 0; instruction_table[i].name != NULL; i++) {
        if (strcmp(instruction_table[i].name, name) == 0) {
            return &instruction_table[i];
        }
    }
    return NULL;
}

int is_valid_register(const char *reg) {
    if (reg[0] != '$') return 0;

    if (reg[1] == 'v') {
        char *endptr;
        int num = strtol(reg + 2, &endptr, 10);
        return *endptr == '\0' && num >= 0 && num <= 1;
    }
    if (reg[1] == 'a') {
        char *endptr;
        int num = strtol(reg + 2, &endptr, 10);
        return *endptr == '\0' && num >= 0 && num <= 4;
    }
    if (reg[1] == 'r') {
        char *endptr;
        int num = strtol(reg + 2, &endptr, 10);
        return *endptr == '\0' && num >= 0 && num <= 15;
    }
    if (reg[1] == 's') {
        char *endptr;
        int num = strtol(reg + 2, &endptr, 10);
        return *endptr == '\0' && num >= 0 && num <= 4;
    }

    if (strcmp(reg, "$zero") == 0) return 1;
    if (strcmp(reg, "$sp") == 0) return 1;
    if (strcmp(reg, "$ra") == 0) return 1;

    return 0;
}

int parse_register(const char *reg) {
    if (!is_valid_register(reg)) return -1;

    if (reg[1] == 'v') {
        const int v_offset = 1;
        return strtol(reg + 2, NULL, 10) + v_offset;
    }

    if (reg[1] == 'a') {
        const int a_offset = 3;
        return strtol(reg + 2, NULL, 10) + a_offset;
    }
    if (reg[1] == 'r') {
        const int r_offset = 9;
        return strtol(reg + 2, NULL, 10) + r_offset;
    }
    if (reg[1] == 's') {
        const int s_offset = 23;
        return strtol(reg + 2, NULL, 10) + s_offset;
    }

    if (strcmp(reg, "$zero") == 0) return 0;
    if (strcmp(reg, "$sp") == 0) return 29;
    if (strcmp(reg, "$ra") == 0) return 31;

    return -1;
}

int16_t parse_immediate(const char *imm) {
    char *endptr;
    long value;

    if (strncmp(imm, "0x", 2) == 0) {
        value = strtol(imm, &endptr, 16);
    } else if (strncmp(imm, "0b", 2) == 0) {
        value = strtol(imm, &endptr, 2);
    } else {
        value = strtol(imm, &endptr, 10);
    }

    if (*endptr != '\0') return 0;
    return (int16_t) value;
}

uint32_t parse_address(const char *addr) {
    char *endptr;
    unsigned long value;

    if (strncmp(addr, "0x", 2) == 0) {
        value = strtoul(addr, &endptr, 16);
    } else {
        value = strtoul(addr, &endptr, 10);
    }

    if (*endptr != '\0') return 0;
    return (uint32_t) value;
}

bool is_label_reference(const char *token) {
    if (!token) return false;

    if (isdigit(token[0]) ||
        (token[0] == '0' && (token[1] == 'x' || token[1] == 'b'))) {
        return false;
    }

    for (const char *p = token; *p; p++) {
        if (!isalnum(*p) && *p != '_') {
            return false;
        }
    }

    return true;
}

static Instruction parse_r_type(const InstructionDef *def, char *tokens[], int token_count) {
    Instruction inst = {0};
    inst.type = R_TYPE;
    inst.data.r.opcode = def->opcode;
    inst.data.r.funct = def->funct;

    if (token_count != 4) {
        inst.type = R_TYPE;
        return inst;
    }

    inst.data.r.rd = parse_register(tokens[1]);
    inst.data.r.rs = parse_register(tokens[2]);
    inst.data.r.rt = parse_register(tokens[3]);
    inst.data.r.shamt = 0;

    return inst;
}

static Instruction parse_i_type(const InstructionDef *def, char *tokens[], int token_count) {
    Instruction inst = {0};
    inst.type = I_TYPE;
    inst.data.i.opcode = def->opcode;

    if (strcmp(def->name, "lw") == 0 || strcmp(def->name, "sw") == 0) {
        if (token_count != 3) {
            return inst;
        }

        inst.data.i.rt = parse_register(tokens[1]);

        char *paren = strchr(tokens[2], '(');
        if (paren) {
            *paren = '\0';
            inst.data.i.immediate = parse_immediate(tokens[2]);

            char *reg_start = paren + 1;
            char *close_paren = strchr(reg_start, ')');
            if (close_paren) *close_paren = '\0';
            inst.data.i.rs = parse_register(reg_start);
        }
    } else if (strcmp(def->name, "beq") == 0 || strcmp(def->name, "bneq") == 0 ||
               strcmp(def->name, "bltz") == 0 || strcmp(def->name, "bgtz") == 0 ||
               strcmp(def->name, "blt") == 0 || strcmp(def->name, "bgt") == 0) {
        if (token_count != 4) {
            inst.type = I_TYPE;
            return inst;
        }

        inst.data.i.rs = parse_register(tokens[1]);
        inst.data.i.rt = parse_register(tokens[2]);

        if (is_label_reference(tokens[3])) {
            strncpy(inst.label_ref, tokens[3], sizeof(inst.label_ref) - 1);
            inst.label_ref[sizeof(inst.label_ref) - 1] = '\0';
            inst.data.i.immediate = 0;
        } else {
            inst.type = -1;
            return inst;
        }
    } else {
        if (token_count != 4) {
            inst.type = I_TYPE;
            return inst;
        }

        inst.data.i.rt = parse_register(tokens[1]);
        inst.data.i.rs = parse_register(tokens[2]);
        inst.data.i.immediate = parse_immediate(tokens[3]);
    }

    return inst;
}

static Instruction parse_j_type(const InstructionDef *def, char *tokens[], int token_count) {
    Instruction inst = {0};
    inst.type = J_TYPE;
    inst.data.j.opcode = def->opcode;

    if (token_count != 2) {
        inst.type = J_TYPE;
        return inst;
    }

    if (is_label_reference(tokens[1])) {
        strncpy(inst.label_ref, tokens[1], sizeof(inst.label_ref) - 1);
        inst.label_ref[sizeof(inst.label_ref) - 1] = '\0';
        inst.data.j.address = 0;
    } else {
        inst.data.j.address = parse_address(tokens[1]);
    }

    return inst;
}

Instruction parse_instruction(const char *line) {
    Instruction inst = {.type = -1};
    char buffer[256];
    strncpy(buffer, line, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    char *tokens[10];
    int token_count = 0;

    char *token = strtok(buffer, " \t\n\r,");
    while (token && token_count < 10) {
        tokens[token_count++] = token;
        token = strtok(NULL, " \t\n\r,");
    }

    if (token_count == 0) return inst;

    const InstructionDef *def = find_instruction(tokens[0]);
    if (!def) return inst;

    switch (def->type) {
        case R_TYPE:
            return parse_r_type(def, tokens, token_count);
        case I_TYPE:
            return parse_i_type(def, tokens, token_count);
        case J_TYPE:
            return parse_j_type(def, tokens, token_count);
        default:
            return inst;
    }
}
