// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef examples_DEFINED
#define examples_DEFINED

#include "tools/Registry.h"
#include "skia.h"

#include <cmath>
#include <string>

namespace fiddle {
struct Example {
    void (*fFunc)(SkCanvas*);
    const char* fName;
    double fAnimationDuration;
    int fImageIndex;
    int fWidth;
    int fHeight;
    int fOffscreenWidth;
    int fOffscreenHeight;
    int fOffscreenSampleCount;
    bool fText;
    bool fSRGB;
    bool fF16;
    bool fOffscreen;
    bool fOffscreenTexturable;
    bool fOffscreenMipMap;
};
}

extern GrBackendTexture backEndTexture;
extern GrBackendRenderTarget backEndRenderTarget;
extern GrBackendTexture backEndTextureRenderTarget;
extern SkBitmap source;
extern sk_sp<SkImage> image;
extern double duration; // The total duration of the animation in seconds.
extern double frame;    // A value in [0, 1] of where we are in the animation.

#define REGISTER_FIDDLE(NAME, WIDTH, HEIGHT, TEXT, IMG_INDEX, DURATION, SRGB, F16,   \
                        OFSCR, OFSCR_WIDTH, OFSCR_HEIGHT, OFSCR_SAMPLECOUNT,         \
                        OFSCR_TEXTURABLE, OFSCR_MIPMAP)                              \
    namespace example_##NAME { void draw(SkCanvas*);  }                              \
    sk_tools::Registry<fiddle::Example> reg_##NAME(                                  \
        fiddle::Example{&example_##NAME::draw, #NAME, DURATION, IMG_INDEX,           \
                        WIDTH, HEIGHT, OFSCR_WIDTH, OFSCR_HEIGHT, OFSCR_SAMPLECOUNT, \
                        TEXT, SRGB, F16, OFSCR, OFSCR_TEXTURABLE, OFSCR_MIPMAP});    \
    namespace example_##NAME

#define REG_FIDDLE_SRGB(NAME, W, H, T, I, DURATION, F16)   \
    REGISTER_FIDDLE(NAME, W, H, T, I, DURATION, true, F16, \
                    false, 64, 64, 0, false, false)

#define REG_FIDDLE_ANIMATED(NAME, W, H, T, I, DURATION)       \
    REGISTER_FIDDLE(NAME, W, H, T, I, DURATION, false, false, \
                    false, 64, 64, 0, false, false)

#define REG_FIDDLE(NAME, W, H, TEXT, I)         \
    REG_FIDDLE_ANIMATED(NAME, W, H, TEXT, I, 0)

#endif  // examples_DEFINED
