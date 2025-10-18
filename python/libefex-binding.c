#include "../includes/libefex.h"
#include "../includes/efex-payloads.h"
#include "../includes/efex-protocol.h"
#include "../includes/usb_layer.h"

#define PY_SSIZE_T_CLEAN
#include <Python.h>

// Define Python object structure to encapsulate sunxi_efex_ctx_t
typedef struct {
    PyObject_HEAD
    struct sunxi_efex_ctx_t ctx;
} PyEfexContext;

// Architecture enum mapping
static PyObject *PyEfexArch_Enum;

// Context object creation and destruction functions
static int PyEfexContext_init(PyEfexContext *self, PyObject *args, PyObject *kwds);
static void PyEfexContext_dealloc(PyEfexContext *self);

// Device operation functions
static PyObject *py_sunxi_scan_usb_device(PyObject *self, PyObject *args);
static PyObject *py_sunxi_usb_init(PyObject *self, PyObject *args);
static PyObject *py_sunxi_efex_init(PyObject *self, PyObject *args);

// Memory operation functions
static PyObject *py_sunxi_efex_writel(PyObject *self, PyObject *args);
static PyObject *py_sunxi_efex_readl(PyObject *self, PyObject *args);
static PyObject *py_sunxi_efex_write_memory(PyObject *self, PyObject *args);
static PyObject *py_sunxi_efex_read_memory(PyObject *self, PyObject *args);

// Execution operation functions
static PyObject *py_sunxi_efex_exec(PyObject *self, PyObject *args);

// Payload operation functions
static PyObject *py_sunxi_efex_payloads_init(PyObject *self, PyObject *args);
static PyObject *py_sunxi_efex_payloads_readl(PyObject *self, PyObject *args);
static PyObject *py_sunxi_efex_payloads_writel(PyObject *self, PyObject *args);

// Get device response data
static PyObject *py_sunxi_get_device_resp(PyObject *self, PyObject *args);

// Context type object
static PyTypeObject PyEfexContextType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "libefex.Context",
    .tp_doc = "EFEX context object",
    .tp_basicsize = sizeof(PyEfexContext),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = PyType_GenericNew,
    .tp_init = (initproc)PyEfexContext_init,
    .tp_dealloc = (destructor)PyEfexContext_dealloc,
};

// Module function list
static PyMethodDef LibEfexMethods[] = {
    {"scan_usb_device", py_sunxi_scan_usb_device, METH_O, "Scan for USB devices in EFEX mode"},
    {"usb_init", py_sunxi_usb_init, METH_O, "Initialize USB connection"},
    {"efex_init", py_sunxi_efex_init, METH_O, "Initialize EFEX context"},
    {"writel", py_sunxi_efex_writel, METH_VARARGS, "Write 32-bit value to memory"},
    {"readl", py_sunxi_efex_readl, METH_VARARGS, "Read 32-bit value from memory"},
    {"write_memory", py_sunxi_efex_write_memory, METH_VARARGS, "Write block of memory"},
    {"read_memory", py_sunxi_efex_read_memory, METH_VARARGS, "Read block of memory"},
    {"exec", py_sunxi_efex_exec, METH_VARARGS, "Execute code at address"},
    {"payloads_init", py_sunxi_efex_payloads_init, METH_O, "Initialize payloads for architecture"},
    {"payloads_readl", py_sunxi_efex_payloads_readl, METH_VARARGS, "Read 32-bit value using payload"},
    {"payloads_writel", py_sunxi_efex_payloads_writel, METH_VARARGS, "Write 32-bit value using payload"},
    {"get_device_resp", py_sunxi_get_device_resp, METH_O, "Get device response data"},
    {NULL, NULL, 0, NULL}
};

// Module definition
static struct PyModuleDef libefexmodule = {
    PyModuleDef_HEAD_INIT,
    "libefex",
    "Python bindings for libefex - Allwinner EFEX mode interaction library",
    -1,
    LibEfexMethods,
    NULL,
    NULL,
    NULL,
    NULL
};

// Context initialization function
static int PyEfexContext_init(PyEfexContext *self, PyObject *args, PyObject *kwds) {
    memset(&self->ctx, 0, sizeof(struct sunxi_efex_ctx_t));
    return 0;
}

