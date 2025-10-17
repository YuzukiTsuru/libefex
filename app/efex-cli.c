#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#endif

#include "libefex.h"
#include "fel-protocol.h"
#include "fel-payloads.h"
#include "usb_layer.h"

static void print_usage(void) {
    fprintf(stderr,
            "usage:\n"
            "    efex version                                        - Show chip version\n"
            "    efex hexdump <address> <length>                     - Dumps memory region in hex\n"
            "    efex dump <address> <length>                        - Binary memory dump to stdout\n"
            "    efex read32 <address>                               - Read 32-bits value from device memory\n"
            "    efex write32 <address> <value>                      - Write 32-bits value to device memory\n"
            "    efex read <address> <length> <file>                 - Read memory to file\n"
            "    efex write <address> <file>                         - Write file to memory\n"
            "    efex exec <address>                                 - Call function address\n"
            "[options]\n"
            "     -p payloads [arm, aarch64, e907]\n");
}

static int parse_u32(const char *s, uint32_t *out) {
    if (!s)
        return -1;
    errno = 0;
    const unsigned long long v = strtoull(s, NULL, 0); // auto-detect base (0x for hex)
    if (errno != 0)
        return -1;
    if (v > 0xFFFFFFFFULL)
        return -1;
    *out = (uint32_t) v;
    return 0;
}

static int parse_size(const char *s, size_t *out) {
    if (!s)
        return -1;
    errno = 0;
    const unsigned long long v = strtoull(s, NULL, 0);
    if (errno != 0)
        return -1;
    *out = (size_t) v;
    return 0;
}

static void hex_dump_region(const uint32_t base, const unsigned char *buf, const size_t len) {
    for (size_t j = 0; j < len; j += 16) {
        printf("%08x: ", (unsigned) (base + (uint32_t) j));
        for (size_t i = 0; i < 16; i++) {
            if (j + i < len)
                printf("%02x ", buf[j + i]);
            else
                printf("   ");
        }
        putchar(' ');
        for (size_t i = 0; i < 16; i++) {
            if (j + i < len) {
                const unsigned char c = buf[j + i];
                putchar((c >= 32 && c <= 126) ? c : '.');
            } else {
                putchar(' ');
            }
        }
        putchar('\n');
    }
}

static enum sunxi_fel_payloads_arch parse_arch(const char *s) {
    if (!s)
        return PAYLOAD_ARCH_RISCV32_E907; // default
    if (strcmp(s, "arm") == 0)
        return PAYLOAD_ARCH_ARM32;
    if (strcmp(s, "aarch64") == 0)
        return PAYLOAD_ARCH_AARCH64;
    if (strcmp(s, "e907") == 0)
        return PAYLOAD_ARCH_RISCV32_E907;
    fprintf(stderr, "Unknown payload arch '%s', defaulting to e907\n", s);
    return PAYLOAD_ARCH_RISCV32_E907;
}

