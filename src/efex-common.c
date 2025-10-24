#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compiler.h"
#include "efex-common.h"
#include "efex-protocol.h"
#include "efex-usb.h"
#include "ending.h"

int sunxi_send_efex_request(const struct sunxi_efex_ctx_t *ctx, const enum sunxi_efex_cmd_t type, const uint32_t addr,
							const uint32_t length) {
	if (!ctx) {
		return EFEX_ERR_NULL_PTR;
	}

	const struct sunxi_efex_request_t req = {
			.cmd	 = cpu_to_le16(type),
			.tag	 = 0x0,
			.address = cpu_to_le32(addr),
			.len	 = cpu_to_le32(length),
	};

	const int ret = sunxi_usb_write(ctx, &req, sizeof(struct sunxi_efex_request_t));
	if (ret != 0) {
		return EFEX_ERR_USB_TRANSFER;
	}

	return EFEX_ERR_SUCCESS;
}

int sunxi_read_efex_status(const struct sunxi_efex_ctx_t *ctx) {
	if (!ctx) {
		return EFEX_ERR_NULL_PTR;
	}

	const struct sunxi_efex_response_t resp = {0};
	const int ret							= sunxi_usb_read(ctx, &resp, sizeof(resp));
	if (ret != 0) {
		return EFEX_ERR_USB_TRANSFER;
	}
	return resp.status;
}

enum sunxi_verify_device_mode_t sunxi_efex_get_device_mode(const struct sunxi_efex_ctx_t *ctx) {
	return ctx->resp.mode;
}

const char *sunxi_efex_get_device_mode_str(const struct sunxi_efex_ctx_t *ctx) {
	const enum sunxi_verify_device_mode_t mode = sunxi_efex_get_device_mode(ctx);

	switch (mode) {
		case DEVICE_MODE_NULL:
			return "DEVICE_MODE_NULL";
		case DEVICE_MODE_FEL:
			return "DEVICE_MODE_FEL";
		case DEVICE_MODE_SRV:
			return "DEVICE_MODE_SRV";
		case DEVICE_MODE_UPDATE_COOL:
			return "DEVICE_MODE_UPDATE_COOL";
		case DEVICE_MODE_UPDATE_HOT:
			return "DEVICE_MODE_UPDATE_HOT";
		default:
			return "UNKNOWN_DEVICE_MODE";
	}
}

int sunxi_efex_init(struct sunxi_efex_ctx_t *ctx) {
	if (!ctx) {
		return EFEX_ERR_NULL_PTR;
	}

	// Send request to find chips
	int ret = sunxi_send_efex_request(ctx, EFEX_CMD_VERIFY_DEVICE, 0, 0);
	if (ret != EFEX_ERR_SUCCESS) {
		return ret;
	}

	// Read response
	ret = sunxi_usb_read(ctx, &ctx->resp, sizeof(ctx->resp));
	if (ret != EFEX_ERR_SUCCESS) {
		return EFEX_ERR_USB_TRANSFER;
	}

	// Read status
	ret = sunxi_read_efex_status(ctx);
	if (ret < 0) {
		return ret;
	}

	// Process the chip data
	ctx->resp.id				 = le32_to_cpu(ctx->resp.id);
	ctx->resp.firmware			 = le32_to_cpu(ctx->resp.firmware);
	ctx->resp.mode				 = le16_to_cpu(ctx->resp.mode);
	ctx->resp.data_start_address = le32_to_cpu(ctx->resp.data_start_address);
	ctx->resp.data_length		 = ctx->resp.data_length;
	ctx->resp.data_flag			 = ctx->resp.data_flag;

	return EFEX_ERR_SUCCESS;
}

const char *sunxi_efex_strerror(const int error_code) {
	switch (error_code) {
		case EFEX_ERR_SUCCESS:
			return "Success";
		case EFEX_ERR_INVALID_PARAM:
			return "Invalid parameter";
		case EFEX_ERR_NULL_PTR:
			return "Null pointer error";
		case EFEX_ERR_MEMORY:
			return "Memory allocation error";
		case EFEX_ERR_USB_INIT:
			return "USB initialization failed";
		case EFEX_ERR_USB_DEVICE_NOT_FOUND:
			return "Device not found";
		case EFEX_ERR_USB_OPEN:
			return "Failed to open device";
		case EFEX_ERR_USB_TRANSFER:
			return "USB transfer failed";
		case EFEX_ERR_USB_TIMEOUT:
			return "USB transfer timeout";
		case EFEX_ERR_PROTOCOL:
			return "Protocol error";
		case EFEX_ERR_INVALID_RESPONSE:
			return "Invalid response from device";
		case EFEX_ERR_UNEXPECTED_STATUS:
			return "Unexpected status code";
		case EFEX_ERR_OPERATION_FAILED:
			return "Operation failed";
		case EFEX_ERR_DEVICE_BUSY:
			return "Device is busy";
		case EFEX_ERR_DEVICE_NOT_READY:
			return "Device not ready";
		case EFEX_ERR_FLASH_ACCESS:
			return "Flash access error";
		case EFEX_ERR_FLASH_SIZE_PROBE:
			return "Flash size probing failed";
		case EFEX_ERR_FLASH_SET_ONOFF:
			return "Failed to set flash on/off";
		case EFEX_ERR_VERIFICATION:
			return "Verification failed";
		case EFEX_ERR_CRC_MISMATCH:
			return "CRC mismatch error";
		case EFEX_ERR_FILE_OPEN:
			return "Failed to open file";
		case EFEX_ERR_FILE_READ:
			return "Failed to read file";
		case EFEX_ERR_FILE_WRITE:
			return "Failed to write file";
		case EFEX_ERR_FILE_SIZE:
			return "File size error";
		case EFEX_ERR_INVALID_DEVICE_MODE:
			return "Invalid device mode";
		default:
			return "Unknown error";
	}
}
