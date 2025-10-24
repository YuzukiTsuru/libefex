#include <stdio.h>

#include "efex-common.h"
#include "libefex.h"


int main() {
	struct sunxi_efex_ctx_t ctx = {0};
	int ret = 0;

	// Test common efex init
	printf("Starting efex common tests\n");
	ret = sunxi_scan_usb_device(&ctx);
	if (ret != EFEX_ERR_SUCCESS) {
		fprintf(stderr, "ERROR: %s\r\n", sunxi_efex_strerror(ret));
		return ret;
	}
	ret = sunxi_usb_init(&ctx);
	if (ret != EFEX_ERR_SUCCESS) {
		fprintf(stderr, "ERROR: %s\r\n", sunxi_efex_strerror(ret));
		sunxi_usb_exit(&ctx);
		return ret;
	}
	ret = sunxi_efex_init(&ctx);
	if (ret != EFEX_ERR_SUCCESS) {
		fprintf(stderr, "ERROR: %s\r\n", sunxi_efex_strerror(ret));
		sunxi_usb_exit(&ctx);
		return ret;
	}

	printf("Found EFEX device\n");
	printf("Magic: %s\n", ctx.resp.magic);
	printf("ID: 0x%08x\n", ctx.resp.id);
	printf("Firmware: 0x%08x\n", ctx.resp.firmware);
	printf("Mode: 0x%04x\n", ctx.resp.mode);
	printf("Data Flag: 0x%02x\n", ctx.resp.data_flag);
	printf("Data Length: 0x%02x\n", ctx.resp.data_length);
	printf("Data Start Address: 0x%08x\n", ctx.resp.data_start_address);

	printf("Reserved: ");
	for (int i = 0; i < sizeof(ctx.resp.reserved); i++) {
		printf("%02x ", (unsigned char) ctx.resp.reserved[i]);
	}
	printf("\n");

	printf("efex common tests done\n");

	// Test writel/readl
	uint32_t read_val;
	ret = sunxi_efex_fel_read(&ctx, ctx.resp.data_start_address, (char *) &read_val, sizeof(read_val));
	if (ret == EFEX_ERR_SUCCESS) {
		printf("Orig: 0x%08x\n", read_val);
	} else {
		fprintf(stderr, "Failed to read register: %s\n", sunxi_efex_strerror(ret));
	}

	uint32_t write_val = 0x55AA55AA;
	ret = sunxi_efex_fel_write(&ctx, ctx.resp.data_start_address, (const char *) &write_val, sizeof(write_val));
	if (ret != EFEX_ERR_SUCCESS) {
		fprintf(stderr, "Failed to write register: %s\n", sunxi_efex_strerror(ret));
	}

	ret = sunxi_efex_fel_read(&ctx, ctx.resp.data_start_address, (char *) &read_val, sizeof(read_val));
	if (ret == EFEX_ERR_SUCCESS) {
		printf("New: 0x%08x\n", read_val);
	} else {
		fprintf(stderr, "Failed to read register: %s\n", sunxi_efex_strerror(ret));
	}

	// Test payload readl/writel
	sunxi_efex_fel_payloads_init(ARCH_RISCV32_E907);

	uint32_t id[4] = {0};
	int errors[4] = {0};

	// Execute all read operations first
	uint32_t id0_val, id1_val, id2_val, id3_val;
	errors[0] = sunxi_efex_fel_payloads_readl(&ctx, 0x03006200 + 0x0, &id0_val);
	errors[1] = sunxi_efex_fel_payloads_readl(&ctx, 0x03006200 + 0x4, &id1_val);
	errors[2] = sunxi_efex_fel_payloads_readl(&ctx, 0x03006200 + 0x8, &id2_val);
	errors[3] = sunxi_efex_fel_payloads_readl(&ctx, 0x03006200 + 0xc, &id3_val);

	// Then handle errors and set values
	for (int i = 0; i < 4; i++) {
		if (errors[i] != EFEX_ERR_SUCCESS) {
			fprintf(stderr, "Failed to read payload %d: %s\n", i, sunxi_efex_strerror(errors[i]));
		}
	}

	// Set successfully read values
	if (errors[0] == EFEX_ERR_SUCCESS)
		id[0] = id0_val;
	if (errors[1] == EFEX_ERR_SUCCESS)
		id[1] = id1_val;
	if (errors[2] == EFEX_ERR_SUCCESS)
		id[2] = id2_val;
	if (errors[3] == EFEX_ERR_SUCCESS)
		id[3] = id3_val;

	// Finally output the result
	printf("sid: %08x%08x%08x%08x\n", id[0], id[1], id[2], id[3]);

	uint32_t reg_val;
	int ret_val = sunxi_efex_fel_payloads_readl(&ctx, 0x02001000, &reg_val);
	if (ret_val != EFEX_ERR_SUCCESS)
		reg_val = ret_val;
	if (reg_val < 0) {
		fprintf(stderr, "Failed to read payload register: %s\n", sunxi_efex_strerror(reg_val));
	} else {
		printf("reg_val: 0x%08x\n", reg_val);
		reg_val |= (1 << 31);
		printf("reg_val: 0x%08x\n", reg_val);

		ret = sunxi_efex_fel_payloads_writel(&ctx, reg_val, 0x02001000);
		if (ret != EFEX_ERR_SUCCESS) {
			fprintf(stderr, "Failed to write payload register: %s\n", sunxi_efex_strerror(ret));
		}
	}

	ret = sunxi_efex_fel_payloads_writel(&ctx, (0x16aa << 16) | (0x1 << 0), 0x06012000 + 0x08);
	if (ret != EFEX_ERR_SUCCESS) {
		fprintf(stderr, "Failed to write payload register: %s\n", sunxi_efex_strerror(ret));
	}

	uint32_t reg_val2;
	int ret_val2 = sunxi_efex_fel_payloads_readl(&ctx, 0x06012000, &reg_val2);
	if (ret_val2 != EFEX_ERR_SUCCESS) {
		fprintf(stderr, "Failed to read payload register: %s\n", sunxi_efex_strerror(ret_val2));
	} else {
		reg_val = reg_val2;
	}
	if (reg_val < 0) {
		fprintf(stderr, "Failed to read payload register: %s\n", sunxi_efex_strerror(reg_val));
	} else {
		printf("reg_val: 0x%08x\n", reg_val);
	}

	sunxi_usb_exit(&ctx);
	return 0;
}
