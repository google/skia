// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef examples_DEFINED
#define examples_DEFINED

#include "Registry.h"
#include "skia.h"

#include <cmath>
#include <string>

namespace fiddle {
struct Example {
    void (*fFunc)(SkCanvas*);
    const char* fName;
    int fImageIndex;
    int fWidth;
    int fHeight;
    bool fText;
    bool fSRGB;
    bool fF16;
    bool fAnimation;
    bool fOffscreen;
};
}

extern GrBackendTexture backEndTexture;
extern GrBackendRenderTarget backEndRenderTarget;
extern GrBackendTexture backEndTextureRenderTarget;
extern SkBitmap source;
extern sk_sp<SkImage> image;
extern double duration; // The total duration of the animation in seconds.
extern double frame;    // A value in [0, 1] of where we are in the animation.

#define REG_FIDDLE(NAME, W, H, TEXT, I)                                            \
    namespace example_##NAME { void draw(SkCanvas*);  }                            \
    sk_tools::Registry<fiddle::Example> reg_##NAME(                                \
        fiddle::Example{&example_##NAME::draw, #NAME, I, W, H, TEXT, false, false, false, false}); \
    namespace example_##NAME

#endif  // examples_DEFINED
