
#include "wv2_cookie.h"
#include "wv2_utils.h"

using namespace Microsoft;

DLL_EXPORTS(CookieManager_CreateCookie, BOOL)
(ICoreWebView2CookieManager* ckm, LPWSTR name, LPWSTR value, LPWSTR domain,
 LPWSTR path, LPVOID* ptr) {
  if (!ckm) return FALSE;

  ICoreWebView2Cookie* ck = nullptr;
  auto ret = SUCCEEDED(ckm->CreateCookie(name, value, domain, path, &ck));
  
  *ptr = ck;

  return ret;
}

DLL_EXPORTS(CookieManager_CopyCookie, BOOL)
(ICoreWebView2CookieManager* ckm, ICoreWebView2Cookie* ck, LPVOID* ptr) {
  if (!ckm) return FALSE;

  ICoreWebView2Cookie* cck = nullptr;
  auto ret = SUCCEEDED(ckm->CopyCookie(ck, &cck));

  *ptr = cck;

  return ret;
}

DLL_EXPORTS(CookieList_GetCount, uint32_t)
(ICoreWebView2CookieList* ck) {
  if (!ck) return FALSE;

  uint32_t size = 0;
  ck->get_Count(&size);

  return size;
}

DLL_EXPORTS(CookieList_GetCookieAtIndex, BOOL)
(ICoreWebView2CookieList* ck, uint32_t index, LPVOID* ptr) {
  if (!ck) return FALSE;

  ICoreWebView2Cookie* ckd = nullptr;
  auto ret = SUCCEEDED(ck->GetValueAtIndex(index, &ckd));

  *ptr = ckd;

  return ret;
}



using GetCookieCB = HRESULT(CALLBACK*)(LPVOID list, LPVOID param);
DLL_EXPORTS(CookieManager_GetCookie, BOOL)
(ICoreWebView2CookieManager* ckm, LPWSTR url, GetCookieCB callback,
 LPVOID param) {
  if (!ckm) return FALSE;

  ICoreWebView2Cookie* cck = nullptr;
  auto ret = SUCCEEDED(ckm->GetCookies(
      url,
      WRL::Callback<ICoreWebView2GetCookiesCompletedHandler>(
          [callback, param](HRESULT result,
                            ICoreWebView2CookieList* cookieList) -> HRESULT {
            if (cookieList) cookieList->AddRef();

            return callback(cookieList, param);
          })
          .Get()));

  return ret;
}

DLL_EXPORTS(CookieManager_GetCookie_Sync, BOOL)
(ICoreWebView2CookieManager* ckm, LPWSTR url, LPVOID* ptr, uint32_t* size) {
  if (!ckm) return FALSE;

  Waitable* waiter = CreateWaitable(true);

  auto ret = SUCCEEDED(ckm->GetCookies(
      url,
      WRL::Callback<ICoreWebView2GetCookiesCompletedHandler>(
          [waiter, ptr, size](HRESULT result,
                              ICoreWebView2CookieList* cookieList) -> HRESULT {
            uint32_t count = 0;
            cookieList->get_Count(&count);

            LPVOID* buf =
                static_cast<LPVOID*>(wv2_Utility_Malloc(count * sizeof(void*)));
            for (size_t i = 0; i < count; i++) {
              ICoreWebView2Cookie* ckData = nullptr;
              cookieList->GetValueAtIndex(i, &ckData);
              buf[i] = ckData;
            }

            *ptr = buf;
            *size = count;

            ActiveWaitable(waiter);

            return S_OK;
          })
          .Get()));

  WaitOfMsgLoop(waiter);

  return ret;
}

DLL_EXPORTS(CookieManager_SetCookie, BOOL)
(ICoreWebView2CookieManager* ckm, ICoreWebView2Cookie* ck) {
  if (!ckm) return FALSE;

  auto ret = SUCCEEDED(ckm->AddOrUpdateCookie(ck));

  return ret;
}

DLL_EXPORTS(CookieManager_DeleteCookie, BOOL)
(ICoreWebView2CookieManager* ckm, ICoreWebView2Cookie* ck) {
  if (!ckm) return FALSE;

  auto ret = SUCCEEDED(ckm->DeleteCookie(ck));

  return ret;
}

DLL_EXPORTS(CookieManager_DeleteCookies, BOOL)
(ICoreWebView2CookieManager* ckm, LPWSTR name, LPWSTR url) {
  if (!ckm) return FALSE;

  auto ret = SUCCEEDED(ckm->DeleteCookies(name, url));

  return ret;
}

DLL_EXPORTS(CookieManager_DeleteCookieWithParam, BOOL)
(ICoreWebView2CookieManager* ckm, LPWSTR name, LPWSTR domain, LPWSTR path) {
  if (!ckm) return FALSE;

  auto ret = SUCCEEDED(ckm->DeleteCookiesWithDomainAndPath(name, domain, path));

  return ret;
}

