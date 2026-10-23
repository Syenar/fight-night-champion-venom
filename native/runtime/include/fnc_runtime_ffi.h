#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void* fnc_runtime_create(const uint8_t* root, uintptr_t len);
int32_t fnc_runtime_initialize(void* runtime);
int32_t fnc_runtime_tick(void* runtime, uint64_t ticks);
int32_t fnc_runtime_present_sbs(void* runtime, uint64_t frame_id,
                                uint32_t width, uint32_t height,
                                uint64_t left_view_id, uint64_t right_view_id);
void fnc_runtime_destroy(void* runtime);

#ifdef __cplusplus
}
#endif

