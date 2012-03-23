
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
        kPNG_Type
    };
    static SkImageEncoder* Create(Type);

    virtual ~SkImageEncoder();
    
    /*  Quality ranges from 0..100 */
    enum {
        kDefaultQuality = 80
    };

    bool encodeFile(const char file[], const SkBitmap&, int quality);
    bool encodeStream(SkWStream*, const SkBitmap&, int quality);

    static bool EncodeFile(const char file[], const SkBitmap&, Type,
                           int quality);
    static bool EncodeStream(SkWStream*, const SkBitmap&, Type,
                           int quality);

protected:
    virtual bool onEncode(SkWStream*, const SkBitmap&, int quality) = 0;
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

#endif
