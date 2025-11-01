#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

#include <sys/time.h>

#include "efex-common.h"
#include "efex-payloads.h"
#include "efex-protocol.h"
#include "efex-usb.h"
#include "libefex.h"

struct progress_t {
	uint64_t total;
	uint64_t done;
	double start;
};

static struct progress_t *progress = NULL;

static double gettime(void) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec + (double) tv.tv_usec / 1000000.0;
}

static const char *format_eta(const double remaining) {
	static char result[6] = "";
	int seconds = remaining + 0.5;
	if (seconds >= 0 && seconds < 6000) {
		snprintf(result, sizeof(result), "%02d:%02d", seconds / 60, seconds % 60);
		return result;
	}
	return "--:--";
}

static char *ssize(char *buf, double size) {
	const char *unit[] = {"B", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};
	int count = 0;

	while ((size > 1024) && (count < 8)) {
		size /= 1024;
		count++;
	}
	sprintf(buf, "%5.3f %s", size, unit[count]);
	return buf;
}

static void progress_start(const ssize_t total) {
	if (progress && (total > 0)) {
		progress->total = total;
		progress->done = 0;
		progress->start = gettime();
	}
}

void progress_update(const ssize_t bytes) {
	char buf1[32] = {0}, buf2[32] = {0};

	if (progress) {
		progress->done += bytes;
		const double ratio = progress->total > 0 ? (double) progress->done / (double) progress->total : 0.0;
		const double speed = (double) progress->done / (gettime() - progress->start);
		const double eta = speed > 0 ? (progress->total - progress->done) / speed : 0;
		const int pos = 48 * ratio;
		printf("\r%3.0f%% [", ratio * 100);
		for (int i = 0; i < pos; i++)
			putchar('=');
		for (int i = pos; i < 48; i++)
			putchar(' ');
		if (progress->done < progress->total)
			printf("] %s/s, ETA %s        \r", ssize(buf1, speed), format_eta(eta));
		else
			printf("] %s, %s/s        \r", ssize(buf1, progress->done), ssize(buf2, speed));
		fflush(stdout);
	}
}

static void progress_stop() {
	if (progress)
		printf("\r\n");
}

