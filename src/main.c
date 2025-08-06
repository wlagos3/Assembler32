#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "instruction.h"
#include "assembler.h"

#define MAX_LINE_LENGTH 256

char *instruction_to_str(uint32_t instruction) {
    char *line = malloc(33);
    for (int i = 0; i < 31; i++) {
        line[i] = instruction & (1 << 31 - i) ? '1' : '0';
    }
    line[32] = '\0';
    return line;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <assembly_file>\n", argv[0]);
        return 1;
    }
    FILE *assembly_source = fopen(argv[1], "r");
    if (!assembly_source) {
        printf("Failed to open assembly file: %s\n", argv[1]);
        return 1;
    }
    Assembler *assembler = assembler_create();
    if (!assembler) {
        printf("Failed to create assembler\n");
        fclose(assembly_source);
        return 1;
    }
    char line[MAX_LINE_LENGTH];
    int line_number = 0;
    int error_count = 0;
    while (fgets(line, sizeof(line), assembly_source)) {
        line_number++;
        char *trimmed = line;
        while (*trimmed == ' ' || *trimmed == '\t') trimmed++;
        if (*trimmed == '\n' || *trimmed == '\0' || *trimmed == '#' || *trimmed == ';') {
            continue;
        }
        char *newline = strchr(line, '\n');
        if (newline) *newline = '\0';
        printf("Parsing line %d: %s\n", line_number, line);
        Instruction instruction = parse_instruction(line);
        InstructionValidateResult result = assembler_add_and_validate_instruction(assembler, instruction);
        if (result != ASSEMBLER_SUCCESS) {
            printf("Error on line %d: %s\n", line_number, assembler_get_error_message(assembler));
            error_count++;
        } else {
            printf("Successfully added instruction on line %d\n", line_number);
        }
    }
    fclose(assembly_source);
    if (error_count > 0) {
        printf("\nAssembly failed with %d errors\n", error_count);
        assembler_destroy(assembler);
        return 1;
    }
    printf("\nGenerating machine code...\n");
    uint32_t *machine_code = assembler_generate_machine_code(assembler);

    if (machine_code) {
        printf("Machine code generated successfully:\n");
        for (uint32_t i = 0; i < assembler->instruction_count; i++) {
            char *instruction_str = instruction_to_str(machine_code[i]);
            printf("%s\n", instruction_str);
            free(instruction_str);
        }
    } else {
        printf("Failed to generate machine code\n");
        assembler_destroy(assembler);
        return 1;
    }

    assembler_destroy(assembler);
    return 0;
}
