#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void* fnc_runtime_create(const uint8_t* root, uintptr_t len);
int32_t fnc_runtime_initialize(void* runtime);
int32_t fnc_runtime_tick(void* runtime, uint64_t ticks);
int32_t fnc_runtime_present_sbs(void* runtime, uint32_t width, uint32_t height);
void fnc_runtime_destroy(void* runtime);

#ifdef __cplusplus
}
#endif
