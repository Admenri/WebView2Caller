
#include "wv2_download.h"

using namespace Microsoft;

DLL_EXPORTS(DownloadArgs_GetDeferral, BOOL)
(ICoreWebView2DownloadStartingEventArgs* args, LPVOID* ptr) {
  if (!args) return FALSE;

  ICoreWebView2Deferral* deferral = nullptr;
  HRESULT hr = args->GetDeferral(&deferral);

  *ptr = deferral;

  return SUCCEEDED(hr);
}

DLL_EXPORTS(DownloadArgs_Cancel_Attr, BOOL)
(ICoreWebView2DownloadStartingEventArgs* args, BOOL get, BOOL value) {
  if (!args) return FALSE;

  if (get) {
    BOOL result = FALSE;
    args->get_Cancel(&result);
    return result;
  } else {
    return SUCCEEDED(args->put_Cancel(value));
  }
}

DLL_EXPORTS(DownloadArgs_Handled_Attr, BOOL)
(ICoreWebView2DownloadStartingEventArgs* args, BOOL get, BOOL value) {
  if (!args) return FALSE;

  if (get) {
    BOOL result = FALSE;
    args->get_Handled(&result);
    return result;
  } else {
    return SUCCEEDED(args->put_Handled(value));
  }
}

DLL_EXPORTS(DownloadArgs_ResultPath_Attr, BOOL)
(ICoreWebView2DownloadStartingEventArgs* args, BOOL get, LPVOID* ptr, uint32_t* size) {
  if (!args) return FALSE;

  if (get) {
    LPWSTR result = FALSE;
    HRESULT hr = args->get_ResultFilePath(&result);
    *ptr = result;
    *size = lstrlenW(result) * 2 + 2;
    return SUCCEEDED(hr);
  } else {
    return SUCCEEDED(args->put_ResultFilePath(reinterpret_cast<LPWSTR>(ptr)));
  }
}

DLL_EXPORTS(DownloadArgs_GetOperation, BOOL)
(ICoreWebView2DownloadStartingEventArgs* args, LPVOID* ptr) {
  if (!args) return FALSE;

  ICoreWebView2DownloadOperation* p = nullptr;
  HRESULT hr = args->get_DownloadOperation(&p);

  *ptr = p;

  return SUCCEEDED(hr);
}



DLL_EXPORTS(DownloadOperation_GetURL, BOOL)
(ICoreWebView2DownloadOperation* obj, LPVOID* ptr, uint32_t *size) {
  if (!obj) return FALSE;

  LPWSTR uri = nullptr;
  HRESULT hr = obj->get_Uri(&uri);

  *ptr = uri;
  *size = lstrlenW(uri) * 2 + 2;

  return SUCCEEDED(hr);
}

DLL_EXPORTS(DownloadOperation_GetContentDisposition, BOOL)
(ICoreWebView2DownloadOperation* obj, LPVOID* ptr, uint32_t* size) {
  if (!obj) return FALSE;

  LPWSTR uri = nullptr;
  HRESULT hr = obj->get_ContentDisposition(&uri);

  *ptr = uri;
  *size = lstrlenW(uri) * 2 + 2;

  return SUCCEEDED(hr);
}

DLL_EXPORTS(DownloadOperation_GetMimeType, BOOL)
(ICoreWebView2DownloadOperation* obj, LPVOID* ptr, uint32_t* size) {
  if (!obj) return FALSE;

  LPWSTR uri = nullptr;
  HRESULT hr = obj->get_MimeType(&uri);

  *ptr = uri;
  *size = lstrlenW(uri) * 2 + 2;

  return SUCCEEDED(hr);
}

DLL_EXPORTS(DownloadOperation_GetEstimatedEndTime, BOOL)
(ICoreWebView2DownloadOperation* obj, LPVOID* ptr, uint32_t* size) {
  if (!obj) return FALSE;

  LPWSTR uri = nullptr;
  HRESULT hr = obj->get_EstimatedEndTime(&uri);

  *ptr = uri;
  *size = lstrlenW(uri) * 2 + 2;

  return SUCCEEDED(hr);
}

DLL_EXPORTS(DownloadOperation_GetResultPath, BOOL)
(ICoreWebView2DownloadOperation* obj, LPVOID* ptr, uint32_t* size) {
  if (!obj) return FALSE;

  LPWSTR uri = nullptr;
  HRESULT hr = obj->get_ResultFilePath(&uri);

  *ptr = uri;
  *size = lstrlenW(uri) * 2 + 2;

  return SUCCEEDED(hr);
}

DLL_EXPORTS(DownloadOperation_GetTotalBytes, BOOL)
(ICoreWebView2DownloadOperation* obj, int64_t *value) {
  if (!obj) return FALSE;
  
  HRESULT hr = obj->get_TotalBytesToReceive(value);

  return SUCCEEDED(hr);
}

DLL_EXPORTS(DownloadOperation_GetReceivedBytes, BOOL)
(ICoreWebView2DownloadOperation* obj, int64_t* value) {
  if (!obj) return FALSE;

  HRESULT hr = obj->get_BytesReceived(value);

  return SUCCEEDED(hr);
}