// Context destruction function
static void PyEfexContext_dealloc(PyEfexContext *self) {
    // Clean up resources
    if (self->ctx.hdl) {
        libusb_close(self->ctx.hdl);
        self->ctx.hdl = NULL;
    }
    Py_TYPE(self)->tp_free((PyObject *)self);
}

// Device scanning function
static PyObject *py_sunxi_scan_usb_device(PyObject *self, PyObject *args) {
    PyEfexContext *context;
    if (!PyArg_Parse(args, "O!", &PyEfexContextType, &context)) {
        return NULL;
    }
    
    int ret = sunxi_scan_usb_device(&context->ctx);
    return PyLong_FromLong(ret);
}

// USB initialization function
static PyObject *py_sunxi_usb_init(PyObject *self, PyObject *args) {
    PyEfexContext *context;
    if (!PyArg_Parse(args, "O!", &PyEfexContextType, &context)) {
        return NULL;
    }
    
    int ret = sunxi_usb_init(&context->ctx);
    return PyLong_FromLong(ret);
}

// EFEX initialization function
static PyObject *py_sunxi_efex_init(PyObject *self, PyObject *args) {
    PyEfexContext *context;
    if (!PyArg_Parse(args, "O!", &PyEfexContextType, &context)) {
        return NULL;
    }
    
    int ret = sunxi_efex_init(&context->ctx);
    return PyLong_FromLong(ret);
}

// Write 32-bit value function
static PyObject *py_sunxi_efex_writel(PyObject *self, PyObject *args) {
    PyEfexContext *context;
    uint32_t val, addr;
    
    if (!PyArg_ParseTuple(args, "O!II", &PyEfexContextType, &context, &val, &addr)) {
        return NULL;
    }
    
    sunxi_efex_writel(&context->ctx, val, addr);
    Py_RETURN_NONE;
}

// Read 32-bit value function
static PyObject *py_sunxi_efex_readl(PyObject *self, PyObject *args) {
    PyEfexContext *context;
    uint32_t addr;
    
    if (!PyArg_ParseTuple(args, "O!I", &PyEfexContextType, &context, &addr)) {
        return NULL;
    }
    
    uint32_t val = sunxi_efex_readl(&context->ctx, addr);
    return PyLong_FromUnsignedLong(val);
}

// Write memory block function
static PyObject *py_sunxi_efex_write_memory(PyObject *self, PyObject *args) {
    PyEfexContext *context;
    uint32_t addr;
    PyObject *data_obj;
    const char *data;
    Py_ssize_t len;
    
    if (!PyArg_ParseTuple(args, "O!IO", &PyEfexContextType, &context, &addr, &data_obj)) {
        return NULL;
    }
    
    if (!PyBytes_AsStringAndSize(data_obj, &data, &len)) {
        return NULL;
    }
    
    sunxi_efex_write_memory(&context->ctx, addr, data, (size_t)len);
    Py_RETURN_NONE;
}

// Read memory block function
static PyObject *py_sunxi_efex_read_memory(PyObject *self, PyObject *args) {
    PyEfexContext *context;
    uint32_t addr;
    Py_ssize_t len;
    
    if (!PyArg_ParseTuple(args, "O!II", &PyEfexContextType, &context, &addr, &len)) {
        return NULL;
    }
    
    char *buffer = (char *)malloc(len);
    if (!buffer) {
        PyErr_SetString(PyExc_MemoryError, "Failed to allocate memory");
        return NULL;
    }
    
    sunxi_efex_read_memory(&context->ctx, addr, buffer, (size_t)len);
    PyObject *result = PyBytes_FromStringAndSize(buffer, len);
    free(buffer);
    
    return result;
}

// Execute code function
static PyObject *py_sunxi_efex_exec(PyObject *self, PyObject *args) {
    PyEfexContext *context;
    uint32_t addr;
    
    if (!PyArg_ParseTuple(args, "O!I", &PyEfexContextType, &context, &addr)) {
        return NULL;
    }
    
    sunxi_efex_exec(&context->ctx, addr);
    Py_RETURN_NONE;
}

