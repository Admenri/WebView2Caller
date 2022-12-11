#include "wv2_webrequest.h"

#include <string>
#include <vector>

using namespace Microsoft;

DLL_EXPORTS(ResourceRequestArgs_GetDeferral, BOOL)
(ICoreWebView2WebResourceRequestedEventArgs* args, LPVOID* ptr) {
  if (!args) return FALSE;
  
  ICoreWebView2Deferral* deferral = nullptr;
  auto ret = SUCCEEDED(args->GetDeferral(&deferral));

  *ptr = deferral;

  return ret;
}

DLL_EXPORTS(ResourceRequestArgs_GetResourceContext, int)
(ICoreWebView2WebResourceRequestedEventArgs* args) {
  if (!args) return FALSE;

  COREWEBVIEW2_WEB_RESOURCE_CONTEXT type;
  args->get_ResourceContext(&type);
  
  return type;
}

DLL_EXPORTS(ResourceRequestArgs_GetRequest, BOOL)
(ICoreWebView2WebResourceRequestedEventArgs* args, LPVOID* ptr) {
  if (!args) return FALSE;

  ICoreWebView2WebResourceRequest* request = nullptr;
  auto ret = SUCCEEDED(args->get_Request(&request));

  *ptr = request;

  return ret;
}

DLL_EXPORTS(ResourceRequestArgs_GetResponse, BOOL)
(ICoreWebView2WebResourceRequestedEventArgs* args, LPVOID* ptr) {
  if (!args) return FALSE;

  ICoreWebView2WebResourceResponse* response = nullptr;
  auto ret = SUCCEEDED(args->get_Response(&response));

  *ptr = response;

  return ret;
}

DLL_EXPORTS(ResourceRequestArgs_SetResponse, BOOL)
(ICoreWebView2WebResourceRequestedEventArgs* args,
 ICoreWebView2WebResourceResponse* ptr) {
  if (!args) return FALSE;

  return SUCCEEDED(args->put_Response(ptr));
}



DLL_EXPORTS(ResourceRequest_GetURL, BOOL)
(ICoreWebView2WebResourceRequest* obj, LPVOID* ptr, uint32_t *size) {
  if (!obj) return FALSE;

  LPWSTR value = nullptr;

  auto ret = SUCCEEDED(obj->get_Uri(&value));

  *ptr = value;
  *size = lstrlenW(value) * 2 + 2;

  return ret;
}

DLL_EXPORTS(ResourceRequest_SetURL, BOOL)
(ICoreWebView2WebResourceRequest* obj, LPWSTR uri) {
  if (!obj) return FALSE;

  return SUCCEEDED(obj->put_Uri(uri));
}

DLL_EXPORTS(ResourceRequest_GetMethod, BOOL)
(ICoreWebView2WebResourceRequest* obj, LPVOID* ptr, uint32_t* size) {
  if (!obj) return FALSE;

  LPWSTR value = nullptr;

  auto ret = SUCCEEDED(obj->get_Method(&value));

  *ptr = value;
  *size = lstrlenW(value) * 2 + 2;

  return ret;
}

DLL_EXPORTS(ResourceRequest_SetMethod, BOOL)
(ICoreWebView2WebResourceRequest* obj, LPWSTR m) {
  if (!obj) return FALSE;

  return SUCCEEDED(obj->put_Method(m));
}

DLL_EXPORTS(ResourceRequest_GetData, BOOL)
(ICoreWebView2WebResourceRequest* obj, LPVOID* pptr, uint32_t* psize) {
  if (!obj) return FALSE;

  WRL::ComPtr<IStream> is = nullptr;
  auto ret = SUCCEEDED(obj->get_Content(&is));

  if (!is) return FALSE;

  STATSTG stat;
  is->Stat(&stat, STATFLAG_NONAME);

  LARGE_INTEGER linfo;
  linfo.QuadPart = 0;
  is->Seek(linfo, STREAM_SEEK_SET, NULL);

  uint32_t size = stat.cbSize.LowPart;
  uint8_t* buf = static_cast<uint8_t*>(wv2_Utility_Malloc(size));
  ULONG dummy = 0;
  is->Read(buf, size, &dummy);

  *pptr = buf;
  *psize = size;

  return ret;
}

DLL_EXPORTS(ResourceRequest_SetData, BOOL)
(ICoreWebView2WebResourceRequest* obj, LPVOID pptr, uint32_t psize) {
  if (!obj) return FALSE;

  WRL::ComPtr<IStream> is = nullptr;
  HGLOBAL hMem = GlobalAlloc(GMEM_ZEROINIT, psize);
  if (!hMem) return FALSE;

  LPVOID pMem = GlobalLock(hMem);
  if (!pMem) return FALSE;

  RtlCopyMemory(pMem, pptr, psize);
  GlobalUnlock(hMem);
  CreateStreamOnHGlobal(hMem, TRUE, &is);

  auto ret = SUCCEEDED(obj->put_Content(is.Get()));

  return ret;
}

