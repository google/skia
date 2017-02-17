/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrCrossContextTextureData_DEFINED
#define GrCrossContextTextureData_DEFINED

#include "GrTypes.h"

class SK_API GrCrossContextTextureData : SkNoncopyable {
public:
    virtual ~GrCrossContextTextureData() {}
    virtual GrBackend getBackend() const = 0;
    virtual GrBackendObject getBackendObject() const = 0;
};

#endif
