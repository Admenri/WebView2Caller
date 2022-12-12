
#include "wv2_contextmenu.h"
using namespace Microsoft;

DLL_EXPORTS(Webview_ContextMenuArgs_GetDeferral, BOOL)
(ICoreWebView2ContextMenuRequestedEventArgs* args) {
  if (!args) return FALSE;

  ICoreWebView2Deferral* deferral = nullptr;
  HRESULT hr = args->GetDeferral(&deferral);

  return SUCCEEDED(hr);
}

DLL_EXPORTS(Webview_ContextMenuArgs_Handled_Attr, BOOL)
(ICoreWebView2ContextMenuRequestedEventArgs* args, BOOL get, BOOL value) {
  if (!args) return FALSE;

  if (get) {
    BOOL handled = FALSE;
    args->get_Handled(&handled);
    return handled;
  } else {
    return SUCCEEDED(args->put_Handled(value));
  }
}

DLL_EXPORTS(Webview_ContextMenuArgs_SelectedCommandID_Attr, int)
(ICoreWebView2ContextMenuRequestedEventArgs* args, BOOL get, int value) {
  if (!args) return FALSE;

  if (get) {
    int handled = 0;
    args->get_SelectedCommandId(&handled);
    return handled;
  } else {
    return SUCCEEDED(args->put_SelectedCommandId(value));
  }
}

DLL_EXPORTS(Webview_ContextMenuArgs_GetPos, BOOL)
(ICoreWebView2ContextMenuRequestedEventArgs* args, POINT* pt) {
  if (!args) return FALSE;

  return SUCCEEDED(args->get_Location(pt));
}

DLL_EXPORTS(Webview_ContextMenuArgs_GetTargetInfo, int)
(ICoreWebView2ContextMenuRequestedEventArgs* args, int id, LPVOID* ptr,
 uint32_t* size) {
  if (!args) return FALSE;

  WRL::ComPtr<ICoreWebView2ContextMenuTarget> target = nullptr;
  HRESULT hr = args->get_ContextMenuTarget(&target);

  COREWEBVIEW2_CONTEXT_MENU_TARGET_KIND kind;
  BOOL value = FALSE;
  LPWSTR v = nullptr;

  switch (id) {
    case 0:
      target->get_Kind(&kind);
      return kind;
      break;
    case 1:
      target->get_IsEditable(&value);
      return value;
      break;
    case 2:
      target->get_IsRequestedForMainFrame(&value);
      return value;
      break;
    case 3:
      target->get_PageUri(&v);
      *ptr = v;
      *size = lstrlenW(v) * 2 + 2;
      break;
    case 4:
      target->get_FrameUri(&v);
      *ptr = v;
      *size = lstrlenW(v) * 2 + 2;
      break;
    case 5:
      target->get_HasLinkUri(&value);
      return value;
      break;
    case 6:
      target->get_LinkUri(&v);
      *ptr = v;
      *size = lstrlenW(v) * 2 + 2;
      break;
    case 7:
      target->get_HasLinkText(&value);
      return value;
      break;
    case 8:
      target->get_LinkText(&v);
      *ptr = v;
      *size = lstrlenW(v) * 2 + 2;
      break;
    case 9:
      target->get_HasSourceUri(&value);
      return value;
      break;
    case 10:
      target->get_SourceUri(&v);
      *ptr = v;
      *size = lstrlenW(v) * 2 + 2;
      break;
    case 11:
      target->get_HasSelection(&value);
      return value;
      break;
    case 12:
      target->get_SelectionText(&v);
      *ptr = v;
      *size = lstrlenW(v) * 2 + 2;
      break;
    default:
      // unreached
      break;
  }

  return SUCCEEDED(hr);
}

DLL_EXPORTS(Webview_ContextMenuArgs_GetItemCollection, BOOL)
(ICoreWebView2ContextMenuRequestedEventArgs* args, LPVOID* ptr) {
  if (!args) return FALSE;

  ICoreWebView2ContextMenuItemCollection* items = nullptr;
  HRESULT hr = args->get_MenuItems(&items);

  *ptr = items;

  return SUCCEEDED(hr);
}


DLL_EXPORTS(MenuItemCollection_GetCount, uint32_t)
(ICoreWebView2ContextMenuItemCollection* args) {
  if (!args) return FALSE;

  uint32_t count = 0;
  args->get_Count(&count);

  return count;
}

DLL_EXPORTS(MenuItemCollection_GetValueAt, BOOL)
(ICoreWebView2ContextMenuItemCollection* args, uint32_t index, LPVOID* ptr) {
  if (!args) return FALSE;
  
  ICoreWebView2ContextMenuItem* value = nullptr;
  HRESULT hr = args->GetValueAtIndex(index, &value);

  *ptr = value;

  return SUCCEEDED(hr);
}

