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
// input SkStreamRewindable.

template SkImageDecoder_DecodeReg* SkImageDecoder_DecodeReg::gHead;

SkImageDecoder* image_decoder_from_stream(SkStreamRewindable*);

SkImageDecoder* image_decoder_from_stream(SkStreamRewindable* stream) {
    SkImageDecoder* codec = nullptr;
    const SkImageDecoder_DecodeReg* curr = SkImageDecoder_DecodeReg::Head();
    while (curr) {
        codec = curr->factory()(stream);
        // we rewind here, because we promise later when we call "decode", that
        // the stream will be at its beginning.
        bool rewindSuceeded = stream->rewind();

        // our image decoder's require that rewind is supported so we fail early
        // if we are given a stream that does not support rewinding.
        if (!rewindSuceeded) {
            SkDEBUGF(("Unable to rewind the image stream."));
            delete codec;
            return nullptr;
        }

        if (codec) {
            return codec;
        }
        curr = curr->next();
    }
    return nullptr;
}

template SkImageDecoder_FormatReg* SkImageDecoder_FormatReg::gHead;

SkImageDecoder::Format SkImageDecoder::GetStreamFormat(SkStreamRewindable* stream) {
    const SkImageDecoder_FormatReg* curr = SkImageDecoder_FormatReg::Head();
    while (curr != nullptr) {
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
