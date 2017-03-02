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

class GrSemaphore;

class SK_API GrExternalTextureData : SkNoncopyable {
public:
    GrExternalTextureData(sk_sp<GrSemaphore>);
    virtual ~GrExternalTextureData();
    virtual GrBackend getBackend() const = 0;
    sk_sp<GrSemaphore> getSemaphoreRef() const;

protected:
    virtual GrBackendObject getBackendObject() const = 0;

    sk_sp<GrSemaphore> fSemaphore;

    friend class SkCrossContextImageData;
};

#endif