static void print_usage(void) {
	fprintf(stderr, "usage:\n"
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
		return EFEX_ERR_INVALID_PARAM;
	errno = 0;
	const unsigned long long v = strtoull(s, NULL, 0); // auto-detect base (0x for hex)
	if (errno != 0)
		return EFEX_ERR_INVALID_PARAM;
	if (v > 0xFFFFFFFFULL)
		return EFEX_ERR_INVALID_PARAM;
	*out = (uint32_t) v;
	return EFEX_ERR_SUCCESS;
}

static int parse_size(const char *s, size_t *out) {
	if (!s)
		return EFEX_ERR_INVALID_PARAM;
	errno = 0;
	const unsigned long long v = strtoull(s, NULL, 0);
	if (errno != 0)
		return EFEX_ERR_INVALID_PARAM;
	*out = (size_t) v;
	return EFEX_ERR_SUCCESS;
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

static enum sunxi_efex_fel_payloads_arch parse_arch(const char *s) {
	if (!s)
		return ARCH_RISCV; // default
	if (strcmp(s, "arm") == 0)
		return ARCH_ARM32;
	if (strcmp(s, "aarch64") == 0)
		return ARCH_AARCH64;
	if (strcmp(s, "riscv") == 0)
		return ARCH_RISCV;
	fprintf(stderr, "Unknown payload arch '%s', defaulting to riscv\n", s);
	return ARCH_RISCV;
}

int main(const int argc, char **argv) {
	if (argc < 2) {
		print_usage();
		return 1;
	}

	// Option parsing: look for -p <arch>
	enum sunxi_efex_fel_payloads_arch arch; // default
	progress = (struct progress_t *) malloc(sizeof(struct progress_t));
	if (!progress) {
		fprintf(stderr, "ERROR: %s\n", sunxi_efex_strerror(EFEX_ERR_MEMORY));
		return 1;
	}

	int use_payloads = 0;
	for (int i = 1; i < argc - 1; ++i) {
		if (strcmp(argv[i], "-p") == 0) {
			use_payloads = 1;
			arch = parse_arch(argv[i + 1]);
			// Init payloads per selection
			int ret = sunxi_efex_fel_payloads_init(arch);
			if (ret != EFEX_ERR_SUCCESS) {
				fprintf(stderr, "ERROR: Failed to initialize payloads: %s\n", sunxi_efex_strerror(ret));
				return 1;
			}
		}
	}

	// Setup context and device
	struct sunxi_efex_ctx_t ctx = {0};
	int ret = EFEX_ERR_SUCCESS;

	ret = sunxi_scan_usb_device(&ctx);
	if (ret != EFEX_ERR_SUCCESS) {
		fprintf(stderr, "ERROR: %s\n", sunxi_efex_strerror(ret));
		return 2;
	}

	ret = sunxi_usb_init(&ctx);
	if (ret != EFEX_ERR_SUCCESS) {
		fprintf(stderr, "ERROR: %s\n", sunxi_efex_strerror(ret));
		sunxi_usb_exit(&ctx);
		return 3;
	}

	ret = sunxi_efex_init(&ctx);
	if (ret != EFEX_ERR_SUCCESS) {
		fprintf(stderr, "ERROR: %s\n", sunxi_efex_strerror(ret));
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
		int parse_ret = parse_u32(argv[2], &addr);
		if (parse_ret != EFEX_ERR_SUCCESS) {
			fprintf(stderr, "Invalid address: %s\n", sunxi_efex_strerror(parse_ret));
			exit_code = 1;
			goto cleanup;
		}
		parse_ret = parse_size(argv[3], &length);
		if (parse_ret != EFEX_ERR_SUCCESS) {
			fprintf(stderr, "Invalid length: %s\n", sunxi_efex_strerror(parse_ret));
			exit_code = 1;
			goto cleanup;
		}
		const size_t chunk = 4096;
		unsigned char *buf = (unsigned char *) malloc(chunk);
		if (!buf) {
			fprintf(stderr, "ERROR: %s\n", sunxi_efex_strerror(EFEX_ERR_MEMORY));
			exit_code = 1;
			goto cleanup;
		}
		size_t remaining = length;
		uint32_t cur = addr;
		while (remaining > 0) {
			const size_t n = remaining < chunk ? remaining : chunk;
			// payloads version only has readl/writel, no read/write functions
			ret = sunxi_efex_fel_read(&ctx, cur, (char *) buf, (ssize_t) n);
			if (ret != EFEX_ERR_SUCCESS) {
				fprintf(stderr, "ERROR: %s\n", sunxi_efex_strerror(ret));
				free(buf);
				exit_code = 5;
				goto cleanup;
			}
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
		int parse_ret = parse_u32(argv[2], &addr);
		if (parse_ret != EFEX_ERR_SUCCESS) {
			fprintf(stderr, "Invalid address: %s\n", sunxi_efex_strerror(parse_ret));
			exit_code = 1;
			goto cleanup;
		}
		parse_ret = parse_size(argv[3], &length);
		if (parse_ret != EFEX_ERR_SUCCESS) {
			fprintf(stderr, "Invalid length: %s\n", sunxi_efex_strerror(parse_ret));
			exit_code = 1;
			goto cleanup;
		}
#ifdef _WIN32
		_setmode(_fileno(stdout), _O_BINARY);
#endif
		const size_t chunk = 65536;
		unsigned char *buf = (unsigned char *) malloc(chunk);
		if (!buf) {
			fprintf(stderr, "ERROR: %s\n", sunxi_efex_strerror(EFEX_ERR_MEMORY));
			exit_code = 1;
			goto cleanup;
		}
		size_t remaining = length;
		uint32_t cur = addr;
		while (remaining > 0) {
			const size_t n = remaining < chunk ? remaining : chunk;
			ret = sunxi_efex_fel_read(&ctx, cur, (char *) buf, (ssize_t) n);
			if (ret != EFEX_ERR_SUCCESS) {
				fprintf(stderr, "ERROR: %s\n", sunxi_efex_strerror(ret));
				free(buf);
				exit_code = 5;
				goto cleanup;
			}
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
		int parse_ret = parse_u32(argv[2], &addr);
		if (parse_ret != EFEX_ERR_SUCCESS) {
			fprintf(stderr, "Invalid address: %s\n", sunxi_efex_strerror(parse_ret));
			exit_code = 1;
			goto cleanup;
		}
		uint32_t val;
		// For read32, use payloads version when -p parameter is specified
		if (use_payloads) {
			ret = sunxi_efex_fel_payloads_readl(&ctx, addr, &val);
		} else {
			// Use sunxi_efex_fel_read function to read 4 bytes for normal version
			ret = sunxi_efex_fel_read(&ctx, addr, (char *) &val, sizeof(val));
		}
		if (ret != EFEX_ERR_SUCCESS) {
			fprintf(stderr, "ERROR: %s\n", sunxi_efex_strerror(ret));
			exit_code = 5;
			goto cleanup;
		}
		printf("0x%08x\n", val);
	} else if (strcmp(cmd, "write32") == 0) {
		if (argc < 4) {
			print_usage();
			exit_code = 1;
			goto cleanup;
		}
		uint32_t addr = 0, value = 0;
		int parse_ret = parse_u32(argv[2], &addr);
		if (parse_ret != EFEX_ERR_SUCCESS) {
			fprintf(stderr, "Invalid address: %s\n", sunxi_efex_strerror(parse_ret));
			exit_code = 1;
			goto cleanup;
		}
		parse_ret = parse_u32(argv[3], &value);
		if (parse_ret != EFEX_ERR_SUCCESS) {
			fprintf(stderr, "Invalid value: %s\n", sunxi_efex_strerror(parse_ret));
			exit_code = 1;
			goto cleanup;
		}
		// For write32, use payloads version when -p parameter is specified
		if (use_payloads) {
			ret = sunxi_efex_fel_payloads_writel(&ctx, value, addr);
		} else {
			// Use sunxi_efex_fel_write function to write 4 bytes for normal version
			ret = sunxi_efex_fel_write(&ctx, addr, (const char *) &value, sizeof(value));
		}
		if (ret != EFEX_ERR_SUCCESS) {
			fprintf(stderr, "ERROR: %s\n", sunxi_efex_strerror(ret));
			exit_code = 5;
			goto cleanup;
		}
	} else if (strcmp(cmd, "read") == 0) {
		if (argc < 5) {
			print_usage();
			exit_code = 1;
			goto cleanup;
		}
		uint32_t addr = 0;
		size_t length = 0;
		int parse_ret = parse_u32(argv[2], &addr);
		if (parse_ret != EFEX_ERR_SUCCESS) {
			fprintf(stderr, "Invalid address: %s\n", sunxi_efex_strerror(parse_ret));
			exit_code = 1;
			goto cleanup;
		}
		parse_ret = parse_size(argv[3], &length);
		if (parse_ret != EFEX_ERR_SUCCESS) {
			fprintf(stderr, "Invalid length: %s\n", sunxi_efex_strerror(parse_ret));
			exit_code = 1;
			goto cleanup;
		}
		const char *file = argv[4];
		FILE *fp = fopen(file, "wb");
		if (!fp) {
			fprintf(stderr, "ERROR: %s: '%s'\n", sunxi_efex_strerror(EFEX_ERR_FILE_OPEN), file);
			exit_code = 1;
			goto cleanup;
		}
		const size_t chunk = 65536;
		unsigned char *buf = (unsigned char *) malloc(chunk);
		if (!buf) {
			fprintf(stderr, "ERROR: %s\n", sunxi_efex_strerror(EFEX_ERR_MEMORY));
			fclose(fp);
			exit_code = 1;
			goto cleanup;
		}
		size_t remaining = length;
		uint32_t cur = addr;
		progress_start(length);
		while (remaining > 0) {
			size_t n = remaining < chunk ? remaining : chunk;
			progress_update(n);
			// payloads version only has readl/writel, no read/write functions
			ret = sunxi_efex_fel_read(&ctx, cur, (char *) buf, (ssize_t) n);
			if (ret != EFEX_ERR_SUCCESS) {
				fprintf(stderr, "ERROR: %s\n", sunxi_efex_strerror(ret));
				free(buf);
				fclose(fp);
				exit_code = 5;
				goto cleanup;
			}
			fwrite(buf, 1, n, fp);
			cur += (uint32_t) n;
			remaining -= n;
		}
		progress_stop();
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
		int parse_ret = parse_u32(argv[2], &addr);
		if (parse_ret != EFEX_ERR_SUCCESS) {
			fprintf(stderr, "Invalid address: %s\n", sunxi_efex_strerror(parse_ret));
			exit_code = 1;
			goto cleanup;
		}
		FILE *fp = fopen(file, "rb");
		if (!fp) {
			fprintf(stderr, "ERROR: %s: '%s'\n", sunxi_efex_strerror(EFEX_ERR_FILE_OPEN), file);
			exit_code = 1;
			goto cleanup;
		}
		// Get file size
		fseek(fp, 0, SEEK_END);
		const long file_size = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		if (file_size <= 0) {
			fprintf(stderr, "ERROR: %s: '%s'\n", sunxi_efex_strerror(EFEX_ERR_FILE_SIZE), file);
			fclose(fp);
			exit_code = 1;
			goto cleanup;
		}
		const size_t chunk = 65536;
		unsigned char *buf = (unsigned char *) malloc(chunk);
		if (!buf) {
			fprintf(stderr, "ERROR: %s\n", sunxi_efex_strerror(EFEX_ERR_MEMORY));
			fclose(fp);
			exit_code = 1;
			goto cleanup;
		}
		size_t offset = 0;
		size_t nread;
		progress_start(file_size);
		while ((nread = fread(buf, 1, chunk, fp)) > 0) {
			progress_update(nread);
			// payloads version only has readl/writel, no read/write functions
			ret = sunxi_efex_fel_write(&ctx, addr + (uint32_t) offset, (const char *) buf, (ssize_t) nread);
			if (ret != EFEX_ERR_SUCCESS) {
				fprintf(stderr, "ERROR: %s\n", sunxi_efex_strerror(ret));
				free(buf);
				fclose(fp);
				exit_code = 5;
				goto cleanup;
			}
			offset += nread;
		}
		progress_stop();
		free(buf);
		fclose(fp);
	} else if (strcmp(cmd, "exec") == 0) {
		if (argc < 3) {
			print_usage();
			exit_code = 1;
			goto cleanup;
		}
		uint32_t addr = 0;
		int parse_ret = parse_u32(argv[2], &addr);
		if (parse_ret != EFEX_ERR_SUCCESS) {
			fprintf(stderr, "Invalid address: %s\n", sunxi_efex_strerror(parse_ret));
			exit_code = 1;
			goto cleanup;
		}
		// payloads version doesn't have exec function
		ret = sunxi_efex_fel_exec(&ctx, addr);
		if (ret != EFEX_ERR_SUCCESS) {
			fprintf(stderr, "ERROR: %s\n", sunxi_efex_strerror(ret));
			exit_code = 5;
			goto cleanup;
		}
	} else {
		print_usage();
		exit_code = 1;
	}

cleanup:
	sunxi_usb_exit(&ctx);
	if (progress)
		free(progress);
	return exit_code;
}
