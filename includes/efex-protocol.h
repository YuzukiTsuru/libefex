#ifndef EFEX_PROTOCOL_H
#define EFEX_PROTOCOL_H

#ifdef __cplusplus
extern "C" {

#endif

#include <stdint.h>
#include "compiler.h"

/**
 * @brief Error code definitions for EFEX operations
 */
enum sunxi_efex_error_t {
	/* Generic Errors */
	EFEX_ERR_SUCCESS = 0,        /**< Success */
	EFEX_ERR_INVALID_PARAM = -1, /**< Invalid parameter */
	EFEX_ERR_NULL_PTR = -2,      /**< Null pointer error */
	EFEX_ERR_MEMORY = -3,        /**< Memory allocation error */
	EFEX_ERR_NOT_SUPPORT = -4,   /**< Operation not supported */

	/* USB Communication Errors */
	EFEX_ERR_USB_INIT = -10,             /**< USB initialization failed */
	EFEX_ERR_USB_DEVICE_NOT_FOUND = -11, /**< Device not found */
	EFEX_ERR_USB_OPEN = -12,             /**< Failed to open device */
	EFEX_ERR_USB_TRANSFER = -13,         /**< USB transfer failed */
	EFEX_ERR_USB_TIMEOUT = -14,          /**< USB transfer timeout */

	/* Protocol Errors */
	EFEX_ERR_PROTOCOL = -20,            /**< Protocol error */
	EFEX_ERR_INVALID_RESPONSE = -21,    /**< Invalid response from device */
	EFEX_ERR_UNEXPECTED_STATUS = -22,   /**< Unexpected status code */
	EFEX_ERR_INVALID_STATE = -23,       /**< Invalid device state */
	EFEX_ERR_INVALID_DEVICE_MODE = -24, /**< Invalid device mode */

	/* Operation Errors */
	EFEX_ERR_OPERATION_FAILED = -30, /**< Operation failed */
	EFEX_ERR_DEVICE_BUSY = -31,      /**< Device is busy */
	EFEX_ERR_DEVICE_NOT_READY = -32, /**< Device not ready */

	/* Flash Related Errors */
	EFEX_ERR_FLASH_ACCESS = -40,     /**< Flash access error */
	EFEX_ERR_FLASH_SIZE_PROBE = -41, /**< Flash size probing failed */
	EFEX_ERR_FLASH_SET_ONOFF = -42,  /**< Failed to set flash on/off */

	/* Verification Errors */
	EFEX_ERR_VERIFICATION = -50, /**< Verification failed */
	EFEX_ERR_CRC_MISMATCH = -51, /**< CRC mismatch error */

	/* File Operation Errors */
	EFEX_ERR_FILE_OPEN = -60,  /**< Failed to open file */
	EFEX_ERR_FILE_READ = -61,  /**< Failed to read file */
	EFEX_ERR_FILE_WRITE = -62, /**< Failed to write file */
	EFEX_ERR_FILE_SIZE = -63,  /**< File size error */
};


enum sunxi_efex_cmd_t {
	/* Common Commands */
	EFEX_CMD_VERIFY_DEVICE = 0x0001,
	EFEX_CMD_SWITCH_ROLE = 0x0002,
	EFEX_CMD_IS_READY = 0x0003,
	EFEX_CMD_GET_CMD_SET_VER = 0x0004,
	EFEX_CMD_DISCONNECT = 0x0010,
	/* FEL Commands */
	EFEX_CMD_FEL_WRITE = 0x0101,
	EFEX_CMD_FEL_EXEC = 0x0102,
	EFEX_CMD_FEL_READ = 0x0103,
	/* FES Commands */
	EFEX_CMD_FES_TRANS = 0x0201,
	EFEX_CMD_FES_RUN = 0x0202,
	EFEX_CMD_FES_INFO = 0x0203,
	EFEX_CMD_FES_GET_MSG = 0x0204,
	EFEX_CMD_FES_UNREG_FED = 0x0205,
	EFEX_CMD_FES_DOWN = 0x0206,
	EFEX_CMD_FES_UP = 0x0207,
	EFEX_CMD_FES_VERIFY = 0x0208,
	EFEX_CMD_FES_QUERY_STORAGE = 0x0209,
	EFEX_CMD_FES_FLASH_SET_ON = 0x020A,
	EFEX_CMD_FES_FLASH_SET_OFF = 0x020B,
	EFEX_CMD_FES_VERIFY_VALUE = 0x020C,
	EFEX_CMD_FES_VERIFY_STATUS = 0x020D,
	EFEX_CMD_FES_FLASH_SIZE_PROBE = 0x020E,
	EFEX_CMD_FES_TOOL_MODE = 0x020F,
	EFEX_CMD_FES_VERIFY_UBOOT_BLK = 0x0214,
	EFEX_CMD_FES_FORCE_ERASE_FLASH = 0x0220,
	EFEX_CMD_FES_FORCE_ERASE_KEY = 0x0221,
	EFEX_CMD_FES_QUERY_SECURE = 0x0230,
	EFEX_CMD_FES_QUERY_INFO = 0x0231,
	EFEX_CMD_FES_GET_CHIPID = 0x0232
};

enum sunxi_verify_device_mode_t {
	DEVICE_MODE_NULL = 0x0,
	DEVICE_MODE_FEL = 0x1,
	DEVICE_MODE_SRV = 0x2,
	DEVICE_MODE_UPDATE_COOL = 0x3,
	DEVICE_MODE_UPDATE_HOT = 0x4,
};

/* clang-format off */
EFEX_PACKED_BEGIN
struct sunxi_usb_request_t {
    union {
        char magic[4];
        uint32_t magics;
    };

    uint32_t tab;
    uint32_t data_length;
    uint16_t resvered1;
    uint8_t resvered2;
    uint8_t cmd_length;
    uint8_t cmd_package[16];
} EFEX_PACKED;
EFEX_PACKED_END

EFEX_PACKED_BEGIN
struct sunxi_usb_response_t {
    union {
        char magic[4];
        uint32_t magics;
    };

    uint32_t tag;
    uint32_t residue;
    uint8_t status;
} EFEX_PACKED;
EFEX_PACKED_END

EFEX_PACKED_BEGIN
struct sunxi_efex_request_t {
    uint16_t cmd;
    uint16_t tag;
    uint32_t address;
    uint32_t len;
    uint32_t flags;
} EFEX_PACKED;
EFEX_PACKED_END

EFEX_PACKED_BEGIN
struct sunxi_efex_response_t {
    uint16_t magic;
    uint16_t tag;
    uint8_t status;
    uint8_t reserve[3];
} EFEX_PACKED;
EFEX_PACKED_END

EFEX_PACKED_BEGIN
struct sunxi_fes_reserved_t {
    char* reserved[12];
} EFEX_PACKED;
EFEX_PACKED_END

EFEX_PACKED_BEGIN
struct sunxi_fes_flash_t {
    uint32_t flash_type;
    char* reserved[8];
} EFEX_PACKED;
EFEX_PACKED_END

EFEX_PACKED_BEGIN
struct sunxi_fes_trans_t {
    uint32_t addr;
    uint32_t len;
    uint32_t flags;
} EFEX_PACKED;
EFEX_PACKED_END

EFEX_PACKED_BEGIN
struct sunxi_fes_verify_value_t {
    uint32_t addr;
    uint64_t size;
} EFEX_PACKED;
EFEX_PACKED_END

EFEX_PACKED_BEGIN
struct sunxi_fes_verify_status_t {
    uint32_t addr;
    uint32_t size;
    uint32_t tag;
} EFEX_PACKED;
EFEX_PACKED_END

EFEX_PACKED_BEGIN
struct sunxi_fes_verify_resp_t {
    uint32_t flag;
    int32_t fes_crc;
    int32_t media_crc;
} EFEX_PACKED;
EFEX_PACKED_END

EFEX_PACKED_BEGIN
struct sunxi_fes_xfer_t {
    uint16_t cmd;
    uint16_t tag;
    char buf[12];
    union {
        char magic[4];
        uint32_t magics;
    };
} EFEX_PACKED;
EFEX_PACKED_END
/* clang-format on */

#define EFEX_CODE_MAX_SIZE (64 * 1024)

struct sunxi_efex_device_resp_t {
	char magic[8];
	uint32_t id;
	uint32_t firmware;
	uint16_t mode;
	uint8_t data_flag;
	uint8_t data_length;
	uint32_t data_start_address;
	uint8_t reserved[8];
};

struct sunxi_efex_ctx_t {
	void *hdl;
	void *usb_context;
	char *dev_name;
	int epout;
	int epin;
	struct sunxi_efex_device_resp_t resp;
};


#ifdef __cplusplus
}
#endif

#endif // EFEX_PROTOCOL_H
