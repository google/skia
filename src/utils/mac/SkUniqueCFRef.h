/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkUniqueCFRef_DEFINED
#define SkUniqueCFRef_DEFINED

#include "SkTypes.h"  // Keep this before any #ifdef ...
#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)

#include "SkTemplates.h"
#include "SkTLogic.h"

#include <CoreFoundation/CoreFoundation.h>
#include <memory>

template <typename CFRef> using SkUniqueCFRef =
    std::unique_ptr<skstd::remove_pointer_t<CFRef>,
                    SkFunctionWrapper<void, skstd::remove_pointer_t<CFTypeRef>, CFRelease>>;

#endif
#endif
