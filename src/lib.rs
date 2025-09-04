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
pub struct luadec_result_t {
    _private: [u8; 0],
}

extern "C" {
    /// Decompile bytecode from a buffer
    pub fn luadec_decompile_buffer(bytecode: *const c_char, size: size_t) -> *mut luadec_result_t;
    
    /// Free the result structure
    pub fn luadec_free_result(result: *mut luadec_result_t);
    
    /// Get the decompiled result string (NULL if error occurred)
    pub fn luadec_get_result(result: *const luadec_result_t) -> *const c_char;
    
    /// Get the error string (NULL if no error)
    pub fn luadec_get_error(result: *const luadec_result_t) -> *const c_char;
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_decompile_buffer() {
        // This is just a placeholder test to ensure the bindings compile
        unsafe {
            // Read test2.lua
            let result = std::fs::read("test2.lua").unwrap();
            let bytecode = result.as_ptr() as *const c_char;
            let size: size_t = result.len() as size_t;
            let result = luadec_decompile_buffer(bytecode, size);
            assert!(!result.is_null());
            luadec_free_result(result);
        }
    }
}