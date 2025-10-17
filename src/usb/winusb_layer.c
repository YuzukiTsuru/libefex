#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "ending.h"
#include "usb_layer.h"
#include "fel-protocol.h"

#ifdef _WIN32
#include <windows.h>
#include <initguid.h>
#include <SetupAPI.h>

#define IOCTL_AWUSB_SEND_DATA CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0807, METHOD_OUT_DIRECT, FILE_ANY_ACCESS)
#define IOCTL_AWUSB_RECV_DATA CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0808, METHOD_IN_DIRECT, FILE_ANY_ACCESS)
DEFINE_GUID(GUID_DEVINTERFACE_USB_DEVICE, 0xA5DCBF10L, 0x6530, 0x11D2, 0x90, 0x1F, 0x00, 0xC0, 0x4F, 0xB9, 0x51, 0xED);

int sunxi_usb_bulk_send(void *handle, const int ep, const char *buf, ssize_t len) {
    HANDLE const usb_handle = (HANDLE) handle;
    const size_t max_chunk = 128 * 1024;
    DWORD bytes_sent = 0;

    while (len > 0) {
        const size_t chunk = len <= max_chunk ? len : max_chunk;

        sunxi_usb_hex_dump(buf, chunk, "SEND");

        BOOL const result = DeviceIoControl(usb_handle, IOCTL_AWUSB_SEND_DATA, NULL,
                                            (DWORD) 0, (LPVOID) buf, chunk, &bytes_sent, NULL);

        if (!result) {
            fprintf(stderr, "USB bulk send failed with error code %lu\n", GetLastError());
            return -1;
        }

        if (bytes_sent == 0) {
            fprintf(stderr, "USB bulk sent 0 data.\n");
            return -1;
        }

        len -= bytes_sent;
        buf += bytes_sent;
    }
    return 0;
}

int sunxi_usb_bulk_recv(void *handle, const int ep, char *buf, const ssize_t len) {
    HANDLE const usb_handle = (HANDLE) handle;
    DWORD bytes_received = 0;

    const BOOL result = DeviceIoControl(usb_handle, IOCTL_AWUSB_RECV_DATA, NULL, 0,
                                        (LPVOID) buf, (DWORD) len, &bytes_received, NULL);

    if (!result) {
        fprintf(stderr, "USB bulk receive failed with error code %lu\n", GetLastError());
        return -1;
    }

    if (bytes_received == 0) {
        fprintf(stderr, "USB bulk received 0 data.\n");
        return -1;
    }

    sunxi_usb_hex_dump(buf, (size_t)bytes_received, "RECV");

    return 0;
}

int sunxi_scan_usb_device(struct sunxi_fel_ctx_t *ctx) {
    SP_DEVICE_INTERFACE_DATA interface_data;
    ULONG index = 0;
    BOOL device_found = FALSE;

    const LPGUID usb_device_guid = (LPGUID) &GUID_DEVINTERFACE_USB_DEVICE;

    const HDEVINFO device_info_set = SetupDiGetClassDevs(usb_device_guid, NULL, NULL,
                                                         DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

    if (device_info_set == INVALID_HANDLE_VALUE) {
        return -1;
    }

    const PSP_DEVICE_INTERFACE_DETAIL_DATA detail_data = (PSP_DEVICE_INTERFACE_DETAIL_DATA)
            GlobalAlloc(LMEM_ZEROINIT, 1024);
    if (detail_data == NULL) {
        SetupDiDestroyDeviceInfoList(device_info_set);
        return -1;
    }

    detail_data->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
    BOOL result = TRUE;

    while (result) {
        interface_data.cbSize = sizeof(interface_data);
        result = SetupDiEnumDeviceInterfaces(device_info_set, NULL, usb_device_guid,
                                             (ULONG) index, &interface_data);

        if (result) {
            result = SetupDiGetInterfaceDeviceDetail(device_info_set, &interface_data,
                                                     detail_data, 1024, NULL, NULL);

            if (result) {
                if (strstr(detail_data->DevicePath, "vid_1f3a&pid_efe8")) {
                    ctx->dev_name = (char *) malloc(strlen(detail_data->DevicePath) + 1);
                    if (ctx->dev_name != NULL) {
                        strcpy(ctx->dev_name, detail_data->DevicePath);
                    } else {
                        fprintf(stderr, "ERROR: Failed to allocate memory for device name\n");
                        GlobalFree(detail_data);
                        SetupDiDestroyDeviceInfoList(device_info_set);
                        return -1;
                    }
                    device_found = TRUE;
                    break;
                }
                index++;
            }
        }
    }

    SetupDiDestroyDeviceInfoList(device_info_set);

    return device_found;
}

int sunxi_usb_init(struct sunxi_fel_ctx_t *ctx) {
    if (!ctx || !ctx->dev_name) {
        fprintf(stderr, "ERROR: Invalid context or device name\n");
        return -1;
    }

    ctx->hdl = CreateFile(ctx->dev_name, GENERIC_WRITE | GENERIC_READ,
                          FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

    if (ctx->hdl == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "ERROR: Can't open USB device\n");
        return -1;
    }

    return 1;
}

int sunxi_usb_exit(struct sunxi_fel_ctx_t *ctx) {
    if (ctx) {
        /* 释放设备句柄 */
        if (ctx->hdl != NULL && (HANDLE) ctx->hdl != INVALID_HANDLE_VALUE) {
            CloseHandle((HANDLE) ctx->hdl);
            ctx->hdl = NULL;
        }
        /* 释放设备名称内存 */
        if (ctx->dev_name != NULL) {
            free(ctx->dev_name);
            ctx->dev_name = NULL;
        }
        return 0;
    }
    return -1;
}

#endif // _WIN32
