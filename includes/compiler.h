#ifndef EFEX_COMPILER_H
#define EFEX_COMPILER_H

// Define cross-platform packing macros to address MSVC's lack of support for __attribute__((packed))
#ifdef _MSC_VER
// Microsoft Visual C++ compiler
#define EFEX_PACKED_BEGIN __pragma(pack(push, 1))
#define EFEX_PACKED_END __pragma(pack(pop))
#define EFEX_PACKED 
// 抑制特定警告
#pragma warning(disable: 4018) // 有符号/无符号不匹配比较
#pragma warning(disable: 4267) // 从size_t转换到较小整数类型
#pragma warning(disable: 4996) // 不安全的函数（如strcpy）
#define _CRT_SECURE_NO_WARNINGS // 禁用CRT安全警告
#else
// GCC, Clang and other compilers
#define EFEX_PACKED_BEGIN
#define EFEX_PACKED_END
#define EFEX_PACKED __attribute__((packed))
#endif

#endif // EFEX_COMPILER_H