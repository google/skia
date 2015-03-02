/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCodec_DEFINED
#define SkCodec_DEFINED

#include "SkImageGenerator.h"
#include "SkImageInfo.h"
#include "SkSize.h"
#include "SkTemplates.h"
#include "SkTypes.h"

class SkData;
class SkStream;

/**
 *  Abstraction layer directly on top of an image codec.
 */
class SkCodec : public SkImageGenerator {
public:
    /**
     *  If this stream represents an encoded image that we know how to decode,
     *  return an SkCodec that can decode it. Otherwise return NULL.
     *
     *  If NULL is returned, the stream is deleted immediately. Otherwise, the
     *  SkCodec takes ownership of it, and will delete it when done with it.
     */
    static SkCodec* NewFromStream(SkStream*);

    /**
     *  If this data represents an encoded image that we know how to decode,
     *  return an SkCodec that can decode it. Otherwise return NULL.
     *
     *  Will take a ref if it returns a codec, else will not affect the data.
     */
    static SkCodec* NewFromData(SkData*);

    /**
     *  Return a size that approximately supports the desired scale factor.
     *  The codec may not be able to scale efficiently to the exact scale
     *  factor requested, so return a size that approximates that scale.
     *
     *  FIXME: Move to SkImageGenerator?
     */
    SkISize getScaledDimensions(float desiredScale) const;

protected:
    SkCodec(const SkImageInfo&, SkStream*);

    /**
     *  The SkAlphaType is a conservative answer. i.e. it is possible that it
     *  initially returns a non-opaque answer, but completing the decode
     *  reveals that the image is actually opaque.
     */
    bool onGetInfo(SkImageInfo* info) SK_OVERRIDE {
        *info = fInfo;
        return true;
    }

    // Helper for subclasses.
    const SkImageInfo& getOriginalInfo() { return fInfo; }

    virtual SkISize onGetScaledDimensions(float /* desiredScale */) const {
        // By default, scaling is not supported.
        return fInfo.dimensions();
    }

    /**
     *  If the stream was previously read, attempt to rewind.
     *  @returns:
     *      true
     *       - if the stream needed to be rewound, and the rewind
     *         succeeded.
     *       - if the stream did not need to be rewound.
     *      false
     *       - if the stream needed to be rewound, and rewind failed.
     *  Subclasses MUST call this function before reading the stream (e.g. in
     *  onGetPixels). If it returns false, onGetPixels should return
     *  kCouldNotRewind.
     */
    bool SK_WARN_UNUSED_RESULT rewindIfNeeded();

private:
    const SkImageInfo fInfo;
    SkAutoTDelete<SkStream> fStream;
    bool fNeedsRewind;
};
#endif // SkCodec_DEFINED
