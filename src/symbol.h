// ============================================================
// symbol.h - Symbol Table for CROP Compiler
// ============================================================
// Manages variables, functions, and type declarations
// ============================================================

#ifndef SYMBOL_H
#define SYMBOL_H

#include "ast.h"
#include <stdbool.h>

// ============================================================
// Symbol types
// ============================================================

typedef enum {
    SYM_VAR,        // Global variable
    SYM_LET,        // Local immutable variable
    SYM_SENSOR,     // Sensor declaration
    SYM_OUTPUT,     // Output declaration
    SYM_FN,         // Function
    SYM_MACHINE,    // State machine
    SYM_STATE,      // State within a machine
    SYM_PAYLOAD,    // Payload struct
    SYM_PARAM,      // Function parameter
    SYM_EXTERN      // External C function
} SymbolType;

// ============================================================
// Symbol entry
// ============================================================

typedef struct Symbol {
    char* name;
    SymbolType sym_type;
    DataType data_type;
    struct Symbol* next;
    int scope_level;
    bool is_volatile;
    bool is_initialized;
    bool is_mutable;
    
    // Array info
    int array_size;
    
    // Fixed-point info
    int fixed_w;
    int fixed_s;
    
    // Function info
    struct Symbol* params;
    DataType return_type;
    int bounded_cycles;
    char* extern_lang;
    
    // Sensor info
    char* pin;
    char* protocol;
    int poll_ms;
    bool has_range;
    ASTNode* range_start;
    ASTNode* range_end;
    
    // Payload info
    struct Symbol* fields;
    
    // State machine info
    struct Symbol* states;
    struct Symbol* initial_state;
    
    // Location
    int line;
    int column;
} Symbol;

// ============================================================
// Scope
// ============================================================

typedef struct Scope {
    Symbol* symbols;
    struct Scope* parent;
    int level;
    char* name;
} Scope;

// ============================================================
// Global symbol table
// ============================================================

extern Scope* global_scope;
extern Scope* current_scope;

// ============================================================
// Function prototypes
// ============================================================

// Scope management
Scope* create_scope(Scope* parent, char* name);
void push_scope(char* name);
void pop_scope();
Scope* get_current_scope();

// Symbol creation
Symbol* create_symbol(char* name, SymbolType type, DataType data_type);
void add_symbol(Symbol* sym);
Symbol* lookup_symbol(const char* name);
Symbol* lookup_symbol_in_scope(Scope* scope, const char* name);

// Symbol getters
const char* symbol_type_name(SymbolType type);

// Symbol table debugging
void print_symbol_table();
void print_scope(Scope* scope, int indent);

// Cleanup
void free_symbol(Symbol* sym);
void free_scope(Scope* scope);

#endif