DLL_EXPORTS(DownloadOperation_GetState, int)
(ICoreWebView2DownloadOperation* obj) {
  if (!obj) return FALSE;

  COREWEBVIEW2_DOWNLOAD_STATE value;
  obj->get_State(&value);

  return value;
}

DLL_EXPORTS(DownloadOperation_GetInterruptReason, int)
(ICoreWebView2DownloadOperation* obj) {
  if (!obj) return FALSE;

  COREWEBVIEW2_DOWNLOAD_INTERRUPT_REASON value;
  obj->get_InterruptReason(&value);

  return value;
}

DLL_EXPORTS(DownloadOperation_GetCanResume, BOOL)
(ICoreWebView2DownloadOperation* obj) {
  if (!obj) return FALSE;
  
  BOOL value = FALSE;
  obj->get_CanResume(&value);

  return value;
}

DLL_EXPORTS(DownloadOperation_Cancel, BOOL)
(ICoreWebView2DownloadOperation* obj) {
  if (!obj) return FALSE;

  return SUCCEEDED(obj->Cancel());
}

DLL_EXPORTS(DownloadOperation_Pause, BOOL)
(ICoreWebView2DownloadOperation* obj) {
  if (!obj) return FALSE;

  return SUCCEEDED(obj->Pause());
}

DLL_EXPORTS(DownloadOperation_Resume, BOOL)
(ICoreWebView2DownloadOperation* obj) {
  if (!obj) return FALSE;

  return SUCCEEDED(obj->Resume());
}

using BytesReceivedChangedCB = HRESULT(CALLBACK*)(LPVOID operation, LPVOID param);
DLL_EXPORTS(DownloadOperation_Attach_BytesReceivedChanged, int64_t)
(ICoreWebView2DownloadOperation* webview, BytesReceivedChangedCB callback,
 LPVOID param) {
  if (!webview) return FALSE;

  EventRegistrationToken token;

  webview->add_BytesReceivedChanged(
      WRL::Callback<ICoreWebView2BytesReceivedChangedEventHandler>(
          [callback, param](ICoreWebView2DownloadOperation* sender,
                            IUnknown* args) -> HRESULT {
            sender->AddRef();

            HRESULT hr = callback(sender, param);

            return hr;
          })
          .Get(),
      &token);

  return token.value;
}

DLL_EXPORTS(DownloadOperation_Detach_BytesReceivedChanged, BOOL)
(ICoreWebView2DownloadOperation* webview, int64_t value) {
  if (!webview) return FALSE;
  EventRegistrationToken token = {value};

  return SUCCEEDED(webview->remove_BytesReceivedChanged(token));
}

using EstimatedEndTimeChangedCB = HRESULT(CALLBACK*)(LPVOID operation,
                                                  LPVOID param);
DLL_EXPORTS(DownloadOperation_Attach_EstimatedEndTimeChanged, int64_t)
(ICoreWebView2DownloadOperation* webview, EstimatedEndTimeChangedCB callback,
 LPVOID param) {
  if (!webview) return FALSE;

  EventRegistrationToken token;

  webview->add_EstimatedEndTimeChanged(
      WRL::Callback<ICoreWebView2EstimatedEndTimeChangedEventHandler>(
          [callback, param](ICoreWebView2DownloadOperation* sender,
                            IUnknown* args) -> HRESULT {
            sender->AddRef();

            HRESULT hr = callback(sender, param);

            return hr;
          })
          .Get(),
      &token);

  return token.value;
}

DLL_EXPORTS(DownloadOperation_Detach_EstimatedEndTimeChanged, BOOL)
(ICoreWebView2DownloadOperation* webview, int64_t value) {
  if (!webview) return FALSE;
  EventRegistrationToken token = {value};

  return SUCCEEDED(webview->remove_EstimatedEndTimeChanged(token));
}

using StateChangedCB = HRESULT(CALLBACK*)(LPVOID operation,
                                                     LPVOID param);
DLL_EXPORTS(DownloadOperation_Attach_StateChanged, int64_t)
(ICoreWebView2DownloadOperation* webview, StateChangedCB callback,
 LPVOID param) {
  if (!webview) return FALSE;

  EventRegistrationToken token;

  webview->add_StateChanged(
      WRL::Callback<ICoreWebView2StateChangedEventHandler>(
          [callback, param](ICoreWebView2DownloadOperation* sender,
                            IUnknown* args) -> HRESULT {
            sender->AddRef();

            HRESULT hr = callback(sender, param);

            return hr;
          })
          .Get(),
      &token);

  return token.value;
}

DLL_EXPORTS(DownloadOperation_Detach_StateChanged, BOOL)
(ICoreWebView2DownloadOperation* webview, int64_t value) {
  if (!webview) return FALSE;
  EventRegistrationToken token = {value};

  return SUCCEEDED(webview->remove_StateChanged(token));
}
