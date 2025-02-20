//
// Created by Neko on 25.01.2025.
//

#include <dbg/log.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ASSERT(x) debug_print("STBI_ASSERT(%d) called", x)
#define STBI_NO_STDIO
#define STBI_NO_SIMD

#include "stb_image.h"