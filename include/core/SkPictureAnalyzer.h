/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPictureAnalyzer_DEFINED
#define SkPictureAnalyzer_DEFINED

#include "SkRefCnt.h"
#include "SkRegion.h"
#include "SkTypes.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"

class SkPath;
class SkPicture;

/** \class SkPictureGpuAnalyzer

     Gathers GPU-related statistics for one or more SkPictures.
*/
class SK_API SkPictureGpuAnalyzer final : public SkNoncopyable {
public:
    explicit SkPictureGpuAnalyzer(sk_sp<GrContextThreadSafeProxy> = nullptr);
    explicit SkPictureGpuAnalyzer(const sk_sp<SkPicture>& picture,
                                  sk_sp<GrContextThreadSafeProxy> = nullptr);

    /**
     *  Process the given picture and accumulate its stats.
     */
    void analyzePicture(const SkPicture*);

    /**
     *  Process an explicit clipPath op.
     */
    void analyzeClipPath(const SkPath&, SkRegion::Op, bool doAntiAlias);

    /**
     *  Reset all accumulated stats.
     */
    void reset();

    /**
     *  Returns true if the analyzed pictures are suitable for rendering on the GPU.
     */
    bool suitableForGpuRasterization(const char** whyNot = nullptr) const;

    /**
     * Returns the number of commands which are slow to draw on the GPU, capped at the predicate
     * max.
     */
    uint32_t numSlowGpuCommands() { return fNumSlowPaths; }

private:
    uint32_t fNumSlowPaths;

    typedef SkNoncopyable INHERITED;
};

#endif // SK_SUPPORT_GPU

#endif // SkPictureAnalyzer_DEFINED
