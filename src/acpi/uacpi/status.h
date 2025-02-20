#pragma once

#include <uacpi/internal/compiler.h>
#include <uacpi/platform/types.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum uacpi_status
    {
        UACPI_STATUS_OK = 0,
        UACPI_STATUS_MAPPING_FAILED = 1,
        UACPI_STATUS_OUT_OF_MEMORY = 2,
        UACPI_STATUS_BAD_CHECKSUM = 3,
        UACPI_STATUS_INVALID_SIGNATURE = 4,
        UACPI_STATUS_INVALID_TABLE_LENGTH = 5,
        UACPI_STATUS_NOT_FOUND = 6,
        UACPI_STATUS_INVALID_ARGUMENT = 7,
        UACPI_STATUS_UNIMPLEMENTED = 8,
        UACPI_STATUS_ALREADY_EXISTS = 9,
        UACPI_STATUS_INTERNAL_ERROR = 10,
        UACPI_STATUS_TYPE_MISMATCH = 11,
        UACPI_STATUS_INIT_LEVEL_MISMATCH = 12,
        UACPI_STATUS_NAMESPACE_NODE_DANGLING = 13,
        UACPI_STATUS_NO_HANDLER = 14,
        UACPI_STATUS_NO_RESOURCE_END_TAG = 15,
        UACPI_STATUS_COMPILED_OUT = 16,
        UACPI_STATUS_HARDWARE_TIMEOUT = 17,
        UACPI_STATUS_TIMEOUT = 18,
        UACPI_STATUS_OVERRIDDEN = 19,
        UACPI_STATUS_DENIED = 20,

        // All errors that have bytecode-related origin should go here
        UACPI_STATUS_AML_UNDEFINED_REFERENCE = 0x0EFF0000,
        UACPI_STATUS_AML_INVALID_NAMESTRING = 0x0EFF0001,
        UACPI_STATUS_AML_OBJECT_ALREADY_EXISTS = 0x0EFF0002,
        UACPI_STATUS_AML_INVALID_OPCODE = 0x0EFF0003,
        UACPI_STATUS_AML_INCOMPATIBLE_OBJECT_TYPE = 0x0EFF0004,
        UACPI_STATUS_AML_BAD_ENCODING = 0x0EFF0005,
        UACPI_STATUS_AML_OUT_OF_BOUNDS_INDEX = 0x0EFF0006,
        UACPI_STATUS_AML_SYNC_LEVEL_TOO_HIGH = 0x0EFF0007,
        UACPI_STATUS_AML_INVALID_RESOURCE = 0x0EFF0008,
        UACPI_STATUS_AML_LOOP_TIMEOUT = 0x0EFF0009,
        UACPI_STATUS_AML_CALL_STACK_DEPTH_LIMIT = 0x0EFF000A,
    } uacpi_status;

    const uacpi_char *uacpi_status_to_string(uacpi_status);

#define uacpi_unlikely_error(expr) uacpi_unlikely((expr) != UACPI_STATUS_OK)
#define uacpi_likely_error(expr) uacpi_likely((expr) != UACPI_STATUS_OK)

#define uacpi_unlikely_success(expr) uacpi_unlikely((expr) == UACPI_STATUS_OK)
#define uacpi_likely_success(expr) uacpi_likely((expr) == UACPI_STATUS_OK)

#ifdef __cplusplus
}
#endif
