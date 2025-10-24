#ifndef EFEX_COMPILER_H
#define EFEX_COMPILER_H

#ifdef __cplusplus
extern "C" {
#endif

// Define cross-platform packing macros to address MSVC's lack of support for __attribute__((packed))
#ifdef _MSC_VER
#include <BaseTsd.h>
#include <sys/types.h>

#define ssize_t SSIZE_T
// Microsoft Visual C++ compiler
#define EFEX_PACKED_BEGIN __pragma(pack(push, 1))
#define EFEX_PACKED_END __pragma(pack(pop))
#define EFEX_PACKED
#pragma warning(disable : 4018)
#pragma warning(disable : 4267)
#pragma warning(disable : 4996)
#define _CRT_SECURE_NO_WARNINGS
#else
// GCC, Clang and other compilers
#define EFEX_PACKED_BEGIN
#define EFEX_PACKED_END
#define EFEX_PACKED __attribute__((packed))
#endif


#ifdef __cplusplus
}
#endif

#endif // EFEX_COMPILER_H
