use std::fs::File;
use std::io::{self, Read, Write};
use std::time::Instant;

use clap::{Parser, Subcommand};
use libefex::{Context, PayloadArch};

/// Progress structure
struct Progress {
    total: u64,
    done: u64,
    start: Instant,
}

impl Progress {
    /// Create a new progress instance
    fn new(total: u64) -> Self {
        Progress {
            total,
            done: 0,
            start: Instant::now(),
        }
    }

    /// Update progress
    fn update(&mut self, bytes: u64) {
        self.done += bytes;
        self.display();
    }

    /// Display progress
    fn display(&self) {
        let ratio = if self.total > 0 {
            self.done as f64 / self.total as f64
        } else {
            0.0
        };

        let elapsed = self.start.elapsed().as_secs_f64();
        let speed = if elapsed > 0.0 {
            self.done as f64 / elapsed
        } else {
            0.0
        };

        let eta = if speed > 0.0 {
            (self.total - self.done) as f64 / speed
        } else {
            0.0
        };

        let percent = ratio * 100.0;
        let bar_length = 48;
        let pos = (ratio * bar_length as f64) as usize;

        print!("{:3.0}% [", percent);
        for i in 0..bar_length {
            if i < pos {
                print!("=");
            } else {
                print!(" ");
            }
        }

        print!("] {}/s, ETA {:.0}s        ", self.format_size(speed), eta);
        std::io::stdout().flush().unwrap();
    }

    /// Stop progress display
    fn stop(&self) {
        println!();
    }

    /// Format size
    fn format_size(&self, size: f64) -> String {
        let units = ["B", "KB", "MB", "GB", "TB"];
        let mut value = size;
        let mut unit_idx = 0;

        while value > 1024.0 && unit_idx < units.len() - 1 {
            value /= 1024.0;
            unit_idx += 1;
        }

        format!("{:.2} {}", value, units[unit_idx])
    }
}

/// Command line argument parsing
#[derive(Parser)]
#[command(author, version, about, long_about = None)]
struct Args {
    #[command(subcommand)]
    command: Commands,

    /// Select payload architecture
    #[arg(short = 'p', long = "payloads")]
    payloads: Option<String>,
}

/// Subcommand enumeration
#[derive(Subcommand)]
enum Commands {
    /// Display chip version
    Version,

    /// Dump memory region in hexadecimal format
    Hexdump {
        /// Start address
        address: String,

        /// Length
        length: String,
    },

    /// Binary memory dump to stdout
    Dump {
        /// Start address
        address: String,

        /// Length
        length: String,
    },

    /// Read 32-bit value from device memory
    Read32 {
        /// Address
        address: String,
    },

    /// Write 32-bit value to device memory
    Write32 {
        /// Address
        address: String,

        /// Value
        value: String,
    },

    /// Read from memory to file
    Read {
        /// Start address
        address: String,

        /// Length
        length: String,

        /// Output file
        file: String,
    },

    /// Write file to memory
    Write {
        /// Start address
        address: String,

        /// Input file
        file: String,
    },

    /// Call function address
    Exec {
        /// Address
        address: String,
    },
}

/// Parse hexadecimal or decimal string to u32
fn parse_u32(s: &str) -> Result<u32, anyhow::Error> {
    let value = if s.starts_with("0x") || s.starts_with("0X") {
        u32::from_str_radix(&s[2..], 16)?
    } else {
        s.parse::<u32>()?
    };
    Ok(value)
}

/// Parse hexadecimal or decimal string to usize
fn parse_size(s: &str) -> Result<usize, anyhow::Error> {
    let value = if s.starts_with("0x") || s.starts_with("0X") {
        usize::from_str_radix(&s[2..], 16)?
    } else {
        s.parse::<usize>()?
    };
    Ok(value)
}

/// Parse architecture string
fn parse_arch(s: &str) -> PayloadArch {
    match s.to_lowercase().as_str() {
        "arm" => PayloadArch::Arm32,
        "aarch64" => PayloadArch::Aarch64,
        "riscv" => PayloadArch::Riscv,
        _ => {
            eprintln!("Unknown payload arch '{}', defaulting to riscv", s);
            PayloadArch::Riscv
        }
    }
}

/// Hexadecimal dump
fn hex_dump(base: u32, buf: &[u8]) {
    for (i, chunk) in buf.chunks(16).enumerate() {
        let addr = base + (i * 16) as u32;
        print!("{:08x}: ", addr);

        // Print hexadecimal
        for (_j, byte) in chunk.iter().enumerate() {
            print!("{:02x} ", byte);
        }

        // Fill with spaces
        for _ in chunk.len()..16 {
            print!("   ");
        }

        print!(" ");

        // Print ASCII
        for byte in chunk {
            if *byte >= 32 && *byte <= 126 {
                print!("{}", *byte as char);
            } else {
                print!(".");
            }
        }

        println!();
    }
}