int main(const int argc, char **argv) {
    if (argc < 2) {
        print_usage();
        return 1;
    }

    // Option parsing: look for -p <arch>
    enum sunxi_fel_payloads_arch arch = PAYLOAD_ARCH_RISCV32_E907; // default
    for (int i = 1; i < argc - 1; ++i) {
        if (strcmp(argv[i], "-p") == 0) {
            arch = parse_arch(argv[i + 1]);
        }
    }

    // Init payloads per selection
    sunxi_fel_payloads_init(arch);

    // Setup context and device
    struct sunxi_fel_ctx_t ctx = {0};
    if (sunxi_scan_usb_device(&ctx) <= 0) {
        fprintf(stderr, "ERROR: No matching USB device found\n");
        return 2;
    }
    if (sunxi_usb_init(&ctx) <= 0) {
        fprintf(stderr, "ERROR: Failed to initialize USB device\n");
        sunxi_usb_exit(&ctx);
        return 3;
    }
    if (sunxi_fel_init(&ctx) != 0) {
        fprintf(stderr, "ERROR: FEL init failed\n");
        sunxi_usb_exit(&ctx);
        return 4;
    }

    int exit_code = 0;

    const char *cmd = argv[1];
    if (strcmp(cmd, "version") == 0) {
        printf("Chip ID      : 0x%08x\n", ctx.resp.id);
        printf("Firmware     : 0x%08x\n", ctx.resp.firmware);
        printf("Mode         : 0x%04x\n", ctx.resp.mode);
        printf("Data Addr    : 0x%08x\n", ctx.resp.data_start_address);
        printf("Data Length  : %u\n", (unsigned) ctx.resp.data_length);
        printf("Data Flag    : %u\n", (unsigned) ctx.resp.data_flag);
    } else if (strcmp(cmd, "hexdump") == 0) {
        if (argc < 4) {
            print_usage();
            exit_code = 1;
            goto cleanup;
        }
        uint32_t addr = 0;
        size_t length = 0;
        if (parse_u32(argv[2], &addr) != 0 || parse_size(argv[3], &length) != 0) {
            fprintf(stderr, "Invalid address/length\n");
            exit_code = 1;
            goto cleanup;
        }
        const size_t chunk = 4096;
        unsigned char *buf = (unsigned char *) malloc(chunk);
        if (!buf) {
            fprintf(stderr, "Out of memory\n");
            exit_code = 1;
            goto cleanup;
        }
        size_t remaining = length;
        uint32_t cur = addr;
        while (remaining > 0) {
            const size_t n = remaining < chunk ? remaining : chunk;
            sunxi_fel_read_memory(&ctx, cur, (const char *) buf, (ssize_t) n);
            hex_dump_region(cur, buf, n);
            cur += (uint32_t) n;
            remaining -= n;
        }
        free(buf);
    } else if (strcmp(cmd, "dump") == 0) {
        if (argc < 4) {
            print_usage();
            exit_code = 1;
            goto cleanup;
        }
        uint32_t addr = 0;
        size_t length = 0;
        if (parse_u32(argv[2], &addr) != 0 || parse_size(argv[3], &length) != 0) {
            fprintf(stderr, "Invalid address/length\n");
            exit_code = 1;
            goto cleanup;
        }
#ifdef _WIN32
        _setmode(_fileno(stdout), _O_BINARY);
#endif
        const size_t chunk = 65536;
        unsigned char *buf = (unsigned char *) malloc(chunk);
        if (!buf) {
            fprintf(stderr, "Out of memory\n");
            exit_code = 1;
            goto cleanup;
        }
        size_t remaining = length;
        uint32_t cur = addr;
        while (remaining > 0) {
            const size_t n = remaining < chunk ? remaining : chunk;
            sunxi_fel_read_memory(&ctx, cur, (const char *) buf, (ssize_t) n);
            fwrite(buf, 1, n, stdout);
            cur += (uint32_t) n;
            remaining -= n;
        }
        free(buf);
    } else if (strcmp(cmd, "read32") == 0) {
        if (argc < 3) {
            print_usage();
            exit_code = 1;
            goto cleanup;
        }
        uint32_t addr = 0;
        if (parse_u32(argv[2], &addr) != 0) {
            fprintf(stderr, "Invalid address\n");
            exit_code = 1;
            goto cleanup;
        }
        const uint32_t val = sunxi_fel_payloads_readl(&ctx, addr);
        printf("0x%08x\n", val);
    } else if (strcmp(cmd, "write32") == 0) {
        if (argc < 4) {
            print_usage();
            exit_code = 1;
            goto cleanup;
        }
        uint32_t addr = 0, value = 0;
        if (parse_u32(argv[2], &addr) != 0 || parse_u32(argv[3], &value) != 0) {
            fprintf(stderr, "Invalid address/value\n");
            exit_code = 1;
            goto cleanup;
        }
        sunxi_fel_payloads_writel(&ctx, value, addr);
    } else if (strcmp(cmd, "read") == 0) {
        if (argc < 5) {
            print_usage();
            exit_code = 1;
            goto cleanup;
        }
        uint32_t addr = 0;
        size_t length = 0;
        if (parse_u32(argv[2], &addr) != 0 || parse_size(argv[3], &length) != 0) {
            fprintf(stderr, "Invalid address/length\n");
            exit_code = 1;
            goto cleanup;
        }
        const char *file = argv[4];
        FILE *fp = fopen(file, "wb");
        if (!fp) {
            fprintf(stderr, "Failed to open file '%s'\n", file);
            exit_code = 1;
            goto cleanup;
        }
        const size_t chunk = 65536;
        unsigned char *buf = (unsigned char *) malloc(chunk);
        if (!buf) {
            fprintf(stderr, "Out of memory\n");
            fclose(fp);
            exit_code = 1;
            goto cleanup;
        }
        size_t remaining = length;
        uint32_t cur = addr;
        while (remaining > 0) {
            size_t n = remaining < chunk ? remaining : chunk;
            sunxi_fel_read_memory(&ctx, cur, (const char *) buf, (ssize_t) n);
            fwrite(buf, 1, n, fp);
            cur += (uint32_t) n;
            remaining -= n;
        }
        free(buf);
        fclose(fp);
    } else if (strcmp(cmd, "write") == 0) {
        if (argc < 4) {
            print_usage();
            exit_code = 1;
            goto cleanup;
        }
        uint32_t addr = 0;
        const char *file = argv[3];
        if (parse_u32(argv[2], &addr) != 0) {
            fprintf(stderr, "Invalid address\n");
            exit_code = 1;
            goto cleanup;
        }
        FILE *fp = fopen(file, "rb");
        if (!fp) {
            fprintf(stderr, "Failed to open file '%s'\n", file);
            exit_code = 1;
            goto cleanup;
        }
        const size_t chunk = 65536;
        unsigned char *buf = (unsigned char *) malloc(chunk);
        if (!buf) {
            fprintf(stderr, "Out of memory\n");
            fclose(fp);
            exit_code = 1;
            goto cleanup;
        }
        size_t offset = 0;
        size_t nread;
        while ((nread = fread(buf, 1, chunk, fp)) > 0) {
            sunxi_fel_write_memory(&ctx, addr + (uint32_t) offset, (const char *) buf, (ssize_t) nread);
            offset += nread;
        }
        free(buf);
        fclose(fp);
    } else if (strcmp(cmd, "exec") == 0) {
        if (argc < 3) {
            print_usage();
            exit_code = 1;
            goto cleanup;
        }
        uint32_t addr = 0;
        if (parse_u32(argv[2], &addr) != 0) {
            fprintf(stderr, "Invalid address\n");
            exit_code = 1;
            goto cleanup;
        }
        sunxi_fel_exec(&ctx, addr);
    } else {
        print_usage();
        exit_code = 1;
    }

cleanup:
    sunxi_usb_exit(&ctx);
    return exit_code;
}
