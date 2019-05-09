/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBackendObject_DEFINED
#define GrBackendObject_DEFINED

#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"

class GrBackendObject : public SkRefCnt {
public:

protected:
    friend class GrContext; // for factories

    // Create an uninitialized non-renderable backend texture
    static sk_sp<GrBackendObject> Gimme(GrContext* context, int width, int height, SkColorType ct);

private:
    GrBackendObject(GrBackendTexture);

    typedef SkRefCnt INHERITED;
};

#endif

