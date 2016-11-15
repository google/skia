/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLSLImage_DEFINED
#define GrGLSLImage_DEFINED

#include "GrTypes.h"

class GrGLSLImage {
public:
    virtual ~GrGLSLImage() {}

    GrGLSLImage(uint32_t visibility)
        : fVisibility(visibility) {}

    virtual const char* name() const = 0;
    uint32_t visibility() const { return fVisibility; }

private:
    uint32_t        fVisibility;
};

#endif
