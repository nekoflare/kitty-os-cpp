#ifndef KITTY_OS_CPP_DRIVER_MAIN_HPP
#define KITTY_OS_CPP_DRIVER_MAIN_HPP

#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>
#include <kernel/hal/bus/pci.hpp>

#define driver_type __attribute__((section(".drivers"))) __attribute__((aligned(0x10)))

typedef enum __attribute__((aligned(0x10))) driver_statuses : uint32_t {
    DS_SUCCESS,
    DS_FAILURE,
    DS_NO_GPU_MODE
} driver_status_t;

typedef enum __attribute__((aligned(0x10))) driver_load : uint32_t {
    DL_LOAD_LAST,
    DL_LOAD_ON_PCI_MATCH,
    DL_ALWAYS_LOAD
} driver_load_t;

typedef enum __attribute__((aligned(0x10))) driver_types : uint32_t {
    DT_SERIAL,
    DT_USB_1_0_SERIAL,
    DT_GPU,
    DT_AUDIO,
    DT_AUDIO_GPU // audio + gpu
} driver_type_t;

struct __attribute__((aligned(0x10))) driver_handle_t
{
    uint64_t _Handle;
};

typedef enum __attribute__((aligned(0x10))) pci_requirements
{
    PCI_REQ_VD, // vendor and device is required.
    PCI_REQ_CSP, // class, subclass and prog if is required
} pci_match_requirement_t;

struct __attribute__((aligned(0x10))) pci_requirements_t
{
    uint16_t vendor_id;
    uint16_t device_id;
    uint8_t _class;
    uint8_t subclass;
    uint8_t prog_if;
    pci_match_requirement_t match_requirement;
};

struct __attribute__((aligned(0x10))) driver_entry_t {
    const char* driver_name;
    const char* driver_author;
    const char* driver_description;
    const char* driver_version;

    driver_type_t driver_designation;
    driver_load_t driver_load;
    driver_status_t (*driver_entry)(pci_dev*);
    driver_status_t (*driver_cleanup)();
    driver_status_t (*driver_ioctl)(driver_handle_t*, uint64_t, const char*, char*); // question, question_buffer, answer_buffer

    pci_requirements_t* requirements;
    size_t requirements_count;

    bool is_loaded;
};

#endif //KITTY_OS_CPP_DRIVER_MAIN_HPP
