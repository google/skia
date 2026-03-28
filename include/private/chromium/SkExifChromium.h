/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkExifChromium_DEFINED
#define SkExifChromium_DEFINED

#include "include/private/base/SkAPI.h"

namespace SkExif {

// Force all EXIF parsing to use the C++ SkExif parser instead of the
// build-default parser. This is intended for Chromium to have a kill-switch
// to fall back to the C++ implementation if rust_exif causes issues in the
// field.
//
// This is a global setting and is NOT thread-safe with respect to concurrent
// codec operations. It should be called once early during process startup.
SK_API void ForceSkExif(bool forceSkExif);

}  // namespace SkExif

#endif  // SkExifChromium_DEFINED
