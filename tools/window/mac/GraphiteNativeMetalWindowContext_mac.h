/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GraphiteNativeMetalWindowContext_mac_DEFINED
#define GraphiteNativeMetalWindowContext_mac_DEFINED

#include <memory>

namespace skwindow {
class WindowContext;
struct DisplayParams;
struct MacWindowInfo;

std::unique_ptr<WindowContext> MakeGraphiteNativeMetalForMac(const MacWindowInfo&,
                                                             const DisplayParams&);
}  // namespace skwindow

#endif
