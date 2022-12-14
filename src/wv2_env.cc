
#include "wv2_env.h"
#include "wv2_utils.h"

using namespace Microsoft;

DLL_EXPORTS(Global_EnableHighDPISupport, BOOL)() {
  return ::SetProcessDPIAware();
}

DLL_EXPORTS(Global_CheckEdgeRuntime, BOOL)
(LPWSTR path, LPVOID* ptr, uint32_t *size) {
  LPWSTR ver = nullptr;
  HRESULT hr = GetAvailableCoreWebView2BrowserVersionString(path, &ver);
  
  *ptr = ver;
  *size = lstrlenW(ver) * 2 + 2;

  return SUCCEEDED(hr);
}

struct EnvCreateSettings {
  LPCWSTR additional_parameter;
  LPCWSTR language;
  LPCWSTR version;
  BOOL allowSingleSign;
};

DLL_EXPORTS(Global_CreateEnv_Sync, BOOL)
(EnvCreateSettings* settings, LPCWSTR path, LPCWSTR userData, LPVOID* retPtr) {
  auto options = WRL::Make<CoreWebView2EnvironmentOptions>();

  if (settings) {
    if (settings->additional_parameter && *settings->additional_parameter)
      options->put_AdditionalBrowserArguments(settings->additional_parameter);
    if (settings->language && *settings->language)
      options->put_Language(settings->language);
    if (settings->version && *settings->version)
      options->put_TargetCompatibleBrowserVersion(settings->version);
    options->put_AllowSingleSignOnUsingOSPrimaryAccount(
        settings->allowSingleSign);
  }

  Waitable* waiter = CreateWaitable(true);

  HRESULT hr = CreateCoreWebView2EnvironmentWithOptions(
      path, userData, settings ? options.Get() : nullptr,
      WRL::Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
          [retPtr, waiter](HRESULT result,
                           ICoreWebView2Environment* env) -> HRESULT {
            if (env) {
              env->AddRef();
              *retPtr = env;
            }

            ActiveWaitable(waiter);
            return S_OK;
          })
          .Get());

  WaitOfMsgLoop(waiter);

  return SUCCEEDED(hr);
}

using CreateControllerCB = HRESULT(CALLBACK*)(LPVOID controller, LPVOID param);

DLL_EXPORTS(Env_CreateController, BOOL)
(ICoreWebView2Environment* env, HWND hwnd, CreateControllerCB callback,
 LPVOID param) {
  if (!env) return FALSE;

  HRESULT hr = env->CreateCoreWebView2Controller(
      hwnd,
      WRL::Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
          [callback, param](
              HRESULT errorCode,
              ICoreWebView2Controller* createdController) -> HRESULT {
            if (createdController) {
              if (createdController) createdController->AddRef();

              return callback(createdController, param);
            } else
              return E_FAIL;
          })
          .Get());

  return SUCCEEDED(hr);
}

DLL_EXPORTS(Env_CreateController_Sync, BOOL)
(ICoreWebView2Environment* env, HWND hwnd, LPVOID* retPtr) {
  if (!env) return FALSE;
  Waitable* waiter = CreateWaitable(true);

  HRESULT hr = env->CreateCoreWebView2Controller(
      hwnd,
      WRL::Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
          [waiter, retPtr](
              HRESULT errorCode,
              ICoreWebView2Controller* createdController) -> HRESULT {
            if (createdController) {
              if (createdController) createdController->AddRef();

              *retPtr = createdController;
              ActiveWaitable(waiter);

              return S_OK;
            } else
              return E_FAIL;
          })
          .Get());

  WaitOfMsgLoop(waiter);

  return SUCCEEDED(hr);
}

struct ControllerSettings {
  LPCWSTR profile;
  BOOL privateMode;
};
DLL_EXPORTS(Env_CreateController_Options, BOOL)
(ICoreWebView2Environment10* env, HWND hwnd, ControllerSettings* settings,
 CreateControllerCB callback, LPVOID param) {
  if (!env) return FALSE;

  WRL::ComPtr<ICoreWebView2ControllerOptions> options = nullptr;
  env->CreateCoreWebView2ControllerOptions(&options);

  if (settings) {
    options->put_ProfileName(settings->profile);
    options->put_IsInPrivateModeEnabled(settings->privateMode);
  }

  HRESULT hr = env->CreateCoreWebView2ControllerWithOptions(
      hwnd, options.Get(),
      WRL::Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
          [callback, param](
              HRESULT errorCode,
              ICoreWebView2Controller* createdController) -> HRESULT {
            if (createdController) {
              if (createdController) createdController->AddRef();

              return callback(createdController, param);
            } else
              return E_FAIL;
          })
          .Get());

  return SUCCEEDED(hr);
}

