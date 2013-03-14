
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkImageEncoder_DEFINED
#define SkImageEncoder_DEFINED

#include "SkTypes.h"

class SkBitmap;
class SkWStream;

class SkImageEncoder {
public:
    enum Type {
        kJPEG_Type,
        kPNG_Type,
        kWEBP_Type
    };
    static SkImageEncoder* Create(Type);

    virtual ~SkImageEncoder();

    /*  Quality ranges from 0..100 */
    enum {
        kDefaultQuality = 80
    };

    /**
     * Encode bitmap 'bm' in the desired format, writing results to
     * file 'file', at quality level 'quality' (which can be in range
     * 0-100).
     *
     * Calls the particular implementation's onEncode() method to
     * actually do the encoding.
     */
    bool encodeFile(const char file[], const SkBitmap& bm, int quality);

    /**
     * Encode bitmap 'bm' in the desired format, writing results to
     * stream 'stream', at quality level 'quality' (which can be in
     * range 0-100).
     *
     * Calls the particular implementation's onEncode() method to
     * actually do the encoding.
     */
    bool encodeStream(SkWStream* stream, const SkBitmap& bm, int quality);

    static bool EncodeFile(const char file[], const SkBitmap&, Type,
                           int quality);
    static bool EncodeStream(SkWStream*, const SkBitmap&, Type,
                           int quality);

protected:
    /**
     * Encode bitmap 'bm' in the desired format, writing results to
     * stream 'stream', at quality level 'quality' (which can be in
     * range 0-100).
     *
     * This must be overridden by each SkImageEncoder implementation.
     */
    virtual bool onEncode(SkWStream* stream, const SkBitmap& bm, int quality) = 0;
};

// This macro declares a global (i.e., non-class owned) creation entry point
// for each encoder (e.g., CreateJPEGImageEncoder)
#define DECLARE_ENCODER_CREATOR(codec)          \
    SkImageEncoder *Create ## codec ();

// This macro defines the global creation entry point for each encoder. Each
// encoder implementation that registers with the encoder factory must call it.
#define DEFINE_ENCODER_CREATOR(codec)           \
    SkImageEncoder *Create ## codec () {        \
        return SkNEW( Sk ## codec );            \
    }

// All the encoders known by Skia. Note that, depending on the compiler settings,
// not all of these will be available
DECLARE_ENCODER_CREATOR(JPEGImageEncoder);
DECLARE_ENCODER_CREATOR(PNGImageEncoder);
DECLARE_ENCODER_CREATOR(WEBPImageEncoder);

#endif
