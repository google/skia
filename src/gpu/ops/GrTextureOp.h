/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GrTextureOp_DEFINED
#define GrTextureOp_DEFINED

#include "include/core/SkCanvas.h"
#include "include/core/SkRefCnt.h"
#include "include/private/GrTypesPriv.h"
#include "src/gpu/GrColor.h"
#include "src/gpu/GrSamplerState.h"

class GrClip;
class GrColorSpaceXform;
class GrDrawOp;
class GrRenderTargetContext;
struct SkRect;
class SkMatrix;

class GrDeferredTextureOp {
public:
    /**
     * Controls whether saturate() is called after the texture is color-converted to ensure all
     * color values are in 0..1 range.
     */
    enum class Saturate : bool { kNo = false, kYes = true };

    GrDeferredTextureOp() : fExpected(0) {}
    ~GrDeferredTextureOp() {
        // The deferred op should have been finalized before this is destroyed.
        // FIXME image filters triggers this, i'm betting I'm not finalizing the op everywhere it needs to
        SkASSERT(!fOp);
    }

    // Specify the clip to use for subsequent appends, and provide an optional hint as to the
    // number of appends. May finalize prior appended draws if the new clip stack is not the same
    // as the previous one. If the clips are the same, this does nothing.
    bool open(GrRenderTargetContext*, const GrClip&, GrAAType, Saturate, int expectedCount = 1);

    // Will use the clip that was specified in open(); open() must have been called first.
    // May finalize already appended draws into an op if the new surface proxy view is incompatible.
    void append(GrRenderTargetContext*,
                GrSurfaceProxyView, SkAlphaType srcAlphaType,
                sk_sp<GrColorSpaceXform>, GrSamplerState::Filter,
                const SkPMColor4f&, DrawQuad*, const SkRect* domain = nullptr);

    // Finalize the accumulate draws and add the op to the context. This does nothing if there
    // are no accumulated draws.
    void finalizeAndSubmit(GrRenderTargetContext* rtc) {
        this->submit(rtc, true);
    }

    bool needsToSubmit() const { return !!fOp; }

#if GR_TEST_UTILS
    static uint32_t ClassID();
#endif

private:
    int fExpected;

    GrAppliedClip fClip;
    SkRect        fClipBounds;

    GrAAType      fAAType;
    Saturate      fSaturate;

    // Has not been finalized yet, nor added to a GrRTC
    std::unique_ptr<GrDrawOp> fOp;

    void submit(GrRenderTargetContext*, bool close);
};

#endif  // GrTextureOp_DEFINED
