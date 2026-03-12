use std::env;
use std::fs;
use std::path::PathBuf;

pub fn link_framework(name: &str) {
    println!("cargo:rustc-link-lib=framework={}", name);
}

fn link(name: &str, bundled: bool) {
    use std::env::var;
    let target = var("TARGET").unwrap();
    let target: Vec<_> = target.split('-').collect();
    if target.get(2) == Some(&"windows") {
        println!("cargo:rustc-link-lib=dylib={}", name);
        if bundled && target.get(3) == Some(&"gnu") {
            let dir = var("CARGO_MANIFEST_DIR").unwrap();
            println!("cargo:rustc-link-search=native={}/{}", dir, target[0]);
        }
    }
}

fn build_libusb(libusb_source: &PathBuf, out_dir: &PathBuf) {
    let include_dir = out_dir.join("include");
    fs::create_dir_all(&include_dir).unwrap();

    // Copy libusb.h
    fs::copy(
        libusb_source.join("libusb/libusb.h"),
        include_dir.join("libusb.h"),
    )
    .unwrap();

    // Create config.h
    let config_h_path = include_dir.join("config.h");

    let target_os = std::env::var("CARGO_CFG_TARGET_OS").unwrap_or_default();
    let target_env = std::env::var("CARGO_CFG_TARGET_ENV").unwrap_or_default();
    let target_family = std::env::var("CARGO_CFG_TARGET_FAMILY").unwrap_or_default();

    if target_os == "windows" && target_env == "msvc" {
        // Use MSVC config
        fs::copy(
            libusb_source.join("msvc/config.h"),
            &config_h_path,
        )
        .unwrap();
    } else {
        // Create a minimal config.h for other platforms
        let mut config_content = String::new();

        config_content.push_str("#ifndef CONFIG_H\n");
        config_content.push_str("#define CONFIG_H\n");
        config_content.push_str("#define PRINTF_FORMAT(a, b)\n");
        config_content.push_str("#define ENABLE_LOGGING 1\n");

        if target_family == "unix" {
            config_content.push_str("#define HAVE_SYS_TIME_H 1\n");
            config_content.push_str("#define HAVE_NFDS_T 1\n");
            config_content.push_str("#define PLATFORM_POSIX 1\n");
            config_content.push_str("#define HAVE_CLOCK_GETTIME 1\n");
            config_content.push_str("#define DEFAULT_VISIBILITY __attribute__((visibility(\"default\")))\n");
        }

        if target_os == "linux" {
            config_content.push_str("#define OS_LINUX 1\n");
            config_content.push_str("#define HAVE_ASM_TYPES_H 1\n");
            config_content.push_str("#define _GNU_SOURCE 1\n");
            config_content.push_str("#define HAVE_TIMERFD 1\n");
            config_content.push_str("#define HAVE_EVENTFD 1\n");
        }

        if target_os == "macos" {
            config_content.push_str("#define OS_DARWIN 1\n");
            config_content.push_str("#define TARGET_OS_OSX 1\n");
        }

        if target_os == "windows" {
            config_content.push_str("#define OS_WINDOWS 1\n");
            config_content.push_str("#define PLATFORM_WINDOWS 1\n");
            config_content.push_str("#define DEFAULT_VISIBILITY\n");
        }

        config_content.push_str("#endif\n");
        fs::write(&config_h_path, config_content).unwrap();
    }

    println!("cargo:include={}", include_dir.to_str().unwrap());

    // Build libusb using cc
    let mut base_config = cc::Build::new();
    base_config.include(&include_dir);
    base_config.include(libusb_source.join("libusb"));

    // Core libusb source files
    base_config.file(libusb_source.join("libusb/core.c"));
    base_config.file(libusb_source.join("libusb/descriptor.c"));
    base_config.file(libusb_source.join("libusb/hotplug.c"));
    base_config.file(libusb_source.join("libusb/io.c"));
    base_config.file(libusb_source.join("libusb/strerror.c"));
    base_config.file(libusb_source.join("libusb/sync.c"));

    // Platform-specific configuration
    if target_os == "macos" {
        base_config.define("OS_DARWIN", Some("1"));
        base_config.define("TARGET_OS_OSX", Some("1"));
        base_config.file(libusb_source.join("libusb/os/darwin_usb.c"));
        link_framework("CoreFoundation");
        link_framework("IOKit");
        link_framework("Security");
        link("objc", false);
    }

    if target_os == "linux" {
        base_config.define("OS_LINUX", Some("1"));
        base_config.define("HAVE_ASM_TYPES_H", Some("1"));
        base_config.define("_GNU_SOURCE", Some("1"));
        base_config.define("HAVE_TIMERFD", Some("1"));
        base_config.define("HAVE_EVENTFD", Some("1"));
        base_config.file(libusb_source.join("libusb/os/linux_netlink.c"));
        base_config.file(libusb_source.join("libusb/os/linux_usbfs.c"));
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

        if target_os != "android" {
            if let Ok(lib) = pkg_config::probe_library("libudev") {
                base_config.define("USE_UDEV", Some("1"));
                base_config.define("HAVE_LIBUDEV", Some("1"));
                base_config.file(libusb_source.join("libusb/os/linux_udev.c"));
                for path in lib.include_paths {
                    base_config.include(path.to_str().unwrap());
                }
            }
        }

        base_config.file(libusb_source.join("libusb/os/events_posix.c"));
        base_config.file(libusb_source.join("libusb/os/threads_posix.c"));
    }

    if target_os == "windows" {
        if target_env == "msvc" {
            base_config.flag("/source-charset:utf-8");
        }

        base_config.warnings(false);
        base_config.define("OS_WINDOWS", Some("1"));
        base_config.define("DEFAULT_VISIBILITY", Some(""));
        base_config.define("PLATFORM_WINDOWS", Some("1"));
        base_config.file(libusb_source.join("libusb/os/events_windows.c"));
        base_config.file(libusb_source.join("libusb/os/threads_windows.c"));
        base_config.file(libusb_source.join("libusb/os/windows_common.c"));
        base_config.file(libusb_source.join("libusb/os/windows_usbdk.c"));
        base_config.file(libusb_source.join("libusb/os/windows_winusb.c"));

        link("user32", false);
    }

    base_config.compile("usb-1.0");
    println!("cargo:rustc-link-lib=static=usb-1.0");
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
    let libusb_source = root_dir.join("lib").join("libusb-cmake").join("libusb");
    let libusb_include = libusb_source.join("libusb");

    let out_dir = PathBuf::from(env::var("OUT_DIR").expect("OUT_DIR not set"));

    // Build libusb from source using cc (not cmake)
    build_libusb(&libusb_source, &out_dir);

    // Build libefex
    build_libefex(&include_dir, &src_dir, &libusb_include);
}