
#include "wv2_controller.h"
#include "wv2_utils.h"

using namespace Microsoft;

DLL_EXPORTS(Controller_Close, BOOL)(ICoreWebView2Controller* controller) {
  if (!controller) return FALSE;

  auto ret = SUCCEEDED(controller->Close());
  return ret;
}

DLL_EXPORTS(Controller_GetWebview2, BOOL)
(ICoreWebView2Controller* controller, LPVOID* retPtr) {
  if (!controller) return FALSE;

  ICoreWebView2* wv2 = nullptr;
  
  auto ret = SUCCEEDED(controller->get_CoreWebView2(&wv2));
  
  *retPtr = wv2;

  return ret;
}

DLL_EXPORTS(Controller_GetVisible, BOOL)(ICoreWebView2Controller* controller) {
  if (!controller) return FALSE;

  BOOL ret = FALSE;

  controller->get_IsVisible(&ret);
  return ret;
}

DLL_EXPORTS(Controller_SetVisible, BOOL)
(ICoreWebView2Controller* controller, BOOL visible) {
  if (!controller) return FALSE;

  auto ret = SUCCEEDED(controller->put_IsVisible(visible));
  return ret;
}

DLL_EXPORTS(Controller_GetBounds, BOOL)
(ICoreWebView2Controller* controller, RECT* rect) {
  if (!controller) return FALSE;

  auto ret = SUCCEEDED(controller->get_Bounds(rect));
  return ret;
}

DLL_EXPORTS(Controller_SetBounds, BOOL)
(ICoreWebView2Controller* controller, RECT* rect) {
  if (!controller) return FALSE;

  auto ret = SUCCEEDED(controller->put_Bounds(*rect));
  return ret;
}

DLL_EXPORTS(Controller_GetZoomFactor, BOOL)
(ICoreWebView2Controller* controller, double* zoom) {
  if (!controller) return FALSE;

  auto ret = SUCCEEDED(controller->get_ZoomFactor(zoom));
  return ret;
}

DLL_EXPORTS(Controller_SetZoomFactor, BOOL)
(ICoreWebView2Controller* controller, double zoom) {
  if (!controller) return FALSE;

  auto ret = SUCCEEDED(controller->put_ZoomFactor(zoom));
  return ret;
}

DLL_EXPORTS(Controller_NotifyParentChanged, BOOL)
(ICoreWebView2Controller* controller) {
  if (!controller) return FALSE;

  auto ret = SUCCEEDED(controller->NotifyParentWindowPositionChanged());
  return ret;
}

DLL_EXPORTS(Controller_GetParent, BOOL)
(ICoreWebView2Controller* controller, HWND* zoom) {
  if (!controller) return FALSE;

  auto ret = SUCCEEDED(controller->get_ParentWindow(zoom));
  return ret;
}

DLL_EXPORTS(Controller_SetParent, BOOL)
(ICoreWebView2Controller* controller, HWND zoom) {
  if (!controller) return FALSE;

  auto ret = SUCCEEDED(controller->put_ParentWindow(zoom));
  return ret;
}

