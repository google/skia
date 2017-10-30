/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDeferredUpload_DEFINED
#define GrDeferredUpload_DEFINED

#include <functional>
#include "GrTypes.h"

class GrTextureProxy;

/**
 * GrDeferredUploadToken is used to sequence the uploads relative to each other and to draws.
 */
class GrDeferredUploadToken {
public:
    static GrDeferredUploadToken AlreadyFlushedToken() { return GrDeferredUploadToken(0); }

    GrDeferredUploadToken(const GrDeferredUploadToken&) = default;
    GrDeferredUploadToken& operator=(const GrDeferredUploadToken&) = default;
    bool operator==(const GrDeferredUploadToken& that) const {
        return fSequenceNumber == that.fSequenceNumber;
    }
    bool operator!=(const GrDeferredUploadToken& that) const { return !(*this == that); }
    bool inInterval(const GrDeferredUploadToken& start, const GrDeferredUploadToken& finish) {
        return fSequenceNumber >= start.fSequenceNumber &&
               fSequenceNumber <= finish.fSequenceNumber;
    }

private:
    GrDeferredUploadToken();
    explicit GrDeferredUploadToken(uint64_t sequenceNumber) : fSequenceNumber(sequenceNumber) {}
    friend class GrOpFlushState;
    uint64_t fSequenceNumber;
};

/**
 * Passed to a deferred upload when it is executed, this method allows the deferred upload to
 * actually write its pixel data into a texture.
 */
using GrDeferredTextureUploadWritePixelsFn =
        std::function<bool(GrTextureProxy*, int left, int top, int width, int height,
                           GrPixelConfig config, const void* buffer, size_t rowBytes)>;

/**
 * A deferred texture upload is simply a std::function that takes a
 * GrDeferredTextureUploadWritePixelsFn as a parameter. It is called when it should perform its
 * upload as the draw/upload sequence is executed.
 */
using GrDeferredTextureUploadFn = std::function<void(GrDeferredTextureUploadWritePixelsFn&)>;

#endif