DLL_EXPORTS(Env_CreateController_Options_Sync, BOOL)
(ICoreWebView2Environment10* env, HWND hwnd, ControllerSettings* settings,
 LPVOID* retPtr) {
  if (!env) return FALSE;
  Waitable* waiter = CreateWaitable(true);

  WRL::ComPtr<ICoreWebView2ControllerOptions> options;
  env->CreateCoreWebView2ControllerOptions(&options);

  if (settings) {
    options->put_ProfileName(settings->profile);
    options->put_IsInPrivateModeEnabled(settings->privateMode);
  }

  HRESULT hr = env->CreateCoreWebView2ControllerWithOptions(
      hwnd, options.Get(),
      WRL::Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
          [waiter, retPtr](
              HRESULT errorCode,
              ICoreWebView2Controller* createdController) -> HRESULT {
            if (createdController) {
              if (createdController) createdController->AddRef();

              *retPtr = createdController;
              ActiveWaitable(waiter);

              return S_OK;
            } else
              return E_FAIL;
          })
          .Get());

  WaitOfMsgLoop(waiter);

  return SUCCEEDED(hr);
}

DLL_EXPORTS(Env_CreateWebResourceResponse, BOOL)
(ICoreWebView2Environment* env, LPVOID pptr, uint32_t psize, int code,
 LPWSTR reason, LPWSTR headers, LPVOID* ret) {
  if (!env) return FALSE;

  WRL::ComPtr<IStream> is = nullptr;
  HGLOBAL hMem = GlobalAlloc(GMEM_ZEROINIT, psize);
  if (!hMem) return FALSE;

  LPVOID pMem = GlobalLock(hMem);
  if (!pMem) return FALSE;

  RtlCopyMemory(pMem, pptr, psize);
  GlobalUnlock(hMem);
  CreateStreamOnHGlobal(hMem, TRUE, &is);

  ICoreWebView2WebResourceResponse* retObj = nullptr;
  HRESULT hr =
      env->CreateWebResourceResponse(is.Get(), code, reason, headers, &retObj);

  *ret = retObj;

  return SUCCEEDED(hr);
}

DLL_EXPORTS(Env_CreateWebResourceRequest, BOOL)
(ICoreWebView2Environment2* env, LPWSTR url, LPWSTR method, LPVOID pptr,
 uint32_t psize, LPWSTR headers, LPVOID* ret) {
  if (!env) return FALSE;

  WRL::ComPtr<IStream> is = nullptr;
  HGLOBAL hMem = GlobalAlloc(GMEM_ZEROINIT, psize);
  if (!hMem) return FALSE;

  LPVOID pMem = GlobalLock(hMem);
  if (!pMem) return FALSE;

  RtlCopyMemory(pMem, pptr, psize);
  GlobalUnlock(hMem);
  CreateStreamOnHGlobal(hMem, TRUE, &is);

  ICoreWebView2WebResourceRequest* retObj = nullptr;
  HRESULT hr =
      env->CreateWebResourceRequest(url, method, is.Get(), headers, &retObj);

  *ret = retObj;

  return SUCCEEDED(hr);
}

DLL_EXPORTS(Env_CreateContextMenuItem, BOOL)
(ICoreWebView2Environment9* env, LPWSTR label, LPVOID pptr, uint32_t psize,
 COREWEBVIEW2_CONTEXT_MENU_ITEM_KIND type, LPVOID* ret) {
  if (!env) return FALSE;

  WRL::ComPtr<IStream> is = nullptr;
  HGLOBAL hMem = GlobalAlloc(GMEM_ZEROINIT, psize);
  if (!hMem) return FALSE;

  LPVOID pMem = GlobalLock(hMem);
  if (!pMem) return FALSE;

  RtlCopyMemory(pMem, pptr, psize);
  GlobalUnlock(hMem);
  CreateStreamOnHGlobal(hMem, TRUE, &is);

  ICoreWebView2ContextMenuItem* retObj = nullptr;
  HRESULT hr = env->CreateContextMenuItem(label, is.Get(), type, &retObj);

  *ret = retObj;

  return SUCCEEDED(hr);
}