struct BGColor {
  uint8_t a;
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

DLL_EXPORTS(Controller_GetBackgroundColor, BOOL)
(ICoreWebView2Controller* controller, BGColor* cr) {
  if (!controller) return FALSE;

  WRL::ComPtr<ICoreWebView2Controller2> con = nullptr;
  controller->QueryInterface<ICoreWebView2Controller2>(&con);

  auto ret = SUCCEEDED(con->get_DefaultBackgroundColor(
      reinterpret_cast<COREWEBVIEW2_COLOR*>(cr)));
  return ret;
}

DLL_EXPORTS(Controller_SetBackgroundColor, BOOL)
(ICoreWebView2Controller* controller, BGColor* cr) {
  if (!controller) return FALSE;

  WRL::ComPtr<ICoreWebView2Controller2> con = nullptr;
  controller->QueryInterface<ICoreWebView2Controller2>(&con);

  auto ret = SUCCEEDED(con->put_DefaultBackgroundColor(
      *reinterpret_cast<COREWEBVIEW2_COLOR*>(cr)));
  return ret;
}

using AcceleratorKeyPressedCB = BOOL(CALLBACK*)(LPVOID controller, COREWEBVIEW2_KEY_EVENT_KIND type,
  uint32_t vkey, int32_t lparam, uint32_t repeat, uint32_t scancode, BOOL extendkey, BOOL menukeydown,
  BOOL keydown, BOOL keyrelease, LPVOID param);
DLL_EXPORTS(Controller_Attach_AcceleratorKeyPressed, int64_t)
(ICoreWebView2Controller* controller, AcceleratorKeyPressedCB callback,
 LPVOID param) {
  if (!controller) return FALSE;

  EventRegistrationToken token;

  controller->add_AcceleratorKeyPressed(
      WRL::Callback<ICoreWebView2AcceleratorKeyPressedEventHandler>(
          [callback, param](
              ICoreWebView2Controller* sender,
              ICoreWebView2AcceleratorKeyPressedEventArgs* args) -> HRESULT {
            sender->AddRef();

            COREWEBVIEW2_KEY_EVENT_KIND sType;
            uint32_t sVkey = 0;
            int32_t sLparam = 0;
            COREWEBVIEW2_PHYSICAL_KEY_STATUS sStatus;

            args->get_KeyEventKind(&sType);
            args->get_VirtualKey(&sVkey);
            args->get_KeyEventLParam(&sLparam);
            args->get_PhysicalKeyStatus(&sStatus);

            BOOL hr = callback(
                sender, sType, sVkey, sLparam, sStatus.RepeatCount,
                sStatus.ScanCode, sStatus.IsExtendedKey, sStatus.IsMenuKeyDown,
                sStatus.WasKeyDown, sStatus.IsKeyReleased, param);

            args->put_Handled(hr);

            return S_OK;
          })
          .Get(),
      &token);

  return token.value;
}

DLL_EXPORTS(Controller_Detach_AcceleratorKeyPressed, BOOL)
(ICoreWebView2Controller* controller, int64_t value) {
  if (!controller) return FALSE;
  EventRegistrationToken token = {value};

  return SUCCEEDED(controller->remove_AcceleratorKeyPressed(token));
}


DLL_EXPORTS(Controller_GetAllowExternalDrop, BOOL)
(ICoreWebView2Controller* controller) {
  if (!controller) return FALSE;

  WRL::ComPtr<ICoreWebView2Controller4> con = nullptr;
  controller->QueryInterface<ICoreWebView2Controller4>(&con);

  BOOL value = FALSE;
  con->get_AllowExternalDrop(&value);

  return value;
}

DLL_EXPORTS(Controller_SetAllowExternalDrop, BOOL)
(ICoreWebView2Controller* controller, BOOL allow) {
  if (!controller) return FALSE;

  WRL::ComPtr<ICoreWebView2Controller4> con = nullptr;
  controller->QueryInterface<ICoreWebView2Controller4>(&con);

  return con->put_AllowExternalDrop(allow);
}


using ZoomFactorChangedCB = BOOL(CALLBACK*)(LPVOID controller, LPVOID param);
DLL_EXPORTS(Controller_Attach_ZoomFactorChanged, int64_t)
(ICoreWebView2Controller* controller, ZoomFactorChangedCB callback,
 LPVOID param) {
  if (!controller) return FALSE;

  EventRegistrationToken token;

  controller->add_ZoomFactorChanged(
      WRL::Callback<ICoreWebView2ZoomFactorChangedEventHandler>(
          [callback, param](ICoreWebView2Controller* sender,
                            IUnknown* args) -> HRESULT {
            sender->AddRef();

            BOOL hr = callback(sender, param);

            return S_OK;
          })
          .Get(),
      &token);

  return token.value;
}

DLL_EXPORTS(Controller_Detach_ZoomFactorChanged, BOOL)
(ICoreWebView2Controller* controller, int64_t value) {
  if (!controller) return FALSE;
  EventRegistrationToken token = {value};

  return SUCCEEDED(controller->remove_ZoomFactorChanged(token));
}