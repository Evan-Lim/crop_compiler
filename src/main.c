// ============================================================
// main.c - CROP Compiler Main Entry Point
// ============================================================
// This file contains the main() function and the compiler
// driver logic.
// ============================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "ast.h"
#include "symbol.h"
#include "typecheck.h"
#include "codegen.h"
#include "options.h"
#include "parser.h"

// ============================================================
// External declarations from lexer/parser
// ============================================================

extern int yyparse();
extern FILE* yyin;
extern ASTNode* program_ast;
extern int yylineno;

// ============================================================
// Compiler options
// ============================================================

CompilerOptions options;

// ============================================================
// Help message
// ============================================================

void print_help() {
    printf("CROP Compiler V2.1.2\n");
    printf("Usage: crop-compiler [options] <input.crop>\n\n");
    printf("Options:\n");
    printf("  --target=<arch>   Target architecture (esp32, stm32f4, rp2040, native)\n");
    printf("                    Default: native\n");
    printf("  --no-fpu          Disable floating point (error if f32/f64 used)\n");
    printf("  --verify          Enable SMT verification\n");
    printf("  --no-bounds-check Disable runtime array bounds checking (unsafe)\n");
    printf("  --optimise=<0-3>  Optimization level (default: 1)\n");
    printf("  --output=<dir>    Output directory for generated C code\n");
    printf("  --help            Show this help message\n\n");
    printf("Examples:\n");
    printf("  crop-compiler examples/hello.crop\n");
    printf("  crop-compiler examples/greenhouse.crop --target=esp32 --verify\n");
    printf("  crop-compiler examples/blink.crop --no-fpu --optimise=3\n");
}

// ============================================================
// Main function
// ============================================================

int main(int argc, char** argv) {
    // Initialize default options
    init_options(&options);
    
    // Parse command line
    if (!parse_options(argc, argv, &options)) {
        print_help();
        return 1;
    }
    
    // Check if help was requested
    if (options.show_help) {
        print_help();
        return 0;
    }
    
    // Check input file
    if (!options.input_file) {
        fprintf(stderr, "ERROR: No input file specified.\n");
        print_help();
        return 1;
    }
    
    printf("CROP Compiler V2.1.2\n");
    printf("Input: %s\n", options.input_file);
    printf("Target: %s\n", options.target);
    printf("Optimization level: %d\n", options.optimize_level);
    printf("\n");
    
    // ============================================================
    // Phase 1: Lex and Parse
    // ============================================================
    
    printf("Phase 1: Parsing...\n");
    
    FILE* input = fopen(options.input_file, "r");
    if (!input) {
        fprintf(stderr, "ERROR: Cannot open input file: %s\n", options.input_file);
        return 1;
    }
    
    yyin = input;
    int parse_result = yyparse();
    fclose(input);
    
    if (parse_result != 0 || !program_ast) {
        fprintf(stderr, "ERROR: Parsing failed.\n");
        return 1;
    }
    
    printf("  ✓ Parsed successfully\n");
    
    // ============================================================
    // Phase 2: Semantic Analysis
    // ============================================================
    
    printf("Phase 2: Semantic Analysis...\n");
    
    if (!semantic_analyze(program_ast, &options)) {
        fprintf(stderr, "ERROR: Semantic analysis failed.\n");
        return 1;
    }
    
    printf("  ✓ Semantic analysis passed\n");
    
    // ============================================================
    // Phase 3: SMT Verification (if enabled)
    // ============================================================
    
    if (options.verify) {
        printf("Phase 3: SMT Verification...\n");
        if (!check_invariants(program_ast)) {
            fprintf(stderr, "ERROR: Invariant violation detected!\n");
            return 1;
        }
        printf("  ✓ All invariants verified\n");
    }
    
    // ============================================================
    // Phase 4: Code Generation
    // ============================================================
    
    printf("Phase 4: Generating C code...\n");
    
    // Determine output file name
    char output_c[256];
    if (options.output_file) {
        strcpy(output_c, options.output_file);
    } else {
        char* base = strdup(options.input_file);
        char* dot = strrchr(base, '.');
        if (dot) *dot = '\0';
        sprintf(output_c, "%s.c", base);
        free(base);
    }
    
    if (!generate_c_code(program_ast, output_c, &options)) {
        fprintf(stderr, "ERROR: Code generation failed.\n");
        return 1;
    }
    
    printf("  ✓ Generated: %s\n", output_c);
    
    // ============================================================
    // Phase 5: Compile to executable
    // ============================================================
    
    printf("Phase 5: Compiling to executable...\n");
    
    char compile_cmd[1024];
    char executable[256];
    
    // Determine executable name
    char* base = strdup(options.input_file);
    char* dot = strrchr(base, '.');
    if (dot) *dot = '\0';
    sprintf(executable, "%s", base);
    free(base);
    
    // Build compile command based on target
    if (strcmp(options.target, "native") == 0) {
        snprintf(compile_cmd, sizeof(compile_cmd),
                 "gcc -O%d %s -o %s -lm",
                 options.optimize_level, output_c, executable);
    } else if (strcmp(options.target, "esp32") == 0) {
        snprintf(compile_cmd, sizeof(compile_cmd),
                 "xtensa-esp32-elf-gcc -O%d %s -o %s.elf -lm",
                 options.optimize_level, output_c, executable);
    } else if (strcmp(options.target, "stm32f4") == 0) {
        snprintf(compile_cmd, sizeof(compile_cmd),
                 "arm-none-eabi-gcc -O%d %s -o %s.elf -lm",
                 options.optimize_level, output_c, executable);
    } else if (strcmp(options.target, "rp2040") == 0) {
        snprintf(compile_cmd, sizeof(compile_cmd),
                 "arm-none-eabi-gcc -O%d %s -o %s.elf -lm",
                 options.optimize_level, output_c, executable);
    } else {
        fprintf(stderr, "WARNING: Unknown target '%s', using gcc\n", options.target);
        snprintf(compile_cmd, sizeof(compile_cmd),
                 "gcc -O%d %s -o %s -lm",
                 options.optimize_level, output_c, executable);
    }
    
    int result = system(compile_cmd);
    if (result != 0) {
        fprintf(stderr, "ERROR: Compilation to executable failed.\n");
        fprintf(stderr, "Command: %s\n", compile_cmd);
        return 1;
    }
    
    printf("  ✓ Compiled: %s\n", executable);
    printf("\n");
    printf("============================================================\n");
    printf("✓ SUCCESS! CROP compilation complete.\n");
    printf("  Executable: %s\n", executable);
    printf("  To run: ./%s\n", executable);
    printf("============================================================\n");
    
    return 0;
}
