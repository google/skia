
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

typedef SkTRegistry<SkImageDecoder*, SkStream*> DecodeReg;

// N.B. You can't use "DecodeReg::gHead here" due to complex C++
// corner cases.
template DecodeReg* SkTRegistry<SkImageDecoder*, SkStream*>::gHead;

SkImageDecoder* SkImageDecoder::Factory(SkStream* stream) {
    SkImageDecoder* codec = NULL;
    const DecodeReg* curr = DecodeReg::Head();
    while (curr) {
        codec = curr->factory()(stream);
        // we rewind here, because we promise later when we call "decode", that
        // the stream will be at its beginning.
        bool rewindSuceeded = stream->rewind();

        // our image decoder's require that rewind is supported so we fail early
        // if we are given a stream that does not support rewinding.
        if (!rewindSuceeded) {
            SkDEBUGF(("Unable to rewind the image stream."));
            SkDELETE(codec);
            return NULL;
        }

        if (codec) {
            return codec;
        }
        curr = curr->next();
    }
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
