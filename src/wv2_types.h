#ifndef WV2_TYPES_H
#define WV2_TYPES_H
#pragma once

#include "WebView2.h"
#include "WebView2EnvironmentOptions.h"

#include <wrl/event.h>

#include <stdint.h>
#include <windows.h>

#define DLL_EXPORTS(name, type) \
  extern "C" __declspec(dllexport) type __cdecl wv2_##name

#endif
