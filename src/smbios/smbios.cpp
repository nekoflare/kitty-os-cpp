#include <debug.hpp>
#include <limine.h>
#include <smbios/smbios.hpp>
#include <sys/symbols.hpp>

limine_smbios_request smbios_request = {.id = LIMINE_SMBIOS_REQUEST, .revision = 0, .response = nullptr};

struct smbios_entry_point_32 *smbios_32 = nullptr;
struct smbios_entry_point_64 *smbios_64 = nullptr;

void initialize_smbios()
{
    if (smbios_request.response == nullptr)
    {
        debug_printf("SMBIOS response is nullptr.\n");
    }

    if (smbios_request.response->entry_32)
        debug_printf("SMBIOS-32 is available.\n");
    if (smbios_request.response->entry_64)
        debug_printf("SMBIOS-64 is available.\n");

    if (smbios_request.response->entry_32)
    {
        auto is_valid = validate_smbios_entry_point(
            reinterpret_cast<struct smbios_entry_point_32 *>(smbios_request.response->entry_32));
        if (!is_valid)
        {
            debug_printf("Invalid SMBIOS 32-bit structure.\n");
            asm volatile("cli; hlt");
        }
    }

    if (smbios_request.response->entry_64)
    {
        auto is_valid = validate_smbios_entry_point_64(
            reinterpret_cast<struct smbios_entry_point_64 *>(smbios_request.response->entry_64));
        if (!is_valid)
        {
            debug_printf("Invalid SMBIOS 64-bit structure.\n");
            asm volatile("cli; hlt");
        }
    }

    smbios_32 = reinterpret_cast<decltype(smbios_32)>(smbios_request.response->entry_32);
    smbios_64 = reinterpret_cast<decltype(smbios_64)>(smbios_request.response->entry_64);
}

// Validate the SMBIOS 2.1 (32-bit) Entry Point structure
bool validate_smbios_entry_point(const struct smbios_entry_point_32 *entry_point)
{
    // Verify anchor string is "_SM_"
    if (memcmp(entry_point->anchor_string, SMBIOS_ANCHOR_STRING, 4) != 0)
    {
        return false;
    }

    // Calculate and verify the checksum of the EPS
    uint8_t checksum = 0;
    const uint8_t *entry_data = (const uint8_t *)entry_point;
    for (size_t i = 0; i < entry_point->eps_length; ++i)
    {
        checksum += entry_data[i];
    }
    if (checksum != 0)
    {
        return false;
    }

    // Verify the intermediate anchor string is "_DMI_"
    if (memcmp(entry_point->intermediate_anchor_string, DMI_ANCHOR_STRING, 5) != 0)
    {
        return false;
    }

    // Calculate and verify the checksum of the Intermediate EPS
    checksum = 0;
    const uint8_t *intermediate_data = &entry_data[0x10]; // Start at offset 10h
    for (size_t i = 0; i < 0x0F; ++i)
    {
        checksum += intermediate_data[i];
    }
    if (checksum != 0)
    {
        return false;
    }

    return true;
}

// Validate the SMBIOS DMI table data
bool validate_smbios_dmi_data(const struct smbios_entry_point_32 *entry_point, const void *dmi_table, size_t table_size)
{
    // Ensure table size matches the length specified in the entry point
    if (table_size != entry_point->structure_table_length)
    {
        return false;
    }

    // Ensure table address matches the entry point
    if ((uintptr_t)dmi_table != entry_point->structure_table_address)
    {
        return false;
    }

    // Validate each structure in the table
    const uint8_t *table_ptr = (const uint8_t *)dmi_table;
    size_t remaining_size = table_size;
    while (remaining_size >= sizeof(struct smbios_structure_header))
    {
        const struct smbios_structure_header *header = (const struct smbios_structure_header *)table_ptr;

        // Ensure structure length is valid
        if (header->length > remaining_size || header->length < sizeof(struct smbios_structure_header))
        {
            return false;
        }

        // Move to the next structure
        size_t structure_size = header->length;
        table_ptr += structure_size;
        remaining_size -= structure_size;
    }

    // Ensure no remaining bytes left unprocessed
    return remaining_size == 0;
}

// Validate the SMBIOS 3.0 (64-bit) Entry Point structure
bool validate_smbios_entry_point_64(const struct smbios_entry_point_64 *entry_point)
{
    // Verify anchor string is "_SM3_"
    if (memcmp(entry_point->anchor_string, "_SM3_", 5) != 0)
    {
        return false;
    }

    // Calculate and verify the checksum of the EPS
    uint8_t checksum = 0;
    const uint8_t *entry_data = (const uint8_t *)entry_point;
    for (size_t i = 0; i < entry_point->eps_length; ++i)
    {
        checksum += entry_data[i];
    }
    if (checksum != 0)
    {
        return false;
    }

    // Additional field checks
    if (entry_point->eps_length < sizeof(struct smbios_entry_point_64))
    {
        // Ensure EPS length is at least the size of this structure
        return false;
    }

    if (entry_point->structure_table_max_size == 0)
    {
        // Structure table size must be non-zero
        return false;
    }

    if (entry_point->structure_table_address == 0)
    {
        // Structure table address must be valid
        return false;
    }

    return true;
}

