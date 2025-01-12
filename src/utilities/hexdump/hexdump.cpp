#include <utilities/utilities.hpp>
#include <debug.hpp>
#include <vector>

void hexdump(const void *data, size_t size)
{
    const unsigned char *byteData = static_cast<const unsigned char *>(data);
    size_t offset = 0;
    std::vector<unsigned char> previousBlock;
    bool isPreviousBlockRepeated = false;

    while (offset < size) {
        size_t lineStart = offset;
        size_t lineEnd = std::min(offset + 16, size);
        bool isSameAsPreviousBlock = true;

        // Check if the current block is the same as the previous block
        if (previousBlock.size() == (lineEnd - lineStart)) {
            for (size_t i = 0; i < (lineEnd - lineStart); ++i) {
                if (byteData[offset + i] != previousBlock[i]) {
                    isSameAsPreviousBlock = false;
                    break;
                }
            }
        } else {
            isSameAsPreviousBlock = false;
        }

        // If it's a repeated block, print "..." only once
        if (isSameAsPreviousBlock) {
            if (!isPreviousBlockRepeated) {
                debug_printf("   ...\n");
                isPreviousBlockRepeated = true;
            }
            offset += 16;
            continue;
        } else {
            isPreviousBlockRepeated = false; // Reset when a new block is found
        }

        // Print the offset at the start of each line
        debug_printf("%08zx  ", offset);

        // Print the hex values (16 bytes per line)
        for (size_t i = 0; i < 16; ++i) {
            if (offset + i < size) {
                debug_printf("%02x ", byteData[offset + i]);
            } else {
                debug_printf("   ");  // Padding for incomplete lines
            }
        }

        // Print the ASCII representation (non-printable characters as '.')
        debug_printf(" |");
        for (size_t i = 0; i < 16; ++i) {
            if (offset + i < size) {
                unsigned char ch = byteData[offset + i];
                debug_printf("%c", (ch >= 32 && ch <= 126) ? ch : '.');
            } else {
                debug_printf(" ");  // Padding for incomplete lines
            }
        }
        debug_printf("|\n");

        // Save the current block as the previous block for comparison
        previousBlock.clear();
        for (size_t i = 0; i < (lineEnd - lineStart); ++i) {
            previousBlock.push_back(byteData[offset + i]);
        }

        // Move to the next 16-byte chunk
        offset += 16;
    }
}
