//! Architecture-related constant definitions

/// Supported processor architecture enumeration
#[derive(Clone, Copy, Debug, PartialEq, Eq, Hash)]
pub enum Arch {
    /// ARM 32-bit architecture
    ARM32,
    
    /// ARM 64-bit architecture
    AARCH64,
    
    /// RISC-V 32-bit E907 architecture
    RISCV32_E907,
}

impl Arch {
    pub(crate) fn to_raw(&self) -> i32 {
        match self {
            Arch::ARM32 => super::ARCH_ARM32 as i32,
            Arch::AARCH64 => super::ARCH_AARCH64 as i32,
            Arch::RISCV32_E907 => super::ARCH_RISCV32_E907 as i32,
        }
    }
}