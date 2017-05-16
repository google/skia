//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// OSPixmap.h: Definition of an abstract pixmap class

#ifndef SAMPLE_UTIL_PIXMAP_H_
#define SAMPLE_UTIL_PIXMAP_H_

#include <stdlib.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "Event.h"

class OSPixmap
{
  public:
    OSPixmap() {}
    virtual ~OSPixmap() {}

    virtual bool initialize(EGLNativeDisplayType display, size_t width, size_t height, int depth) = 0;

    virtual EGLNativePixmapType getNativePixmap() const = 0;
};

OSPixmap *CreateOSPixmap();

#endif // SAMPLE_UTIL_PIXMAP_H_
