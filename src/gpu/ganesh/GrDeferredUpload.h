/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDeferredUpload_DEFINED
#define GrDeferredUpload_DEFINED

#include "src/gpu/AtlasTypes.h"

#include <cstddef>
#include <functional>

class GrTextureProxy;
enum class GrColorType;
struct SkIRect;

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
 * Passed to a deferred upload when it is executed, this method allows the deferred upload to
 * actually write its pixel data into a texture.
 */
using GrDeferredTextureUploadWritePixelsFn = std::function<bool(GrTextureProxy*,
                                                                SkIRect,
                                                                GrColorType srcColorType,
                                                                const void*,
                                                                size_t rowBytes)>;

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

    virtual const skgpu::TokenTracker* tokenTracker() = 0;

    /** Returns the token of the draw that this upload will occur before. */
    virtual skgpu::AtlasToken addInlineUpload(GrDeferredTextureUploadFn&&) = 0;

    /** Returns the token of the draw that this upload will occur before. Since ASAP uploads
        are done first during a flush, this will be the first token since the most recent
        flush. */
    virtual skgpu::AtlasToken addASAPUpload(GrDeferredTextureUploadFn&& upload) = 0;
};

#endif
