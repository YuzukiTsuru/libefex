#ifndef EFEX_COMPILER_H
#define EFEX_COMPILER_H

// Define cross-platform packing macros to address MSVC's lack of support for __attribute__((packed))
#ifdef _MSC_VER
// Microsoft Visual C++ compiler
#define EFEX_PACKED_BEGIN __pragma(pack(push, 1))
#define EFEX_PACKED_END __pragma(pack(pop))
#define EFEX_PACKED 
#else
// GCC, Clang and other compilers
#define EFEX_PACKED_BEGIN
#define EFEX_PACKED_END
#define EFEX_PACKED __attribute__((packed))
#endif

#endif // EFEX_COMPILER_H