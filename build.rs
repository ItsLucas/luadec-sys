use std::env;
use std::process::Command;

fn main() {
    let target_os = env::var("CARGO_CFG_TARGET_OS").unwrap();
    let manifest_dir = env::var("CARGO_MANIFEST_DIR").unwrap();
    
    let lua_dir = format!("{}/vendor/lua-5.1", manifest_dir);
    let luadec_dir = format!("{}/vendor/luadec", manifest_dir);
    let _bin_dir = format!("{}/vendor/bin", manifest_dir);
    let src_dir = format!("{}/src", lua_dir);
    
    println!("cargo:rerun-if-changed=vendor/lua-5.1");
    println!("cargo:rerun-if-changed=vendor/luadec");
    println!("cargo:rerun-if-changed=src/wrapper.c");
    
    // Determine the platform-specific make target for Lua
    let lua_platform = match target_os.as_str() {
        "macos" => "macosx",
        "linux" => "linux",
        _ => "generic",
    };
    
    // Build Lua 5.1 first
    println!("Building Lua 5.1 for platform: {}", lua_platform);
    let lua_build = Command::new("make")
        .current_dir(&lua_dir)
        .arg(lua_platform)
        .status()
        .expect("Failed to build Lua 5.1");
    
    if !lua_build.success() {
        panic!("Failed to build Lua 5.1");
    }
    
    // Build luadec
    println!("Building luadec");
    let luadec_build = Command::new("make")
        .current_dir(&luadec_dir)
        .arg("LUAVER=5.1")
        .status()
        .expect("Failed to build luadec");
    
    if !luadec_build.success() {
        panic!("Failed to build luadec");
    }
    
    // Create our C wrapper
    cc::Build::new()
        .file("src/wrapper.c")
        .include(&src_dir)
        .include(&luadec_dir)
        .define("LUAVER", "5.1")
        .compile("luadec_wrapper");
    
    // Link against the built libraries
    println!("cargo:rustc-link-search=native={}/src", lua_dir);
    println!("cargo:rustc-link-lib=static=lua");
    
    // Create a static library from luadec object files
    let mut lib_builder = cc::Build::new();
    let luadec_objects = [
        "luadec.o", "guess.o", "decompile.o", "disassemble.o", "proto.o", 
        "StringBuffer.o", "structs.o", "statement.o", 
        "macro-array.o", "expression.o"
    ];
    
    for obj in &luadec_objects {
        lib_builder.object(format!("{}/{}", luadec_dir, obj));
    }
    
    lib_builder.compile("luadec_objects");
    
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