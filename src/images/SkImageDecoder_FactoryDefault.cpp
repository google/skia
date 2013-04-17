
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

extern SkImageDecoder* image_decoder_from_stream(SkStream*);

SkImageDecoder* SkImageDecoder::Factory(SkStream* stream) {
    return image_decoder_from_stream(stream);
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
