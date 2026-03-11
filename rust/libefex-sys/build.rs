use std::env;
use std::fs;
use std::path::PathBuf;

pub fn link_framework(name: &str) {
    println!("cargo:rustc-link-lib=framework={}", name);
}

fn build_libusb_cmake(libusb_cmake_dir: &PathBuf) -> PathBuf {
    let mut cmake_config = cmake::Config::new(libusb_cmake_dir);
    cmake_config
        .define("LIBUSB_BUILD_SHARED_LIBS", "ON")
        .define("LIBUSB_BUILD_TESTING", "OFF")
        .define("LIBUSB_BUILD_EXAMPLES", "OFF")
        .define("LIBUSB_ENABLE_LOGGING", "ON");

    // Platform-specific options
    if std::env::var("CARGO_CFG_TARGET_OS") == Ok("linux".into()) {
        cmake_config.define("LIBUSB_ENABLE_UDEV", "ON");
    }

    let dst = cmake_config.build();

    // The library output location depends on the platform and generator
    let lib_dir = dst.join("lib");
    if lib_dir.exists() {
        println!("cargo:rustc-link-search=native={}", lib_dir.display());
    }

    // Also check for multi-config generators (like Visual Studio)
    for config in &["Release", "Debug", ""] {
        let config_dir = if config.is_empty() {
            dst.join("lib")
        } else {
            dst.join("lib").join(config)
        };
        if config_dir.exists() {
            println!("cargo:rustc-link-search=native={}", config_dir.display());
        }
    }

    // Link the library
    println!("cargo:rustc-link-lib=dylib=usb-1.0");

    dst
}

fn copy_dll_to_output(dll_path: &PathBuf) {
    let out_dir = PathBuf::from(env::var("OUT_DIR").expect("OUT_DIR not set"));
    let target_dir = out_dir
        .ancestors()
        .nth(3)
        .expect("Failed to find target directory");

    if let Some(dll_name) = dll_path.file_name() {
        let dest_path = target_dir.join(dll_name);
        if dll_path.exists() {
            let _ = fs::copy(dll_path, &dest_path);
        }
    }
}

fn find_built_dll(build_dir: &PathBuf) -> Option<PathBuf> {
    let search_paths = vec![
        build_dir.join("lib").join("libusb-1.0.dll"),
        build_dir.join("lib").join("Release").join("libusb-1.0.dll"),
        build_dir.join("lib").join("Debug").join("libusb-1.0.dll"),
        build_dir.join("bin").join("libusb-1.0.dll"),
        build_dir.join("bin").join("Release").join("libusb-1.0.dll"),
        build_dir.join("bin").join("Debug").join("libusb-1.0.dll"),
    ];

    for path in search_paths {
        if path.exists() {
            return Some(path);
        }
    }
    None
}

fn build_libefex(include_dir: &PathBuf, src_dir: &PathBuf, libusb_include: &PathBuf) {
    let mut c_files = vec![
        src_dir.join("efex-common.c"),
        src_dir.join("efex-fel.c"),
        src_dir.join("efex-fes.c"),
        src_dir.join("efex-payloads.c"),
        src_dir.join("efex-usb.c"),
        src_dir.join("usb/usb_layer.c"),
        src_dir.join("usb/usb_layer_libusb.c"),
        src_dir.join("arch/aarch64.c"),
        src_dir.join("arch/arm.c"),
        src_dir.join("arch/riscv.c"),
    ];

    if std::env::var("CARGO_CFG_TARGET_OS") == Ok("windows".into()) {
        c_files.push(src_dir.join("usb/usb_layer_winusb.c"));
    }

    let mut builder = cc::Build::new();
    builder.include(include_dir);
    builder.include(libusb_include);
    builder.files(&c_files);

    if std::env::var("CARGO_CFG_TARGET_OS") == Ok("windows".into()) {
        builder.define("_CRT_SECURE_NO_WARNINGS", None);
        builder.warnings(false);
        println!("cargo:rustc-link-lib=user32");
        println!("cargo:rustc-link-lib=kernel32");
        println!("cargo:rustc-link-lib=advapi32");
        println!("cargo:rustc-link-lib=shell32");
        println!("cargo:rustc-link-lib=setupapi");
    }

    builder.compile("efex");
    println!("cargo:rustc-link-lib=static=efex");
}

fn main() {
    let current_dir = env::current_dir().expect("Failed to get current directory");
    let root_dir = current_dir
        .parent()
        .unwrap()
        .parent()
        .expect("Failed to get project root directory");

    let include_dir = root_dir.join("includes");
    let src_dir = root_dir.join("src");
    let libusb_cmake_dir = root_dir.join("lib").join("libusb-cmake");
    let libusb_include = libusb_cmake_dir.join("libusb").join("libusb");

    // Build libusb from source using CMake
    let build_dir = build_libusb_cmake(&libusb_cmake_dir);

    // Find and copy DLL on Windows
    if std::env::var("CARGO_CFG_TARGET_OS") == Ok("windows".into()) {
        if let Some(dll_path) = find_built_dll(&build_dir) {
            copy_dll_to_output(&dll_path);
        }
    }

    // Build libefex
    build_libefex(&include_dir, &src_dir, &libusb_include);
}