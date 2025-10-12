#ifndef EFEX_ENDING_H
#define EFEX_ENDING_H

#include <stdint.h>

#if (defined(_WIN16) || defined(_WIN32) || defined(_WIN64))
# ifndef __WINDOWS__
#  define __WINDOWS__
# endif
#endif
#if defined(__linux__) || defined(__CYGWIN__)
# include <endian.h>
#elif defined(__MACH__)
# include <sys/types.h>
#elif defined(__OpenBSD__) || defined(__NetBSD__) || defined(__FreeBSD__) || defined(__DragonFly__)
# include <sys/endian.h>
#elif defined(__WINDOWS__)
# include <windows.h>
#ifndef _MSC_VER
# include <sys/param.h>
#else
#define BIG_ENDIAN	4321
#define LITTLE_ENDIAN	1234
# define BYTE_ORDER	LITTLE_ENDIAN
#endif
#else
# error "platform not supported!"
#endif

/*
 * byteorder
 */
#define SWAB16(x)  (((x) << 8) | ((x) >> 8))

#define SWAB32(x)  (((x) << 24) | ((x) >> 24) | \
    (((x) & (uint32_t) 0x0000ff00UL) << 8) | \
    (((x) & (uint32_t) 0x00ff0000UL) >> 8))

#define SWAB64(x)  (((x) << 56) | ((x) >> 56) | \
    (((x) & (uint64_t) 0x000000000000ff00ULL) << 40) | \
    (((x) & (uint64_t) 0x0000000000ff0000ULL) << 24) | \
    (((x) & (uint64_t) 0x00000000ff000000ULL) << 8) | \
    (((x) & (uint64_t) 0x000000ff00000000ULL) >> 8) | \
    (((x) & (uint64_t) 0x0000ff0000000000ULL) >> 24) | \
    (((x) & (uint64_t) 0x00ff000000000000ULL) >> 40))

#if BYTE_ORDER == BIG_ENDIAN
#define cpu_to_le64(x)	(SWAB64((uint64_t)(x)))
#define le64_to_cpu(x)	(SWAB64((uint64_t)(x)))
#define cpu_to_le32(x)	(SWAB32((uint32_t)(x)))
#define le32_to_cpu(x)	(SWAB32((uint32_t)(x)))
#define cpu_to_le16(x)	(SWAB16((uint16_t)(x)))
#define le16_to_cpu(x)	(SWAB16((uint16_t)(x)))
#define cpu_to_be64(x)	((uint64_t)(x))
#define be64_to_cpu(x)	((uint64_t)(x))
#define cpu_to_be32(x)	((uint32_t)(x))
#define be32_to_cpu(x)	((uint32_t)(x))
#define cpu_to_be16(x)	((uint16_t)(x))
#define be16_to_cpu(x)	((uint16_t)(x))
#else
#define cpu_to_le64(x)	((uint64_t)(x))
#define le64_to_cpu(x)	((uint64_t)(x))
#define cpu_to_le32(x)	((uint32_t)(x))
#define le32_to_cpu(x)	((uint32_t)(x))
#define cpu_to_le16(x)	((uint16_t)(x))
#define le16_to_cpu(x)	((uint16_t)(x))
#define cpu_to_be64(x)	(SWAB64((uint64_t)(x)))
#define be64_to_cpu(x)	(SWAB64((uint64_t)(x)))
#define cpu_to_be32(x)	(SWAB32((uint32_t)(x)))
#define be32_to_cpu(x)	(SWAB32((uint32_t)(x)))
#define cpu_to_be16(x)	(SWAB16((uint16_t)(x)))
#define be16_to_cpu(x)	(SWAB16((uint16_t)(x)))
#endif

#endif //EFEX_ENDING_H
