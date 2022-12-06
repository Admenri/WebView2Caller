
#include "wv2_env.h"
#include "wv2_utils.h"

#include <UrlMon.h>
#pragma comment(lib, "urlmon.lib")

using namespace Microsoft;

DLL_EXPORTS(Global_EnableHighDPISupport, BOOL)() {
  return ::SetProcessDPIAware();
}

using DownloadRuntimeCB = void(CALLBACK*)(int statusCode);
// Run Download and Install in another thread so we don't block the UI thread
static DWORD WINAPI DownloadAndInstallWV2RT(_In_ LPVOID lpParameter) {
  int returnCode = 2;  // Download failed
  // Use fwlink to download WebView2 Bootstrapper at runtime and invoke
  // installation Broken/Invalid Https Certificate will fail to download Use of
  // the download link below is governed by the below terms. You may acquire the
  // link for your use at
  // https://developer.microsoft.com/microsoft-edge/webview2/. Microsoft owns
  // all legal right, title, and interest in and to the WebView2 Runtime
  // Bootstrapper ("Software") and related documentation, including any
  // intellectual property in the Software. You must acquire all code, including
  // any code obtained from a Microsoft URL, under a separate license directly
  // from Microsoft, including a Microsoft download site (e.g.,
  // https://developer.microsoft.com/microsoft-edge/webview2/).
  HRESULT hr = URLDownloadToFileW(
      NULL, L"https://go.microsoft.com/fwlink/p/?LinkId=2124703",
      L".\\MicrosoftEdgeWebview2Setup.exe", 0, 0);
  if (hr == S_OK) {
    // Either Package the WebView2 Bootstrapper with your app or download it
    // using fwlink Then invoke install at Runtime.
    SHELLEXECUTEINFO shExInfo = {0};
    shExInfo.cbSize = sizeof(shExInfo);
    shExInfo.fMask = SEE_MASK_NOASYNC;
    shExInfo.hwnd = 0;
    shExInfo.lpVerb = L"runas";
    shExInfo.lpFile = L"MicrosoftEdgeWebview2Setup.exe";
    shExInfo.lpParameters = L" /silent /install";
    shExInfo.lpDirectory = 0;
    shExInfo.nShow = 0;
    shExInfo.hInstApp = 0;

    if (ShellExecuteEx(&shExInfo)) {
      returnCode = 0;  // Install successfull
    } else {
      returnCode = 1;  // Install failed
    }
  }
  
  // Execute callback
  if (lpParameter) static_cast<DownloadRuntimeCB>(lpParameter)(returnCode);
  return returnCode;
}

DLL_EXPORTS(Global_CheckEdgeRuntime, BOOL)
(LPWSTR path, BOOL allowDownload, DownloadRuntimeCB callback) {
  LPWSTR ver = nullptr;
  HRESULT hr = GetAvailableCoreWebView2BrowserVersionString(path, &ver);
  if (hr == S_OK && ver)
    return TRUE;
  else if (allowDownload)
    CreateThread(0, 0, DownloadAndInstallWV2RT, callback, 0, 0);

  return FALSE;
}

struct EnvCreateSettings {
  LPCWSTR additional_parameter;
  LPCWSTR language;
  LPCWSTR version;
  BOOL allowSingleSign;
};

DLL_EXPORTS(Global_CreateEnv_Sync, BOOL)
(EnvCreateSettings* settings, LPCWSTR path, LPCWSTR userData,
 LPVOID* retPtr) {
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

  WaitOfSleep(waiter, 300);

  return SUCCEEDED(hr);
}



using CreateControllerCB = HRESULT(CALLBACK*)(LPVOID controller, LPVOID param);

DLL_EXPORTS(Env_CreateController, BOOL)
(ICoreWebView2Environment* env, HWND hwnd, RECT* lpRect, CreateControllerCB callback, LPVOID param) {
  if (!env) return FALSE;
  
  HRESULT hr = env->CreateCoreWebView2Controller(
      hwnd,
      WRL::Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
          [lpRect, callback, param](
              HRESULT errorCode,
              ICoreWebView2Controller* createdController) -> HRESULT {
            if (createdController) {
              if (createdController) {
                createdController->AddRef();
                createdController->put_Bounds(*lpRect);
              }

              return callback(createdController, param);
            } else
              return E_FAIL;
          })
          .Get());

  return SUCCEEDED(hr);
}

DLL_EXPORTS(Env_CreateController_Sync, BOOL)
(ICoreWebView2Environment* env, HWND hwnd, RECT* lpRect, LPVOID* retPtr) {
  if (!env) return FALSE;
  Waitable* waiter = CreateWaitable(true);

  HRESULT hr = env->CreateCoreWebView2Controller(
      hwnd,
      WRL::Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
          [waiter, retPtr, lpRect](
              HRESULT errorCode,
              ICoreWebView2Controller* createdController) -> HRESULT {
            if (createdController) {
              if (createdController) {
                createdController->AddRef();
                createdController->put_Bounds(*lpRect);
              }

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
 RECT* lpRect, CreateControllerCB callback, LPVOID param) {
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
          [lpRect, callback, param](
              HRESULT errorCode,
              ICoreWebView2Controller* createdController) -> HRESULT {
            if (createdController) {
              if (createdController) {
                createdController->AddRef();
                createdController->put_Bounds(*lpRect);
              }

              return callback(createdController, param);
            } else
              return E_FAIL;
          })
          .Get());

  return SUCCEEDED(hr);
}

DLL_EXPORTS(Env_CreateController_Options_Sync, BOOL)
(ICoreWebView2Environment10* env, HWND hwnd, ControllerSettings *settings, RECT* lpRect,
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
          [waiter, retPtr, lpRect](
              HRESULT errorCode,
              ICoreWebView2Controller* createdController) -> HRESULT {
            if (createdController) {
              if (createdController) {
                createdController->AddRef();
                createdController->put_Bounds(*lpRect);
              }

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
(ICoreWebView2Environment* env, LPVOID pptr, uint32_t psize, int code, LPWSTR reason, LPWSTR headers, LPVOID* ret) {
  if (!env) return FALSE;
  Waitable* waiter = CreateWaitable(true);

  WRL::ComPtr<IStream> is = nullptr;
  HGLOBAL hMem = GlobalAlloc(GMEM_ZEROINIT, psize);
  if (!hMem) return FALSE;

  LPVOID pMem = GlobalLock(hMem);
  if (!pMem) return FALSE;

  RtlCopyMemory(pMem, pptr, psize);
  GlobalUnlock(hMem);
  CreateStreamOnHGlobal(hMem, TRUE, &is);

  ICoreWebView2WebResourceResponse* retObj = nullptr;
  HRESULT hr = env->CreateWebResourceResponse(is.Get(), code, reason, headers, &retObj);

  *ret = retObj;

  return SUCCEEDED(hr);
}