DLL_EXPORTS(CookieManager_DeleteAllCookies, BOOL)
(ICoreWebView2CookieManager* ckm) {
  if (!ckm) return FALSE;

  auto ret = SUCCEEDED(ckm->DeleteAllCookies());

  return ret;
}


DLL_EXPORTS(CookieData_GetName, BOOL)
(ICoreWebView2Cookie* ckd, LPVOID* ptr, uint32_t* size) {
  if (!ckd) return FALSE;

  LPWSTR str = nullptr;
  auto ret = SUCCEEDED(ckd->get_Name(&str));

  *ptr = str;
  *size = lstrlenW(str) * 2 + 2;

  return ret;
}

DLL_EXPORTS(CookieData_GetValue, BOOL)
(ICoreWebView2Cookie* ckd, LPVOID* ptr, uint32_t* size) {
  if (!ckd) return FALSE;

  LPWSTR str = nullptr;
  auto ret = SUCCEEDED(ckd->get_Value(&str));

  *ptr = str;
  *size = lstrlenW(str) * 2 + 2;

  return ret;
}

DLL_EXPORTS(CookieData_GetDomain, BOOL)
(ICoreWebView2Cookie* ckd, LPVOID* ptr, uint32_t* size) {
  if (!ckd) return FALSE;

  LPWSTR str = nullptr;
  auto ret = SUCCEEDED(ckd->get_Domain(&str));

  *ptr = str;
  *size = lstrlenW(str) * 2 + 2;

  return ret;
}

DLL_EXPORTS(CookieData_GetPath, BOOL)
(ICoreWebView2Cookie* ckd, LPVOID* ptr, uint32_t* size) {
  if (!ckd) return FALSE;

  LPWSTR str = nullptr;
  auto ret = SUCCEEDED(ckd->get_Path(&str));

  *ptr = str;
  *size = lstrlenW(str) * 2 + 2;

  return ret;
}

DLL_EXPORTS(CookieData_SetValue, BOOL)
(ICoreWebView2Cookie* ckd, LPWSTR str) {
  if (!ckd) return FALSE;

  return SUCCEEDED(ckd->put_Value(str));
}

DLL_EXPORTS(CookieData_GetExpires, double)
(ICoreWebView2Cookie* ckd) {
  if (!ckd) return 0;

  double data = 0.0f;
  ckd->get_Expires(&data);

  return data;
}

DLL_EXPORTS(CookieData_SetExpires, BOOL)
(ICoreWebView2Cookie* ckd, double expires) {
  if (!ckd) return FALSE;
  
  auto ret = SUCCEEDED(ckd->put_Expires(expires));

  return ret;
}

DLL_EXPORTS(CookieData_GetHTTPOnly, BOOL)
(ICoreWebView2Cookie* ckd) {
  if (!ckd) return FALSE;

  BOOL data = FALSE;
  ckd->get_IsHttpOnly(&data);

  return data;
}

DLL_EXPORTS(CookieData_SetHTTPOnly, BOOL)
(ICoreWebView2Cookie* ckd, BOOL value) {
  if (!ckd) return FALSE;

  auto ret = SUCCEEDED(ckd->put_IsHttpOnly(value));

  return ret;
}

DLL_EXPORTS(CookieData_GetSameSite, COREWEBVIEW2_COOKIE_SAME_SITE_KIND)
(ICoreWebView2Cookie* ckd) {
  if (!ckd) return COREWEBVIEW2_COOKIE_SAME_SITE_KIND();

  COREWEBVIEW2_COOKIE_SAME_SITE_KIND data = COREWEBVIEW2_COOKIE_SAME_SITE_KIND();
  ckd->get_SameSite(&data);

  return data;
}

DLL_EXPORTS(CookieData_SetSameSite, BOOL)
(ICoreWebView2Cookie* ckd, COREWEBVIEW2_COOKIE_SAME_SITE_KIND value) {
  if (!ckd) return FALSE;

  auto ret = SUCCEEDED(ckd->put_SameSite(value));

  return ret;
}

DLL_EXPORTS(CookieData_GetSecure, BOOL)
(ICoreWebView2Cookie* ckd) {
  if (!ckd) return FALSE;

  BOOL data = FALSE;
  ckd->get_IsSecure(&data);

  return data;
}

DLL_EXPORTS(CookieData_SetSecure, BOOL)
(ICoreWebView2Cookie* ckd, BOOL value) {
  if (!ckd) return FALSE;

  auto ret = SUCCEEDED(ckd->put_IsSecure(value));

  return ret;
}

DLL_EXPORTS(CookieData_GetSession, BOOL)
(ICoreWebView2Cookie* ckd) {
  if (!ckd) return FALSE;

  BOOL data = FALSE;
  ckd->get_IsSession(&data);

  return data;
}
