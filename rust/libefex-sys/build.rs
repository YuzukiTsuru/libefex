use std::env;
use std::fs;
use std::path::PathBuf;

fn find_latest_vs_version(libusb_dir: &PathBuf) -> Option<String> {
    let versions = ["VS2022", "VS2019", "VS2017", "VS2015", "VS2013"];
    for version in versions.iter() {
        let version_dir = libusb_dir.join(version);
        if version_dir.exists() {
            return Some(version.to_string());
        }
    }
    None
}

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
        src_dir.join("usb").join("usb_layer.c"),
        src_dir.join("usb").join("usb_layer_libusb.c"),
        src_dir.join("arch").join("aarch64.c"),
        src_dir.join("arch").join("arm.c"),
        src_dir.join("arch").join("riscv.c"),
    ];

    // Add winusb backend on Windows
    if cfg!(target_os = "windows") {
        c_files.push(src_dir.join("usb").join("usb_layer_winusb.c"));
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

        // Link libusb on Windows platform (for libusb backend support)
        // Determine toolchain and architecture
        let target_arch =
            env::var("CARGO_CFG_TARGET_ARCH").unwrap_or_else(|_| "x86_64".to_string());
        let target_env = env::var("CARGO_CFG_TARGET_ENV").unwrap_or_else(|_| "msvc".to_string());

        let libusb_lib_dir = if target_env == "msvc" {
            // MSVC toolchain
            let libusb_dir = root_dir.join("lib").join("libusb");
            let vs_version = find_latest_vs_version(&libusb_dir).unwrap_or_else(|| {
                println!("cargo:warning=No VS version found, defaulting to VS2022");
                "VS2022".to_string()
            });
            let arch_dir = if target_arch == "x86_64" {
                "MS64"
            } else {
                "MS32"
            };
            libusb_dir.join(vs_version).join(arch_dir).join("dll")
        } else {
            // MinGW toolchain
            let arch_dir = match target_arch.as_str() {
                "x86_64" => "MinGW64",
                "i686" | "x86" => "MinGW32",
                "aarch64" => "MinGW-llvm-aarch64",
                _ => "MinGW64",
            };
            root_dir
                .join("lib")
                .join("libusb")
                .join(arch_dir)
                .join("dll")
        };

        println!(
            "cargo:warning=Target arch: {}, env: {}",
            target_arch, target_env
        );
        println!("cargo:warning=LibUSB lib dir: {:?}", libusb_lib_dir);

        println!("cargo:rustc-link-search={}", libusb_lib_dir.display());
        println!("cargo:rustc-link-lib=dylib=libusb-1.0");
        println!("cargo:rustc-link-lib=user32");
        println!("cargo:rustc-link-lib=kernel32");
        println!("cargo:rustc-link-lib=advapi32");
        println!("cargo:rustc-link-lib=shell32");
        println!("cargo:rustc-link-lib=setupapi");

        // Ensure these libraries are linked in downstream crates
        println!("cargo:rustc-link-arg=libusb-1.0.lib");
        println!("cargo:rustc-link-arg=user32.lib");
        println!("cargo:rustc-link-arg=kernel32.lib");
        println!("cargo:rustc-link-arg=advapi32.lib");
        println!("cargo:rustc-link-arg=shell32.lib");
        println!("cargo:rustc-link-arg=setupapi.lib");

        // Copy libusb DLL to output directory
        let dll_name = "libusb-1.0.dll";
        let src_dll = libusb_lib_dir.join(dll_name);

        println!("cargo:warning=Source DLL path: {:?}", src_dll);
        println!("cargo:warning=Source DLL exists: {}", src_dll.exists());

        // Get the output directory where the DLL should be copied
        let out_dir = PathBuf::from(env::var("OUT_DIR").expect("OUT_DIR not set"));
        let target_dir = out_dir
            .ancestors()
            .nth(3)
            .expect("Failed to find target directory");
        let dest_dll = target_dir.join(dll_name);

        println!("cargo:warning=Target DLL path: {:?}", dest_dll);

        // Create target directory if it doesn't exist
        if let Some(dest_dir) = dest_dll.parent() {
            fs::create_dir_all(dest_dir).unwrap_or_else(|e| {
                println!(
                    "cargo:warning=Failed to create directory {:?}: {}",
                    dest_dir, e
                );
            });
        }

        // Copy the DLL
        if src_dll.exists() {
            match fs::copy(&src_dll, &dest_dll) {
                Ok(_) => println!("cargo:warning=Copied {} to {:?}", dll_name, dest_dll),
                Err(e) => println!("cargo:warning=Failed to copy {}: {}", dll_name, e),
            }
        } else {
            println!("cargo:warning=Source DLL not found: {:?}", src_dll);
        }
    } else if cfg!(target_os = "macos") {
        // macOS configuration
        builder.include(include_dir).files(c_files).warnings(false);

        let target_arch = env::var("CARGO_CFG_TARGET_ARCH").unwrap_or_else(|_| "aarch64".to_string());
        let homebrew_prefix = if target_arch == "aarch64" {
            "/opt/homebrew"
        } else {
            "/usr/local"
        };

        // Add include paths
        builder.include(format!("{}/include", homebrew_prefix));
        builder.include(format!("{}/include/libusb-1.0", homebrew_prefix));

        // Link libusb
        println!("cargo:rustc-link-search={}/lib", homebrew_prefix);
        println!("cargo:rustc-link-lib=dylib=usb-1.0");
    } else {
        // Linux configuration
        builder.include(include_dir).files(c_files).warnings(false);

        // Use pkg-config to find libusb on Linux platforms
        let libusb = pkg_config::Config::new()
            .probe("libusb-1.0")
            .expect("Failed to find libusb-1.0");

        // Add libusb include paths from pkg-config
        for include in libusb.include_paths {
            builder.include(include);
        }
    }

    builder.compile("efex");

    // Output link information
    println!("cargo:rustc-link-lib=static=efex");
}
