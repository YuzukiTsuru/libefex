#ifndef FEX_COMPILER_H
#define FEX_COMPILER_H

// Define cross-platform packing macros to address MSVC's lack of support for __attribute__((packed))
#ifdef _MSC_VER
// Microsoft Visual C++ compiler
#define FEX_PACKED_BEGIN __pragma(pack(push, 1))
#define FEX_PACKED_END __pragma(pack(pop))
#define FEX_PACKED 
#else
// GCC, Clang and other compilers
#define FEX_PACKED_BEGIN
#define FEX_PACKED_END
#define FEX_PACKED __attribute__((packed))
#endif

#endif // FEX_COMPILER_H