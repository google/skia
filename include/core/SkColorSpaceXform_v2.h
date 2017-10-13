/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorSpaceXform_v2_DEFINED
#define SkColorSpaceXform_v2_DEFINED

#include "SkColorSpace.h"
#include "../jumper/SkJumper.h"

class SkColorSpaceXform_v2 {
public:

    // The same formats as SkColorType, plus a couple more,
    // and with some names tweaked to more accurately reflect memory layout.
    enum class ColorType {
        Alpha_8,     // 8-bit alpha
        Gray_8,      // 8-bit gray

        BGR_565,     // 5-bit blue, 6-bit green, 5-bit red
        ABGR_4444,   // 4-bit alpha, blue, green, red

        RGBA_8888,   // 8-bit red, green, blue, alpha
        BGRA_8888,   // 8-bit blue, green, red, alpha

        RGBA_F16,    // half-precision float red, green, blue, alpha
        RGBA_F32,    // single-precision float red, green, blue, alpha

        RGB_BE16,    // big-endian 16-bit red, green, blue                     ( source only )
        RGBA_BE16,   // big-endian 16-bit red, green, blue, alpha              ( source only )

        CMYK_8888,   // 8-bit _inverse_ cyan,magenta,yellow,key (black)        ( source only )
    };

    // The same formats as SkAlphaType, with premul a bit more finely sliced.
    enum class AlphaType {
        Unpremul,        // Pixels are not premultiplied by alpha:                 a, tf(r)   ...
        LinearPremul,    // Pixels are premultiplied before the transfer function: a, tf(r*a) ...
        NonlinearPremul, // Pixels are premultiplied after the transfer function:  a, tf(r)*a ...
        Opaque,          // We will ignore any alpha value found in the source,
                         // and always write opaque pixels to the destination.
    };

    // If you're doing a one-off transform, you can just call this function.
    static bool Apply(SkColorSpace* dstCS, ColorType dstCT, AlphaType dstAT,
                      SkColorSpace* srcCS, ColorType srcCT, AlphaType srcAT,
                      void* dst,       size_t dstRB,
                      const void* src, size_t srcRB,
                      int w, int h);

    // It's worth holding onto an object if you're doing multiple transforms.
    SkColorSpaceXform_v2(SkColorSpace* dstCS, ColorType dstCT, AlphaType dstAT,
                         SkColorSpace* srcCS, ColorType srcCT, AlphaType srcAT);
    bool operator()(void*       dst, size_t dstRB,
                    const void* src, size_t srcRB,
                    int w, int h);

private:
    sk_sp<SkColorSpace>    fDstCS, fSrcCS;
    SkColorSpaceTransferFn fDstTF[3], fSrcTF[4];  // Usually one each for R,G,B, but CMYK needs 4.
    float                  fGamutTransform[12];   // 3x4 column-major matrix
    SkJumper_MemoryCtx     fDst, fSrc;
    std::function<void(size_t,size_t,size_t,size_t)> fRun;
};

#endif//SkColorSpaceXform_v2_DEFINED
