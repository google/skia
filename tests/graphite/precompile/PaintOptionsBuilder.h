/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef PaintOptionsBuilder_DEFINED
#define PaintOptionsBuilder_DEFINED

#include "include/gpu/graphite/precompile/PaintOptions.h"

namespace PaintOptionsUtils {

enum ImgColorInfo {
    kAlpha,
    kAlphaSRGB,
    kPremul,
    kSRGB,
};

enum ImgTileModeOptions {
    kNone,
    kClamp,
    kRepeat,
};

// This enum directly maps to YUVImageShaderFlags but, crucially, is more compact.
enum YUVSamplingOptions {
    kNoCubic,         // YUVImageShaderFlags::kExcludeCubic
    kHWAndShader,     // YUVImageShaderFlags::kNoCubicNoNonSwizzledHW
};

enum LinearGradientOptions {
    kSmall,
    kComplex,  // idiosyncratic case - c.f. Builder::linearGrad
};

// This is a minimal builder object that allows for compact construction of the most common
// PaintOptions combinations - eliminating a lot of boilerplate.
class Builder {
public:
    Builder() {}

    // Shaders
    Builder& hwImg(ImgColorInfo ci, ImgTileModeOptions tmOptions = kNone);
    Builder& yuv(YUVSamplingOptions options);
    Builder& linearGrad(LinearGradientOptions options);
    Builder& blend();

    // ColorFilters
    Builder& matrixCF();
    Builder& porterDuffCF();

    // Blendmodes
    Builder& clear()   { return this->addBlendMode(SkBlendMode::kClear);   }
    Builder& dstIn()   { return this->addBlendMode(SkBlendMode::kDstIn);   }
    Builder& src()     { return this->addBlendMode(SkBlendMode::kSrc);     }
    Builder& srcOver() { return this->addBlendMode(SkBlendMode::kSrcOver); }

    // Misc settings
    Builder& transparent() { fPaintOptions.setPaintColorIsOpaque(false); return *this; }
    Builder& dither()      { fPaintOptions.setDither(true);              return *this; }

    operator skgpu::graphite::PaintOptions() const { return fPaintOptions; }

private:
    skgpu::graphite::PaintOptions fPaintOptions;

    Builder& addBlendMode(SkBlendMode bm) {
        fPaintOptions.addBlendMode(bm);
        return *this;
    }
};

} // namespace PaintOptionsUtils

#endif // PaintOptionsBuilder_DEFINED
