// Copyright (c) 2017 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.
//
// ---------------------------------------------------------------------------
//
// This file was generated by the CEF translator tool. If making changes by
// hand only do so within the body of existing method and function
// implementations. See the translator.README.txt file in the tools directory
// for more information.
//

#ifndef CEF_LIBCEF_DLL_CTOCPP_DIALOG_HANDLER_CTOCPP_H_
#define CEF_LIBCEF_DLL_CTOCPP_DIALOG_HANDLER_CTOCPP_H_
#pragma once

#if !defined(BUILDING_CEF_SHARED)
#error This file can be included DLL-side only
#endif

#include <vector>
#include "include/cef_dialog_handler.h"
#include "include/capi/cef_dialog_handler_capi.h"
#include "libcef_dll/ctocpp/ctocpp_ref_counted.h"

// Wrap a C structure with a C++ class.
// This class may be instantiated and accessed DLL-side only.
class CefDialogHandlerCToCpp
    : public CefCToCppRefCounted<CefDialogHandlerCToCpp, CefDialogHandler,
        cef_dialog_handler_t> {
 public:
  CefDialogHandlerCToCpp();

  // CefDialogHandler methods.
  bool OnFileDialog(CefRefPtr<CefBrowser> browser, FileDialogMode mode,
      const CefString& title, const CefString& default_file_path,
      const std::vector<CefString>& accept_filters, int selected_accept_filter,
      CefRefPtr<CefFileDialogCallback> callback) override;
};

#endif  // CEF_LIBCEF_DLL_CTOCPP_DIALOG_HANDLER_CTOCPP_H_