DLL_EXPORTS(ResourceRequest_GetHeaders, BOOL)
(ICoreWebView2WebResourceRequest* obj, LPVOID* ptr) {
  if (!obj) return FALSE;

  ICoreWebView2HttpRequestHeaders* header;
  auto ret = SUCCEEDED(obj->get_Headers(&header));

  *ptr = header;

  return ret;
}



DLL_EXPORTS(ResourceResponse_GetReasonPhrase, BOOL)
(ICoreWebView2WebResourceResponse* obj, LPVOID* ptr, uint32_t* size) {
  if (!obj) return FALSE;

  LPWSTR value = nullptr;

  auto ret = SUCCEEDED(obj->get_ReasonPhrase(&value));

  *ptr = value;
  *size = lstrlenW(value) * 2 + 2;

  return ret;
}

DLL_EXPORTS(ResourceResponse_SetReasonPhrase, BOOL)
(ICoreWebView2WebResourceResponse* obj, LPWSTR uri) {
  if (!obj) return FALSE;

  return SUCCEEDED(obj->put_ReasonPhrase(uri));
}

DLL_EXPORTS(ResourceResponse_GetStatusCode, int)
(ICoreWebView2WebResourceResponse* obj) {
  if (!obj) return FALSE;

  int code = 0;
  obj->get_StatusCode(&code);

  return code;
}

DLL_EXPORTS(ResourceResponse_SetStatusCode, BOOL)
(ICoreWebView2WebResourceResponse* obj, int code) {
  if (!obj) return FALSE;

  return SUCCEEDED(obj->put_StatusCode(code));
}

DLL_EXPORTS(ResourceResponse_GetData, BOOL)
(ICoreWebView2WebResourceResponse* obj, LPVOID* pptr, uint32_t* psize) {
  if (!obj) return FALSE;

  WRL::ComPtr<IStream> is = nullptr;
  auto ret = SUCCEEDED(obj->get_Content(&is));

  STATSTG stat;
  is->Stat(&stat, STATFLAG_NONAME);

  LARGE_INTEGER linfo;
  linfo.QuadPart = 0;
  is->Seek(linfo, STREAM_SEEK_SET, NULL);

  uint32_t size = stat.cbSize.LowPart;
  uint8_t* buf = static_cast<uint8_t*>(wv2_Utility_Malloc(size));
  ULONG dummy = 0;
  is->Read(buf, size, &dummy);

  *pptr = buf;
  *psize = size;

  return ret;
}

DLL_EXPORTS(ResourceResponse_SetData, BOOL)
(ICoreWebView2WebResourceResponse* obj, LPVOID pptr, uint32_t psize) {
  if (!obj) return FALSE;

  WRL::ComPtr<IStream> is = nullptr;
  HGLOBAL hMem = GlobalAlloc(GMEM_ZEROINIT, psize);
  if (!hMem) return FALSE;

  LPVOID pMem = GlobalLock(hMem);
  if (!pMem) return FALSE;

  RtlCopyMemory(pMem, pptr, psize);
  GlobalUnlock(hMem);
  CreateStreamOnHGlobal(hMem, TRUE, &is);

  auto ret = SUCCEEDED(obj->put_Content(is.Get()));

  return ret;
}

