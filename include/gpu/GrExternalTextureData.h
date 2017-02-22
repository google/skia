/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrExternalTextureData_DEFINED
#define GrExternalTextureData_DEFINED

#include "GrTypes.h"
#include "GrTypesPriv.h"

class SK_API GrExternalTextureData : SkNoncopyable {
public:
    GrExternalTextureData(GrFence fence) : fFence(fence) {}
    virtual ~GrExternalTextureData() {}
    virtual GrBackend getBackend() const = 0;
    GrFence getFence() const { return fFence; }

protected:
    virtual GrBackendObject getBackendObject() const = 0;

    GrFence fFence;

    friend class SkCrossContextImageData;
};

#endif
