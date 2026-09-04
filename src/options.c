// ============================================================
// options.c - Compiler options implementation
// ============================================================

#include "options.h"
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

void init_options(CompilerOptions* opts) {
    opts->input_file = NULL;
    opts->output_file = NULL;
    opts->target = strdup("native");
    opts->no_fpu = false;
    opts->verify = false;
    opts->no_bounds_check = false;
    opts->optimize_level = 1;
    opts->show_help = false;
}

bool parse_options(int argc, char** argv, CompilerOptions* opts) {
    static struct option long_options[] = {
        {"target", required_argument, 0, 't'},
        {"no-fpu", no_argument, 0, 'f'},
        {"verify", no_argument, 0, 'v'},
        {"no-bounds-check", no_argument, 0, 'b'},
        {"optimise", required_argument, 0, 'O'},
        {"output", required_argument, 0, 'o'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    
    int opt;
    int option_index = 0;
    
    while ((opt = getopt_long(argc, argv, "t:fv bO:o:h", long_options, &option_index)) != -1) {
        switch (opt) {
            case 't':
                free(opts->target);
                opts->target = strdup(optarg);
                break;
            case 'f':
                opts->no_fpu = true;
                break;
            case 'v':
                opts->verify = true;
                break;
            case 'b':
                opts->no_bounds_check = true;
                break;
            case 'O':
                opts->optimize_level = atoi(optarg);
                if (opts->optimize_level < 0) opts->optimize_level = 0;
                if (opts->optimize_level > 3) opts->optimize_level = 3;
                break;
            case 'o':
                opts->output_file = strdup(optarg);
                break;
            case 'h':
                opts->show_help = true;
                return true;
            default:
                return false;
        }
    }
    
    // Input file is the last argument
    if (optind < argc) {
        opts->input_file = strdup(argv[optind]);
    }
    
    return true;
}
