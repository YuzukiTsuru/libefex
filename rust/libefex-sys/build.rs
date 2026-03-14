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

fn build_libusb_dylib_macos(libusb_source: &PathBuf, out_dir: &PathBuf) -> PathBuf {
    let include_dir = out_dir.join("include");
    fs::create_dir_all(&include_dir).unwrap();

    fs::copy(
        libusb_source.join("libusb/libusb.h"),
        include_dir.join("libusb.h"),
    )
    .unwrap();

    let config_h_path = include_dir.join("config.h");
    
    let mut config_content = String::new();
    config_content.push_str("#ifndef CONFIG_H\n");
    config_content.push_str("#define CONFIG_H\n");
    config_content.push_str("#define PRINTF_FORMAT(a, b)\n");
    config_content.push_str("#define ENABLE_LOGGING 1\n");
    config_content.push_str("#define HAVE_SYS_TIME_H 1\n");
    config_content.push_str("#define HAVE_NFDS_T 1\n");
    config_content.push_str("#define PLATFORM_POSIX 1\n");
    config_content.push_str("#define HAVE_CLOCK_GETTIME 1\n");
    config_content.push_str("#define DEFAULT_VISIBILITY __attribute__((visibility(\"default\")))\n");
    config_content.push_str("#define OS_DARWIN 1\n");
    config_content.push_str("#define TARGET_OS_OSX 1\n");
    config_content.push_str("#endif\n");
    fs::write(&config_h_path, config_content).unwrap();

    println!("cargo:include={}", include_dir.to_str().unwrap());

    let mut build = cc::Build::new();
    build.include(&include_dir);
    build.include(libusb_source.join("libusb"));
    build.define("OS_DARWIN", Some("1"));
    build.define("TARGET_OS_OSX", Some("1"));
    build.define("HAVE_SYS_TIME_H", Some("1"));
    build.define("HAVE_NFDS_T", Some("1"));
    build.define("PLATFORM_POSIX", Some("1"));
    build.define("HAVE_CLOCK_GETTIME", Some("1"));
    build.define(
        "DEFAULT_VISIBILITY",
        Some("__attribute__((visibility(\"default\")))"),
    );

    build.file(libusb_source.join("libusb/core.c"));
    build.file(libusb_source.join("libusb/descriptor.c"));
    build.file(libusb_source.join("libusb/hotplug.c"));
    build.file(libusb_source.join("libusb/io.c"));
    build.file(libusb_source.join("libusb/strerror.c"));
    build.file(libusb_source.join("libusb/sync.c"));
    build.file(libusb_source.join("libusb/os/darwin_usb.c"));
    build.file(libusb_source.join("libusb/os/events_posix.c"));
    build.file(libusb_source.join("libusb/os/threads_posix.c"));

    build.compile("usb-1.0-static");

    let target_dir = out_dir.join("dylib");
    fs::create_dir_all(&target_dir).unwrap();

    let dylib_path = target_dir.join("libusb-1.0.dylib");
    let static_lib = out_dir.join("libusb-1.0-static.a");

    link_framework("CoreFoundation");
    link_framework("IOKit");
    link_framework("Security");

    let cc_status = std::process::Command::new("cc")
        .args([
            "-dynamiclib",
            "-o", dylib_path.to_str().unwrap(),
            "-install_name", "@rpath/libusb-1.0.dylib",
            static_lib.to_str().unwrap(),
            "-framework", "CoreFoundation",
            "-framework", "IOKit",
            "-framework", "Security",
            "-lobjc",
        ])
        .current_dir(&target_dir)
        .status()
        .expect("Failed to run cc");
    
    if !cc_status.success() {
        panic!("Failed to create dylib");
    }

    dylib_path
}

