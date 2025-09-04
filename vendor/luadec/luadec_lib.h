#ifndef LUADEC_LIB_H
#define LUADEC_LIB_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Opaque result structure
typedef struct luadec_result_t luadec_result_t;

// Main decompilation function
// Takes bytecode buffer and returns result structure
// Caller must free the result with luadec_free_result()
luadec_result_t* luadec_decompile_buffer(const char* bytecode, size_t size);

// Free the result structure and all associated memory
void luadec_free_result(luadec_result_t* result);

// Get decompiled Lua source code (returns NULL if there was an error)
const char* luadec_get_result(luadec_result_t* result);

// Get error message (returns NULL if there was no error)
const char* luadec_get_error(luadec_result_t* result);

#ifdef __cplusplus
}
#endif

#endif // LUADEC_LIB_H