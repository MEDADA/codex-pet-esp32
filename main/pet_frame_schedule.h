#pragma once

#include <stdbool.h>
#include <stdint.h>

static inline unsigned pet_frame_back_buffer(int front)
{
    return front == 0 ? 1 : 0;
}

static inline bool pet_frame_is_current(uint32_t generation, uint32_t ticket)
{
    return generation == ticket;
}

static inline uint64_t pet_frame_due(uint64_t now, uint64_t start, unsigned frames, uint32_t period_ms)
{
    return (now - start) * frames / period_ms;
}

static inline uint64_t pet_frame_request(uint64_t due, uint64_t shown)
{
    return due > shown ? due : shown + 1;
}
