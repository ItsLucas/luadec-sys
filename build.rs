use std::env;

fn main() {
    let target_os = env::var("CARGO_CFG_TARGET_OS").unwrap();
    let manifest_dir = env::var("CARGO_MANIFEST_DIR").unwrap();
    
    println!("cargo:rerun-if-changed=vendor/lua-5.1");
    println!("cargo:rerun-if-changed=vendor/luadec");
    println!("cargo:rerun-if-changed=src/wrapper.c");
    
    let lua_src_dir = format!("{}/vendor/lua-5.1/src", manifest_dir);
    let luadec_src_dir = format!("{}/vendor/luadec", manifest_dir);
    
    // Platform-specific compile flags for Lua
    let mut lua_cflags = vec!["-O2", "-Wall"];
    match target_os.as_str() {
        "macos" => {
            lua_cflags.push("-DLUA_USE_MACOSX");
        }
        "linux" => {
            lua_cflags.push("-DLUA_USE_LINUX");
        }
        _ => {
            lua_cflags.push("-DLUA_USE_POSIX");
        }
    }
    
    // Build Lua 5.1 library
    let mut lua_build = cc::Build::new();
    lua_build
        .include(&lua_src_dir)
        .define("LUAVER", "5.1");
        
    for flag in &lua_cflags {
        lua_build.flag(flag);
    }
    
    // Core Lua source files
    let lua_sources = [
        "lapi.c", "lcode.c", "ldebug.c", "ldo.c", "ldump.c", "lfunc.c",
        "lgc.c", "llex.c", "lmem.c", "lobject.c", "lopcodes.c", "lparser.c",
        "lstate.c", "lstring.c", "ltable.c", "ltm.c", "lundump.c", "lvm.c",
        "lzio.c", "lauxlib.c", "lbaselib.c", "ldblib.c", "liolib.c",
        "lmathlib.c", "loslib.c", "ltablib.c", "lstrlib.c", "loadlib.c", "linit.c"
    ];
    
    for source in &lua_sources {
        lua_build.file(format!("{}/{}", lua_src_dir, source));
    }
    
    lua_build.compile("lua");
    
    // Build LuaDec library
    let mut luadec_build = cc::Build::new();
    luadec_build
        .include(&lua_src_dir)
        .include(&luadec_src_dir)
        .define("LUAVER", "5.1");
        
    for flag in &lua_cflags {
        luadec_build.flag(flag);
    }
    
    // LuaDec source files
    let luadec_sources = [
        "decompile.c", "guess.c", "disassemble.c", "proto.c",
        "StringBuffer.c", "structs.c", "statement.c", 
        "macro-array.c", "expression.c", "lundump-5.1.c"
    ];
    
    for source in &luadec_sources {
        luadec_build.file(format!("{}/{}", luadec_src_dir, source));
    }
    
    luadec_build.compile("luadec");
    
    // Build our C wrapper
    cc::Build::new()
        .file("src/wrapper.c")
        .include(&lua_src_dir)
        .include(&luadec_src_dir)
        .define("LUAVER", "5.1")
        .compile("luadec_wrapper");
    
    // Link system libraries
    match target_os.as_str() {
        "macos" => {
            println!("cargo:rustc-link-lib=dylib=c");
        }
        "linux" => {
            println!("cargo:rustc-link-lib=dylib=m");
            println!("cargo:rustc-link-lib=dylib=dl");
        }
        _ => {}
    }
}