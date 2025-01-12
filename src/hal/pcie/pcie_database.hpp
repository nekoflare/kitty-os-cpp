#pragma once

#include <hal/pcie/pcie.hpp>
#include <stddef.h>

struct pcie_database_entry
{
    const char *vendor_name;
    const char *device_name;
    const uint16_t vendor_id;
    const uint16_t device_id;
    const uint16_t subvendor_id;
    const uint16_t subdevice_id;
    const char *subsystem_name;
};

const pcie_database_entry *find_in_pcie_database(uint16_t vendor_id, uint16_t device_id);

extern pcie_database_entry pcie_device_database[35153];
constexpr size_t pcie_device_database_size = 35153;
