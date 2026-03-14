use std::{env, fs, path::PathBuf};

fn link(name: &str, _bundled: bool) {
    let target = env::var("TARGET").unwrap();
    let target: Vec<_> = target.split('-').collect();
    if target.get(2) == Some(&"windows") {
        println!("cargo:rustc-link-lib=dylib={}", name);
    }
}

pub fn link_framework(name: &str) {
    println!("cargo:rustc-link-lib=framework={}", name);
}

fn get_macos_major_version() -> Option<usize> {
    if !cfg!(target_os = "macos") {
        return None;
    }

    let output = std::process::Command::new("sw_vers")
        .args(["-productVersion"])
        .output()
        .ok()?;
    let version = std::str::from_utf8(&output.stdout).ok()?.trim_end();
    let components: Vec<&str> = version.split('.').collect();
    let major: usize = components[0].parse().ok()?;
    Some(major)
}

fn find_libusb_pkg(statik: bool) -> bool {
    if env::var("CARGO_CFG_TARGET_ENV") == Ok("msvc".into()) {
        #[cfg(target_os = "windows")]
        return match vcpkg::Config::new().find_package("libusb") {
            Ok(_) => true,
            Err(e) => {
                if pkg_config::probe_library("libusb-1.0").is_ok() {
                    true
                } else {
                    println!("cargo:warning=Can't find libusb pkg: {:?}", e);
                    false
                }
            }
        };
    }

    let needs_rustc_issue_96943_workaround: bool = get_macos_major_version()
        .map(|major| major >= 11)
        .unwrap_or_default();

    match pkg_config::Config::new().statik(statik).probe("libusb-1.0") {
        Ok(l) => {
            for lib in l.libs {
                if statik {
                    if needs_rustc_issue_96943_workaround && lib == "objc" {
                        continue;
                    }
                    println!("cargo:rustc-link-lib=static={}", lib);
                }
            }
            if statik {
                println!("cargo:static=1");
            }
            assert!(l.include_paths.len() <= 1);
            for path in l.include_paths {
                println!("cargo:include={}", path.to_str().unwrap());
            }
            println!("cargo:version_number={}", l.version);
            true
        }
        Err(e) => {
            println!("cargo:warning=Can't find libusb pkg: {:?}", e);
            false
        }
    }
}

fn build_libusb_static(libusb_source: &PathBuf) -> PathBuf {
    let out_dir = PathBuf::from(env::var("OUT_DIR").unwrap());
    let include_dir = out_dir.join("include");

    println!("cargo:vendored=1");
    println!("cargo:static=1");

    fs::create_dir_all(&include_dir).unwrap();
    fs::copy(
        libusb_source.join("libusb/libusb.h"),
        include_dir.join("libusb.h"),
    )
    .unwrap();
    println!("cargo:include={}", include_dir.to_str().unwrap());

    fs::File::create(format!("{}/{}", include_dir.display(), "config.h")).unwrap();

    let mut base_config = cc::Build::new();
    base_config.include(&include_dir);
    base_config.include(libusb_source.join("libusb"));

    base_config.define("PRINTF_FORMAT(a, b)", Some(""));
    base_config.define("ENABLE_LOGGING", Some("1"));

    let target_os = env::var("CARGO_CFG_TARGET_OS").unwrap_or_default();
    let target_env = env::var("CARGO_CFG_TARGET_ENV").unwrap_or_default();
    let target_family = env::var("CARGO_CFG_TARGET_FAMILY").unwrap_or_default();

    if target_env == "msvc" {
        fs::copy(
            libusb_source.join("msvc/config.h"),
            include_dir.join("config.h"),
        )
        .unwrap();
    }

    if target_os == "macos" {
        base_config.define("OS_DARWIN", Some("1"));
        base_config.define("TARGET_OS_OSX", Some("1"));
        base_config.file(libusb_source.join("libusb/os/darwin_usb.c"));
        link_framework("CoreFoundation");
        link_framework("IOKit");
        link_framework("Security");
        link("objc", false);
    }

    if target_os == "linux" || target_os == "android" {
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
            };
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
        base_config.file(libusb_source.join("libusb/os/events_windows.c"));
        base_config.file(libusb_source.join("libusb/os/threads_windows.c"));
        base_config.file(libusb_source.join("libusb/os/windows_common.c"));
        base_config.file(libusb_source.join("libusb/os/windows_usbdk.c"));
        base_config.file(libusb_source.join("libusb/os/windows_winusb.c"));

        base_config.define("DEFAULT_VISIBILITY", Some(""));
        base_config.define("PLATFORM_WINDOWS", Some("1"));
        link("user32", false);
    }

    base_config.file(libusb_source.join("libusb/core.c"));
    base_config.file(libusb_source.join("libusb/descriptor.c"));
    base_config.file(libusb_source.join("libusb/hotplug.c"));
    base_config.file(libusb_source.join("libusb/io.c"));
    base_config.file(libusb_source.join("libusb/strerror.c"));
    base_config.file(libusb_source.join("libusb/sync.c"));

    base_config.compile("usb-1.0");
    println!("cargo:rustc-link-lib=static=usb-1.0");

    include_dir
}

