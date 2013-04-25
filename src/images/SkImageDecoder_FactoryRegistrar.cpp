/*
 * Copyright 2013 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkErrorInternals.h"
#include "SkImageDecoder.h"
#include "SkStream.h"
#include "SkTRegistry.h"

// This file is used for registration of SkImageDecoders. It also holds a function
// for checking all the the registered SkImageDecoders for one that matches an
// input SkStream.

typedef SkTRegistry<SkImageDecoder*, SkStream*> DecodeReg;

// N.B. You can't use "DecodeReg::gHead here" due to complex C++
// corner cases.
template DecodeReg* SkTRegistry<SkImageDecoder*, SkStream*>::gHead;

SkImageDecoder* image_decoder_from_stream(SkStream*);

SkImageDecoder* image_decoder_from_stream(SkStream* stream) {
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

typedef SkTRegistry<SkImageDecoder::Format, SkStream*> FormatReg;

template FormatReg* SkTRegistry<SkImageDecoder::Format, SkStream*>::gHead;

SkImageDecoder::Format SkImageDecoder::GetStreamFormat(SkStream* stream) {
    const FormatReg* curr = FormatReg::Head();
    while (curr != NULL) {
        Format format = curr->factory()(stream);
        if (!stream->rewind()) {
            SkErrorInternals::SetError(kInvalidOperation_SkError,
                                       "Unable to rewind the image stream\n");
            return kUnknown_Format;
        }
        if (format != kUnknown_Format) {
            return format;
        }
        curr = curr->next();
    }
    return kUnknown_Format;
}