fn main() -> anyhow::Result<()> {
    let args = Args::parse();

    // Initialize context
    let mut ctx = Context::new();

    // Scan USB devices
    ctx.scan_usb_device()?;

    // Initialize USB
    ctx.usb_init()?;

    // Initialize EFEX
    ctx.efex_init()?;

    // Handle -p parameter
    let use_payloads = args.payloads.is_some();
    if use_payloads {
        let arch_str = args.payloads.as_ref().unwrap();
        let arch = parse_arch(arch_str);
        libefex::payloads::init(arch)?;
    }

    // Execute command
    match args.command {
        Commands::Version => {
            // Display device information
            println!("Chip ID      : 0x{:08x}", unsafe {
                (*ctx.as_ptr()).resp.id
            });
            println!("Firmware     : 0x{:08x}", unsafe {
                (*ctx.as_ptr()).resp.firmware
            });
            println!("Mode         : 0x{:04x}", unsafe {
                (*ctx.as_ptr()).resp.mode
            });
            println!("Data Addr    : 0x{:08x}", unsafe {
                (*ctx.as_ptr()).resp.data_start_address
            });
            println!("Data Length  : {}", unsafe {
                (*ctx.as_ptr()).resp.data_length
            });
            println!("Data Flag    : {}", unsafe {
                (*ctx.as_ptr()).resp.data_flag
            });
        }

        Commands::Hexdump { address, length } => {
            let addr = parse_u32(&address)?;
            let len = parse_size(&length)?;

            const CHUNK_SIZE: usize = 4096;
            let mut buf = vec![0u8; CHUNK_SIZE];
            let mut remaining = len;
            let mut cur_addr = addr;

            while remaining > 0 {
                let chunk_len = std::cmp::min(remaining, CHUNK_SIZE);
                ctx.fel_read(cur_addr, &mut buf[..chunk_len])?;
                hex_dump(cur_addr, &buf[..chunk_len]);

                cur_addr += chunk_len as u32;
                remaining -= chunk_len;
            }
        }

        Commands::Dump { address, length } => {
            let addr = parse_u32(&address)?;
            let len = parse_size(&length)?;

            const CHUNK_SIZE: usize = 65536;
            let mut buf = vec![0u8; CHUNK_SIZE];
            let mut remaining = len;
            let mut cur_addr = addr;

            while remaining > 0 {
                let chunk_len = std::cmp::min(remaining, CHUNK_SIZE);
                ctx.fel_read(cur_addr, &mut buf[..chunk_len])?;
                io::stdout().write_all(&buf[..chunk_len])?;

                cur_addr += chunk_len as u32;
                remaining -= chunk_len;
            }
        }

        Commands::Read32 { address } => {
            let addr = parse_u32(&address)?;
            let value = if use_payloads {
                libefex::payloads::readl(&ctx, addr)?
            } else {
                let mut buf = [0u8; 4];
                ctx.fel_read(addr, &mut buf)?;
                u32::from_le_bytes(buf)
            };
            println!("0x{:08x}", value);
        }

        Commands::Write32 { address, value } => {
            let addr = parse_u32(&address)?;
            let val = parse_u32(&value)?;

            if use_payloads {
                libefex::payloads::writel(&ctx, val, addr)?;
            } else {
                let buf = val.to_le_bytes();
                ctx.fel_write(addr, &buf)?;
            }
        }

        Commands::Read {
            address,
            length,
            file,
        } => {
            let addr = parse_u32(&address)?;
            let len = parse_size(&length)?;

            let mut file = File::create(file)?;
            let mut progress = Progress::new(len as u64);

            const CHUNK_SIZE: usize = 65536;
            let mut buf = vec![0u8; CHUNK_SIZE];
            let mut remaining = len;
            let mut cur_addr = addr;

            while remaining > 0 {
                let chunk_len = std::cmp::min(remaining, CHUNK_SIZE);
                ctx.fel_read(cur_addr, &mut buf[..chunk_len])?;
                file.write_all(&buf[..chunk_len])?;

                progress.update(chunk_len as u64);

                cur_addr += chunk_len as u32;
                remaining -= chunk_len;
            }

            progress.stop();
        }

        Commands::Write { address, file } => {
            let addr = parse_u32(&address)?;

            let mut file = File::open(file)?;
            let file_size = file.metadata()?.len();

            let mut progress = Progress::new(file_size);

            const CHUNK_SIZE: usize = 65536;
            let mut buf = vec![0u8; CHUNK_SIZE];
            let mut offset = 0;

            loop {
                let nread = file.read(&mut buf)?;
                if nread == 0 {
                    break;
                }

                ctx.fel_write(addr + offset as u32, &buf[..nread])?;
                progress.update(nread as u64);

                offset += nread;
            }

            progress.stop();
        }

        Commands::Exec { address } => {
            let addr = parse_u32(&address)?;
            ctx.fel_exec(addr)?;
        }
    }

    Ok(())
}