fn build_libusb_shared(libusb_source: &PathBuf) -> PathBuf {
    let out_dir = PathBuf::from(env::var("OUT_DIR").unwrap());
    let include_dir = out_dir.join("include");

    println!("cargo:vendored=1");

    fs::create_dir_all(&include_dir).unwrap();
    fs::copy(
        libusb_source.join("libusb/libusb.h"),
        include_dir.join("libusb.h"),
    )
    .unwrap();
    println!("cargo:include={}", include_dir.to_str().unwrap());

    let target_os = env::var("CARGO_CFG_TARGET_OS").unwrap_or_default();
    let target_env = env::var("CARGO_CFG_TARGET_ENV").unwrap_or_default();
    let target_family = env::var("CARGO_CFG_TARGET_FAMILY").unwrap_or_default();

    if target_env == "msvc" {
        fs::copy(
            libusb_source.join("msvc/config.h"),
            include_dir.join("config.h"),
        )
        .unwrap();
    } else {
        let mut config_content = String::new();
        config_content.push_str("#ifndef CONFIG_H\n#define CONFIG_H\n");
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
        fs::write(include_dir.join("config.h"), config_content).unwrap();
    }

    let mut base_config = cc::Build::new();
    base_config.include(&include_dir);
    base_config.include(libusb_source.join("libusb"));

    base_config.define("PRINTF_FORMAT(a, b)", Some(""));
    base_config.define("ENABLE_LOGGING", Some("1"));

    let mut additional_libs: Vec<String> = Vec::new();

    if target_os == "macos" {
        base_config.define("OS_DARWIN", Some("1"));
        base_config.define("TARGET_OS_OSX", Some("1"));
        base_config.file(libusb_source.join("libusb/os/darwin_usb.c"));
        link_framework("CoreFoundation");
        link_framework("IOKit");
        link_framework("Security");
        link("objc", false);
    }

    if target_os == "linux" || target_os == "android" {
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
            };
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
        base_config.file(libusb_source.join("libusb/os/events_windows.c"));
        base_config.file(libusb_source.join("libusb/os/threads_windows.c"));
        base_config.file(libusb_source.join("libusb/os/windows_common.c"));
        base_config.file(libusb_source.join("libusb/os/windows_usbdk.c"));
        base_config.file(libusb_source.join("libusb/os/windows_winusb.c"));

        base_config.define("DEFAULT_VISIBILITY", Some(""));
        base_config.define("PLATFORM_WINDOWS", Some("1"));
        additional_libs.push("user32".to_string());
    }

    base_config.file(libusb_source.join("libusb/core.c"));
    base_config.file(libusb_source.join("libusb/descriptor.c"));
    base_config.file(libusb_source.join("libusb/hotplug.c"));
    base_config.file(libusb_source.join("libusb/io.c"));
    base_config.file(libusb_source.join("libusb/strerror.c"));
    base_config.file(libusb_source.join("libusb/sync.c"));

    base_config.compile("usb-1.0-static");

    let static_lib = if target_os == "windows" && target_env == "msvc" {
        out_dir.join("usb-1.0-static.lib")
    } else {
        out_dir.join("libusb-1.0-static.a")
    };

    let shared_dir = out_dir.join("shared");
    fs::create_dir_all(&shared_dir).unwrap();

    if target_os == "windows" {
        let def_path = libusb_source.join("libusb/libusb-1.0.def");

        if target_env == "msvc" {
            let dll_path = shared_dir.join("libusb-1.0.dll");
            let lib_path = shared_dir.join("usb-1.0.lib");
            let rc_path = libusb_source.join("libusb/libusb-1.0.rc");
            let rc_obj = shared_dir.join("libusb-1.0.res");

            let has_link = std::process::Command::new("link")
                .arg("/?")
                .output()
                .is_ok();

            if !has_link {
                println!("cargo:warning=link.exe not found in PATH, falling back to static linking");
                println!("cargo:warning=To build shared library, run from Visual Studio Developer Command Prompt");
                println!("cargo:rustc-link-lib=static=usb-1.0-static");
                return include_dir;
            }

            let mut link_args: Vec<String> = vec![
                "/DLL".to_string(),
                format!("/OUT:{}", dll_path.display()),
                format!("/DEF:{}", def_path.display()),
                format!("/IMPLIB:{}", lib_path.display()),
                static_lib.display().to_string(),
                "user32.lib".to_string(),
                "kernel32.lib".to_string(),
                "advapi32.lib".to_string(),
            ];

            let rc_status = std::process::Command::new("rc")
                .args(["/fo", rc_obj.to_str().unwrap(), rc_path.to_str().unwrap()])
                .current_dir(libusb_source.join("libusb"))
                .status();

            if let Ok(status) = rc_status {
                if status.success() {
                    link_args.push(rc_obj.display().to_string());
                }
            }

            let link_output = std::process::Command::new("link")
                .args(&link_args)
                .output()
                .expect("Failed to run link.exe");

            if !link_output.status.success() {
                println!("cargo:warning=link.exe failed, falling back to static linking");
                println!("cargo:warning=link.exe stderr: {}", String::from_utf8_lossy(&link_output.stderr));
                println!("cargo:rustc-link-lib=static=usb-1.0-static");
                return include_dir;
            }

            let actual_lib = if lib_path.exists() {
                lib_path.clone()
            } else {
                let alt_lib = shared_dir.join("libusb-1.0.lib");
                if alt_lib.exists() {
                    println!("cargo:warning=Import library generated as {} instead of {}", alt_lib.display(), lib_path.display());
                    alt_lib
                } else {
                    println!("cargo:warning=Import library not found, falling back to static linking");
                    println!("cargo:rustc-link-lib=static=usb-1.0-static");
                    return include_dir;
                }
            };

            println!("cargo:rustc-link-search=native={}", shared_dir.display());
            println!("cargo:rustc-link-lib=static={}", actual_lib.file_stem().unwrap().to_str().unwrap());

            copy_dll_to_target(&dll_path);
        } else {
            println!("cargo:warning=Windows GNU target not supported for shared library, falling back to static linking");
            println!("cargo:rustc-link-lib=static=usb-1.0-static");
            return include_dir;
        }
    } else if target_os == "macos" {
        let dylib_path = shared_dir.join("libusb-1.0.dylib");

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
            .current_dir(&shared_dir)
            .status()
            .expect("Failed to run cc");

        if !cc_status.success() {
            panic!("Failed to create dylib");
        }

        println!("cargo:rustc-link-search=native={}", shared_dir.display());
        println!("cargo:rustc-link-lib=dylib=usb-1.0");

        copy_dylib_to_target(&dylib_path);
    } else if target_os == "linux" {
        let so_path = shared_dir.join("libusb-1.0.so");

        let mut gcc_args: Vec<&str> = vec![
            "-shared",
            "-o", so_path.to_str().unwrap(),
            static_lib.to_str().unwrap(),
        ];

        if target_os != "android" {
            gcc_args.push("-ludev");
        }

        let gcc_status = std::process::Command::new("gcc")
            .args(&gcc_args)
            .current_dir(&shared_dir)
            .status()
            .expect("Failed to run gcc");

        if !gcc_status.success() {
            panic!("Failed to create shared library");
        }

        println!("cargo:rustc-link-search=native={}", shared_dir.display());
        println!("cargo:rustc-link-lib=dylib=usb-1.0");

        copy_so_to_target(&so_path);
    }

    include_dir
}

