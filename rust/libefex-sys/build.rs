use std::env;

fn main() {
    // Get current directory
    let current_dir = env::current_dir().expect("Failed to get current directory");
    println!("cargo:warning=Current directory: {:?}", current_dir);

    // Get project root directory (need to go up two levels: libefex-sys -> rust -> project root)
    let root_dir = current_dir
        .parent()
        .unwrap()
        .parent()
        .expect("Failed to get project root directory");
    println!("cargo:warning=Root directory: {:?}", root_dir);

    // Set include paths
    let include_dir = root_dir.join("includes");
    let src_dir = root_dir.join("src");
    let libusb_include_dir = root_dir.join("lib").join("libusb").join("include");

    println!("cargo:warning=Include directory: {:?}", include_dir);
    println!("cargo:warning=Source directory: {:?}", src_dir);
    println!(
        "cargo:warning=LibUSB include directory: {:?}",
        libusb_include_dir
    );

    // Collect all C source files
    let mut c_files = vec![
        src_dir.join("efex-common.c"),
        src_dir.join("efex-fel.c"),
        src_dir.join("efex-fes.c"),
        src_dir.join("efex-payloads.c"),
        src_dir.join("efex-usb.c"),
        src_dir.join("arch").join("aarch64.c"),
        src_dir.join("arch").join("arm.c"),
        src_dir.join("arch").join("riscv.c"),
    ];

    // Add USB layer based on platform
    if cfg!(target_os = "windows") {
        c_files.push(src_dir.join("usb").join("winusb_layer.c"));
    } else {
        c_files.push(src_dir.join("usb").join("libusb_layer.c"));
    }

    // Compile C code
    let mut builder = cc::Build::new();

    // Platform-specific configuration
    if cfg!(target_os = "windows") {
        // Windows-specific configuration
        builder
            .include(include_dir)
            .include(libusb_include_dir)
            .files(c_files)
            .define("_CRT_SECURE_NO_WARNINGS", None)
            .warnings(false);

        // Link libusb on Windows platform
        let libusb_lib_dir = root_dir
            .join("lib")
            .join("libusb")
            .join("VS2022")
            .join("MS64")
            .join("static");
        println!("cargo:rustc-link-search={}", libusb_lib_dir.display());
        println!("cargo:rustc-link-lib=static=libusb-1.0");
        println!("cargo:rustc-link-lib=user32");
        println!("cargo:rustc-link-lib=kernel32");
        println!("cargo:rustc-link-lib=advapi32");
        println!("cargo:rustc-link-lib=shell32");
        println!("cargo:rustc-link-lib=setupapi");
    } else {
        // Linux/macOS configuration
        builder
            .include(include_dir)
            .files(c_files)
            .warnings(false);

        // Use pkg-config to find libusb on Linux/macOS platforms
        pkg_config::Config::new()
            .probe("libusb-1.0")
            .expect("Failed to find libusb-1.0");
    }

    builder.compile("efex");

    // Output link information
    println!("cargo:rustc-link-lib=static=efex");
}
