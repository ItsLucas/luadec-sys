# luadec-sys

[![Crates.io](https://img.shields.io/crates/v/luadec-sys.svg)](https://crates.io/crates/luadec-sys)
[![Documentation](https://docs.rs/luadec-sys/badge.svg)](https://docs.rs/luadec-sys)

Raw FFI bindings for [LuaDec](https://github.com/viruscamp/luadec), a Lua 5.1 bytecode decompiler.

This crate provides low-level unsafe bindings to the C library. For a safe, high-level API, use the [`luadec`](https://crates.io/crates/luadec) crate instead.

## Requirements

- C compiler (gcc/clang)
- make
- Lua 5.1 source code (included as git submodule)

## Platform Support

- Linux (tested)
- macOS (tested)
- Other Unix-like systems (should work)

## Safety

This crate is `unsafe` by design as it provides raw FFI bindings. All functions can cause undefined behavior if used incorrectly. Use the safe `luadec` wrapper crate instead.

## License

This project follows the same license as the original LuaDec project. Lua itself is licensed under the MIT license.