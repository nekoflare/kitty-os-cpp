#include "clock.h"

#include <dbg/log.h>
#include <functional>
#include <hal/cpu/io.h>
#include <hal/irq/irq.h>
#include <kernel.h>
#include <vector>

struct callback
{
    uint64_t uid;
    std::function<void()> f;
    double ms;
    double last_called;
};

std::vector<callback> callbacks;
uint64_t freq = 2000;
double time_counter = 0;
static uint64_t last_uid = 0;

void set_pit_frequency(uint16_t target_frequency)
{
    if (target_frequency < 19)
        target_frequency = 19;
    if (target_frequency > 1193180)
        target_frequency = 1193180;

    const uint32_t temp_divisor = (1193180 + target_frequency / 2) / target_frequency;
    const uint16_t divisor = static_cast<uint16_t>(std::clamp(temp_divisor, 1U, 65535U));

    const uint16_t actual_frequency = 1193180 / divisor;

    outb(0x43, 0x34);
    outb(0x40, static_cast<uint8_t>(divisor & 0xFF));
    outb(0x40, static_cast<uint8_t>((divisor >> 8) & 0xFF));

    debug_print("PIT set to %d Hz (target: %d Hz, divisor: %d)", actual_frequency, target_frequency, divisor);
}

handle listen_on_clock(const std::function<void()> &callback, double ms)
{
    last_uid++; // Increment before using it
    callbacks.push_back({last_uid, callback, ms, 0});
    return reinterpret_cast<handle>(last_uid);
}

void delete_clock(handle clock)
{
    auto target_uid = reinterpret_cast<uint64_t>(clock);
    std::erase_if(callbacks, [target_uid](const callback &cb) { return cb.uid == target_uid; });
}

void handle_clock(handle irq)
{
    time_counter += 1000.0 / static_cast<double>(freq);

    for (auto &cb : callbacks)
    {
        if (time_counter - cb.last_called >= cb.ms)
        {
            cb.f(); // This might modify callbacks, so we iterate with an index
            cb.last_called = time_counter;
        }
    }
}

void initialize_clock()
{
    handle irq_handle;
    hook_irq(&irq_handle, false, 0, 0, handle_clock);
    set_pit_frequency(freq);
}