fn build_libusb_static(libusb_source: &PathBuf, out_dir: &PathBuf) -> PathBuf {
    let include_dir = out_dir.join("include");
    fs::create_dir_all(&include_dir).unwrap();

    fs::copy(
        libusb_source.join("libusb/libusb.h"),
        include_dir.join("libusb.h"),
    )
    .unwrap();

    let config_h_path = include_dir.join("config.h");

    let target_os = std::env::var("CARGO_CFG_TARGET_OS").unwrap_or_default();
    let target_env = std::env::var("CARGO_CFG_TARGET_ENV").unwrap_or_default();
    let target_family = std::env::var("CARGO_CFG_TARGET_FAMILY").unwrap_or_default();

    if target_os == "windows" && target_env == "msvc" {
        fs::copy(
            libusb_source.join("msvc/config.h"),
            &config_h_path,
        )
        .unwrap();
    } else {
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

    let mut base_config = cc::Build::new();
    base_config.include(&include_dir);
    base_config.include(libusb_source.join("libusb"));

    base_config.file(libusb_source.join("libusb/core.c"));
    base_config.file(libusb_source.join("libusb/descriptor.c"));
    base_config.file(libusb_source.join("libusb/hotplug.c"));
    base_config.file(libusb_source.join("libusb/io.c"));
    base_config.file(libusb_source.join("libusb/strerror.c"));
    base_config.file(libusb_source.join("libusb/sync.c"));

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

    let lib_name = "usb-1.0";
    base_config.compile(lib_name);
    
    out_dir.join(format!("lib{}.a", lib_name))
}

fn build_libusb_dll(libusb_source: &PathBuf, out_dir: &PathBuf, target_env: &str) -> Option<PathBuf> {
    let include_dir = out_dir.join("include");
    fs::create_dir_all(&include_dir).unwrap();

    fs::copy(
        libusb_source.join("libusb/libusb.h"),
        include_dir.join("libusb.h"),
    )
    .unwrap();

    let config_h_path = include_dir.join("config.h");
    fs::copy(
        libusb_source.join("msvc/config.h"),
        &config_h_path,
    )
    .unwrap();

    println!("cargo:include={}", include_dir.to_str().unwrap());

    let mut build = cc::Build::new();
    build.include(&include_dir);
    build.include(libusb_source.join("libusb"));

    build.file(libusb_source.join("libusb/core.c"));
    build.file(libusb_source.join("libusb/descriptor.c"));
    build.file(libusb_source.join("libusb/hotplug.c"));
    build.file(libusb_source.join("libusb/io.c"));
    build.file(libusb_source.join("libusb/strerror.c"));
    build.file(libusb_source.join("libusb/sync.c"));

    if target_env == "msvc" {
        build.flag("/source-charset:utf-8");
    }

    build.warnings(false);
    build.define("OS_WINDOWS", Some("1"));
    build.define("DEFAULT_VISIBILITY", Some(""));
    build.define("PLATFORM_WINDOWS", Some("1"));
    build.file(libusb_source.join("libusb/os/events_windows.c"));
    build.file(libusb_source.join("libusb/os/threads_windows.c"));
    build.file(libusb_source.join("libusb/os/windows_common.c"));
    build.file(libusb_source.join("libusb/os/windows_usbdk.c"));
    build.file(libusb_source.join("libusb/os/windows_winusb.c"));

    build.compile("usb-1.0-static");

    let target_dir = out_dir.join("dll");
    fs::create_dir_all(&target_dir).unwrap();

    let dll_path = target_dir.join("libusb-1.0.dll");
    let lib_path = target_dir.join("libusb-1.0.lib");
    let def_path = libusb_source.join("libusb/libusb-1.0.def");

    let static_lib = if target_env == "msvc" {
        out_dir.join("usb-1.0-static.lib")
    } else {
        out_dir.join("libusb-1.0-static.a")
    };
    
    if target_env == "msvc" {
        let has_rc = std::process::Command::new("rc")
            .arg("--help")
            .output()
            .is_ok();
        
        let has_link = std::process::Command::new("link")
            .arg("--help")
            .output()
            .is_ok();
        
        if !has_link {
            println!("cargo:warning=link.exe not found, falling back to static linking");
            return None;
        }

        let rc_path = libusb_source.join("libusb/libusb-1.0.rc");
        let rc_obj = target_dir.join("libusb-1.0.res");
        
        let mut link_args = vec![
            "/DLL".to_string(),
            format!("/OUT:{}", dll_path.display()),
            format!("/DEF:{}", def_path.display()),
            format!("/IMPLIB:{}", lib_path.display()),
            static_lib.display().to_string(),
            "user32.lib".to_string(),
            "kernel32.lib".to_string(),
            "advapi32.lib".to_string(),
        ];

        if has_rc {
            let rc_status = std::process::Command::new("rc")
                .args(["/fo", rc_obj.to_str().unwrap(), rc_path.to_str().unwrap()])
                .current_dir(libusb_source.join("libusb"))
                .status()
                .expect("Failed to run rc");
            
            if rc_status.success() {
                link_args.push(rc_obj.display().to_string());
            }
        }

        let link_output = std::process::Command::new("link")
            .args(&link_args)
            .current_dir(&target_dir)
            .output()
            .expect("Failed to run link");
        
        if !link_output.status.success() {
            println!("cargo:warning=link.exe failed, falling back to static linking");
            println!("cargo:warning=link.exe stdout: {}", String::from_utf8_lossy(&link_output.stdout));
            println!("cargo:warning=link.exe stderr: {}", String::from_utf8_lossy(&link_output.stderr));
            return None;
        }
    } else {
        let has_dlltool = std::process::Command::new("dlltool")
            .arg("--help")
            .output()
            .is_ok();
        
        let has_gcc = std::process::Command::new("gcc")
            .arg("--help")
            .output()
            .is_ok();
        
        if !has_dlltool || !has_gcc {
            println!("cargo:warning=dlltool or gcc not found, falling back to static linking");
            return None;
        }

        let dlltool_status = std::process::Command::new("dlltool")
            .args([
                "-d", def_path.to_str().unwrap(),
                "-l", lib_path.to_str().unwrap(),
                "-D", dll_path.to_str().unwrap(),
            ])
            .current_dir(&target_dir)
            .status()
            .expect("Failed to run dlltool");
        
        if !dlltool_status.success() {
            println!("cargo:warning=dlltool failed, falling back to static linking");
            return None;
        }

        let gcc_status = std::process::Command::new("gcc")
            .args([
                "-shared",
                "-o", dll_path.to_str().unwrap(),
                static_lib.to_str().unwrap(),
                "-luser32",
                "-lkernel32",
                "-ladvapi32",
                "-Wl,--out-implib", lib_path.to_str().unwrap(),
            ])
            .current_dir(&target_dir)
            .status()
            .expect("Failed to run gcc");
        
        if !gcc_status.success() {
            println!("cargo:warning=gcc failed, falling back to static linking");
            return None;
        }
    }

    Some(dll_path)
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

    let target_os = std::env::var("CARGO_CFG_TARGET_OS").unwrap_or_default();
    let target_env = std::env::var("CARGO_CFG_TARGET_ENV").unwrap_or_default();

    if target_os == "windows" {
        if let Some(dll_path) = build_libusb_dll(&libusb_source, &out_dir, &target_env) {
            let dll_dir = dll_path.parent().unwrap();
            println!("cargo:rustc-link-search=native={}", dll_dir.display());
            println!("cargo:rustc-link-lib=dylib=usb-1.0");

            let profile = env::var("PROFILE").unwrap();
            let target = env::var("TARGET").unwrap();
            let manifest_dir = env::var("CARGO_MANIFEST_DIR").unwrap();
            
            let manifest_path = PathBuf::from(&manifest_dir);
            let src_tauri_dir = manifest_path
                .parent().unwrap()
                .parent().unwrap();
            
            let workspace_target = src_tauri_dir
                .join("target")
                .join(&target)
                .join(&profile);
            
            fs::create_dir_all(&workspace_target).ok();
            let dest_dll = workspace_target.join("libusb-1.0.dll");
            if dll_path.exists() {
                fs::copy(&dll_path, &dest_dll).expect("Failed to copy DLL to target directory");
                println!("cargo:warning=Copied libusb-1.0.dll to {}", dest_dll.display());
            }
            
            println!("cargo:rerun-if-changed={}", dll_path.display());
        } else {
            println!("cargo:rustc-link-lib=static=usb-1.0-static");
        }
    } else if target_os == "macos" {
        let dylib_path = build_libusb_dylib_macos(&libusb_source, &out_dir);
        
        let dylib_dir = dylib_path.parent().unwrap();
        println!("cargo:rustc-link-search=native={}", dylib_dir.display());
        println!("cargo:rustc-link-lib=dylib=usb-1.0");

        let profile = env::var("PROFILE").unwrap();
        let target = env::var("TARGET").unwrap();
        let manifest_dir = env::var("CARGO_MANIFEST_DIR").unwrap();
        
        let manifest_path = PathBuf::from(&manifest_dir);
        let src_tauri_dir = manifest_path
            .parent().unwrap()
            .parent().unwrap();
        
        let workspace_target = src_tauri_dir
            .join("target")
            .join(&target)
            .join(&profile);
        
        fs::create_dir_all(&workspace_target).ok();
        let dest_dylib = workspace_target.join("libusb-1.0.dylib");
        if dylib_path.exists() {
            fs::copy(&dylib_path, &dest_dylib).expect("Failed to copy dylib to target directory");
            println!("cargo:warning=Copied libusb-1.0.dylib to {}", dest_dylib.display());
        }
        
        println!("cargo:rerun-if-changed={}", dylib_path.display());
    } else {
        build_libusb_static(&libusb_source, &out_dir);
        println!("cargo:rustc-link-lib=static=usb-1.0");
    }

    build_libefex(&include_dir, &src_dir, &libusb_include);
}
