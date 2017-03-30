/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorSpaceXformCanvas_DEFINED
#define SkColorSpaceXformCanvas_DEFINED

#include <SkCanvas.h>
#include <SkColorSpace.h>
#include <memory>

// Proxy SkCanvas calls to unowned target, transforming colors into targetCS as it goes.
// May return nullptr if |targetCS| is unsupported.
std::unique_ptr<SkCanvas> SK_API SkCreateColorSpaceXformCanvas(SkCanvas* target,
                                                               sk_sp<SkColorSpace> targetCS);

#endif  //SkColorSpaceXformCanvas_DEFINED
