use std::env;
use std::path::PathBuf;

fn main() {
    // Compile C code
    let mut build = cc::Build::new();
    
    // Add source files
    let src_dir = PathBuf::from("..").join("src");
    let include_dir = PathBuf::from("..").join("includes");
    let lib_dir = PathBuf::from("..").join("lib");
    
    build.file(src_dir.join("fel-protocol.c"));
    build.file(src_dir.join("usb_layer.c"));
    build.file(src_dir.join("fel-payloads.c"));
    build.file(src_dir.join("arch").join("riscv32_e907").join("fel-payloads.c"));
    
    // Add include directories
    build.include(&include_dir);
    
    // Add libusb header path
    let libusb_include_path = lib_dir.join("libusb").join("include");
    if libusb_include_path.exists() {
        build.include(libusb_include_path);
    }
    
    // Detect target system
    if cfg!(target_os = "windows") {
        // Windows platform configuration
        build.define("_WIN32", None);
        
        // Add libusb library path
        let libusb_lib_path = lib_dir.join("libusb").join("VS2022").join("MS64").join("static");
        if libusb_lib_path.exists() {
            println!("cargo:rustc-link-search=native={}", libusb_lib_path.display());
        }
        
        // Link libusb library
        println!("cargo:rustc-link-lib=static=libusb-1.0");
        println!("cargo:rustc-link-lib=static=ole32");
        println!("cargo:rustc-link-lib=static=setupapi");
    } else if cfg!(target_os = "linux") || cfg!(target_os = "macos") {
        // Linux/macOS platform configuration
        // Link system libusb library
        println!("cargo:rustc-link-lib=dylib=usb-1.0");
    }
    
    // Build C code
    build.compile("efex");
    
    // Generate Rust bindings
    let bindings = bindgen::Builder::default()
        .header(include_dir.join("libefex.h").to_str().unwrap())
        .header(include_dir.join("fel-protocol.h").to_str().unwrap())
        .header(include_dir.join("usb_layer.h").to_str().unwrap())
        .header(include_dir.join("fel-payloads.h").to_str().unwrap())
        .clang_arg(format!("-I{}", include_dir.to_str().unwrap()))
        .parse_callbacks(Box::new(bindgen::CargoCallbacks::new()))
        .generate()
        .expect("Unable to generate bindings");
    
    // Output bindings file
    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
    bindings
        .write_to_file(out_path.join("bindings.rs"))
        .expect("Couldn't write bindings!");
    
    // Tell cargo when to rebuild
    println!("cargo:rerun-if-changed=build.rs");
    println!("cargo:rerun-if-changed=../includes/");
    println!("cargo:rerun-if-changed=../src/");
}