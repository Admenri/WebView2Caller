
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
