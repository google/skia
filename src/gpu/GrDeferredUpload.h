/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDeferredUpload_DEFINED
#define GrDeferredUpload_DEFINED

#include <functional>
#include "include/gpu/GrTypes.h"
#include "include/private/GrTypesPriv.h"

class GrTextureProxy;

/**
 * A word about deferred uploads and tokens: Ops should usually schedule their uploads to occur at
 * the beginning of a frame whenever possible. These are called ASAP uploads. Of course, this
 * requires that there are no draws that have yet to be flushed that rely on the old texture
 * contents. In that case the ASAP upload would happen prior to the draw and therefore the draw
 * would read the new (wrong) texture data. When this read-before-write data hazard exists they
 * should schedule an inline upload.
 *
 * Ops, in conjunction with helpers such as GrDrawOpAtlas, use upload tokens to know what the most
 * recent draw was that referenced a resource (or portion of a resource). Each draw is assigned a
 * token. A resource (or portion thereof) can be tagged with the most recent reading draw's token.
 * The deferred upload's target provides a facility for testing whether the draw corresponding to
 * the token has been flushed. If it has not been flushed then the op must perform an inline upload
 * instead so that the upload occurs after the draw depending on the old contents and before the
 * draw depending on the updated contents. When scheduling an inline upload the op provides the
 * token of the draw that the upload must occur before.
 */

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
    bool operator<(const GrDeferredUploadToken that) const {
        return fSequenceNumber < that.fSequenceNumber;
    }
    bool operator<=(const GrDeferredUploadToken that) const {
        return fSequenceNumber <= that.fSequenceNumber;
    }
    bool operator>(const GrDeferredUploadToken that) const {
        return fSequenceNumber > that.fSequenceNumber;
    }
    bool operator>=(const GrDeferredUploadToken that) const {
        return fSequenceNumber >= that.fSequenceNumber;
    }

    GrDeferredUploadToken& operator++() {
        ++fSequenceNumber;
        return *this;
    }
    GrDeferredUploadToken operator++(int) {
        auto old = fSequenceNumber;
        ++fSequenceNumber;
        return GrDeferredUploadToken(old);
    }

    GrDeferredUploadToken next() const { return GrDeferredUploadToken(fSequenceNumber + 1); }

    /** Is this token in the [start, end] inclusive interval? */
    bool inInterval(const GrDeferredUploadToken& start, const GrDeferredUploadToken& end) {
        return *this >= start && *this <= end;
    }

private:
    GrDeferredUploadToken() = delete;
    explicit GrDeferredUploadToken(uint64_t sequenceNumber) : fSequenceNumber(sequenceNumber) {}
    uint64_t fSequenceNumber;
};

/*
 * The GrTokenTracker encapsulates the incrementing and distribution of tokens.
 */
class GrTokenTracker {
public:
    /** Gets the token one beyond the last token that has been flushed. */
    GrDeferredUploadToken nextTokenToFlush() const { return fLastFlushedToken.next(); }

    /** Gets the next draw token that will be issued by this target. This can be used by an op
        to record that the next draw it issues will use a resource (e.g. texture) while preparing
        that draw. */
    GrDeferredUploadToken nextDrawToken() const { return fLastIssuedToken.next(); }

private:
    // Only these three classes get to increment the token counters
    friend class SkInternalAtlasTextContext;
    friend class GrOpFlushState;
    friend class TestingUploadTarget;

    /** Issues the next token for a draw. */
    GrDeferredUploadToken issueDrawToken() { return ++fLastIssuedToken; }

    /** Advances the last flushed token by one. */
    GrDeferredUploadToken flushToken() { return ++fLastFlushedToken; }

    GrDeferredUploadToken fLastIssuedToken = GrDeferredUploadToken::AlreadyFlushedToken();
    GrDeferredUploadToken fLastFlushedToken = GrDeferredUploadToken::AlreadyFlushedToken();
};

/**
 * Passed to a deferred upload when it is executed, this method allows the deferred upload to
 * actually write its pixel data into a texture.
 */
using GrDeferredTextureUploadWritePixelsFn =
        std::function<bool(GrTextureProxy*, int left, int top, int width, int height,
                           GrColorType srcColorType, const void* buffer, size_t rowBytes)>;

/**
 * A deferred texture upload is simply a std::function that takes a
 * GrDeferredTextureUploadWritePixelsFn as a parameter. It is called when it should perform its
 * upload as the draw/upload sequence is executed.
 */
using GrDeferredTextureUploadFn = std::function<void(GrDeferredTextureUploadWritePixelsFn&)>;

/**
 * An interface for scheduling deferred uploads. It accepts asap and deferred inline uploads.
 */
class GrDeferredUploadTarget {
public:
    virtual ~GrDeferredUploadTarget() {}

    virtual const GrTokenTracker* tokenTracker() = 0;

    /** Returns the token of the draw that this upload will occur before. */
    virtual GrDeferredUploadToken addInlineUpload(GrDeferredTextureUploadFn&&) = 0;

    /** Returns the token of the draw that this upload will occur before. Since ASAP uploads
        are done first during a flush, this will be the first token since the most recent
        flush. */
    virtual GrDeferredUploadToken addASAPUpload(GrDeferredTextureUploadFn&& upload) = 0;
};

#endif
