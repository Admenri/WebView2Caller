
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
                    return callback(errorCode, resultObjectAsJson,
                                    lstrlenW(resultObjectAsJson) * 2 + 2,
                                    param);
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
                    *ptr = resultObjectAsJson;
                    *size = lstrlenW(resultObjectAsJson) * 2 + 2;
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
            uint8_t* buf = new uint8_t[size];
            ULONG dummy = 0;
            is->Read(buf, size, &dummy);

            HRESULT hr = callback(buf, size, param);

            delete[] buf;

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


DLL_EXPORTS(Webview_HTTPRequestHeader_GetHeader, BOOL)
(ICoreWebView2HttpRequestHeaders* header, LPWSTR name, LPVOID* ptr,
 uint32_t* size) {
  if (!header) return FALSE;

  LPWSTR value = nullptr;
  auto ret = SUCCEEDED(header->GetHeader(name, &value));

  *ptr = value;
  *size = lstrlenW(value) * 2 + 2;

  return ret;
}

DLL_EXPORTS(Webview_HTTPRequestHeader_Contains, BOOL)
(ICoreWebView2HttpRequestHeaders* header, LPWSTR name) {
  if (!header) return FALSE;

  BOOL ret = FALSE;
  header->Contains(name, &ret);

  return ret;
}

DLL_EXPORTS(Webview_HTTPRequestHeader_SetHeader, BOOL)
(ICoreWebView2HttpRequestHeaders* header, LPWSTR name, LPWSTR value) {
  if (!header) return FALSE;

  return SUCCEEDED(header->SetHeader(name, value));
}

DLL_EXPORTS(Webview_HTTPRequestHeader_RemoveHeader, BOOL)
(ICoreWebView2HttpRequestHeaders* header, LPWSTR name) {
  if (!header) return FALSE;

  return SUCCEEDED(header->RemoveHeader(name));
}

DLL_EXPORTS(Webview_HTTPRequestHeader_GetRawData, BOOL)
(ICoreWebView2HttpRequestHeaders* header, LPVOID* arrayPtr, uint32_t* size) {
  if (!header) return FALSE;

  ICoreWebView2HttpHeadersCollectionIterator* iter = nullptr;
  auto ret = SUCCEEDED(header->GetIterator(&iter));
  if (!iter) return FALSE;

  std::vector<std::pair<LPWSTR, LPWSTR>> data;

  *size = 0;
  BOOL hasData = FALSE;
  while (SUCCEEDED(iter->get_HasCurrentHeader(&hasData)) && hasData) {
    ++(*size);

    std::pair<LPWSTR ,LPWSTR> pair;
    iter->GetCurrentHeader(&pair.first, &pair.second);
    data.push_back(std::move(pair));

    iter->MoveNext(&hasData);
  }

  LPSTR buf = static_cast<LPSTR>(wv2_Utility_Malloc((*size) * 2 * sizeof(void*)));
  for (size_t i = 0; i < data.size(); i++) {
    memcpy(buf + i * 2 * sizeof(void*), &data[i].first, sizeof(void*));
    memcpy(buf + i * 2 * sizeof(void*) + sizeof(void*), &data[i].second, sizeof(void*));
  }

  *arrayPtr = buf;

  return ret;
}

DLL_EXPORTS(Webview_HTTPRequestHeader_GetHeaders, BOOL)
(ICoreWebView2HttpRequestHeaders* header, LPWSTR name, LPVOID* arrayPtr, uint32_t* size) {
  if (!header) return FALSE;

  ICoreWebView2HttpHeadersCollectionIterator* iter = nullptr;
  auto ret = SUCCEEDED(header->GetHeaders(name, &iter));
  if (!iter) return FALSE;

  std::vector<std::pair<LPWSTR, LPWSTR>> data;

  *size = 0;
  BOOL hasData = FALSE;
  while (SUCCEEDED(iter->get_HasCurrentHeader(&hasData)) && hasData) {
    ++(*size);

    std::pair<LPWSTR, LPWSTR> pair;
    iter->GetCurrentHeader(&pair.first, &pair.second);
    data.push_back(std::move(pair));

    iter->MoveNext(&hasData);
  }

  LPSTR buf =
      static_cast<LPSTR>(wv2_Utility_Malloc((*size) * 2 * sizeof(void*)));
  for (size_t i = 0; i < data.size(); i++) {
    memcpy(buf + i * 2 * sizeof(void*), &data[i].first, sizeof(void*));
    memcpy(buf + i * 2 * sizeof(void*) + sizeof(void*), &data[i].second,
           sizeof(void*));
  }

  *arrayPtr = buf;

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
