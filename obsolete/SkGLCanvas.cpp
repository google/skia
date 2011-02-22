/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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
