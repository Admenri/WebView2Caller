#ifndef WV2_UTILS_H
#define WV2_UTILS_H
#pragma once

#include "wv2_types.h"
#include <string>

struct Waitable;
Waitable* CreateWaitable(BOOL release_after_wait);
void ReleaseWaitable(Waitable* obj);
void ActiveWaitable(Waitable* obj);
void WaitOfSleep(Waitable* obj, uint32_t max_ms);
void WaitOfMsgLoop(Waitable* obj);

DLL_EXPORTS(Utility_Malloc, void*)(uint32_t size);
DLL_EXPORTS(Utility_Mfree, void)(void* mem);

namespace conv {

LPWSTR ToUnicode(const char* str);
LPSTR ToAnsi(const wchar_t* str);

}  // namespace conv

#endif
