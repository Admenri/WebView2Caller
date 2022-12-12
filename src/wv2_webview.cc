
#include "wv2_controller.h"
#include "wv2_types.h"
#include "wv2_utils.h"

#include <string>
#include <vector>

#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

using namespace Microsoft;

DLL_EXPORTS(Webview_CanGoBack, BOOL)(ICoreWebView2* webview) {
  if (!webview) return FALSE;

  BOOL ret = FALSE;

  webview->get_CanGoBack(&ret);

  return ret;
}

DLL_EXPORTS(Webview_CanGoForward, BOOL)(ICoreWebView2* webview) {
  if (!webview) return FALSE;

  BOOL ret = FALSE;

  webview->get_CanGoForward(&ret);

  return ret;
}

DLL_EXPORTS(Webview_GoBack, BOOL)(ICoreWebView2* webview) {
  if (!webview) return FALSE;

  return SUCCEEDED(webview->GoBack());
}

DLL_EXPORTS(Webview_GoForward, BOOL)(ICoreWebView2* webview) {
  if (!webview) return FALSE;

  return SUCCEEDED(webview->GoForward());
}

DLL_EXPORTS(Webview_Reload, BOOL)(ICoreWebView2* webview) {
  if (!webview) return FALSE;

  return SUCCEEDED(webview->Reload());
}

DLL_EXPORTS(Webview_Stop, BOOL)(ICoreWebView2* webview) {
  if (!webview) return FALSE;

  return SUCCEEDED(webview->Stop());
}

DLL_EXPORTS(Webview_Navigate, BOOL)(ICoreWebView2* webview, LPCWSTR url) {
  if (!webview) return FALSE;

  // Check uri
  std::wstring uri(url);
  HRESULT hr = webview->Navigate(uri.c_str());
  if (hr == E_INVALIDARG) {
    if (uri.find(L' ') == std::wstring::npos &&
        uri.find(L'.') != std::wstring::npos) {
      hr = webview->Navigate((L"http://" + uri).c_str());
    } else
      return FALSE;
  }

  return SUCCEEDED(hr);
}

DLL_EXPORTS(Webview_NavigateToString, BOOL)
(ICoreWebView2* webview, LPCWSTR html) {
  if (!webview) return FALSE;

  return SUCCEEDED(webview->NavigateToString(html));
}

