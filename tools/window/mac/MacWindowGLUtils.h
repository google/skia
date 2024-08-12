/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef MacWindowGLUtils_DEFINED
#define MacWindowGLUtils_DEFINED

#include "include/private/base/SkAssert.h"

#include <Cocoa/Cocoa.h>

namespace skwindow {

static inline NSOpenGLPixelFormat* GetGLPixelFormat(int sampleCount) {
    constexpr int kMaxAttributes = 19;
    NSOpenGLPixelFormatAttribute attributes[kMaxAttributes];
    int numAttributes = 0;
    attributes[numAttributes++] = NSOpenGLPFAAccelerated;
    attributes[numAttributes++] = NSOpenGLPFAClosestPolicy;
    attributes[numAttributes++] = NSOpenGLPFADoubleBuffer;
    attributes[numAttributes++] = NSOpenGLPFAOpenGLProfile;
    attributes[numAttributes++] = NSOpenGLProfileVersion3_2Core;
    attributes[numAttributes++] = NSOpenGLPFAColorSize;
    attributes[numAttributes++] = 24;
    attributes[numAttributes++] = NSOpenGLPFAAlphaSize;
    attributes[numAttributes++] = 8;
    attributes[numAttributes++] = NSOpenGLPFADepthSize;
    attributes[numAttributes++] = 0;
    attributes[numAttributes++] = NSOpenGLPFAStencilSize;
    attributes[numAttributes++] = 8;
    if (sampleCount > 1) {
        attributes[numAttributes++] = NSOpenGLPFAMultisample;
        attributes[numAttributes++] = NSOpenGLPFASampleBuffers;
        attributes[numAttributes++] = 1;
        attributes[numAttributes++] = NSOpenGLPFASamples;
        attributes[numAttributes++] = sampleCount;
    } else {
        attributes[numAttributes++] = NSOpenGLPFASampleBuffers;
        attributes[numAttributes++] = 0;
    }
    attributes[numAttributes++] = 0;
    SkASSERT(numAttributes <= kMaxAttributes);
    return [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes];
}

}  // namespace skwindow

#endif
