/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImageEncoderPriv_DEFINED
#define SkImageEncoderPriv_DEFINED

#include "SkImageEncoder.h"

#ifndef SK_SUPPORT_LEGACY_IMAGE_ENCODER_CLASS

#include "SkTRegistry.h"

class SkImageEncoder {
public:
    typedef SkEncodedImageFormat Type;
    static SkImageEncoder* Create(SkEncodedImageFormat);

    virtual ~SkImageEncoder() {}

    /*  Quality ranges from 0..100 */
    enum {
        kDefaultQuality = 80
    };

    bool encodeStream(SkWStream* dst, const SkBitmap& src, int quality) {
        return this->onEncode(dst, src, SkMin32(100, SkMax32(0, quality)));
    }

protected:
    /**
     * Encode bitmap 'bm' in the desired format, writing results to
     * stream 'stream', at quality level 'quality' (which can be in
     * range 0-100).
     */
    virtual bool onEncode(SkWStream* stream, const SkBitmap& bm, int quality) = 0;
};

// This macro declares a global (i.e., non-class owned) creation entry point
// for each encoder (e.g., CreateJPEGImageEncoder)
#define DECLARE_ENCODER_CREATOR(codec)          \
    SK_API SkImageEncoder *Create ## codec ();

// This macro defines the global creation entry point for each encoder. Each
// encoder implementation that registers with the encoder factory must call it.
#define DEFINE_ENCODER_CREATOR(codec) \
    SkImageEncoder* Create##codec() { return new Sk##codec; }

// All the encoders known by Skia. Note that, depending on the compiler settings,
// not all of these will be available
DECLARE_ENCODER_CREATOR(JPEGImageEncoder);
DECLARE_ENCODER_CREATOR(PNGImageEncoder);
DECLARE_ENCODER_CREATOR(KTXImageEncoder);
DECLARE_ENCODER_CREATOR(WEBPImageEncoder);

#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
SkImageEncoder* CreateImageEncoder_CG(SkEncodedImageFormat type);
#endif

#if defined(SK_BUILD_FOR_WIN)
SkImageEncoder* CreateImageEncoder_WIC(SkEncodedImageFormat type);
#endif

// Typedef to make registering encoder callback easier
// This has to be defined outside SkImageEncoder. :(
typedef SkTRegistry<SkImageEncoder*(*)(SkEncodedImageFormat)> SkImageEncoder_EncodeReg;

#endif  // SK_SUPPORT_LEGACY_IMAGE_ENCODER_CLASS

#endif // SkImageEncoderPriv_DEFINED
