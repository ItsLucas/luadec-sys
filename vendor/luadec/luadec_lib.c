#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lua.h"
#include "lauxlib.h"

#include "lfunc.h"
#include "lmem.h"
#include "lobject.h"
#include "lopcodes.h"
#include "lstring.h"
#include "lundump.h"

#include "lua-compat.h"
#include "proto.h"
#include "decompile.h"

// Forward declarations of globals from luadec.c
extern int debug;
extern int locals;
extern int localdeclare[255][255];
extern int functionnum;
extern int process_sub;
extern int func_check;
extern int string_encoding;
extern int guess_locals;
extern lua_State* glstate;
extern Proto* glproto;

// Forward declarations of functions from luadec.c and decompile.c
extern Proto* toproto(lua_State* L, int i);
extern Proto* combine(lua_State* L, int n);
extern void InitOperators(void);
extern int luaU_guess_locals(Proto * f, int main);
extern char* luadec_strdup(const char* src);

// Result structure for library interface
typedef struct {
    char* result;
    char* error;
} luadec_result_t;

// Initialize globals like main() does
static void init_luadec_globals(void) {
    int f, i;
    
    debug = 0;
    locals = 0;
    functionnum = 0;
    process_sub = 1;
    func_check = 0;
    guess_locals = 1;
    string_encoding = GBK; // Default encoding from original
    
    // Initialize localdeclare array
    for (f = 0; f < 255; f++) {
        for (i = 0; i < 255; i++) {
            localdeclare[f][i] = -1;
        }
    }
    
    InitOperators();
}

// Library function to decompile bytecode buffer
luadec_result_t* luadec_decompile_buffer(const char* bytecode, size_t size) {
    luadec_result_t* result = (luadec_result_t*)malloc(sizeof(luadec_result_t));
    result->result = NULL;
    result->error = NULL;
    
    if (!bytecode || size == 0) {
        result->error = strdup("Invalid bytecode buffer");
        return result;
    }
    
    // Initialize globals
    init_luadec_globals();
    
    // Create Lua state
    lua_State* L = lua_open();
    if (!L) {
        result->error = strdup("Failed to create Lua state");
        return result;
    }
    
    glstate = L;
    
    // Load bytecode from buffer
    int load_result = luaL_loadbuffer(L, bytecode, size, "=(luadec_lib)");
    if (load_result != 0) {
        const char* error_msg = lua_tostring(L, -1);
        if (error_msg) {
            result->error = strdup(error_msg);
        } else {
            result->error = strdup("Failed to load bytecode");
        }
        lua_close(L);
        glstate = NULL;
        return result;
    }
    
    // Get the Proto structure (using combine like main() does)
    Proto* f = combine(L, 1);  // We have 1 loaded chunk
    if (!f) {
        result->error = strdup("Failed to get Proto structure");
        lua_close(L);
        glstate = NULL;
        return result;
    }
    
    glproto = f;
    
    // Apply local variable guessing if enabled (like main() does)
    if (guess_locals) {
        luaU_guess_locals(f, 0);
    }
    
    // Use ProcessCode directly to get the decompiled string
    char* code = ProcessCode(f, 0, 0, luadec_strdup("0"));
    
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
void luadec_free_result(luadec_result_t* result) {
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
const char* luadec_get_result(luadec_result_t* result) {
    return result ? result->result : NULL;
}

// Get error string (returns NULL if there was no error)
const char* luadec_get_error(luadec_result_t* result) {
    return result ? result->error : NULL;
}