using ExecuteScriptCB = HRESULT(CALLBACK*)(HRESULT code, LPCVOID ptr, uint32_t size, LPVOID param);
DLL_EXPORTS(Webview_ExecuteScript, BOOL)
(ICoreWebView2* webview, LPCWSTR script, ExecuteScriptCB callback,
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

DLL_EXPORTS(Webview_ExecuteScript_Sync, BOOL)
(ICoreWebView2* webview, LPCWSTR script, LPCVOID* ptr, uint32_t* size) {
  if (!webview) return FALSE;

  Waitable* waiter = CreateWaitable(true);

  auto ret = SUCCEEDED(webview->ExecuteScript(
      script, WRL::Callback<ICoreWebView2ExecuteScriptCompletedHandler>(
                  [waiter, ptr, size](HRESULT errorCode,
                                      LPCWSTR resultObjectAsJson) -> HRESULT {
                    uint32_t sizeTmp = lstrlenW(resultObjectAsJson) * 2 + 2;
                    LPWSTR newStr = static_cast<LPWSTR>(wv2_Utility_Malloc(sizeTmp));
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

DLL_EXPORTS(Webview_GetURL, BOOL)
(ICoreWebView2* webview, LPVOID* ptr, uint32_t* size) {
  if (!webview) return FALSE;

  LPWSTR source = nullptr;
  auto ret = SUCCEEDED(webview->get_Source(&source));

  *ptr = source;
  *size = lstrlenW(source) * 2 + 2;

  return ret;
}

DLL_EXPORTS(Webview_GetDocumentTitle, BOOL)
(ICoreWebView2* webview, LPVOID* ptr, uint32_t* size) {
  if (!webview) return FALSE;

  LPWSTR source = nullptr;
  auto ret = SUCCEEDED(webview->get_DocumentTitle(&source));

  *ptr = source;
  *size = lstrlenW(source) * 2 + 2;

  return ret;
}

using CallDevtoolsProtocolsMethodCB = HRESULT(CALLBACK*)(LPVOID result, uint32_t size, LPVOID param);
DLL_EXPORTS(Webview_CallDevtoolsProtocolsMethod, BOOL)
(ICoreWebView2* webview, LPWSTR method, LPWSTR paramJSON,
 CallDevtoolsProtocolsMethodCB callback, LPVOID param) {
  if (!webview) return FALSE;

  LPWSTR source = nullptr;
  auto ret = SUCCEEDED(webview->CallDevToolsProtocolMethod(
      method, paramJSON,
      WRL::Callback<ICoreWebView2CallDevToolsProtocolMethodCompletedHandler>(
          [callback, param](HRESULT errorCode,
                            LPCWSTR returnObjectAsJson) -> HRESULT {
            if (callback) {
              uint32_t sizeTmp = lstrlenW(returnObjectAsJson) * 2 + 2;
              LPWSTR newStr = static_cast<LPWSTR>(wv2_Utility_Malloc(sizeTmp));
              if (newStr) {
                newStr[sizeTmp - 1] = 0;
                lstrcpyW(newStr, returnObjectAsJson);
              }

              return callback(newStr, sizeTmp, param);
            } else
              return S_OK;
          })
          .Get()));

  return ret;
}

DLL_EXPORTS(Webview_CallDevtoolsProtocolsMethod_Sync, BOOL)
(ICoreWebView2* webview, LPWSTR method, LPWSTR paramJSON, LPCVOID* ptr,
 uint32_t* size) {
  if (!webview) return FALSE;

  Waitable* waiter = CreateWaitable(true);

  auto ret = SUCCEEDED(webview->CallDevToolsProtocolMethod(
      method, paramJSON,
      WRL::Callback<ICoreWebView2CallDevToolsProtocolMethodCompletedHandler>(
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

DLL_EXPORTS(Webview_GetBrowserProcessID, uint32_t)(ICoreWebView2* webview) {
  if (!webview) return FALSE;

  uint32_t pid = 0;
  webview->get_BrowserProcessId(&pid);

  return pid;
}

using CapturePreviewCB = HRESULT(CALLBACK*)(LPVOID lpMem, uint32_t size, LPVOID param);
DLL_EXPORTS(Webview_CapturePreview, BOOL)
(ICoreWebView2* webview, COREWEBVIEW2_CAPTURE_PREVIEW_IMAGE_FORMAT format,
 CapturePreviewCB callback, LPVOID param) {
  if (!webview) return FALSE;

  IStream* is = SHCreateMemStream(nullptr, 0);

  HRESULT hr = webview->CapturePreview(
      format, is,
      WRL::Callback<ICoreWebView2CapturePreviewCompletedHandler>(
          [is, callback, param](HRESULT errorCode) -> HRESULT {
            STATSTG stat;
            is->Stat(&stat, STATFLAG_NONAME);

            LARGE_INTEGER linfo;
            linfo.QuadPart = 0;
            is->Seek(linfo, STREAM_SEEK_SET, NULL);

            uint32_t size = stat.cbSize.LowPart;
            uint8_t* buf = static_cast<uint8_t*>(wv2_Utility_Malloc(size));
            ULONG dummy = 0;
            is->Read(buf, size, &dummy);

            HRESULT hr = callback(buf, size, param);

            wv2_Utility_Mfree(buf);

            is->Release();
            return hr;
          })
          .Get());

  return SUCCEEDED(hr);
}

DLL_EXPORTS(Webview_CapturePreview_Sync, BOOL)
(ICoreWebView2* webview, COREWEBVIEW2_CAPTURE_PREVIEW_IMAGE_FORMAT format, LPVOID *ptr, uint32_t *psize) {
  if (!webview) return FALSE;

  Waitable* waiter = CreateWaitable(true);

  IStream* is = SHCreateMemStream(nullptr, 0);

  HRESULT hr = webview->CapturePreview(
      format, is,
      WRL::Callback<ICoreWebView2CapturePreviewCompletedHandler>(
          [waiter, is, ptr, psize](HRESULT errorCode) -> HRESULT {
            STATSTG stat;
            is->Stat(&stat, STATFLAG_NONAME);

            LARGE_INTEGER linfo;
            linfo.QuadPart = 0;
            is->Seek(linfo, STREAM_SEEK_SET, NULL);

            uint32_t size = stat.cbSize.LowPart;
            uint8_t* buf = static_cast<decltype(buf)>(wv2_Utility_Malloc(size));

            ULONG dummy = 0;
            is->Read(buf, size, &dummy);

            *ptr = buf;
            *psize = size;

            is->Release();
            
            ActiveWaitable(waiter);
            return S_OK;
          })
          .Get());

  WaitOfMsgLoop(waiter);

  return SUCCEEDED(hr);
}

DLL_EXPORTS(Webview_OpenDevTools, uint32_t)(ICoreWebView2* webview) {
  if (!webview) return FALSE;

  return SUCCEEDED(webview->OpenDevToolsWindow());
}



DLL_EXPORTS(Webview_NewWindowRequestArgs_GetURL, BOOL)
(ICoreWebView2NewWindowRequestedEventArgs* args, LPVOID* ptr, uint32_t* size) {
  if (!args) return FALSE;

  LPWSTR url = nullptr;
  auto ret = SUCCEEDED(args->get_Uri(&url));

  *ptr = url;
  *size = lstrlenW(url) * 2 + 2;

  return ret;
}

DLL_EXPORTS(Webview_NewWindowRequestArgs_SetNewWindow, BOOL)
(ICoreWebView2NewWindowRequestedEventArgs* args, ICoreWebView2* window) {
  if (!args) return FALSE;

  return SUCCEEDED(args->put_NewWindow(window));
}

DLL_EXPORTS(Webview_NewWindowRequestArgs_GetNewWindow, BOOL)
(ICoreWebView2NewWindowRequestedEventArgs* args, LPVOID* ptr) {
  if (!args) return FALSE;

  ICoreWebView2* window = nullptr;

  auto ret = SUCCEEDED(args->get_NewWindow(&window));

  *ptr = window;

  return ret;
}

DLL_EXPORTS(Webview_NewWindowRequestArgs_SetHandled, BOOL)
(ICoreWebView2NewWindowRequestedEventArgs* args, BOOL handle) {
  if (!args) return FALSE;

  return SUCCEEDED(args->put_Handled(handle));
}

DLL_EXPORTS(Webview_NewWindowRequestArgs_GetHandled, BOOL)
(ICoreWebView2NewWindowRequestedEventArgs* args) {
  if (!args) return FALSE;

  BOOL ret = FALSE;

  args->get_Handled(&ret);

  return ret;
}

DLL_EXPORTS(Webview_NewWindowRequestArgs_GetDeferral, BOOL)
(ICoreWebView2NewWindowRequestedEventArgs* args, LPVOID* ptr) {
  if (!args) return FALSE;

  ICoreWebView2Deferral* deferral = nullptr;

  auto ret = SUCCEEDED(args->GetDeferral(&deferral));

  *ptr = deferral;

  return ret;
}

struct NewWindowFeatures {
  BOOL hasPos;
  BOOL hasSize;
  uint32_t left;
  uint32_t top;
  uint32_t height;
  uint32_t width;
  BOOL menuBar;
  BOOL statusBar;
  BOOL toolBar;
  BOOL scrollBar;
};

DLL_EXPORTS(Webview_NewWindowRequestArgs_GetFeatures, BOOL)
(ICoreWebView2NewWindowRequestedEventArgs* args, NewWindowFeatures* pFeatures) {
  if (!args) return FALSE;

  ICoreWebView2WindowFeatures* features = nullptr;

  auto ret = SUCCEEDED(args->get_WindowFeatures(&features));

  if (pFeatures) {
    features->get_HasPosition(&pFeatures->hasPos);
    features->get_HasSize(&pFeatures->hasSize);
    features->get_Left(&pFeatures->left);
    features->get_Top(&pFeatures->top);
    features->get_Height(&pFeatures->height);
    features->get_Width(&pFeatures->width);
    features->get_ShouldDisplayMenuBar(&pFeatures->menuBar);
    features->get_ShouldDisplayStatus(&pFeatures->statusBar);
    features->get_ShouldDisplayToolbar(&pFeatures->toolBar);
    features->get_ShouldDisplayScrollBars(&pFeatures->scrollBar);
  }

  return ret;
}

using NewWindowRequestCB = HRESULT(CALLBACK*)(LPVOID wv, LPVOID args, LPVOID param);
DLL_EXPORTS(Webview_Attach_NewWindowRequest, int64_t)
(ICoreWebView2* webview, NewWindowRequestCB callback, LPVOID param) {
  if (!webview) return FALSE;

  EventRegistrationToken token;

  webview->add_NewWindowRequested(
      WRL::Callback<ICoreWebView2NewWindowRequestedEventHandler>(
          [callback, param](
              ICoreWebView2* sender,
              ICoreWebView2NewWindowRequestedEventArgs* args) -> HRESULT {
            sender->AddRef();
            args->AddRef();
            return callback(sender, args, param);
          })
          .Get(),
      &token);

  return token.value;
}

DLL_EXPORTS(Webview_Detach_NewWindowRequest, BOOL)
(ICoreWebView2* webview, int64_t value) {
  if (!webview) return FALSE;
  EventRegistrationToken token = {value};
  
  return SUCCEEDED(webview->remove_NewWindowRequested(token));
}


using WindowCloseRequestCB = HRESULT(CALLBACK*)(LPVOID wv, LPVOID param);
DLL_EXPORTS(Webview_Attach_WindowCloseRequest, int64_t)
(ICoreWebView2* webview, WindowCloseRequestCB callback, LPVOID param) {
  if (!webview) return FALSE;

  EventRegistrationToken token;

  webview->add_WindowCloseRequested(
      WRL::Callback<ICoreWebView2WindowCloseRequestedEventHandler>(
          [callback, param](ICoreWebView2* sender, IUnknown* args) -> HRESULT {
            sender->AddRef();
            return callback(sender, param);
          })
          .Get(),
      &token);

  return token.value;
}

DLL_EXPORTS(Webview_Detach_WindowCloseRequest, BOOL)
(ICoreWebView2* webview, int64_t value) {
  if (!webview) return FALSE;
  EventRegistrationToken token = {value};

  return SUCCEEDED(webview->remove_WindowCloseRequested(token));
}


using SourceChangedCB = HRESULT(CALLBACK*)(LPVOID wv, BOOL isNewDoc, LPVOID param);
DLL_EXPORTS(Webview_Attach_SourceChanged, int64_t)
(ICoreWebView2* webview, SourceChangedCB callback, LPVOID param) {
  if (!webview) return FALSE;

  EventRegistrationToken token;

  webview->add_SourceChanged(
      WRL::Callback<ICoreWebView2SourceChangedEventHandler>(
          [callback, param](
              ICoreWebView2* sender,
              ICoreWebView2SourceChangedEventArgs* args) -> HRESULT {
            BOOL newDoc = FALSE;
            args->get_IsNewDocument(&newDoc);
            sender->AddRef();
            return callback(sender, newDoc, param);
          })
          .Get(),
      &token);

  return token.value;
}

DLL_EXPORTS(Webview_Detach_SourceChanged, BOOL)
(ICoreWebView2* webview, int64_t value) {
  if (!webview) return FALSE;
  EventRegistrationToken token = {value};

  return SUCCEEDED(webview->remove_SourceChanged(token));
}

using DocumentTitleChangedCB = HRESULT(CALLBACK*)(LPVOID wv,
                                           LPVOID param);
DLL_EXPORTS(Webview_Attach_DocumentTitleChanged, int64_t)
(ICoreWebView2* webview, DocumentTitleChangedCB callback, LPVOID param) {
  if (!webview) return FALSE;

  EventRegistrationToken token;

  webview->add_DocumentTitleChanged(
      WRL::Callback<ICoreWebView2DocumentTitleChangedEventHandler>(
          [callback, param](ICoreWebView2* sender, IUnknown* args) -> HRESULT {
            sender->AddRef();
            return callback(sender, param);
          })
          .Get(),
      &token);

  return token.value;
}

DLL_EXPORTS(Webview_Detach_DocumentTitleChanged, BOOL)
(ICoreWebView2* webview, int64_t value) {
  if (!webview) return FALSE;
  EventRegistrationToken token = {value};

  return SUCCEEDED(webview->remove_DocumentTitleChanged(token));
}

DLL_EXPORTS(Webview_NavigationStartingEventArgs_GetURL, BOOL)
(ICoreWebView2NavigationStartingEventArgs* args, LPVOID* ptr, uint32_t* size) {
  if (!args) return FALSE;

  LPWSTR url = nullptr;
  auto ret = SUCCEEDED(args->get_Uri(&url));

  *ptr = url;
  *size = lstrlenW(url) * 2 + 2;
  return ret;
}


DLL_EXPORTS(Webview_NavigationStartingEventArgs_PutCancel, BOOL)
(ICoreWebView2NavigationStartingEventArgs* args, BOOL cancel) {
  if (!args) return FALSE;

  auto ret = SUCCEEDED(args->put_Cancel(cancel));
  return ret;
}

DLL_EXPORTS(Webview_NavigationStartingEventArgs_GetRequestHeader, BOOL)
(ICoreWebView2NavigationStartingEventArgs* args, LPVOID* ptr) {
  if (!args) return FALSE;

  ICoreWebView2HttpRequestHeaders* header = nullptr;
  auto ret = SUCCEEDED(args->get_RequestHeaders(&header));

  *ptr = header;

  return ret;
}

using NavigationStartingCB = HRESULT(CALLBACK*)(LPVOID wv, LPVOID args, BOOL user, BOOL redirect, uint64_t navId, LPVOID param);
DLL_EXPORTS(Webview_Attach_NavigationStarting, int64_t)
(ICoreWebView2* webview, NavigationStartingCB callback, LPVOID param) {
  if (!webview) return FALSE;

  EventRegistrationToken token;

  webview->add_NavigationStarting(
      WRL::Callback<ICoreWebView2NavigationStartingEventHandler>(
          [callback, param](
              ICoreWebView2* sender,
              ICoreWebView2NavigationStartingEventArgs* args) -> HRESULT {
            sender->AddRef();
            args->AddRef();

            BOOL success = FALSE, redirect = FALSE;
            uint64_t navId = 0;
            args->get_IsUserInitiated(&success);
            args->get_NavigationId(&navId);
            args->get_IsRedirected(&redirect);

            HRESULT hr =
                callback(sender, args, success, redirect, navId, param);

            return hr;
          })
          .Get(),
      &token);

  return token.value;
}

DLL_EXPORTS(Webview_Detach_NavigationStarting, BOOL)
(ICoreWebView2* webview, int64_t value) {
  if (!webview) return FALSE;
  EventRegistrationToken token = {value};

  return SUCCEEDED(webview->remove_NavigationStarting(token));
}

using NavigationCompletedCB = HRESULT(CALLBACK*)(LPVOID wv, BOOL success, int status, uint64_t navId,
                                                LPVOID param);
DLL_EXPORTS(Webview_Attach_NavigationCompleted, int64_t)
(ICoreWebView2* webview, NavigationCompletedCB callback, LPVOID param) {
  if (!webview) return FALSE;

  EventRegistrationToken token;

  webview->add_NavigationCompleted(
      WRL::Callback<ICoreWebView2NavigationCompletedEventHandler>(
          [callback, param](
              ICoreWebView2* sender,
              ICoreWebView2NavigationCompletedEventArgs* args) -> HRESULT {
            sender->AddRef();

            BOOL success = FALSE;
            uint64_t navId = 0;
            int status = 0;
            args->get_IsSuccess(&success);
            args->get_NavigationId(&navId);
            args->get_WebErrorStatus(
                reinterpret_cast<COREWEBVIEW2_WEB_ERROR_STATUS*>(&status));

            HRESULT hr = callback(sender, success, status, navId, param);

            return hr;
          })
          .Get(),
      &token);

  return token.value;
}

DLL_EXPORTS(Webview_Detach_NavigationCompleted, BOOL)
(ICoreWebView2* webview, int64_t value) {
  if (!webview) return FALSE;
  EventRegistrationToken token = {value};

  return SUCCEEDED(webview->remove_NavigationCompleted(token));
}


DLL_EXPORTS(Webview_GetFullscreenMode, BOOL)
(ICoreWebView2* webview) {
  if (!webview) return FALSE;

  BOOL ret = FALSE;
  webview->get_ContainsFullScreenElement(&ret);

  return ret;
}

using ContainsFullScreenElementChangedCB = HRESULT(CALLBACK*)(LPVOID wv,LPVOID param);
DLL_EXPORTS(Webview_Attach_ContainsFullScreenElementChanged, int64_t)
(ICoreWebView2* webview, ContainsFullScreenElementChangedCB callback,
 LPVOID param) {
  if (!webview) return FALSE;

  EventRegistrationToken token;

  webview->add_ContainsFullScreenElementChanged(
      WRL::Callback<ICoreWebView2ContainsFullScreenElementChangedEventHandler>(
          [callback, param](ICoreWebView2* sender, IUnknown* args) -> HRESULT {
            sender->AddRef();

            HRESULT hr = callback(sender, param);

            return hr;
          })
          .Get(),
      &token);

  return token.value;
}

DLL_EXPORTS(Webview_Detach_ContainsFullScreenElementChanged, BOOL)
(ICoreWebView2* webview, int64_t value) {
  if (!webview) return FALSE;
  EventRegistrationToken token = {value};

  return SUCCEEDED(webview->remove_ContainsFullScreenElementChanged(token));
}

struct WebView2Settings {
  BOOL enableScripts;
  BOOL enableWebMessage;
  BOOL enableDefaultDialogs;
  BOOL enableStatusBar;
  BOOL enableDevTools;
  BOOL enableDefaultContextMenu;
  BOOL enableHostJSObject;
  BOOL enableZoomControl;
  BOOL enableBuiltInError;
  BOOL enableBrowserAcceleratorKeys;
  BOOL enablePasswdAutoSave;
  BOOL enableAutoFillin;
  BOOL enablePinchZoom;
  BOOL enableSwipeNavigation;
  COREWEBVIEW2_PDF_TOOLBAR_ITEMS hiddenPdfToolbar;
};

DLL_EXPORTS(Webview_GetSettings, BOOL)
(ICoreWebView2* webview, WebView2Settings* pset) {
  if (!webview) return FALSE;

  WRL::ComPtr<ICoreWebView2Settings> settings = nullptr;
  auto ret = SUCCEEDED(webview->get_Settings(&settings));

  WRL::ComPtr<ICoreWebView2Settings7> target = nullptr;
  settings->QueryInterface<ICoreWebView2Settings7>(&target);

  if (pset) {
    target->get_IsScriptEnabled(&pset->enableScripts);
    target->get_IsWebMessageEnabled(&pset->enableWebMessage);
    target->get_AreDefaultScriptDialogsEnabled(&pset->enableDefaultDialogs);
    target->get_IsStatusBarEnabled(&pset->enableStatusBar);
    target->get_AreDevToolsEnabled(&pset->enableDevTools);
    target->get_AreDefaultContextMenusEnabled(&pset->enableDefaultContextMenu);
    target->get_AreHostObjectsAllowed(&pset->enableHostJSObject);
    target->get_IsZoomControlEnabled(&pset->enableZoomControl);
    target->get_IsBuiltInErrorPageEnabled(&pset->enableBuiltInError);
    target->get_AreBrowserAcceleratorKeysEnabled(
        &pset->enableBrowserAcceleratorKeys);
    target->get_IsPasswordAutosaveEnabled(&pset->enablePasswdAutoSave);
    target->get_IsGeneralAutofillEnabled(&pset->enableAutoFillin);
    target->get_IsPinchZoomEnabled(&pset->enablePinchZoom);
    target->get_IsSwipeNavigationEnabled(&pset->enableSwipeNavigation);
    target->get_HiddenPdfToolbarItems(&pset->hiddenPdfToolbar);
  }

  return ret;
}

DLL_EXPORTS(Webview_SetSettings, BOOL)
(ICoreWebView2* webview, WebView2Settings* pset) {
  if (!webview) return FALSE;

  WRL::ComPtr<ICoreWebView2Settings> settings = nullptr;
  auto ret = SUCCEEDED(webview->get_Settings(&settings));

  WRL::ComPtr<ICoreWebView2Settings7> target = nullptr;
  settings->QueryInterface<ICoreWebView2Settings7>(&target);

  if (pset) {
    target->put_IsScriptEnabled(pset->enableScripts);
    target->put_IsWebMessageEnabled(pset->enableWebMessage);
    target->put_AreDefaultScriptDialogsEnabled(pset->enableDefaultDialogs);
    target->put_IsStatusBarEnabled(pset->enableStatusBar);
    target->put_AreDevToolsEnabled(pset->enableDevTools);
    target->put_AreDefaultContextMenusEnabled(pset->enableDefaultContextMenu);
    target->put_AreHostObjectsAllowed(pset->enableHostJSObject);
    target->put_IsZoomControlEnabled(pset->enableZoomControl);
    target->put_IsBuiltInErrorPageEnabled(pset->enableBuiltInError);
    target->put_AreBrowserAcceleratorKeysEnabled(
        pset->enableBrowserAcceleratorKeys);
    target->put_IsPasswordAutosaveEnabled(pset->enablePasswdAutoSave);
    target->put_IsGeneralAutofillEnabled(pset->enableAutoFillin);
    target->put_IsPinchZoomEnabled(pset->enablePinchZoom);
    target->put_IsSwipeNavigationEnabled(pset->enableSwipeNavigation);
    target->put_HiddenPdfToolbarItems(pset->hiddenPdfToolbar);
  }

  return ret;
}

DLL_EXPORTS(Webview_GetUserAgent, BOOL)
(ICoreWebView2* webview, LPVOID* ptr, uint32_t* size) {
  if (!webview) return FALSE;
  
  WRL::ComPtr<ICoreWebView2Settings> settings = nullptr;
  auto ret = SUCCEEDED(webview->get_Settings(&settings));

  WRL::ComPtr<ICoreWebView2Settings2> rsettings = nullptr;
  settings->QueryInterface<ICoreWebView2Settings2>(&rsettings);

  LPWSTR strptr = nullptr;
  rsettings->get_UserAgent(&strptr);

  *ptr = strptr;
  *size = lstrlenW(strptr) * 2 + 2;

  return ret;
}

DLL_EXPORTS(Webview_SetUserAgent, BOOL)
(ICoreWebView2* webview, LPWSTR ua) {
  if (!webview) return FALSE;

  WRL::ComPtr<ICoreWebView2Settings> settings = nullptr;
  auto ret = SUCCEEDED(webview->get_Settings(&settings));

  WRL::ComPtr<ICoreWebView2Settings2> rsettings = nullptr;
  settings->QueryInterface<ICoreWebView2Settings2>(&rsettings);

  rsettings->put_UserAgent(ua);

  return ret;
}

DLL_EXPORTS(Webview_GetCookieManager, BOOL)
(ICoreWebView2* webview, LPVOID* ptr) {
  if (!webview) return FALSE;

  WRL::ComPtr<ICoreWebView2_2> tmpWv = nullptr;
  webview->QueryInterface<ICoreWebView2_2>(&tmpWv);

  ICoreWebView2CookieManager* ckManager = nullptr;
  auto ret = SUCCEEDED(tmpWv->get_CookieManager(&ckManager));

  *ptr = ckManager;

  return ret;
}

DLL_EXPORTS(Webview_GetEnvironment, BOOL)
(ICoreWebView2* webview, LPVOID* ptr) {
  if (!webview) return FALSE;

  WRL::ComPtr<ICoreWebView2_2> tmpWv = nullptr;
  webview->QueryInterface<ICoreWebView2_2>(&tmpWv);

  ICoreWebView2Environment* env = nullptr;
  auto ret = SUCCEEDED(tmpWv->get_Environment(&env));

  *ptr = env;

  return ret;
}

DLL_EXPORTS(Webview_OpenTaskManager, BOOL)
(ICoreWebView2* webview) {
  if (!webview) return FALSE;

  WRL::ComPtr<ICoreWebView2_6> tmpWv = nullptr;
  webview->QueryInterface<ICoreWebView2_6>(&tmpWv);

  auto ret = SUCCEEDED(tmpWv->OpenTaskManagerWindow());

  return ret;
}



DLL_EXPORTS(Webview_PermissionRequestArgs_GetURL, BOOL)
(ICoreWebView2PermissionRequestedEventArgs* args, LPVOID* ptr, uint32_t* size) {
  if (!args) return FALSE;
  
  LPWSTR url = nullptr;

  auto ret = SUCCEEDED(args->get_Uri(&url));
  
  *ptr = url;
  *size = lstrlenW(url) * 2 + 2;

  return ret;
}

DLL_EXPORTS(Webview_PermissionRequestArgs_GetType, int)
(ICoreWebView2PermissionRequestedEventArgs* args) {
  if (!args) return FALSE;

  COREWEBVIEW2_PERMISSION_KIND type;

  args->get_PermissionKind(&type);

  return type;
}

DLL_EXPORTS(Webview_PermissionRequestArgs_GetState, int)
(ICoreWebView2PermissionRequestedEventArgs* args) {
  if (!args) return FALSE;

  COREWEBVIEW2_PERMISSION_STATE state;

  args->get_State(&state);

  return state;
}

DLL_EXPORTS(Webview_PermissionRequestArgs_SetState, int)
(ICoreWebView2PermissionRequestedEventArgs* args,
 COREWEBVIEW2_PERMISSION_STATE state) {
  if (!args) return FALSE;

  return SUCCEEDED(args->put_State(state));
}


DLL_EXPORTS(Webview_PermissionRequestArgs_GetDeferral, BOOL)
(ICoreWebView2PermissionRequestedEventArgs* args, LPVOID* ptr) {
  if (!args) return FALSE;

  ICoreWebView2Deferral* deferral = nullptr;

  auto ret = SUCCEEDED(args->GetDeferral(&deferral));

  *ptr = deferral;

  return ret;
}

using PermissionRequestedCB = HRESULT(CALLBACK*)(LPVOID wv, BOOL userInitiated, LPVOID args, LPVOID param);
DLL_EXPORTS(Webview_Attach_PermissionRequested, int64_t)
(ICoreWebView2* webview, PermissionRequestedCB callback,
 LPVOID param) {
  if (!webview) return FALSE;

  EventRegistrationToken token;

  webview->add_PermissionRequested(
      WRL::Callback<ICoreWebView2PermissionRequestedEventHandler>(
          [callback, param](
              ICoreWebView2* sender,
              ICoreWebView2PermissionRequestedEventArgs* args)
              -> HRESULT {
            sender->AddRef();
            args->AddRef();

            BOOL ui = FALSE;
            args->get_IsUserInitiated(&ui);

            HRESULT hr = callback(sender, ui, args, param);

            return hr;
          })
          .Get(),
      &token);

  return token.value;
}

DLL_EXPORTS(Webview_Detach_PermissionRequested, BOOL)
(ICoreWebView2* webview, int64_t value) {
  if (!webview) return FALSE;
  EventRegistrationToken token = {value};

  return SUCCEEDED(webview->remove_PermissionRequested(token));
}

DLL_EXPORTS(Webview_IsDefaultDownloadDialogOpen, BOOL)
(ICoreWebView2* webview) {
  if (!webview) return FALSE;
  
  WRL::ComPtr<ICoreWebView2_9> tmpWv = nullptr;
  webview->QueryInterface<ICoreWebView2_9>(&tmpWv);

  BOOL ret = FALSE;

  tmpWv->get_IsDefaultDownloadDialogOpen(&ret);

  return ret;
}

DLL_EXPORTS(Webview_OpenDefaultDownloadDialog, BOOL)
(ICoreWebView2* webview) {
  if (!webview) return FALSE;

  WRL::ComPtr<ICoreWebView2_9> tmpWv = nullptr;
  webview->QueryInterface<ICoreWebView2_9>(&tmpWv);

  return SUCCEEDED(tmpWv->OpenDefaultDownloadDialog());
}

DLL_EXPORTS(Webview_CloseDefaultDownloadDialog, BOOL)
(ICoreWebView2* webview) {
  if (!webview) return FALSE;

  WRL::ComPtr<ICoreWebView2_9> tmpWv = nullptr;
  webview->QueryInterface<ICoreWebView2_9>(&tmpWv);

  return SUCCEEDED(tmpWv->CloseDefaultDownloadDialog());
}

DLL_EXPORTS(Webview_GetDefaultDownloadDialogMargin, BOOL)
(ICoreWebView2* webview, POINT* pt) {
  if (!webview) return FALSE;

  WRL::ComPtr<ICoreWebView2_9> tmpWv = nullptr;
  webview->QueryInterface<ICoreWebView2_9>(&tmpWv);

  return SUCCEEDED(tmpWv->get_DefaultDownloadDialogMargin(pt));
}

DLL_EXPORTS(Webview_SetDefaultDownloadDialogMargin, BOOL)
(ICoreWebView2* webview, POINT* pt) {
  if (!webview) return FALSE;

  WRL::ComPtr<ICoreWebView2_9> tmpWv = nullptr;
  webview->QueryInterface<ICoreWebView2_9>(&tmpWv);

  return SUCCEEDED(tmpWv->put_DefaultDownloadDialogMargin(*pt));
}


DLL_EXPORTS(Webview_GetProfile, BOOL)
(ICoreWebView2* webview, LPVOID* ptr) {
  if (!webview) return FALSE;

  WRL::ComPtr<ICoreWebView2_13> tmpWv = nullptr;
  webview->QueryInterface<ICoreWebView2_13>(&tmpWv);

  ICoreWebView2Profile* profile = nullptr;

  auto ret = SUCCEEDED(tmpWv->get_Profile(&profile));

  *ptr = profile;

  return ret;
}

DLL_EXPORTS(Profile_GetName, BOOL)
(ICoreWebView2Profile* profile, LPVOID* ptr, uint32_t* size) {
  if (!profile) return FALSE;

  LPWSTR value = nullptr;
  auto ret = SUCCEEDED(profile->get_ProfileName(&value));

  *ptr = value;
  *size = lstrlenW(value) * 2 + 2;

  return ret;
}

DLL_EXPORTS(Profile_GetPrivateInMode, BOOL)
(ICoreWebView2Profile* profile) {
  if (!profile) return FALSE;

  BOOL value = FALSE;
  profile->get_IsInPrivateModeEnabled(&value);

  return value;
}

DLL_EXPORTS(Profile_GetPath, BOOL)
(ICoreWebView2Profile* profile, LPVOID* ptr, uint32_t* size) {
  if (!profile) return FALSE;

  LPWSTR value = nullptr;
  auto ret = SUCCEEDED(profile->get_ProfilePath(&value));

  *ptr = value;
  *size = lstrlenW(value) * 2 + 2;

  return ret;
}

DLL_EXPORTS(Profile_GetDefaultDownloadPath, BOOL)
(ICoreWebView2Profile* profile, LPVOID* ptr, uint32_t* size) {
  if (!profile) return FALSE;

  LPWSTR value = nullptr;
  auto ret = SUCCEEDED(profile->get_DefaultDownloadFolderPath(&value));

  *ptr = value;
  *size = lstrlenW(value) * 2 + 2;

  return ret;
}

DLL_EXPORTS(Profile_SetDefaultDownloadPath, BOOL)
(ICoreWebView2Profile* profile, LPWSTR path) {
  if (!profile) return FALSE;

  return SUCCEEDED(profile->put_DefaultDownloadFolderPath(path));
}

DLL_EXPORTS(Profile_GetColorTheme, int)
(ICoreWebView2Profile* profile) {
  if (!profile) return FALSE;

  COREWEBVIEW2_PREFERRED_COLOR_SCHEME value;
  profile->get_PreferredColorScheme(&value);

  return value;
}

DLL_EXPORTS(Profile_SetColorTheme, BOOL)
(ICoreWebView2Profile* profile, COREWEBVIEW2_PREFERRED_COLOR_SCHEME value) {
  if (!profile) return FALSE;

  return SUCCEEDED(profile->put_PreferredColorScheme(value));
}

using ClearBrowsingDataCB = HRESULT(CALLBACK*)(HRESULT code, LPVOID param);
DLL_EXPORTS(Profile_ClearBrowsingData, BOOL)
(ICoreWebView2Profile* profile, COREWEBVIEW2_BROWSING_DATA_KINDS type,
 ClearBrowsingDataCB callback, LPVOID param) {
  if (!profile) return FALSE;

  WRL::ComPtr<ICoreWebView2Profile2> tmpPf = nullptr;
  profile->QueryInterface<ICoreWebView2Profile2>(&tmpPf);

  auto ret = SUCCEEDED(tmpPf->ClearBrowsingData(
      type, WRL::Callback<ICoreWebView2ClearBrowsingDataCompletedHandler>(
                [callback, param](HRESULT errorCode) -> HRESULT {
                  return callback(errorCode, param);
                })
                .Get()));

  return ret;
}

DLL_EXPORTS(Profile_ClearBrowsingData_Sync, BOOL)
(ICoreWebView2Profile* profile, COREWEBVIEW2_BROWSING_DATA_KINDS type) {
  if (!profile) return FALSE;

  WRL::ComPtr<ICoreWebView2Profile2> tmpPf = nullptr;
  profile->QueryInterface<ICoreWebView2Profile2>(&tmpPf);

  Waitable* waiter = CreateWaitable(true);

  auto ret = SUCCEEDED(tmpPf->ClearBrowsingData(
      type, WRL::Callback<ICoreWebView2ClearBrowsingDataCompletedHandler>(
                [waiter](HRESULT errorCode) -> HRESULT {
                  ActiveWaitable(waiter);
                  return S_OK;
                })
                .Get()));

  WaitOfMsgLoop(waiter);

  return ret;
}

DLL_EXPORTS(Profile_ClearBrowsingDataInTimeRange, BOOL)
(ICoreWebView2Profile* profile, COREWEBVIEW2_BROWSING_DATA_KINDS type,
 double start, double end, ClearBrowsingDataCB callback, LPVOID param) {
  if (!profile) return FALSE;

  WRL::ComPtr<ICoreWebView2Profile2> tmpPf = nullptr;
  profile->QueryInterface<ICoreWebView2Profile2>(&tmpPf);

  auto ret = SUCCEEDED(tmpPf->ClearBrowsingDataInTimeRange(
      type, start, end,
      WRL::Callback<ICoreWebView2ClearBrowsingDataCompletedHandler>(
          [callback, param](HRESULT errorCode) -> HRESULT {
            return callback(errorCode, param);
          })
          .Get()));

  return ret;
}

DLL_EXPORTS(Profile_ClearBrowsingDataInTimeRange_Sync, BOOL)
(ICoreWebView2Profile* profile, COREWEBVIEW2_BROWSING_DATA_KINDS type,
 double start, double end) {
  if (!profile) return FALSE;

  WRL::ComPtr<ICoreWebView2Profile2> tmpPf = nullptr;
  profile->QueryInterface<ICoreWebView2Profile2>(&tmpPf);

  Waitable* waiter = CreateWaitable(true);

  auto ret = SUCCEEDED(tmpPf->ClearBrowsingDataInTimeRange(
      type, start, end,
      WRL::Callback<ICoreWebView2ClearBrowsingDataCompletedHandler>(
          [waiter](HRESULT errorCode) -> HRESULT {
            ActiveWaitable(waiter);
            return S_OK;
          })
          .Get()));

  WaitOfMsgLoop(waiter);

  return ret;
}

DLL_EXPORTS(Profile_ClearAllBrowsingData, BOOL)
(ICoreWebView2Profile* profile, ClearBrowsingDataCB callback, LPVOID param) {
  if (!profile) return FALSE;

  WRL::ComPtr<ICoreWebView2Profile2> tmpPf = nullptr;
  profile->QueryInterface<ICoreWebView2Profile2>(&tmpPf);

  auto ret = SUCCEEDED(tmpPf->ClearBrowsingDataAll(
      WRL::Callback<ICoreWebView2ClearBrowsingDataCompletedHandler>(
          [callback, param](HRESULT errorCode) -> HRESULT {
            return callback(errorCode, param);
          })
          .Get()));

  return ret;
}

DLL_EXPORTS(Profile_ClearAllBrowsingData_Sync, BOOL)
(ICoreWebView2Profile* profile) {
  if (!profile) return FALSE;

  WRL::ComPtr<ICoreWebView2Profile2> tmpPf = nullptr;
  profile->QueryInterface<ICoreWebView2Profile2>(&tmpPf);

  Waitable* waiter = CreateWaitable(true);

  auto ret = SUCCEEDED(tmpPf->ClearBrowsingDataAll(
      WRL::Callback<ICoreWebView2ClearBrowsingDataCompletedHandler>(
          [waiter](HRESULT errorCode) -> HRESULT {
            ActiveWaitable(waiter);
            return S_OK;
          })
          .Get()));

  WaitOfMsgLoop(waiter);

  return ret;
}

DLL_EXPORTS(Webview_AddWebResourceRequestedFilter, BOOL)
(ICoreWebView2* webview, LPWSTR uri, COREWEBVIEW2_WEB_RESOURCE_CONTEXT type) {
  if (!webview) return FALSE;

  return SUCCEEDED(webview->AddWebResourceRequestedFilter(uri, type));
}

DLL_EXPORTS(Webview_RemoveWebResourceRequestedFilter, BOOL)
(ICoreWebView2* webview, LPWSTR uri, COREWEBVIEW2_WEB_RESOURCE_CONTEXT type) {
  if (!webview) return FALSE;

  return SUCCEEDED(webview->RemoveWebResourceRequestedFilter(uri, type));
}

using WebResourceRequestedCB = HRESULT(CALLBACK*)(LPVOID wv, LPVOID args, LPVOID param);
DLL_EXPORTS(Webview_Attach_WebResourceRequested, int64_t)
(ICoreWebView2* webview, WebResourceRequestedCB callback, LPVOID param) {
  if (!webview) return FALSE;

  EventRegistrationToken token;

  webview->add_WebResourceRequested(
      WRL::Callback<ICoreWebView2WebResourceRequestedEventHandler>(
          [callback, param](
              ICoreWebView2* sender,
              ICoreWebView2WebResourceRequestedEventArgs* args) -> HRESULT {
            sender->AddRef();
            args->AddRef();

            HRESULT hr = callback(sender, args, param);

            return hr;
          })
          .Get(),
      &token);

  return token.value;
}

DLL_EXPORTS(Webview_Detach_WebResourceRequested, BOOL)
(ICoreWebView2* webview, int64_t value) {
  if (!webview) return FALSE;
  EventRegistrationToken token = {value};

  return SUCCEEDED(webview->remove_WebResourceRequested(token));
}


using WebResourceResponseReceivedCB = HRESULT(CALLBACK*)(LPVOID wv, LPVOID request, LPVOID response,
                                                  LPVOID param);
DLL_EXPORTS(Webview_Attach_WebResourceResponseReceived, int64_t)
(ICoreWebView2* webview, WebResourceResponseReceivedCB callback, LPVOID param) {
  if (!webview) return FALSE;

  EventRegistrationToken token;

  WRL::ComPtr<ICoreWebView2_2> tmpWv = nullptr;
  webview->QueryInterface<ICoreWebView2_2>(&tmpWv);

  tmpWv->add_WebResourceResponseReceived(
      WRL::Callback<ICoreWebView2WebResourceResponseReceivedEventHandler>(
          [callback, param](ICoreWebView2* sender,
                            ICoreWebView2WebResourceResponseReceivedEventArgs*
                                args) -> HRESULT {
            sender->AddRef();

            ICoreWebView2WebResourceRequest* request = nullptr;
            ICoreWebView2WebResourceResponseView* response = nullptr;
            args->get_Request(&request);
            args->get_Response(&response);

            HRESULT hr = callback(sender, request, response, param);

            return hr;
          })
          .Get(),
      &token);

  return token.value;
}

DLL_EXPORTS(Webview_Detach_WebResourceResponseReceived, BOOL)
(ICoreWebView2* webview, int64_t value) {
  if (!webview) return FALSE;
  EventRegistrationToken token = {value};

  WRL::ComPtr<ICoreWebView2_2> tmpWv = nullptr;
  webview->QueryInterface<ICoreWebView2_2>(&tmpWv);

  return SUCCEEDED(tmpWv->remove_WebResourceResponseReceived(token));
}


DLL_EXPORTS(Webview_SetMute, BOOL)
(ICoreWebView2* webview, BOOL mute) {
  if (!webview) return FALSE;

  WRL::ComPtr<ICoreWebView2_8> tmpWv = nullptr;
  webview->QueryInterface<ICoreWebView2_8>(&tmpWv);

  return SUCCEEDED(tmpWv->put_IsMuted(mute));
}

DLL_EXPORTS(Webview_GetMute, BOOL)
(ICoreWebView2* webview) {
  if (!webview) return FALSE;

  WRL::ComPtr<ICoreWebView2_8> tmpWv = nullptr;
  webview->QueryInterface<ICoreWebView2_8>(&tmpWv);

  BOOL mute = FALSE;

  tmpWv->get_IsMuted(&mute);

  return mute;
}


DLL_EXPORTS(Webview_GetAudioPlaying, BOOL)
(ICoreWebView2* webview) {
  if (!webview) return FALSE;

  WRL::ComPtr<ICoreWebView2_8> tmpWv = nullptr;
  webview->QueryInterface<ICoreWebView2_8>(&tmpWv);

  BOOL ret = FALSE;
  tmpWv->get_IsDocumentPlayingAudio(&ret);

  return ret;
}

using IsDocumentPlayingAudioChangedCB = HRESULT(CALLBACK*)(LPVOID webview, BOOL audiable, LPVOID param);
DLL_EXPORTS(Webview_Attach_IsDocumentPlayingAudioChanged, int64_t)
(ICoreWebView2* webview, IsDocumentPlayingAudioChangedCB callback,
 LPVOID param) {
  if (!webview) return FALSE;

  EventRegistrationToken token;

  WRL::ComPtr<ICoreWebView2_8> tmpWv = nullptr;
  webview->QueryInterface<ICoreWebView2_8>(&tmpWv);

  tmpWv->add_IsDocumentPlayingAudioChanged(
      WRL::Callback<ICoreWebView2IsDocumentPlayingAudioChangedEventHandler>(
          [callback, param](ICoreWebView2* sender, IUnknown* args) -> HRESULT {
            sender->AddRef();

            WRL::ComPtr<ICoreWebView2_8> tmpWv_ = nullptr;
            sender->QueryInterface<ICoreWebView2_8>(&tmpWv_);

            BOOL audible = FALSE;
            tmpWv_->get_IsDocumentPlayingAudio(&audible);

            HRESULT hr = callback(sender, audible, param);

            return hr;
          })
          .Get(),
      &token);

  return token.value;
}

DLL_EXPORTS(Webview_Detach_IsDocumentPlayingAudioChanged, BOOL)
(ICoreWebView2* webview, int64_t value) {
  if (!webview) return FALSE;
  EventRegistrationToken token = {value};

  WRL::ComPtr<ICoreWebView2_8> tmpWv = nullptr;
  webview->QueryInterface<ICoreWebView2_8>(&tmpWv);

  return SUCCEEDED(tmpWv->remove_IsDocumentPlayingAudioChanged(token));
}

using PreloadScriptCB = HRESULT(CALLBACK*)(LPVOID ptr, uint32_t size, LPVOID param);
DLL_EXPORTS(Webview_AddPreloadScript, BOOL)
(ICoreWebView2* webview, LPWSTR script, PreloadScriptCB callback,
 LPVOID param) {
  if (!webview) return FALSE;

  HRESULT hr = webview->AddScriptToExecuteOnDocumentCreated(
      script,
      WRL::Callback<
          ICoreWebView2AddScriptToExecuteOnDocumentCreatedCompletedHandler>(
          [callback, param](HRESULT errorCode, LPCWSTR id) -> HRESULT {
            if (callback) {
              uint32_t sizeTmp = lstrlenW(id) * 2 + 2;
              LPWSTR newStr = static_cast<LPWSTR>(wv2_Utility_Malloc(sizeTmp));
              if (newStr) {
                newStr[sizeTmp - 1] = 0;
                lstrcpyW(newStr, id);
              }
              return callback(newStr, sizeTmp, param);
            }
            return S_OK;
          })
          .Get());

  return SUCCEEDED(hr);
}

DLL_EXPORTS(Webview_AddPreloadScript_Sync, BOOL)
(ICoreWebView2* webview, LPWSTR script, LPVOID* ptr, uint32_t* size) {
  if (!webview) return FALSE;

  Waitable* waiter = CreateWaitable(true);

  HRESULT hr = webview->AddScriptToExecuteOnDocumentCreated(
      script,
      WRL::Callback<
          ICoreWebView2AddScriptToExecuteOnDocumentCreatedCompletedHandler>(
          [waiter, ptr, size](HRESULT errorCode, LPCWSTR id) -> HRESULT {
            uint32_t sizeTmp = lstrlenW(id) * 2 + 2;
            LPWSTR newStr = static_cast<LPWSTR>(wv2_Utility_Malloc(sizeTmp));
            if (newStr) {
              newStr[sizeTmp - 1] = 0;
              lstrcpyW(newStr, id);
            }

            *ptr = newStr;
            *size = sizeTmp;

            ActiveWaitable(waiter);

            return S_OK;
          })
          .Get());

  WaitOfMsgLoop(waiter);

  return SUCCEEDED(hr);
}

DLL_EXPORTS(Webview_RemovePreloadScript, BOOL)
(ICoreWebView2* webview, LPWSTR id) {
  if (!webview) return FALSE;

  return SUCCEEDED(webview->RemoveScriptToExecuteOnDocumentCreated(id));
}

DLL_EXPORTS(Webview_PostWebMessageAsString, BOOL)
(ICoreWebView2* webview, LPWSTR str) {
  if (!webview) return FALSE;

  return SUCCEEDED(webview->PostWebMessageAsString(str));
}

DLL_EXPORTS(Webview_PostWebMessageAsJSON, BOOL)
(ICoreWebView2* webview, LPWSTR str) {
  if (!webview) return FALSE;

  return SUCCEEDED(webview->PostWebMessageAsJson(str));
}

using ScriptDialogOpeningCB = HRESULT(CALLBACK*)(LPVOID webview, LPVOID args, LPVOID param);
DLL_EXPORTS(Webview_Attach_ScriptDialogOpening, int64_t)
(ICoreWebView2* webview, ScriptDialogOpeningCB callback, LPVOID param) {
  if (!webview) return FALSE;

  EventRegistrationToken token;

  webview->add_ScriptDialogOpening(
      WRL::Callback<ICoreWebView2ScriptDialogOpeningEventHandler>(
          [callback, param](
              ICoreWebView2* sender,
              ICoreWebView2ScriptDialogOpeningEventArgs* args) -> HRESULT {
            sender->AddRef();
            args->AddRef();

            HRESULT hr = callback(sender, args, param);

            return hr;
          })
          .Get(),
      &token);

  return token.value;
}

DLL_EXPORTS(Webview_Detach_ScriptDialogOpening, BOOL)
(ICoreWebView2* webview, int64_t value) {
  if (!webview) return FALSE;
  EventRegistrationToken token = {value};

  return SUCCEEDED(webview->remove_ScriptDialogOpening(token));
}

DLL_EXPORTS(Webview_ScriptDialogOpeningArgs_GetURL, BOOL)
(ICoreWebView2ScriptDialogOpeningEventArgs* args, LPVOID* ptr, uint32_t* size) {
  if (!args) return FALSE;

  LPWSTR uri = nullptr;
  HRESULT hr = args->get_Uri(&uri);

  *ptr = uri;
  *size = lstrlenW(uri) * 2 + 2;

  return SUCCEEDED(hr);
}

DLL_EXPORTS(Webview_ScriptDialogOpeningArgs_GetMessage, BOOL)
(ICoreWebView2ScriptDialogOpeningEventArgs* args, LPVOID* ptr, uint32_t* size) {
  if (!args) return FALSE;

  LPWSTR uri = nullptr;
  HRESULT hr = args->get_Message(&uri);

  *ptr = uri;
  *size = lstrlenW(uri) * 2 + 2;

  return SUCCEEDED(hr);
}

DLL_EXPORTS(Webview_ScriptDialogOpeningArgs_GetDefaultText, BOOL)
(ICoreWebView2ScriptDialogOpeningEventArgs* args, LPVOID* ptr, uint32_t* size) {
  if (!args) return FALSE;

  LPWSTR uri = nullptr;
  HRESULT hr = args->get_DefaultText(&uri);

  *ptr = uri;
  *size = lstrlenW(uri) * 2 + 2;

  return SUCCEEDED(hr);
}

DLL_EXPORTS(Webview_ScriptDialogOpeningArgs_GetResultText, BOOL)
(ICoreWebView2ScriptDialogOpeningEventArgs* args, LPVOID* ptr, uint32_t* size) {
  if (!args) return FALSE;

  LPWSTR uri = nullptr;
  HRESULT hr = args->get_ResultText(&uri);

  *ptr = uri;
  *size = lstrlenW(uri) * 2 + 2;

  return SUCCEEDED(hr);
}

DLL_EXPORTS(Webview_ScriptDialogOpeningArgs_GetType, int)
(ICoreWebView2ScriptDialogOpeningEventArgs* args) {
  if (!args) return FALSE;

  COREWEBVIEW2_SCRIPT_DIALOG_KIND type;
  args->get_Kind(&type);

  return type;
}

DLL_EXPORTS(Webview_ScriptDialogOpeningArgs_Accept, BOOL)
(ICoreWebView2ScriptDialogOpeningEventArgs* args) {
  if (!args) return FALSE;

  return SUCCEEDED(args->Accept());
}

DLL_EXPORTS(Webview_ScriptDialogOpeningArgs_SetResultText, BOOL)
(ICoreWebView2ScriptDialogOpeningEventArgs* args, LPWSTR text) {
  if (!args) return FALSE;

  return SUCCEEDED(args->put_ResultText(text));
}

DLL_EXPORTS(Webview_ScriptDialogOpeningArgs_GetDeferral, BOOL)
(ICoreWebView2ScriptDialogOpeningEventArgs* args, LPVOID* ptr) {
  if (!args) return FALSE;

  ICoreWebView2Deferral* deferral = nullptr;
  HRESULT hr = args->GetDeferral(&deferral);

  *ptr = deferral;

  return SUCCEEDED(hr);
}


using FaviconChangedCB = HRESULT(CALLBACK*)(LPVOID webview, LPVOID param);
DLL_EXPORTS(Webview_Attach_FaviconChanged, int64_t)
(ICoreWebView2* webview, FaviconChangedCB callback, LPVOID param) {
  if (!webview) return FALSE;

  EventRegistrationToken token;

  WRL::ComPtr<ICoreWebView2_15> tmpWv = nullptr;
  webview->QueryInterface<ICoreWebView2_15>(&tmpWv);

  tmpWv->add_FaviconChanged(
      WRL::Callback<ICoreWebView2FaviconChangedEventHandler>(
          [callback, param](
              ICoreWebView2* sender,
              IUnknown* args) -> HRESULT {
            sender->AddRef();

            HRESULT hr = callback(sender, param);

            return hr;
          })
          .Get(),
      &token);

  return token.value;
}

DLL_EXPORTS(Webview_Detach_FaviconChanged, BOOL)
(ICoreWebView2* webview, int64_t value) {
  if (!webview) return FALSE;
  EventRegistrationToken token = {value};

  WRL::ComPtr<ICoreWebView2_15> tmpWv = nullptr;
  webview->QueryInterface<ICoreWebView2_15>(&tmpWv);

  return SUCCEEDED(tmpWv->remove_FaviconChanged(token));
}

DLL_EXPORTS(Webview_GetFaviconURL, BOOL)
(ICoreWebView2* webview, LPVOID* ptr, uint32_t* size) {
  if (!webview) return FALSE;

  WRL::ComPtr<ICoreWebView2_15> tmpWv = nullptr;
  webview->QueryInterface<ICoreWebView2_15>(&tmpWv);

  LPWSTR value = nullptr;
  HRESULT hr = tmpWv->get_FaviconUri(&value);
  
  *ptr = value;
  *size = lstrlenW(value) * 2 + 2;

  return SUCCEEDED(hr);
}


using BasicAuthenticationRequestedCB = HRESULT(CALLBACK*)(LPVOID webview, LPVOID args, LPVOID param);
DLL_EXPORTS(Webview_Attach_BasicAuthenticationRequested, int64_t)
(ICoreWebView2* webview, BasicAuthenticationRequestedCB callback,
 LPVOID param) {
  if (!webview) return FALSE;

  EventRegistrationToken token;

  WRL::ComPtr<ICoreWebView2_15> tmpWv = nullptr;
  webview->QueryInterface<ICoreWebView2_15>(&tmpWv);

  tmpWv->add_BasicAuthenticationRequested(
      WRL::Callback<ICoreWebView2BasicAuthenticationRequestedEventHandler>(
          [callback, param](ICoreWebView2* sender,
                            ICoreWebView2BasicAuthenticationRequestedEventArgs*
                                args) -> HRESULT {
            sender->AddRef();
            args->AddRef();

            HRESULT hr = callback(sender, args, param);

            return hr;
          })
          .Get(),
      &token);

  return token.value;
}

DLL_EXPORTS(Webview_Detach_BasicAuthenticationRequested, BOOL)
(ICoreWebView2* webview, int64_t value) {
  if (!webview) return FALSE;
  EventRegistrationToken token = {value};

  WRL::ComPtr<ICoreWebView2_15> tmpWv = nullptr;
  webview->QueryInterface<ICoreWebView2_15>(&tmpWv);

  return SUCCEEDED(tmpWv->remove_BasicAuthenticationRequested(token));
}


DLL_EXPORTS(Webview_BasicAuthenticationRequestedArgs_GetDeferral, BOOL)
(ICoreWebView2BasicAuthenticationRequestedEventArgs* args, LPVOID* ptr) {
  if (!args) return FALSE;
  
  ICoreWebView2Deferral* deferral = nullptr;
  HRESULT hr = args->GetDeferral(&deferral);

  *ptr = deferral;

  return SUCCEEDED(hr);
}

DLL_EXPORTS(Webview_BasicAuthenticationRequestedArgs_GetURL, BOOL)
(ICoreWebView2BasicAuthenticationRequestedEventArgs* args, LPVOID* ptr, uint32_t *size) {
  if (!args) return FALSE;

  LPWSTR value = nullptr;
  HRESULT hr = args->get_Uri(&value);

  *ptr = value;
  *size = lstrlenW(value) * 2 + 2;

  return SUCCEEDED(hr);
}

DLL_EXPORTS(Webview_BasicAuthenticationRequestedArgs_GetChallenge, BOOL)
(ICoreWebView2BasicAuthenticationRequestedEventArgs* args, LPVOID* ptr, uint32_t *size) {
  if (!args) return FALSE;

  LPWSTR value = nullptr;
  HRESULT hr = args->get_Challenge(&value);

  *ptr = value;
  *size = lstrlenW(value) * 2 + 2;

  return SUCCEEDED(hr);
}

DLL_EXPORTS(Webview_BasicAuthenticationRequestedArgs_GetCancel, BOOL)
(ICoreWebView2BasicAuthenticationRequestedEventArgs* args) {
  if (!args) return FALSE;

  BOOL value = FALSE;
  args->get_Cancel(&value);

  return value;
}

DLL_EXPORTS(Webview_BasicAuthenticationRequestedArgs_SetCancel, BOOL)
(ICoreWebView2BasicAuthenticationRequestedEventArgs* args, BOOL value) {
  if (!args) return FALSE;

  return SUCCEEDED(args->put_Cancel(value));
}

DLL_EXPORTS(Webview_BasicAuthenticationRequestedArgs_UserName_Attr, BOOL)
(ICoreWebView2BasicAuthenticationRequestedEventArgs* args, LPVOID* ptr,
 uint32_t* size) {
  if (!args) return FALSE;

  ICoreWebView2BasicAuthenticationResponse* response = nullptr;
  HRESULT hr = args->get_Response(&response);

  if (ptr && size) {
    LPWSTR value = nullptr;
    response->get_UserName(&value);

    *ptr = value;
    *size = lstrlenW(value) * 2 + 2;
  } else {
    response->put_UserName(reinterpret_cast<LPWSTR>(ptr));
  }

  return SUCCEEDED(hr);
}

DLL_EXPORTS(Webview_BasicAuthenticationRequestedArgs_Password_Attr, BOOL)
(ICoreWebView2BasicAuthenticationRequestedEventArgs* args, LPVOID* ptr,
 uint32_t* size) {
  if (!args) return FALSE;

  ICoreWebView2BasicAuthenticationResponse* response = nullptr;
  HRESULT hr = args->get_Response(&response);

  if (ptr && size) {
    LPWSTR value = nullptr;
    response->get_Password(&value);

    *ptr = value;
    *size = lstrlenW(value) * 2 + 2;
  } else {
    response->put_Password(reinterpret_cast<LPWSTR>(ptr));
  }

  return SUCCEEDED(hr);
}


using FrameCreatedCB = HRESULT(CALLBACK*)(LPVOID webview, LPVOID frame, LPVOID param);
DLL_EXPORTS(Webview_Attach_FrameCreated, int64_t)
(ICoreWebView2* webview, FrameCreatedCB callback, LPVOID param) {
  if (!webview) return FALSE;

  EventRegistrationToken token;

  WRL::ComPtr<ICoreWebView2_15> tmpWv = nullptr;
  webview->QueryInterface<ICoreWebView2_15>(&tmpWv);

  tmpWv->add_FrameCreated(
      WRL::Callback<ICoreWebView2FrameCreatedEventHandler>(
          [callback, param](
              ICoreWebView2* sender,
              ICoreWebView2FrameCreatedEventArgs* args) -> HRESULT {
            sender->AddRef();

            ICoreWebView2Frame* frame = nullptr;
            args->get_Frame(&frame);

            HRESULT hr = callback(sender, frame, param);

            return hr;
          })
          .Get(),
      &token);

  return token.value;
}

DLL_EXPORTS(Webview_Detach_FrameCreated, BOOL)
(ICoreWebView2* webview, int64_t value) {
  if (!webview) return FALSE;
  EventRegistrationToken token = {value};

  WRL::ComPtr<ICoreWebView2_15> tmpWv = nullptr;
  webview->QueryInterface<ICoreWebView2_15>(&tmpWv);

  return SUCCEEDED(tmpWv->remove_FrameCreated(token));
}


DLL_EXPORTS(Webview_NavigateRequest, BOOL)
(ICoreWebView2_15* webview, ICoreWebView2WebResourceRequest *obj) {
  if (!webview) return FALSE;
  
  HRESULT hr = webview->NavigateWithWebResourceRequest(obj);

  return SUCCEEDED(hr);
}