fn get_target_dir() -> PathBuf {
    if let Ok(target_dir) = env::var("CARGO_TARGET_DIR") {
        return PathBuf::from(target_dir);
    }

    let out_dir = PathBuf::from(env::var("OUT_DIR").unwrap());

    let mut current = out_dir.as_path();
    while let Some(parent) = current.parent() {
        if parent.ends_with("target") {
            return parent.to_path_buf();
        }
        if parent.file_name().map(|f| f == "target").unwrap_or(false) {
            return parent.to_path_buf();
        }
        current = parent;
    }

    let manifest_dir = PathBuf::from(env::var("CARGO_MANIFEST_DIR").unwrap());
    manifest_dir.join("target")
}

fn copy_dll_to_target(dll_path: &PathBuf) {
    let profile = env::var("PROFILE").unwrap();
    let target = env::var("TARGET").unwrap();

    let target_dir = get_target_dir();
    let dest_dir = target_dir.join(&target).join(&profile);

    if let Err(e) = fs::create_dir_all(&dest_dir) {
        println!("cargo:warning=Failed to create target dir: {}", e);
        return;
    }

    let dest_dll = dest_dir.join("libusb-1.0.dll");
    if dll_path.exists() {
        match fs::copy(dll_path, &dest_dll) {
            Ok(_) => {
                println!("cargo:warning=Copied libusb-1.0.dll to {}", dest_dll.display());
                println!("cargo:rerun-if-changed={}", dll_path.display());
            }
            Err(e) => {
                println!("cargo:warning=Failed to copy DLL: {}", e);
            }
        }
    }
}

