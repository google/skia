/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieExternalLayer_DEFINED
#define SkottieExternalLayer_DEFINED

#include "include/core/SkRefCnt.h"

class SkCanvas;
struct SkSize;

namespace skottie {

class ExternalLayer : public SkRefCnt {
public:
    virtual void render(SkCanvas*, double t) = 0;
};

class PrecompInterceptor : public SkRefCnt {
public:
    virtual sk_sp<ExternalLayer> onLoadPrecomp(const char id[],
                                               const char name[],
                                               const SkSize&) = 0;
};

} // namespace

#endif // SkottieExternalLayer_DEFINED