// Print the SMBIOS entries
void print_smbios_entries()
{
    if (!smbios_32 && !smbios_64)
    {
        debug_printf("No SMBIOS entry points initialized.\n");
        return;
    }

    // Example for 32-bit SMBIOS
    if (smbios_32)
    {
        const uint8_t *table_ptr = reinterpret_cast<const uint8_t *>(smbios_32->structure_table_address);
        size_t table_size = smbios_32->structure_table_length;

        size_t processed_size = 0;
        while (processed_size < table_size)
        {
            const smbios_structure_header *header = reinterpret_cast<const smbios_structure_header *>(table_ptr);

            if (header->length < sizeof(smbios_structure_header))
            {
                debug_printf("Invalid SMBIOS structure header.\n");
                break;
            }

            debug_printf("Type: %u, Length: %u, Handle: %u\n", header->type, header->length, header->handle);

            // Print the string table for this structure
            print_smbios_string_table(header, table_size - processed_size);

            // Get specific string example (e.g., index 1)
            const char *string = get_smbios_string(header, table_size - processed_size, 1);
            if (string)
            {
                debug_printf("String at index 1: %s\n", string);
            }

            // Move to the next structure
            table_ptr += header->length;

            // Skip the unformatted area (string-set termination: double NULL)
            while (processed_size < table_size && (table_ptr[0] != 0 || table_ptr[1] != 0))
            {
                table_ptr++;
                processed_size++;
            }

            // Skip the final two NULLs
            if (processed_size + 2 <= table_size)
            {
                table_ptr += 2;
                processed_size += 2;
            }
        }
    }

    if (smbios_64)
    {
        debug_printf("SMBIOS 64-bit Structures:\n");
        const uint8_t *table_ptr = reinterpret_cast<const uint8_t *>(smbios_64->structure_table_address);
        size_t table_size = smbios_64->structure_table_max_size;

        size_t processed_size = 0;
        while (processed_size < table_size)
        {
            const smbios_structure_header *header = reinterpret_cast<const smbios_structure_header *>(table_ptr);

            // Validate header length
            if (header->length < sizeof(smbios_structure_header))
            {
                debug_printf("Invalid SMBIOS structure header length.\n");
                break;
            }

            // Print structure details
            debug_printf("Type: %u, Length: %u, Handle: %u\n", header->type, header->length, header->handle);

            // Move to the next structure
            table_ptr += header->length;
            processed_size += header->length;

            // Skip the unformatted area (string-set termination: double NULL)
            while (processed_size < table_size && (table_ptr[0] != 0 || table_ptr[1] != 0))
            {
                table_ptr++;
                processed_size++;
            }

            // Skip the final two NULLs
            if (processed_size + 2 <= table_size)
            {
                table_ptr += 2;
                processed_size += 2;
            }
        }
    }
}

// Print all strings in the SMBIOS string table of a structure
void print_smbios_string_table(const smbios_structure_header *header, size_t table_size)
{
    if (!header)
    {
        debug_printf("Invalid SMBIOS structure header.\n");
        return;
    }

    const uint8_t *string_table_start = reinterpret_cast<const uint8_t *>(header) + header->length;
    size_t remaining_size = table_size - header->length;

    size_t string_index = 1; // SMBIOS strings are 1-indexed
    const char *current_string = reinterpret_cast<const char *>(string_table_start);

    debug_printf("SMBIOS String Table:\n");

    while (remaining_size > 0 && *current_string != '\0')
    {
        debug_printf("  [%zu]: %s\n", string_index, current_string);

        // Advance to the next string
        size_t current_string_length = strlen(current_string);
        current_string += current_string_length + 1;
        remaining_size -= current_string_length + 1;

        string_index++;
    }

    if (remaining_size > 0 && *current_string == '\0')
    {
        debug_printf("End of string table.\n");
    }
}

// Get a string from the SMBIOS string table by its index
const char *get_smbios_string(const smbios_structure_header *header, size_t table_size, size_t string_index)
{
    if (!header || string_index == 0)
    {
        debug_printf("Invalid arguments to get_smbios_string.\n");
        return nullptr;
    }

    const uint8_t *string_table_start = reinterpret_cast<const uint8_t *>(header) + header->length;
    size_t remaining_size = table_size - header->length;

    const char *current_string = reinterpret_cast<const char *>(string_table_start);

    size_t current_index = 1; // SMBIOS strings are 1-indexed

    while (remaining_size > 0 && *current_string != '\0')
    {
        if (current_index == string_index)
        {
            return current_string;
        }

        // Advance to the next string
        size_t current_string_length = strlen(current_string);
        current_string += current_string_length + 1;
        remaining_size -= current_string_length + 1;

        current_index++;
    }

    // If we reach here, the string index is out of bounds
    debug_printf("String index %zu out of bounds.\n", string_index);
    return nullptr;
}

EXPORT_SYMBOL(validate_smbios_entry_point);
EXPORT_SYMBOL(validate_smbios_dmi_data);
EXPORT_SYMBOL(validate_smbios_entry_point_64);
EXPORT_SYMBOL(print_smbios_entries);
EXPORT_SYMBOL(print_smbios_string_table);
EXPORT_SYMBOL(get_smbios_string);