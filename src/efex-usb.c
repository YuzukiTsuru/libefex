#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "efex-common.h"
#include "efex-protocol.h"
#include "efex-usb.h"
#include "ending.h"


int sunxi_send_usb_request(const struct sunxi_efex_ctx_t *ctx, const enum sunxi_efex_usb_request_t type,
						   const size_t length) {
	if (!ctx || !ctx->hdl) {
		return EFEX_ERR_NULL_PTR;
	}

	struct sunxi_usb_request_t req = {
			.magic			= SUNXI_USB_REQ_MAGIC,
			.tab			= 0x0,
			.data_length	= cpu_to_le32(length),
			.cmd_length		= SUNXI_EFEX_CMD_LEN,
			.cmd_package[0] = type,
	};
	req.cmd_length = (uint8_t) req.data_length;

	const int ret = sunxi_usb_bulk_send(ctx->hdl, ctx->epout, (const char *) &req, sizeof(struct sunxi_usb_request_t));
	if (ret != 0) {
		return EFEX_ERR_USB_TRANSFER;
	}
	return EFEX_ERR_SUCCESS;
}

int sunxi_read_usb_response(const struct sunxi_efex_ctx_t *ctx) {
	if (!ctx || !ctx->hdl) {
		return EFEX_ERR_NULL_PTR;
	}

	struct sunxi_usb_response_t resp = {0};

	const int ret = sunxi_usb_bulk_recv(ctx->hdl, ctx->epin, (char *) &resp, sizeof(resp));
	if (ret != 0) {
		return EFEX_ERR_USB_TRANSFER;
	}

	if (strncmp(resp.magic, SUNXI_USB_RSP_MAGIC, 4) != 0) {
		return EFEX_ERR_INVALID_RESPONSE;
	}

	return resp.status;
}

int sunxi_usb_write(const struct sunxi_efex_ctx_t *ctx, const void *buf, const size_t len) {
	if (!ctx || !buf) {
		return EFEX_ERR_NULL_PTR;
	}

	int ret = sunxi_send_usb_request(ctx, AW_USB_WRITE, len);
	if (ret != EFEX_ERR_SUCCESS) {
		return ret;
	}

	ret = sunxi_usb_bulk_send(ctx->hdl, ctx->epout, buf, len);
	if (ret != 0) {
		return EFEX_ERR_USB_TRANSFER;
	}

	ret = sunxi_read_usb_response(ctx);
	if (ret != 0) {
		return EFEX_ERR_PROTOCOL;
	}
	return EFEX_ERR_SUCCESS;
}

int sunxi_usb_read(const struct sunxi_efex_ctx_t *ctx, const void *data, const size_t len) {
	if (!ctx || !data) {
		return EFEX_ERR_NULL_PTR;
	}

	int ret = sunxi_send_usb_request(ctx, AW_USB_READ, len);
	if (ret != EFEX_ERR_SUCCESS) {
		return ret;
	}

	ret = sunxi_usb_bulk_recv(ctx->hdl, ctx->epin, (char *) data, len);
	if (ret != 0) {
		return EFEX_ERR_USB_TRANSFER;
	}

	ret = sunxi_read_usb_response(ctx);
	if (ret != 0) {
		return EFEX_ERR_PROTOCOL;
	}
	return EFEX_ERR_SUCCESS;
}

int sunxi_usb_fes_xfer(const struct sunxi_efex_ctx_t *ctx, const enum sunxi_usb_fes_xfer_type_t type,
					   const uint32_t cmd, const char *request_buf, const ssize_t request_len, const char *buf,
					   const ssize_t len) {
	if (!ctx) {
		return EFEX_ERR_NULL_PTR;
	}

	if (ctx->resp.mode != DEVICE_MODE_SRV) {
		return EFEX_ERR_INVALID_PARAM;
	}

	struct sunxi_fes_xfer_t fes_xfer = {
			.cmd   = cpu_to_le16((uint16_t) cmd),
			.tag   = 0x0,
			.magic = SUNXI_USB_REQ_MAGIC,
	};

	if (request_len > 0 && request_len <= sizeof(fes_xfer.buf)) {
		memcpy(fes_xfer.buf, request_buf, request_len);
	}

	int ret = sunxi_usb_bulk_send(ctx->hdl, ctx->epout, (const char *) &fes_xfer, sizeof(fes_xfer));
	if (ret != 0) {
		return EFEX_ERR_USB_TRANSFER;
	}

	if (type == FES_XFER_SEND && len > 0) {
		if (!buf) {
			return EFEX_ERR_NULL_PTR;
		}
		ret = sunxi_usb_bulk_send(ctx->hdl, ctx->epout, buf, len);
		if (ret != 0) {
			return EFEX_ERR_USB_TRANSFER;
		}
	} else if (type == FES_XFER_RECV && len > 0) {
		if (!buf) {
			return EFEX_ERR_NULL_PTR;
		}
		ret = sunxi_usb_bulk_recv(ctx->hdl, ctx->epin, (char *) buf, len);
		if (ret != 0) {
			return EFEX_ERR_USB_TRANSFER;
		}
	}

	ret = sunxi_read_usb_response(ctx);
	if (ret != 0) {
		return EFEX_ERR_PROTOCOL;
	}

	return EFEX_ERR_SUCCESS;
}

void sunxi_usb_hex_dump(const void *buf, size_t len, const char *type) {
#if DEBUG_USB_TRANSFER
	if (!buf) {
		fprintf(stdout, "USB %s len=0\n", type ? type : "");
		fprintf(stdout, "<empty>\n");
		return;
	}

	fprintf(stdout, "USB %s len=%zu\n", type ? type : "", len);

	const unsigned char *p = (const unsigned char *) buf;
	for (size_t j = 0; j < len; j += 16) {
		fprintf(stdout, "%08zx: ", j);
		// hex bytes
		for (size_t i = 0; i < 16; i++) {
			if (j + i < len)
				fprintf(stdout, "%02x ", p[j + i]);
			else
				fprintf(stdout, "   ");
		}
		fputc(' ', stdout);
		// ASCII
		for (size_t i = 0; i < 16; i++) {
			if (j + i >= len)
				fputc(' ', stdout);
			else
				fputc(isprint(p[j + i]) ? p[j + i] : '.', stdout);
		}
		fputc('\n', stdout);
	}
#endif /* DEBUG_USB_TRANSFER */
}
