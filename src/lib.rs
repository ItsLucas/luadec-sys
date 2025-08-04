//! Raw FFI bindings for LuaDec
//!
//! This crate provides low-level FFI bindings to the LuaDec C library.
//! For a high-level safe API, use the `luadec` crate instead.

#![allow(non_camel_case_types)]
#![allow(non_snake_case)]
#![allow(non_upper_case_globals)]

use libc::{c_char, size_t};

/// Opaque structure representing the decompile result from C
#[repr(C)]
pub struct DecompileResult {
    _private: [u8; 0],
}

extern "C" {
    /// Decompile bytecode from a buffer
    pub fn luadec_decompile_buffer(bytecode: *const c_char, size: size_t) -> *mut DecompileResult;
    
    /// Free the result structure
    pub fn luadec_free_result(result: *mut DecompileResult);
    
    /// Get the decompiled result string (NULL if error occurred)
    pub fn luadec_get_result(result: *const DecompileResult) -> *const c_char;
    
    /// Get the error string (NULL if no error)
    pub fn luadec_get_error(result: *const DecompileResult) -> *const c_char;
}