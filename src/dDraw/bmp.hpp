#pragma once

#include <stdint.h>

struct BITMAPFILEHEADER
{
    uint16_t bfType;      // BM
    uint32_t bfSize;      // Size of the file
    uint16_t bfReserved1; // Reserved
    uint16_t bfReserved2; // Reserved
    uint32_t bfOffBits;   // Offset to pixel data
} __attribute__((packed));

struct BITMAPINFOHEADER
{
    uint32_t biSize;         // Size of this header
    int32_t biWidth;         // Bitmap width in pixels
    int32_t biHeight;        // Bitmap height in pixels
    uint16_t biPlanes;       // Number of color planes
    uint16_t biBitCount;     // Bits per pixel
    uint32_t biCompression;  // Compression type
    uint32_t biSizeImage;    // Image size (may be 0 for uncompressed)
    int32_t biXPelsPerMeter; // Pixels per meter X
    int32_t biYPelsPerMeter; // Pixels per meter Y
    uint32_t biClrUsed;      // Number of colors in color table
    uint32_t biClrImportant; // Important color count
} __attribute__((packed));
