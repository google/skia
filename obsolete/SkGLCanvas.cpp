
/*
 * Copyright 2010 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkGLCanvas.h"
#include "SkGLDevice.h"

SkGLCanvas::SkGLCanvas() : SkCanvas(SkNEW(SkGLDeviceFactory)) {}

// static
size_t SkGLCanvas::GetTextureCacheMaxCount() {
    return SkGLDevice::GetTextureCacheMaxCount();
}

// static
void SkGLCanvas::SetTextureCacheMaxCount(size_t count) {
    SkGLDevice::SetTextureCacheMaxCount(count);
}

// static
size_t SkGLCanvas::GetTextureCacheMaxSize() {
    return SkGLDevice::GetTextureCacheMaxSize();
}

// static
void SkGLCanvas::SetTextureCacheMaxSize(size_t size) {
    SkGLDevice::SetTextureCacheMaxSize(size);
}

// static
void SkGLCanvas::DeleteAllTextures() {
    SkGLDevice::DeleteAllTextures();
}

// static
void SkGLCanvas::AbandonAllTextures() {
    SkGLDevice::AbandonAllTextures();
}
