// ============================================================
// symbol.c - Symbol Table Implementation
// ============================================================

#include "symbol.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================
// Global variables
// ============================================================

Scope* global_scope = NULL;
Scope* current_scope = NULL;
// static int next_scope_level = 0;

// ============================================================
// Scope management
// ============================================================

Scope* create_scope(Scope* parent, char* name) {
    Scope* scope = (Scope*)calloc(1, sizeof(Scope));
    scope->parent = parent;
    scope->level = parent ? parent->level + 1 : 0;
    scope->name = name ? strdup(name) : strdup("anonymous");
    scope->symbols = NULL;
    return scope;
}

void push_scope(char* name) {
    Scope* new_scope = create_scope(current_scope, name);
    if (!global_scope) {
        global_scope = new_scope;
    }
    current_scope = new_scope;
}

void pop_scope() {
    if (current_scope && current_scope->parent) {
        current_scope = current_scope->parent;
    } else if (current_scope) {
        // Don't pop global scope
    }
}

Scope* get_current_scope() {
    return current_scope;
}

// ============================================================
// Symbol creation
// ============================================================

Symbol* create_symbol(char* name, SymbolType type, DataType data_type) {
    Symbol* sym = (Symbol*)calloc(1, sizeof(Symbol));
    sym->name = strdup(name);
    sym->sym_type = type;
    sym->data_type = data_type;
    sym->scope_level = current_scope ? current_scope->level : 0;
    sym->is_initialized = false;
    sym->is_mutable = true;
    sym->is_volatile = false;
    sym->array_size = 0;
    sym->fixed_w = 0;
    sym->fixed_s = 0;
    sym->params = NULL;
    sym->return_type = TYPE_UNKNOWN;
    sym->bounded_cycles = 0;
    sym->extern_lang = NULL;
    sym->pin = NULL;
    sym->protocol = NULL;
    sym->poll_ms = 100; // Default
    sym->has_range = false;
    sym->range_start = NULL;
    sym->range_end = NULL;
    sym->fields = NULL;
    sym->states = NULL;
    sym->initial_state = NULL;
    sym->next = NULL;
    sym->line = 0;
    sym->column = 0;
    return sym;
}

void add_symbol(Symbol* sym) {
    if (!current_scope) {
        // Create global scope if it doesn't exist
        push_scope("global");
    }
    
    // Check if symbol already exists in this scope
    Symbol* existing = lookup_symbol_in_scope(current_scope, sym->name);
    if (existing) {
        fprintf(stderr, "ERROR: Symbol '%s' already declared in this scope\n", sym->name);
        return;
    }
    
    // Add to current scope
    sym->next = current_scope->symbols;
    current_scope->symbols = sym;
}

Symbol* lookup_symbol(const char* name) {
    Scope* scope = current_scope;
    while (scope) {
        Symbol* sym = lookup_symbol_in_scope(scope, name);
        if (sym) return sym;
        scope = scope->parent;
    }
    return NULL;
}

Symbol* lookup_symbol_in_scope(Scope* scope, const char* name) {
    Symbol* sym = scope->symbols;
    while (sym) {
        if (strcmp(sym->name, name) == 0) {
            return sym;
        }
        sym = sym->next;
    }
    return NULL;
}

// ============================================================
// Symbol getters
// ============================================================

const char* symbol_type_name(SymbolType type) {
    switch (type) {
        case SYM_VAR: return "variable";
        case SYM_LET: return "immutable";
        case SYM_SENSOR: return "sensor";
        case SYM_OUTPUT: return "output";
        case SYM_FN: return "function";
        case SYM_MACHINE: return "state machine";
        case SYM_STATE: return "state";
        case SYM_PAYLOAD: return "payload";
        case SYM_PARAM: return "parameter";
        case SYM_EXTERN: return "external";
        default: return "unknown";
    }
}

// ============================================================
// Symbol table debugging
// ============================================================

void print_symbol_table() {
    printf("\n========================================\n");
    printf("SYMBOL TABLE\n");
    printf("========================================\n");
    if (global_scope) {
        print_scope(global_scope, 0);
    }
    printf("========================================\n\n");
}

void print_scope(Scope* scope, int indent) {
    if (!scope) return;
    
    char indent_str[128];
    for (int i = 0; i < indent; i++) {
        indent_str[i] = ' ';
    }
    indent_str[indent] = '\0';
    
    printf("%sScope: %s (level %d)\n", indent_str, scope->name, scope->level);
    
    Symbol* sym = scope->symbols;
    while (sym) {
        printf("%s  %s: %s (%s)", indent_str, sym->name, 
               data_type_name(sym->data_type),
               symbol_type_name(sym->sym_type));
        
        if (sym->array_size > 0) {
            printf(" [%d]", sym->array_size);
        }
        if (sym->sym_type == SYM_SENSOR && sym->pin) {
            printf(" pin=%s", sym->pin);
            if (sym->poll_ms > 0) {
                printf(" poll=%dms", sym->poll_ms);
            }
        }
        if (sym->sym_type == SYM_OUTPUT && sym->pin) {
            printf(" pin=%s", sym->pin);
        }
        if (sym->sym_type == SYM_FN || sym->sym_type == SYM_EXTERN) {
            printf(" -> %s", data_type_name(sym->return_type));
            if (sym->bounded_cycles > 0) {
                printf(" bounded=%d", sym->bounded_cycles);
            }
        }
        printf("\n");
        sym = sym->next;
    }
    
    // Recursively print child scopes (if any)
    // Note: We don't store child scopes in this simple implementation
}

// ============================================================
// Cleanup
// ============================================================

void free_symbol(Symbol* sym) {
    if (!sym) return;
    if (sym->name) free(sym->name);
    if (sym->pin) free(sym->pin);
    if (sym->protocol) free(sym->protocol);
    if (sym->extern_lang) free(sym->extern_lang);
    // Note: params, fields, states are Symbol* chains - would need recursive free
    free(sym);
}

void free_scope(Scope* scope) {
    if (!scope) return;
    if (scope->name) free(scope->name);
    
    Symbol* sym = scope->symbols;
    while (sym) {
        Symbol* next = sym->next;
        free_symbol(sym);
        sym = next;
    }
    
    free(scope);
}
