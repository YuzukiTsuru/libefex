//! Rust bindings for libefex

//! libefex is a cross-platform library for interacting with Allwinner chips in FEL mode.

mod context;
mod error;
mod arch;
mod memory;

pub use context::Context;
pub use error::EfexError;
pub use arch::Arch;
pub use memory::{readl, writel, read_memory, write_memory, exec};

// Include auto-generated bindings
include!(concat!(env!("OUT_DIR"), "/bindings.rs"));