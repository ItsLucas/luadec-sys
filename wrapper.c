#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "lobject.h"
#include "lopcodes.h"
#include "lfunc.h"
#include "lmem.h"
#include "lstring.h"
#include "lundump.h"
#include "lstate.h"

#include "decompile.h"
#include "proto.h"
#include "StringBuffer.h"
#include "structs.h"

// Global variables from luadec.c that we need to define
int debug = 0;
int locals = 0; 
int localdeclare[255][255];
int functionnum = 0;
int process_sub = 1;
int func_check = 0;
int guess_locals = 1;
int string_encoding = 0; // ASCII encoding
lua_State* glstate = NULL;
Proto* glproto = NULL;
StringBuffer* errorStr = NULL;

// Function declarations from luadec.c
extern void InitOperators(void);
extern char* ProcessCode(Proto* f, int indent, int func_checking, char* funcnumstr);
extern char* luadec_strdup(const char* s);

// Functions from luadec.c that we need to define
Proto* toproto(lua_State* L, int i) {
    // Get the closure from the stack and extract its Proto
    const TValue* o = L->base + i - 1;  // Lua uses 1-based indexing
    return clvalue(o)->l.p;
}

Proto* combine(lua_State* L, int n) {
    if (n == 1) {
        Proto* f = toproto(L, -1);
        return f;
    } else {
        // For multiple chunks, we'd need to create a combined proto
        // For now, just handle single chunk case
        return toproto(L, -1);
    }
}

int printFileNames(FILE* out) {
    // This function is used for debug output in the original
    // For our wrapper, we can just return 0
    return 0;
}

typedef struct {
    char* result;
    char* error;
} DecompileResult;

// Initialize global state like luadec.c does
static void init_globals(void) {
    int f, i;
    
    debug = 0;
    locals = 0;
    functionnum = 0;
    process_sub = 1;
    func_check = 0;
    guess_locals = 1;  // Enable local variable guessing by default
    
    // Initialize localdeclare array
    for (f = 0; f < 2; f++) {
        for (i = 0; i < 255; i++) {
            localdeclare[f][i] = -1;
        }
    }
    
    InitOperators();
}

// Main decompilation function that takes bytecode buffer
DecompileResult* luadec_decompile_buffer(const char* bytecode, size_t size) {
    DecompileResult* result = (DecompileResult*)malloc(sizeof(DecompileResult));
    result->result = NULL;
    result->error = NULL;
    
    if (!bytecode || size == 0) {
        result->error = strdup("Invalid bytecode buffer");
        return result;
    }
    
    // Initialize global state
    init_globals();
    
    // Create Lua state
    lua_State* L = lua_open();
    if (!L) {
        result->error = strdup("Failed to create Lua state");
        return result;
    }
    
    glstate = L;
    
    // Load bytecode from buffer
    int load_result = luaL_loadbuffer(L, bytecode, size, "=(luadec)");
    if (load_result != 0) {
        const char* error_msg = lua_tostring(L, -1);
        if (error_msg) {
            result->error = strdup(error_msg);
        } else {
            result->error = strdup("Failed to load bytecode");
        }
        lua_close(L);
        return result;
    }
    
    // Get the Proto structure
    Proto* f = combine(L, 1);  // We have 1 loaded chunk
    if (!f) {
        result->error = strdup("Failed to get Proto structure");
        lua_close(L);
        return result;
    }
    
    glproto = f;
    
    // Initialize error string buffer
    errorStr = StringBuffer_new(NULL);
    
    // Perform decompilation
    char* code = ProcessCode(f, 0, 0, luadec_strdup("0"));
    
    // Clean up error buffer
    StringBuffer_delete(errorStr);
    errorStr = NULL;
    
    if (code) {
        result->result = strdup(code);
        free(code);
    } else {
        result->error = strdup("Decompilation failed");
    }
    
    // Clean up
    lua_close(L);
    glstate = NULL;
    glproto = NULL;
    
    return result;
}

// Free the result structure
void luadec_free_result(DecompileResult* result) {
    if (result) {
        if (result->result) {
            free(result->result);
        }
        if (result->error) {
            free(result->error);
        }
        free(result);
    }
}

// Get result string (returns NULL if there was an error)
const char* luadec_get_result(DecompileResult* result) {
    return result ? result->result : NULL;
}

// Get error string (returns NULL if there was no error)
const char* luadec_get_error(DecompileResult* result) {
    return result ? result->error : NULL;
}