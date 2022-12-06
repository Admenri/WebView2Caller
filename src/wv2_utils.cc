
#include "wv2_utils.h"

#include <thread>
#include <mutex>
#include <chrono>

#include <stdint.h>

static inline uint16_t byteswap_ushort(uint16_t number) {
  return _byteswap_ushort(number);
}

using namespace Microsoft;

DLL_EXPORTS(Deferral_Complete, BOOL)(ICoreWebView2Deferral* deferral) {
  auto ret = SUCCEEDED(deferral->Complete());
  deferral->Release();
  return ret;
}

DLL_EXPORTS(Object_AddRef, uint32_t)(IUnknown* obj) {
  if (!obj) return NULL;
  
  return obj->AddRef();
}

DLL_EXPORTS(Object_Release, uint32_t)(IUnknown* obj) {
  if (!obj) return NULL;

  return obj->Release();
}

/*-------------------------------------------------------------------------------*/

struct Waitable {
  BOOL isActive;
  BOOL autoRelease;
};

Waitable* CreateWaitable(BOOL release_after_wait) {
  Waitable* p = new Waitable();
  p->isActive = FALSE;
  p->autoRelease = release_after_wait;
  return p;
}

void ReleaseWaitable(Waitable* obj) {
  obj->isActive = TRUE;
  delete obj;
}

void ActiveWaitable(Waitable* obj) {
  if (!obj) return;
  obj->isActive = TRUE;
}

void WaitOfSleep(Waitable* obj, uint32_t max_ms) {
  if (!obj) return;
  if (max_ms <= 0) return;
  if (obj->isActive) return;

  uint32_t nCount = 0;
  while (!obj->isActive) {
    Sleep(10);

    nCount++;
    if (nCount > max_ms) break;
  }

  if (obj->autoRelease) delete obj;
}

void WaitOfMsgLoop(Waitable* obj) {
  if (!obj) return;
  if (obj->isActive) return;

  // WebView2 Pump Received Host
  // FindWindowEx(HWND_MESSAGE, NULL, L"Chrome_MessageWindow", NULL);

  // WebView2 MessageLoop MSG ID:
  // WM_USER + 1

  MSG uiMsg;
  while (!obj->isActive) {
    if (PeekMessage(&uiMsg, NULL, WM_USER + 1, WM_USER + 1, PM_REMOVE)) {
      TranslateMessage(&uiMsg);
      DispatchMessage(&uiMsg);
      if (uiMsg.message == WM_QUIT) {
        break;
      }
    }
  }

  if (obj->autoRelease) delete obj;
}


DLL_EXPORTS(Utility_Malloc, void*)(uint32_t size) {
  return LocalAlloc(LMEM_ZEROINIT, size);
}

DLL_EXPORTS(Utility_Mfree, void)(void* mem) { LocalFree(mem); }
