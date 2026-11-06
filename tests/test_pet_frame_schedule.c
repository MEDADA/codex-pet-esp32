#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "pet_frame_schedule.h"

int main(void)
{
    assert(pet_frame_back_buffer(0) == 1);
    assert(pet_frame_back_buffer(1) == 0);
    assert(pet_frame_back_buffer(-1) == 0);
    assert(!pet_frame_is_current(8, 7));
    assert(pet_frame_is_current(8, 8));

    assert(pet_frame_due(1000, 1000, 238, 30000) == 0);
    assert(pet_frame_due(31000, 1000, 238, 30000) == 238);
    assert(pet_frame_due(9000, 1000, 96, 8000) == 96);
    assert(pet_frame_due(1166, 1000, 6, 1000) == 0);
    assert(pet_frame_due(1167, 1000, 6, 1000) == 1);
    assert(pet_frame_due((uint64_t)UINT32_MAX + 1001, UINT32_MAX - 999, 6, 1000) == 12);
    assert(pet_frame_request(0, 0) == 1);
    assert(pet_frame_request(3, 1) == 3);
    puts("PASS animation buffer ownership, stale tickets and frame scheduling");
}