fn copy_dylib_to_target(dylib_path: &PathBuf) {
    let profile = env::var("PROFILE").unwrap();
    let target = env::var("TARGET").unwrap();

    let target_dir = get_target_dir();
    let dest_dir = target_dir.join(&target).join(&profile);

    if let Err(e) = fs::create_dir_all(&dest_dir) {
        println!("cargo:warning=Failed to create target dir: {}", e);
        return;
    }

    let dest_dylib = dest_dir.join("libusb-1.0.dylib");
    if dylib_path.exists() {
        match fs::copy(dylib_path, &dest_dylib) {
            Ok(_) => {
                println!("cargo:warning=Copied libusb-1.0.dylib to {}", dest_dylib.display());
                println!("cargo:rerun-if-changed={}", dylib_path.display());
            }
            Err(e) => {
                println!("cargo:warning=Failed to copy dylib: {}", e);
            }
        }
    }
}

fn copy_so_to_target(so_path: &PathBuf) {
    let profile = env::var("PROFILE").unwrap();
    let target = env::var("TARGET").unwrap();

    let target_dir = get_target_dir();
    let dest_dir = target_dir.join(&target).join(&profile);

    if let Err(e) = fs::create_dir_all(&dest_dir) {
        println!("cargo:warning=Failed to create target dir: {}", e);
        return;
    }

    let dest_so = dest_dir.join("libusb-1.0.so");
    if so_path.exists() {
        match fs::copy(so_path, &dest_so) {
            Ok(_) => {
                println!("cargo:warning=Copied libusb-1.0.so to {}", dest_so.display());
                println!("cargo:rerun-if-changed={}", so_path.display());
            }
            Err(e) => {
                println!("cargo:warning=Failed to copy .so: {}", e);
            }
        }
    }
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

    let target_os = env::var("CARGO_CFG_TARGET_OS").unwrap_or_default();

    if target_os == "windows" {
        c_files.push(src_dir.join("usb/usb_layer_winusb.c"));
    }

    let mut builder = cc::Build::new();
    builder.include(include_dir);
    builder.include(libusb_include);
    builder.files(&c_files);

    if target_os == "windows" {
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
    println!("cargo:rerun-if-env-changed=LIBUSB_STATIC");

    let current_dir = env::current_dir().expect("Failed to get current directory");
    let root_dir = current_dir
        .parent()
        .unwrap()
        .parent()
        .expect("Failed to get project root directory");

    let include_dir = root_dir.join("includes");
    let src_dir = root_dir.join("src");
    let libusb_source = root_dir.join("lib").join("libusb-cmake").join("libusb");

    let statik = {
        if cfg!(target_os = "macos") {
            match env::var("LIBUSB_STATIC").unwrap_or_default().as_ref() {
                "" | "0" => false,
                _ => true,
            }
        } else {
            env::var("CARGO_CFG_TARGET_FEATURE")
                .map(|s| s.contains("crt-static"))
                .unwrap_or_default()
        }
    };

    let is_freebsd = env::var("CARGO_CFG_TARGET_OS") == Ok("freebsd".into());
    let vendored = cfg!(feature = "vendored");
    let vendored_shared = cfg!(feature = "vendored-shared");

    let libusb_include = if (!is_freebsd && vendored) || !find_libusb_pkg(statik) {
        if vendored_shared {
            build_libusb_shared(&libusb_source)
        } else {
            build_libusb_static(&libusb_source)
        }
    } else {
        libusb_source.join("libusb")
    };

    build_libefex(&include_dir, &src_dir, &libusb_include);
}
