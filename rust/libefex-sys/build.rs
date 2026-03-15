use std::env;
use std::fs;
use std::path::PathBuf;

pub fn link_framework(name: &str) {
    println!("cargo:rustc-link-lib=framework={}", name);
}

fn is_cross_compiling() -> bool {
    let host = env::var("HOST").unwrap_or_default();
    let target = env::var("TARGET").unwrap_or_default();
    host != target
}

fn find_system_libusb() -> bool {
    if is_cross_compiling() {
        return false;
    }

    match pkg_config::probe_library("libusb-1.0") {
        Ok(lib) => {
            println!("cargo:rustc-link-lib=dylib=usb-1.0");
            for path in lib.include_paths {
                println!("cargo:include={}", path.to_str().unwrap());
            }
            println!("cargo:version_number={}", lib.version);
            true
        }
        Err(e) => {
            println!("cargo:warning=Could not find system libusb: {:?}", e);
            false
        }
    }
}

fn build_libusb_static(libusb_cmake_dir: &PathBuf) {
    let libusb_source = libusb_cmake_dir.join("libusb").join("libusb");

    println!("cargo:rerun-if-env-changed=LIBUSB_STATIC");
    println!("cargo:static=1");
    println!("cargo:vendored=1");

    let out_include = PathBuf::from(env::var("OUT_DIR").unwrap()).join("include");
    fs::create_dir_all(&out_include).unwrap();
    fs::copy(libusb_source.join("libusb.h"), out_include.join("libusb.h")).ok();

    let config_h_path = out_include.join("config.h");
    fs::File::create(&config_h_path).unwrap();

    let mut base_config = cc::Build::new();
    base_config.include(&out_include);
    base_config.include(&libusb_source);
    base_config.include(&libusb_source.join("os"));

    base_config.define("PRINTF_FORMAT(a, b)", Some(""));
    base_config.define("ENABLE_LOGGING", Some("1"));

    let target_os = env::var("CARGO_CFG_TARGET_OS").unwrap_or_default();
    let target_env = env::var("CARGO_CFG_TARGET_ENV").unwrap_or_default();
    let target_family = env::var("CARGO_CFG_TARGET_FAMILY").unwrap_or_default();

    if target_env == "msvc" {
        let msvc_config_h = libusb_cmake_dir
            .join("libusb")
            .join("msvc")
            .join("config.h");
        if msvc_config_h.exists() {
            fs::copy(&msvc_config_h, &config_h_path).unwrap();
        }
        base_config.flag("/source-charset:utf-8");
    }

    if target_os == "macos" {
        base_config.define("OS_DARWIN", Some("1"));
        base_config.define("TARGET_OS_OSX", Some("1"));
        base_config.file(libusb_source.join("os/darwin_usb.c"));
        link_framework("CoreFoundation");
        link_framework("IOKit");
        link_framework("Security");
        println!("cargo:rustc-link-lib=dylib=objc");
    }

    if target_os == "linux" || target_os == "android" {
        base_config.define("OS_LINUX", Some("1"));
        base_config.define("HAVE_ASM_TYPES_H", Some("1"));
        base_config.define("_GNU_SOURCE", Some("1"));
        base_config.define("HAVE_TIMERFD", Some("1"));
        base_config.define("HAVE_EVENTFD", Some("1"));
        base_config.file(libusb_source.join("os/linux_netlink.c"));
        base_config.file(libusb_source.join("os/linux_usbfs.c"));
    }

    if target_family == "unix" {
        base_config.define("HAVE_SYS_TIME_H", Some("1"));
        base_config.define("HAVE_NFDS_T", Some("1"));
        base_config.define("PLATFORM_POSIX", Some("1"));
        base_config.define("HAVE_CLOCK_GETTIME", Some("1"));
        base_config.define(
            "DEFAULT_VISIBILITY",
            Some("__attribute__((visibility(\"default\")))"),
        );

        base_config.file(libusb_source.join("os/events_posix.c"));
        base_config.file(libusb_source.join("os/threads_posix.c"));
    }

    if target_os == "windows" {
        if target_env == "msvc" {
            base_config.flag("/source-charset:utf-8");
        }

        base_config.warnings(false);
        base_config.define("OS_WINDOWS", Some("1"));
        base_config.file(libusb_source.join("os/events_windows.c"));
        base_config.file(libusb_source.join("os/threads_windows.c"));
        base_config.file(libusb_source.join("os/windows_common.c"));
        base_config.file(libusb_source.join("os/windows_usbdk.c"));
        base_config.file(libusb_source.join("os/windows_winusb.c"));

        base_config.define("DEFAULT_VISIBILITY", Some(""));
        base_config.define("PLATFORM_WINDOWS", Some("1"));
        println!("cargo:rustc-link-lib=dylib=user32");
    }

    base_config.file(libusb_source.join("core.c"));
    base_config.file(libusb_source.join("descriptor.c"));
    base_config.file(libusb_source.join("hotplug.c"));
    base_config.file(libusb_source.join("io.c"));
    base_config.file(libusb_source.join("strerror.c"));
    base_config.file(libusb_source.join("sync.c"));

    base_config.compile("usb-1.0-static");
    println!("cargo:rustc-link-lib=static=usb-1.0-static");
}

