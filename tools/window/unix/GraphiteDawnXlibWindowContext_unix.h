/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GraphiteDawnXlibWindowContext_unix_DEFINED
#define GraphiteDawnXlibWindowContext_unix_DEFINED

#include <memory>

#include "tools/sk_app/Window.h"

namespace skwindow {
class WindowContext;
class DisplayParams;
struct XlibWindowInfo;

std::unique_ptr<WindowContext> MakeGraphiteDawnForXlib(const XlibWindowInfo&,
                                                       std::unique_ptr<const DisplayParams>,
                                                       sk_app::Window::BackendType backendType);
}  // namespace skwindow

#endif