DLL_EXPORTS(MenuItemCollection_RemoveAt, BOOL)
(ICoreWebView2ContextMenuItemCollection* args, uint32_t index) {
  if (!args) return FALSE;

  return SUCCEEDED(args->RemoveValueAtIndex(index));
}

DLL_EXPORTS(MenuItemCollection_InsertAt, BOOL)
(ICoreWebView2ContextMenuItemCollection* args, uint32_t index,
 ICoreWebView2ContextMenuItem* item) {
  if (!args) return FALSE;

  return SUCCEEDED(args->InsertValueAtIndex(index, item));
}


DLL_EXPORTS(MenuItem_GetName, BOOL)
(ICoreWebView2ContextMenuItem* item, LPVOID* ptr, uint32_t* size) {
  if (!item) return FALSE;

  LPWSTR value = nullptr;
  HRESULT hr = item->get_Name(&value);

  *ptr = value;
  *size = lstrlenW(value);

  return SUCCEEDED(hr);
}

DLL_EXPORTS(MenuItem_GetLabel, BOOL)
(ICoreWebView2ContextMenuItem* item, LPVOID* ptr, uint32_t* size) {
  if (!item) return FALSE;

  LPWSTR value = nullptr;
  HRESULT hr = item->get_Label(&value);

  *ptr = value;
  *size = lstrlenW(value);

  return SUCCEEDED(hr);
}

DLL_EXPORTS(MenuItem_GetCommandID, int)
(ICoreWebView2ContextMenuItem* item) {
  if (!item) return FALSE;

  int value = 0;
  item->get_CommandId(&value);

  return value;
}

DLL_EXPORTS(MenuItem_GetShortcutKeyDescription, BOOL)
(ICoreWebView2ContextMenuItem* item, LPVOID* ptr, uint32_t* size) {
  if (!item) return FALSE;

  LPWSTR value = nullptr;
  HRESULT hr = item->get_ShortcutKeyDescription(&value);

  *ptr = value;
  *size = lstrlenW(value);

  return SUCCEEDED(hr);
}

DLL_EXPORTS(MenuItem_GetIcon, BOOL)
(ICoreWebView2ContextMenuItem* obj, LPVOID* pptr, uint32_t* psize) {
  if (!obj) return FALSE;

  WRL::ComPtr<IStream> is = nullptr;
  auto ret = SUCCEEDED(obj->get_Icon(&is));

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

DLL_EXPORTS(MenuItem_GetType, int)
(ICoreWebView2ContextMenuItem* item) {
  if (!item) return FALSE;

  COREWEBVIEW2_CONTEXT_MENU_ITEM_KIND value;
  item->get_Kind(&value);

  return value;
}

DLL_EXPORTS(MenuItem_Enabled_Attr, BOOL)
(ICoreWebView2ContextMenuItem* item, BOOL get, BOOL value) {
  if (!item) return FALSE;

  if (get) {
    BOOL value = FALSE;
    item->get_IsEnabled(&value);
    return value;
  } else {
    return SUCCEEDED(item->put_IsEnabled(value));
  }
}

DLL_EXPORTS(MenuItem_Checked_Attr, BOOL)
(ICoreWebView2ContextMenuItem* item, BOOL get, BOOL value) {
  if (!item) return FALSE;

  if (get) {
    BOOL value = FALSE;
    item->get_IsChecked(&value);
    return value;
  } else {
    return SUCCEEDED(item->put_IsChecked(value));
  }
}

DLL_EXPORTS(MenuItem_GetChildren, BOOL)
(ICoreWebView2ContextMenuItem* item, LPVOID* ptr) {
  if (!item) return FALSE;

  ICoreWebView2ContextMenuItemCollection* value = nullptr;
  HRESULT hr = item->get_Children(&value);

  *ptr = value;

  return SUCCEEDED(hr);
}

using ContextMenuSelectedCB = HRESULT(CALLBACK*)(LPVOID item, LPVOID param);
DLL_EXPORTS(MenuItem_Attach_ItemSelected, int64_t)
(ICoreWebView2ContextMenuItem* item, ContextMenuSelectedCB callback,
 LPVOID param) {
  if (!item) return FALSE;

  EventRegistrationToken token;

  item->add_CustomItemSelected(
      WRL::Callback<ICoreWebView2CustomItemSelectedEventHandler>(
          [callback, param](ICoreWebView2ContextMenuItem* sender,
                            IUnknown* args) -> HRESULT {
            sender->AddRef();

            HRESULT hr = callback(sender, param);

            return hr;
          })
          .Get(),
      &token);

  return token.value;
}

DLL_EXPORTS(MenuItem_Detach_ItemSelected, BOOL)
(ICoreWebView2ContextMenuItem* item, int64_t value) {
  if (!item) return FALSE;
  EventRegistrationToken token = {value};

  return SUCCEEDED(item->remove_CustomItemSelected(token));
}
