/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPathRange_DEFINED
#define GrPathRange_DEFINED

#include "GrGpuResource.h"
#include "GrResourceCache.h"
#include "SkStrokeRec.h"

class SkPath;

/**
 * Represents a contiguous range of GPU path objects with a common stroke. The
 * path range is immutable with the exception that individual paths can be
 * initialized lazily. Unititialized paths are silently ignored by drawing
 * functions.
 */
class GrPathRange : public GrGpuResource {
public:
    SK_DECLARE_INST_COUNT(GrPathRange);

    static const bool kIsWrapped = false;

    static GrResourceKey::ResourceType resourceType() {
        static const GrResourceKey::ResourceType type = GrResourceKey::GenerateResourceType();
        return type;
    }

    /**
     * Initialize to a range with a fixed size and stroke. Stroke must not be hairline.
     */
    GrPathRange(GrGpu* gpu, size_t size, const SkStrokeRec& stroke)
        : INHERITED(gpu, kIsWrapped),
          fSize(size),
          fStroke(stroke) {
    }

    size_t getSize() const { return fSize; }
    const SkStrokeRec& getStroke() const { return fStroke; }

    /**
     * Initialize a path in the range. It is invalid to call this method for a
     * path that has already been initialized.
     */
    virtual void initAt(size_t index, const SkPath&) = 0;

protected:
    size_t fSize;
    SkStrokeRec fStroke;

private:
    typedef GrGpuResource INHERITED;
};

#endif
