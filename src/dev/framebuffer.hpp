#ifndef _FRAMEBUFFER_HPP
#define _FRAMEBUFFER_HPP

#include <limine.h>
#include <stddef.h>

void initialize_framebuffer(void);
limine_framebuffer *get_framebuffer(size_t index);
size_t get_framebuffer_count(void);

#endif