DLL_EXPORTS(ResourceResponse_GetHeaders, BOOL)
(ICoreWebView2WebResourceResponse* obj, LPVOID* ptr) {
  if (!obj) return FALSE;

  ICoreWebView2HttpResponseHeaders* header;
  auto ret = SUCCEEDED(obj->get_Headers(&header));

  *ptr = header;

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

DLL_EXPORTS(Webview_HTTPRequestHeader_GetHeaders, BOOL)
(ICoreWebView2HttpRequestHeaders* header, LPWSTR name, LPVOID* arrayPtr,
 uint32_t* size) {
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



DLL_EXPORTS(Webview_HTTPResponseHeader_GetHeader, BOOL)
(ICoreWebView2HttpResponseHeaders* header, LPWSTR name, LPVOID* ptr,
 uint32_t* size) {
  if (!header) return FALSE;

  LPWSTR value = nullptr;
  auto ret = SUCCEEDED(header->GetHeader(name, &value));

  *ptr = value;
  *size = lstrlenW(value) * 2 + 2;

  return ret;
}

DLL_EXPORTS(Webview_HTTPResponseHeader_Contains, BOOL)
(ICoreWebView2HttpResponseHeaders* header, LPWSTR name) {
  if (!header) return FALSE;

  BOOL ret = FALSE;
  header->Contains(name, &ret);

  return ret;
}

DLL_EXPORTS(Webview_HTTPResponseHeader_AppendHeader, BOOL)
(ICoreWebView2HttpResponseHeaders* header, LPWSTR name, LPWSTR value) {
  if (!header) return FALSE;

  return SUCCEEDED(header->AppendHeader(name, value));
}

DLL_EXPORTS(Webview_HTTPResponseHeader_GetRawData, BOOL)
(ICoreWebView2HttpResponseHeaders* header, LPVOID* arrayPtr, uint32_t* size) {
  if (!header) return FALSE;

  ICoreWebView2HttpHeadersCollectionIterator* iter = nullptr;
  auto ret = SUCCEEDED(header->GetIterator(&iter));
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

DLL_EXPORTS(Webview_HTTPResponseHeader_GetHeaders, BOOL)
(ICoreWebView2HttpResponseHeaders* header, LPWSTR name, LPVOID* arrayPtr,
 uint32_t* size) {
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



DLL_EXPORTS(ResourceResponseView_GetReasonPhrase, BOOL)
(ICoreWebView2WebResourceResponseView* obj, LPVOID* ptr, uint32_t* size) {
  if (!obj) return FALSE;

  LPWSTR value = nullptr;

  auto ret = SUCCEEDED(obj->get_ReasonPhrase(&value));

  *ptr = value;
  *size = lstrlenW(value) * 2 + 2;

  return ret;
}

DLL_EXPORTS(ResourceResponseView_GetStatusCode, int)
(ICoreWebView2WebResourceResponseView* obj) {
  if (!obj) return FALSE;

  int code = 0;
  obj->get_StatusCode(&code);

  return code;
}

using ResourceResponseViewGetDataCB = HRESULT(CALLBACK*)(LPVOID ptr, uint32_t size, LPVOID param);
DLL_EXPORTS(ResourceResponseView_GetData, BOOL)
(ICoreWebView2WebResourceResponseView* obj,
 ResourceResponseViewGetDataCB callback, LPVOID param) {
  if (!obj) return FALSE;

  auto ret = SUCCEEDED(obj->GetContent(
      WRL::Callback<
          ICoreWebView2WebResourceResponseViewGetContentCompletedHandler>(
          [callback, param](HRESULT errorCode, IStream* content) -> HRESULT {
            if (content) {
              STATSTG stat;
              content->Stat(&stat, STATFLAG_NONAME);

              LARGE_INTEGER linfo;
              linfo.QuadPart = 0;
              content->Seek(linfo, STREAM_SEEK_SET, NULL);

              uint32_t size = stat.cbSize.LowPart;
              uint8_t* buf = static_cast<uint8_t*>(wv2_Utility_Malloc(size));
              ULONG dummy = 0;
              content->Read(buf, size, &dummy);

              return callback(buf, size, param);
            }
            return callback(nullptr, 0, param);
          })
          .Get()));

  return ret;
}

DLL_EXPORTS(ResourceResponseView_GetData_Sync, BOOL)
(ICoreWebView2WebResourceResponseView* obj, LPVOID* pptr, uint32_t* psize) {
  if (!obj) return FALSE;

  Waitable* waiter = CreateWaitable(true);

  WRL::ComPtr<IStream> is = nullptr;
  auto ret = SUCCEEDED(obj->GetContent(
      WRL::Callback<
          ICoreWebView2WebResourceResponseViewGetContentCompletedHandler>(
          [waiter, &is](HRESULT errorCode, IStream* content) -> HRESULT {
            if (content)
              is = content;
            ActiveWaitable(waiter);
            return S_OK;
          })
          .Get()));

  WaitOfMsgLoop(waiter);

  if (is) return FALSE;

  STATSTG stat;
  is->Stat(&stat, STATFLAG_NONAME);

  LARGE_INTEGER linfo;
  linfo.QuadPart = 0;
  is->Seek(linfo, STREAM_SEEK_SET, NULL);

  uint32_t size = stat.cbSize.LowPart;
  uint8_t* buf = static_cast<uint8_t*>(wv2_Utility_Malloc(size));
  ULONG dummy = 0;
  is->Read(buf, size, &dummy);

  *pptr = buf;
  *psize = size;

  return ret;
}

DLL_EXPORTS(ResourceResponseView_GetHeaders, BOOL)
(ICoreWebView2WebResourceResponseView* obj, LPVOID* ptr) {
  if (!obj) return FALSE;

  ICoreWebView2HttpResponseHeaders* header;
  auto ret = SUCCEEDED(obj->get_Headers(&header));

  *ptr = header;

  return ret;
}



