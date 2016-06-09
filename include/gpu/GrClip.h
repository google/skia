/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrClip_DEFINED
#define GrClip_DEFINED

#include "GrFragmentProcessor.h"
#include "GrTypesPriv.h"
#include "SkClipStack.h"

class GrDrawContext;
class GrPipelineBuilder;

/**
 * Produced by GrClip. It provides a set of modifications to the drawing state that are used to
 * create the final GrPipeline for a GrBatch.
 */
class GrAppliedClip : public SkNoncopyable {
public:
    GrAppliedClip() : fHasStencilClip(false) {}
    GrFragmentProcessor* getClipCoverageFragmentProcessor() const {
        return fClipCoverageFP.get();
    }
    const GrScissorState& scissorState() const { return fScissorState; }
    bool hasStencilClip() const { return fHasStencilClip; }

    void makeStencil(bool hasStencil) {
        fClipCoverageFP = nullptr;
        fScissorState.setDisabled();
        fHasStencilClip = hasStencil;
    }

    void makeScissoredStencil(bool hasStencil, const SkIRect& scissor) {
        fClipCoverageFP = nullptr;
        fScissorState.set(scissor);
        fHasStencilClip = hasStencil;
    }

    void makeFPBased(sk_sp<GrFragmentProcessor> fp) {
        fClipCoverageFP = fp;
        fScissorState.setDisabled();
        fHasStencilClip = false;
    }

    void makeScissoredFPBased(sk_sp<GrFragmentProcessor> fp, SkIRect& scissor) {
        fClipCoverageFP = fp;
        fScissorState.set(scissor);
        fHasStencilClip = false;
    }

private:
    sk_sp<GrFragmentProcessor> fClipCoverageFP;
    GrScissorState             fScissorState;
    bool                       fHasStencilClip;

    typedef SkNoncopyable INHERITED;
};

/**
 * GrClip is an abstract base class for applying a clip. It constructs a clip mask if necessary, and
 * fills out a GrAppliedClip instructing the caller on how to set up the draw state.
 */
class GrClip {
public:
    virtual bool quickContains(const SkRect&) const = 0;
    virtual void getConservativeBounds(int width, int height, SkIRect* devResult,
                                       bool* isIntersectionOfRects = nullptr) const = 0;
    virtual bool apply(GrContext*, const GrPipelineBuilder&, GrDrawContext*,
                       const SkRect* devBounds, GrAppliedClip*) const = 0;

    virtual ~GrClip() {}
};

/**
 * Specialized implementation for no clip.
 */
class GrNoClip final : public GrClip {
private:
    bool quickContains(const SkRect&) const final { return true; }
    void getConservativeBounds(int width, int height, SkIRect* devResult,
                               bool* isIntersectionOfRects) const final;
    bool apply(GrContext*, const GrPipelineBuilder&, GrDrawContext*,
               const SkRect*, GrAppliedClip*) const final { return true; }
};

/**
 * GrFixedClip is a clip that can be represented by fixed-function hardware. It never modifies the
 * stencil buffer itself, but can be configured to use whatever clip is already there.
 */
class GrFixedClip final : public GrClip {
public:
    GrFixedClip() : fHasStencilClip(false) {}
    GrFixedClip(const SkIRect& scissorRect) : fScissorState(scissorRect), fHasStencilClip(false) {}

    void reset() {
        fScissorState.setDisabled();
        fHasStencilClip = false;
    }

    void reset(const SkIRect& scissorRect) {
        fScissorState.set(scissorRect);
        fHasStencilClip = false;
    }

    void enableStencilClip(bool enable) { fHasStencilClip = enable; }

    const GrScissorState& scissorState() const { return fScissorState; }
    bool hasStencilClip() const { return fHasStencilClip; }

    bool quickContains(const SkRect&) const final;
    void getConservativeBounds(int width, int height, SkIRect* devResult,
                               bool* isIntersectionOfRects) const final;

private:
    bool apply(GrContext*, const GrPipelineBuilder&, GrDrawContext*,
               const SkRect* devBounds, GrAppliedClip* out) const final;

    GrScissorState   fScissorState;
    bool             fHasStencilClip;
};

/**
 * GrClipStackClip can apply a generic SkClipStack to the draw state. It may generate clip masks or
 * write to the stencil buffer during apply().
 */
class GrClipStackClip final : public GrClip {
public:
    GrClipStackClip(const SkClipStack* stack = nullptr, const SkIPoint* origin = nullptr) {
        this->reset(stack, origin);
    }

    void reset(const SkClipStack* stack = nullptr, const SkIPoint* origin = nullptr) {
        fOrigin = origin ? *origin : SkIPoint::Make(0, 0);
        fStack.reset(SkSafeRef(stack));
    }

    const SkIPoint& origin() const { return fOrigin; }
    const SkClipStack* clipStack() const { return fStack; }

    bool quickContains(const SkRect&) const final;
    void getConservativeBounds(int width, int height, SkIRect* devResult,
                               bool* isIntersectionOfRects) const final;
    bool apply(GrContext*, const GrPipelineBuilder&, GrDrawContext*,
               const SkRect* devBounds, GrAppliedClip*) const final;

private:
    SkIPoint                          fOrigin;
    SkAutoTUnref<const SkClipStack>   fStack;
};

#endif
