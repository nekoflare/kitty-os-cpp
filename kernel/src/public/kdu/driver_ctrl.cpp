//
// Created by Piotr on 12.05.2024.
//

#include <kstd/kvector.hpp>
#include <public/kdu/driver_ctrl.hpp>

loaded_driver_descriptor* desc_head = nullptr;
uint64_t last_dci = 0; // Last driver identifier

uint64_t allocate_dci()
{
    auto dci = last_dci;
    last_dci++;
    return dci;
}

void append_descriptor(driver_handle_t handle, driver_entry_t* driver)
{
    if (driver == nullptr) {
        kstd::printf("Driver is nullptr.\n");
        return;
    }
    kstd::printf("Append descriptor has been called.\n");
    auto newDesc = new loaded_driver_descriptor;
    newDesc->identifier = allocate_dci();
    newDesc->driver_handle = handle;
    newDesc->driver = driver;
    newDesc->next = desc_head;
    desc_head = newDesc;
}

bool driver_ctrl_find_and_call(pci_dev* pci_handle)
{
    size_t driver_array_size = (__driver_array_end - __driver_array);

    for (size_t i = 0; driver_array_size > i; i++)
    {
        __attribute__((aligned(0x10))) struct driver_entry_t driver_entry = __driver_array[i];

        for (size_t i = 0; driver_entry.requirements_count > i; i++)
        {
            auto* pci_requirement = &driver_entry.requirements[i];

            if (pci_requirement->match_requirement == PCI_REQ_VD &&
                pci_requirement->device_id == pci_handle->device_id &&
                pci_requirement->vendor_id == pci_handle->vendor_id
            )
            {
                // call it.
                auto e = driver_entry.driver_entry(pci_handle);
                append_descriptor(e, &__driver_array[i]);

                return true;
            }

            if (pci_requirement->match_requirement == PCI_REQ_CSP &&
                pci_requirement->_class == pci_handle->class_code &&
                pci_requirement->subclass == pci_handle->subclass_code &&
                pci_requirement->prog_if == pci_handle->prog_if_code
            )
            {
                // call it.
                auto e = driver_entry.driver_entry(pci_handle);
                append_descriptor(e, &__driver_array[i]);

                return true;
            }

            // kstd::printf("requirement: %hhd\n", pci_requirement->match_requirement);
        }
    }

    return false;
}

void driver_ctrl_call_ald()
{
    size_t driver_array_size = (__driver_array_end - __driver_array);

    for (size_t i = 0; driver_array_size > i; i++)
    {
        __attribute__((aligned(0x10))) struct driver_entry_t driver_entry = __driver_array[i];

        if (driver_entry.is_loaded)
        {
            // kstd::printf("The driver is already loaded!\n");
            continue;
        }

        if (!driver_entry.is_loaded && driver_entry.driver_load == DL_ALWAYS_LOAD)
        {
            driver_entry.driver_entry(nullptr);
            driver_entry.is_loaded = true;
        }
    }
}

void driver_ctrl_enumerate_drivers()
{
    size_t driver_array_size = (__driver_array_end - __driver_array);

    kstd::printf("Drivers: ");
    for (size_t i = 0; i < driver_array_size; ++i)
    {
        struct driver_entry_t driver_entry = __driver_array[i];

        kstd::printf("Driver name: %s\nAuthor: %s\nVersion:%s\nDescription:%s\n\n", driver_entry.driver_name, driver_entry.driver_author, driver_entry.driver_version, driver_entry.driver_description);
    }
}

void driver_ctrl_print_ready_drivers()
{
    if (desc_head == nullptr) {
        kstd::printf("No drivers are loaded.\n");
        return;
    }

    auto head = desc_head;
    while (head != nullptr)
    {
        kstd::printf("Driver %s\n", head->driver->driver_name);
        head = head->next;
    }
}

driver_entry_t* driver_ctrl_get_driver(uint64_t dci)
{
    if (desc_head == nullptr) {
        return nullptr;
    }

    auto head = desc_head;
    while (head != nullptr)
    {
        if (head->identifier == dci)
            return head->driver;

        head = head->next;
    }

    return nullptr;
}

int64_t driver_ctrl_find_driver_by_designation(driver_type_t type)
{
    if (desc_head == nullptr) {
        return -1;
    }

    auto head = desc_head;
    while (head != nullptr)
    {
        if (head->driver->driver_designation == type)
            return head->identifier;
        head = head->next;
    }

    return -1;
}

loaded_driver_descriptor* driver_ctrl_get_descriptors()
{
    return desc_head;
}

driver_status_t ioctl_auto(
        driver_type_t type,
        driver_handle_t* ioctl_handle,
        uint64_t ioctl_question,
        const char* ioctl_msg,
        char* ioctl_resp
)
{
    size_t driver_array_size = (__driver_array_end - __driver_array);
    for (size_t i = 0; i < driver_array_size; ++i)
    {
        struct driver_entry_t driver_entry = __driver_array[i];

        if (!driver_entry.is_loaded) continue;
        if (driver_entry.driver_designation != type) continue;
        driver_status_t st = driver_entry.driver_ioctl(ioctl_handle, ioctl_question, ioctl_msg, ioctl_resp);
        if (st != DS_SUCCESS) {
            return st;
        }
    }

    return DS_SUCCESS;
}