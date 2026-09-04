// ============================================================
// options.h - Compiler options
// ============================================================

#ifndef OPTIONS_H
#define OPTIONS_H

#include <stdbool.h>

typedef struct {
    char* input_file;
    char* output_file;
    char* target;
    bool no_fpu;
    bool verify;
    bool no_bounds_check;
    int optimize_level;
    bool show_help;
} CompilerOptions;

void init_options(CompilerOptions* opts);
bool parse_options(int argc, char** argv, CompilerOptions* opts);

#endif