// Payload initialization function
static PyObject *py_sunxi_efex_payloads_init(PyObject *self, PyObject *args) {
    int arch;
    
    if (!PyArg_Parse(args, "i", &arch)) {
        return NULL;
    }
    
    sunxi_efex_payloads_init((enum sunxi_efex_payloads_arch)arch);
    Py_RETURN_NONE;
}

// Payload read 32-bit value function
static PyObject *py_sunxi_efex_payloads_readl(PyObject *self, PyObject *args) {
    PyEfexContext *context;
    uint32_t addr;
    
    if (!PyArg_ParseTuple(args, "O!I", &PyEfexContextType, &context, &addr)) {
        return NULL;
    }
    
    uint32_t val = sunxi_efex_payloads_readl(&context->ctx, addr);
    return PyLong_FromUnsignedLong(val);
}

// Payload write 32-bit value function
static PyObject *py_sunxi_efex_payloads_writel(PyObject *self, PyObject *args) {
    PyEfexContext *context;
    uint32_t val, addr;
    
    if (!PyArg_ParseTuple(args, "O!II", &PyEfexContextType, &context, &val, &addr)) {
        return NULL;
    }
    
    sunxi_efex_payloads_writel(&context->ctx, val, addr);
    Py_RETURN_NONE;
}

// Get device response data function
static PyObject *py_sunxi_get_device_resp(PyObject *self, PyObject *args) {
    PyEfexContext *context;
    if (!PyArg_Parse(args, "O!", &PyEfexContextType, &context)) {
        return NULL;
    }
    
    // Create a dictionary to store device response data
    PyObject *resp_dict = PyDict_New();
    if (!resp_dict) {
        return NULL;
    }
    
    // Extract fields from resp struct and add to dictionary
    struct sunxi_efex_device_resp_t *resp = &context->ctx.resp;
    
    // Process magic field (character array)
    PyObject *magic = PyBytes_FromStringAndSize(resp->magic, 8);
    if (magic) {
        PyDict_SetItemString(resp_dict, "magic", magic);
        Py_DECREF(magic);
    }
    
    // Add other numeric fields
    PyDict_SetItemString(resp_dict, "id", PyLong_FromUnsignedLong(resp->id));
    PyDict_SetItemString(resp_dict, "firmware", PyLong_FromUnsignedLong(resp->firmware));
    PyDict_SetItemString(resp_dict, "mode", PyLong_FromUnsignedLong(resp->mode));
    PyDict_SetItemString(resp_dict, "data_flag", PyLong_FromUnsignedLong(resp->data_flag));
    PyDict_SetItemString(resp_dict, "data_length", PyLong_FromUnsignedLong(resp->data_length));
    PyDict_SetItemString(resp_dict, "data_start_address", PyLong_FromUnsignedLong(resp->data_start_address));
    
    // Process reserved field
    PyObject *reserved = PyBytes_FromStringAndSize(resp->reserved, 8);
    if (reserved) {
        PyDict_SetItemString(resp_dict, "reserved", reserved);
        Py_DECREF(reserved);
    }
    
    return resp_dict;
}

// Module initialization function
PyMODINIT_FUNC PyInit_libefex(void) {
    PyObject *m;
    
    // Initialize type
    if (PyType_Ready(&PyEfexContextType) < 0) {
        return NULL;
    }
    
    // Create module
    m = PyModule_Create(&libefexmodule);
    if (m == NULL) {
        return NULL;
    }
    
    // Add type to module
    Py_INCREF(&PyEfexContextType);
    if (PyModule_AddObject(m, "Context", (PyObject *)&PyEfexContextType) < 0) {
        Py_DECREF(&PyEfexContextType);
        Py_DECREF(m);
        return NULL;
    }
    
    // Create architecture enum
    PyEfexArch_Enum = PyDict_New();
    PyDict_SetItemString(PyEfexArch_Enum, "ARM32", PyLong_FromLong(PAYLOAD_ARCH_ARM32));
    PyDict_SetItemString(PyEfexArch_Enum, "AARCH64", PyLong_FromLong(PAYLOAD_ARCH_AARCH64));
    PyDict_SetItemString(PyEfexArch_Enum, "RISCV32_E907", PyLong_FromLong(PAYLOAD_ARCH_RISCV32_E907));
    PyModule_AddObject(m, "Arch", PyEfexArch_Enum);
    
    return m;
}

