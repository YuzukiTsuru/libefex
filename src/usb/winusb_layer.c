#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "efex-common.h"
#include "efex-protocol.h"
#include "efex-usb.h"
#include "ending.h"

#ifdef _WIN32
#include <SetupAPI.h>
#include <initguid.h>
#include <usbiodef.h>
#include <windows.h>


static int match_vid_pid(const char *device_path) {
	if (!device_path)
		return 0;
	char pattern[64];
	snprintf(pattern, sizeof(pattern), "vid_%04x&pid_%04x", SUNXI_USB_VENDOR, SUNXI_USB_PRODUCT);
	const size_t n = strlen(device_path);

	char *lower = (char *) malloc(n + 1);

	if (!lower)
		return 0;
	for (size_t i = 0; i < n; ++i)
		lower[i] = (char) tolower((unsigned char) device_path[i]);

	lower[n] = '\0';

	const int found = strstr(lower, pattern) != NULL;
	free(lower);
	return found;
}

int sunxi_usb_bulk_send(void *handle, const int ep, const char *buf, ssize_t len) {
	if (!handle || !buf || len <= 0) {
		return EFEX_ERR_NULL_PTR;
	}

	HANDLE const usb_handle = (HANDLE) handle;
	const size_t max_chunk	= 128 * 1024;
	DWORD bytes_sent		= 0;

	while (len > 0) {
		const size_t chunk = len <= max_chunk ? len : max_chunk;

		sunxi_usb_hex_dump(buf, chunk, "SEND");

		const DWORD dwIoControlCode = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0807, METHOD_OUT_DIRECT, FILE_ANY_ACCESS);

		BOOL const result =
				DeviceIoControl(usb_handle, dwIoControlCode, NULL, (DWORD) 0, (LPVOID) buf, chunk, &bytes_sent, NULL);

		if (!result || bytes_sent == 0) {
			return EFEX_ERR_USB_TRANSFER;
		}

		len -= bytes_sent;
		buf += bytes_sent;
	}
	return EFEX_ERR_SUCCESS;
}

int sunxi_usb_bulk_recv(void *handle, const int ep, char *buf, const ssize_t len) {
	if (!handle || !buf || len <= 0) {
		return EFEX_ERR_NULL_PTR;
	}

	HANDLE const usb_handle = (HANDLE) handle;
	DWORD bytes_received	= 0;

	const DWORD dwIoControlCode = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0808, METHOD_IN_DIRECT, FILE_ANY_ACCESS);

	const BOOL result =
			DeviceIoControl(usb_handle, dwIoControlCode, NULL, 0, (LPVOID) buf, (DWORD) len, &bytes_received, NULL);

	if (!result || bytes_received == 0) {
		return EFEX_ERR_USB_TRANSFER;
	}

	sunxi_usb_hex_dump(buf, (size_t) bytes_received, "RECV");

	return EFEX_ERR_SUCCESS;
}

int sunxi_scan_usb_device(struct sunxi_efex_ctx_t *ctx) {
	if (!ctx) {
		return EFEX_ERR_NULL_PTR;
	}

	SP_DEVICE_INTERFACE_DATA interface_data;
	ULONG index		  = 0;
	BOOL device_found = FALSE;

	const LPGUID usb_device_guid = (LPGUID) &GUID_DEVINTERFACE_USB_DEVICE;

	const HDEVINFO device_info_set =
			SetupDiGetClassDevs(usb_device_guid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

	if (device_info_set == INVALID_HANDLE_VALUE) {
		return EFEX_ERR_USB_INIT;
	}

	BOOL result = TRUE;

	while (result) {
		interface_data.cbSize = sizeof(interface_data);
		result = SetupDiEnumDeviceInterfaces(device_info_set, NULL, usb_device_guid, (ULONG) index, &interface_data);

		if (result) {
			DWORD required_size = 0;
			// First call to get required buffer size
			SetupDiGetDeviceInterfaceDetail(device_info_set, &interface_data, NULL, 0, &required_size, NULL);
			if (required_size == 0) {
				index++;
				continue;
			}

			const PSP_DEVICE_INTERFACE_DETAIL_DATA detail_data =
					(PSP_DEVICE_INTERFACE_DETAIL_DATA) malloc(required_size);
			if (detail_data == NULL) {
				SetupDiDestroyDeviceInfoList(device_info_set);
				return EFEX_ERR_MEMORY;
			}
			detail_data->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

			if (SetupDiGetDeviceInterfaceDetail(device_info_set, &interface_data, detail_data, required_size, NULL,
												NULL)) {
				if (match_vid_pid(detail_data->DevicePath)) {
					ctx->dev_name = (char *) malloc(strlen(detail_data->DevicePath) + 1);
					if (ctx->dev_name != NULL) {
						strcpy(ctx->dev_name, detail_data->DevicePath);
						device_found = TRUE;
						free(detail_data);
						break;
					}
					free(detail_data);
					SetupDiDestroyDeviceInfoList(device_info_set);
					return EFEX_ERR_MEMORY;
				}
			}
			free(detail_data);
			index++;
		}
	}

	SetupDiDestroyDeviceInfoList(device_info_set);

	return device_found ? EFEX_ERR_SUCCESS : EFEX_ERR_USB_DEVICE_NOT_FOUND;
}

int sunxi_usb_init(struct sunxi_efex_ctx_t *ctx) {
	if (!ctx || !ctx->dev_name) {
		return EFEX_ERR_NULL_PTR;
	}

	ctx->hdl = CreateFile(ctx->dev_name, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL,
						  OPEN_EXISTING, 0, NULL);

	if (ctx->hdl == INVALID_HANDLE_VALUE) {
		return EFEX_ERR_USB_OPEN;
	}

	return EFEX_ERR_SUCCESS;
}

int sunxi_usb_exit(struct sunxi_efex_ctx_t *ctx) {
	if (!ctx) {
		return EFEX_ERR_NULL_PTR;
	}

	/* Release device handle */
	if (ctx->hdl != NULL && (HANDLE) ctx->hdl != INVALID_HANDLE_VALUE) {
		CloseHandle((HANDLE) ctx->hdl);
		ctx->hdl = NULL;
	}
	/* Free device name memory */
	if (ctx->dev_name != NULL) {
		free(ctx->dev_name);
		ctx->dev_name = NULL;
	}

	return EFEX_ERR_SUCCESS;
}

#endif // _WIN32
