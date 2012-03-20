
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkImageDecoder.h"
#include "SkMovie.h"
#include "SkStream.h"
#include "SkTRegistry.h"

//extern SkImageDecoder* sk_libbmp_dfactory(SkStream*);
//extern SkImageDecoder* sk_libgif_dfactory(SkStream*);
//extern SkImageDecoder* sk_libico_dfactory(SkStream*);
extern SkImageDecoder* sk_libjpeg_dfactory(SkStream*);
//extern SkImageDecoder* sk_libpng_dfactory(SkStream*);
//extern SkImageDecoder* sk_wbmp_dfactory(SkStream*);

// To get the various image decoding classes to register themselves
// pre-main we need to ensure they are linked into the application.
// Ultimately we need to move to using DLLs rather than tightly
// coupling the factory with the file format classes.
void ForceLinking()
{
    SkImageDecoder* codec = NULL;

    // TODO: rather than force the linking here expose a
    // "Sk*ImageDecoderCreate" function for each codec
    // and let the app add these calls to force the linking.
    // Besides decoupling the codecs from the factory this
    // will also give the app the ability to circumvent the
    // factory and explicitly create a decoder w/o reaching
    // into Skia's guts

//    codec = sk_libbmp_dfactory(NULL);
//    codec = sk_libgif_dfactory(NULL);
//    codec = sk_libico_dfactory(NULL);
    codec = sk_libjpeg_dfactory(NULL);
//    codec = sk_libpng_dfactory(NULL);
//    codec = sk_wbmp_dfactory(NULL);
}

typedef SkTRegistry<SkImageDecoder*, SkStream*> DecodeReg;

// N.B. You can't use "DecodeReg::gHead here" due to complex C++
// corner cases.
template DecodeReg* SkTRegistry<SkImageDecoder*, SkStream*>::gHead;

#ifdef SK_ENABLE_LIBPNG
    extern SkImageDecoder* sk_libpng_dfactory(SkStream*);
#endif

SkImageDecoder* SkImageDecoder::Factory(SkStream* stream) {
    SkImageDecoder* codec = NULL;
    const DecodeReg* curr = DecodeReg::Head();
    while (curr) {
        codec = curr->factory()(stream);
        // we rewind here, because we promise later when we call "decode", that
        // the stream will be at its beginning.
        stream->rewind();
        if (codec) {
            return codec;
        }
        curr = curr->next();
    }
#ifdef SK_ENABLE_LIBPNG
    codec = sk_libpng_dfactory(stream);
    stream->rewind();
    if (codec) {
        return codec;
    }
#endif
    return NULL;
}

/////////////////////////////////////////////////////////////////////////

typedef SkTRegistry<SkMovie*, SkStream*> MovieReg;

SkMovie* SkMovie::DecodeStream(SkStream* stream) {
    const MovieReg* curr = MovieReg::Head();
    while (curr) {
        SkMovie* movie = curr->factory()(stream);
        if (movie) {
            return movie;
        }
        // we must rewind only if we got NULL, since we gave the stream to the
        // movie, who may have already started reading from it
        stream->rewind();
        curr = curr->next();
    }
    return NULL;
}
