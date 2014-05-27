/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDecodingImageGenerator_DEFINED
#define SkDecodingImageGenerator_DEFINED

#include "SkBitmap.h"
#include "SkImageGenerator.h"

class SkData;
class SkStreamRewindable;

/**
 *  An implementation of SkImageGenerator that calls into
 *  SkImageDecoder.
 */
namespace SkDecodingImageGenerator {
    /**
     *  These options will be passed on to the image decoder.  The
     *  defaults are sensible.
     *
     *  @param fSampleSize If set to > 1, tells the decoder to return a
     *         smaller than original bitmap, sampling 1 pixel for
     *         every size pixels. e.g. if sample size is set to 3,
     *         then the returned bitmap will be 1/3 as wide and high,
     *         and will contain 1/9 as many pixels as the original.
     *         Note: this is a hint, and the codec may choose to
     *         ignore this, or only approximate the sample size.
     *
     *  @param fDitherImage Set to true if the the decoder should try to
     *         dither the resulting image when decoding to a smaller
     *         color-space.  The default is true.
     *
     *  @param fRequestedColorType If not given, then use whichever
     *         config the decoder wants.  Else try to use this color
     *         type.  If the decoder won't support this color type,
     *         SkDecodingImageGenerator::Create will return
     *         NULL. kIndex_8_SkColorType is not supported.
     *
     *  @param fRequireUnpremul If true, the decoder will attempt to
     *         decode without premultiplying the alpha. If it cannot,
     *         the pixels will be set to NULL.
     */
    struct Options {
        Options()
            : fSampleSize(1)
            , fDitherImage(true)
            , fUseRequestedColorType(false)
            , fRequestedColorType()
            , fRequireUnpremul(false) { }
        Options(int sampleSize, bool dither)
            : fSampleSize(sampleSize)
            , fDitherImage(dither)
            , fUseRequestedColorType(false)
            , fRequestedColorType()
            , fRequireUnpremul(false) { }
        Options(int sampleSize, bool dither, SkColorType colorType)
            : fSampleSize(sampleSize)
            , fDitherImage(dither)
            , fUseRequestedColorType(true)
            , fRequestedColorType(colorType)
            , fRequireUnpremul(false) { }
         Options(int sampleSize, bool dither, SkColorType colorType,
                 bool requireUnpremul)
            : fSampleSize(sampleSize)
            , fDitherImage(dither)
            , fUseRequestedColorType(true)
            , fRequestedColorType(colorType)
            , fRequireUnpremul(requireUnpremul) { }
        const int         fSampleSize;
        const bool        fDitherImage;
        const bool        fUseRequestedColorType;
        const SkColorType fRequestedColorType;
        const bool        fRequireUnpremul;
    };

    /**
     *  These two functions return a SkImageGenerator that calls into
     *  SkImageDecoder.  They return NULL on failure.
     *
     *  The SkData version of this function is preferred.  If the stream
     *  has an underlying SkData (such as a SkMemoryStream) pass that in.
     *
     *  This object will unref the stream when done or on failure.  Since
     *  streams have internal state (position), the caller should not pass
     *  a shared stream in.  Pass either a new duplicated stream in or
     *  transfer ownership of the stream.  This factory asserts
     *  stream->unique().
     *
     *  For example:
     *    SkStreamRewindable* stream;
     *    ...
     *    SkImageGenerator* gen
     *        = SkDecodingImageGenerator::Create(
     *            stream->duplicate(), SkDecodingImageGenerator::Options());
     *    ...
     *    SkDELETE(gen);
     *
     *  @param Options (see above)
     *
     *  @return NULL on failure, a new SkImageGenerator on success.
     */
    SkImageGenerator* Create(SkStreamRewindable* stream,
                             const Options& opt);

    /**
     *  @param data Contains the encoded image data that will be used by
     *         the SkDecodingImageGenerator.  Will be ref()ed by the
     *         SkImageGenerator constructor and and unref()ed on deletion.
     */
    SkImageGenerator* Create(SkData* data, const Options& opt);
};

//  // Example of most basic use case:
//
//  bool install_data(SkData* data, SkBitmap* dst) {
//     return SkInstallDiscardablePixelRef(
//         SkDecodingImageGenerator::Create(
//             data, SkDecodingImageGenerator::Options()), dst, NULL);
//  }
//  bool install_stream(SkStreamRewindable* stream, SkBitmap* dst) {
//     return SkInstallDiscardablePixelRef(
//         SkDecodingImageGenerator::Create(
//             stream, SkDecodingImageGenerator::Options()), dst, NULL);
//  }

#endif  // SkDecodingImageGenerator_DEFINED