fn build_libusb_cmake(libusb_cmake_dir: &PathBuf) -> PathBuf {
    let mut cmake_config = cmake::Config::new(libusb_cmake_dir);
    cmake_config
        .define("LIBUSB_BUILD_SHARED_LIBS", "ON")
        .define("LIBUSB_BUILD_TESTING", "OFF")
        .define("LIBUSB_BUILD_EXAMPLES", "OFF")
        .define("LIBUSB_ENABLE_LOGGING", "ON");

    if env::var("CARGO_CFG_TARGET_OS") == Ok("linux".into()) {
        cmake_config.define("LIBUSB_ENABLE_UDEV", "ON");
    }

    let dst = cmake_config.build();

    let lib_dir = dst.join("lib");
    if lib_dir.exists() {
        println!("cargo:rustc-link-search=native={}", lib_dir.display());
    }

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

fn find_built_dylib(build_dir: &PathBuf) -> Option<PathBuf> {
    let search_paths = vec![
        build_dir.join("lib").join("libusb-1.0.dylib"),
        build_dir
            .join("lib")
            .join("Release")
            .join("libusb-1.0.dylib"),
        build_dir.join("lib").join("Debug").join("libusb-1.0.dylib"),
    ];

    for path in search_paths {
        if path.exists() {
            return Some(path);
        }
    }
    None
}

fn find_built_so(build_dir: &PathBuf) -> Option<PathBuf> {
    let search_paths = vec![
        build_dir.join("lib").join("libusb-1.0.so"),
        build_dir.join("lib").join("Release").join("libusb-1.0.so"),
        build_dir.join("lib").join("Debug").join("libusb-1.0.so"),
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

    if env::var("CARGO_CFG_TARGET_OS") == Ok("windows".into()) {
        c_files.push(src_dir.join("usb/usb_layer_winusb.c"));
    }

    let mut builder = cc::Build::new();
    builder.include(include_dir);
    builder.include(libusb_include);
    builder.files(&c_files);

    if env::var("CARGO_CFG_TARGET_OS") == Ok("windows".into()) {
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

    let cross_compiling = is_cross_compiling();
    let target_os = env::var("CARGO_CFG_TARGET_OS").unwrap_or_default();

    if find_system_libusb() {
        println!("cargo:warning=Using system libusb-1.0");
    } else if cross_compiling && target_os != "windows" {
        build_libusb_static(&libusb_cmake_dir);
    } else {
        let build_dir = build_libusb_cmake(&libusb_cmake_dir);

        if target_os == "windows" {
            if let Some(dll_path) = find_built_dll(&build_dir) {
                copy_dll_to_output(&dll_path);
            }
        } else if target_os == "macos" {
            if let Some(dylib_path) = find_built_dylib(&build_dir) {
                copy_dll_to_output(&dylib_path);
            }
        } else if target_os == "linux" {
            if let Some(so_path) = find_built_so(&build_dir) {
                copy_dll_to_output(&so_path);
            }
        }
    }

    build_libefex(&include_dir, &src_dir, &libusb_include);
}
