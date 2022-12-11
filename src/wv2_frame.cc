
#include "wv2_frame.h"
using namespace Microsoft;

using FrameDestroyedCB = HRESULT(CALLBACK*)(LPVOID frame, LPVOID param);
DLL_EXPORTS(Frame_Attach_Destroyed, int64_t)
(ICoreWebView2Frame* frame, FrameDestroyedCB callback, LPVOID param) {
  if (!frame) return FALSE;

  EventRegistrationToken token;

  frame->add_Destroyed(WRL::Callback<ICoreWebView2FrameDestroyedEventHandler>(
                           [callback, param](ICoreWebView2Frame* sender,
                                             IUnknown* args) -> HRESULT {
                             sender->AddRef();

                             HRESULT hr = callback(sender, param);

                             return hr;
                           })
                           .Get(),
                       &token);

  return token.value;
}

DLL_EXPORTS(Frame_Detach_Destroyed, BOOL)
(ICoreWebView2Frame* frame, int64_t value) {
  if (!frame) return FALSE;
  EventRegistrationToken token = {value};

  return SUCCEEDED(frame->remove_Destroyed(token));
}


using NameChangedCB = HRESULT(CALLBACK*)(LPVOID frame, LPVOID param);
DLL_EXPORTS(Frame_Attach_NameChanged, int64_t)
(ICoreWebView2Frame* frame, NameChangedCB callback, LPVOID param) {
  if (!frame) return FALSE;

  EventRegistrationToken token;

  frame->add_NameChanged(
      WRL::Callback<ICoreWebView2FrameNameChangedEventHandler>(
          [callback, param](ICoreWebView2Frame* sender,
                            IUnknown* args) -> HRESULT {
            sender->AddRef();

            HRESULT hr = callback(sender, param);

            return hr;
          })
          .Get(),
      &token);

  return token.value;
}

DLL_EXPORTS(Frame_Detach_NameChanged, BOOL)
(ICoreWebView2Frame* frame, int64_t value) {
  if (!frame) return FALSE;
  EventRegistrationToken token = {value};

  return SUCCEEDED(frame->remove_NameChanged(token));
}


DLL_EXPORTS(Frame_GetName, BOOL)
(ICoreWebView2Frame* frame, LPVOID* ptr, uint32_t* size) {
  if (!frame) return FALSE;

  LPWSTR value = nullptr;
  HRESULT hr = frame->get_Name(&value);

  *ptr = value;
  *size = lstrlenW(value) * 2 + 2;

  return SUCCEEDED(hr);
}

DLL_EXPORTS(Frame_IsDestroyed, BOOL)
(ICoreWebView2Frame* frame) {
  if (!frame) return TRUE;

  BOOL isDestroyed = FALSE;
  frame->IsDestroyed(&isDestroyed);

  return isDestroyed;
}

using ExecuteScriptCB = HRESULT(CALLBACK*)(HRESULT code, LPCVOID ptr,
                                           uint32_t size, LPVOID param);
DLL_EXPORTS(Frame_ExecuteScript, BOOL)
(ICoreWebView2Frame2* webview, LPCWSTR script, ExecuteScriptCB callback,
 LPVOID param) {
  if (!webview) return FALSE;

  auto ret = SUCCEEDED(webview->ExecuteScript(
      script, WRL::Callback<ICoreWebView2ExecuteScriptCompletedHandler>(
                  [callback, param](HRESULT errorCode,
                                    LPCWSTR resultObjectAsJson) -> HRESULT {
                    if (callback) {
                      uint32_t sizeTmp = lstrlenW(resultObjectAsJson) * 2 + 2;
                      LPWSTR newStr =
                          static_cast<LPWSTR>(wv2_Utility_Malloc(sizeTmp));
                      if (newStr) {
                        newStr[sizeTmp - 1] = 0;
                        lstrcpyW(newStr, resultObjectAsJson);
                      }

                      return callback(errorCode, newStr, sizeTmp, param);
                    } else
                      return S_OK;
                  })
                  .Get()));

  return ret;
}

DLL_EXPORTS(Frame_ExecuteScript_Sync, BOOL)
(ICoreWebView2Frame2* webview, LPCWSTR script, LPCVOID* ptr, uint32_t* size) {
  if (!webview) return FALSE;

  Waitable* waiter = CreateWaitable(true);

  auto ret = SUCCEEDED(webview->ExecuteScript(
      script, WRL::Callback<ICoreWebView2ExecuteScriptCompletedHandler>(
                  [waiter, ptr, size](HRESULT errorCode,
                                      LPCWSTR resultObjectAsJson) -> HRESULT {
                    uint32_t sizeTmp = lstrlenW(resultObjectAsJson) * 2 + 2;
                    LPWSTR newStr =
                        static_cast<LPWSTR>(wv2_Utility_Malloc(sizeTmp));
                    if (newStr) {
                      newStr[sizeTmp - 1] = 0;
                      lstrcpyW(newStr, resultObjectAsJson);

                      *ptr = newStr;
                      *size = sizeTmp;
                    }

                    ActiveWaitable(waiter);

                    return S_OK;
                  })
                  .Get()));

  WaitOfMsgLoop(waiter);

  return ret;
}
