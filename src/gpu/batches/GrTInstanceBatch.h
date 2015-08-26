/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTInstanceBatch_DEFINED
#define GrTInstanceBatch_DEFINED

#include "GrVertexBatch.h"

#include "GrBatchFlushState.h"

/**
 * GrTInstanceBatch is an optional template to help with writing batches
 * To use this template, The 'Impl' must define the following statics:
 *     A Geometry struct
 *
 *     static const int kVertsPerInstance
 *     static const int kIndicesPerInstance
 *
 *     const char* Name()
 *
 *     void InvariantOutputCoverage(GrInitInvariantOutput* out)
 *
 *     void SetBounds(const Geometry& seedGeometry, SkRect* outBounds)
 *
 *     bool CanCombine(const Geometry& mine, const Geometry& theirs,
 *                     const GrPipelineOptimizations&)
 *
 *     const GrGeometryProcessor* CreateGP(const Geometry& seedGeometry,
 *                                         const GrPipelineOptimizations& opts)
 *
 *     const GrIndexBuffer* GetIndexBuffer(GrResourceProvider*)
 *
 *     Tesselate(intptr_t vertices, size_t vertexStride, const Geometry& geo,
 *               const GrPipelineOptimizations& opts)
 */
template <typename Impl>
class GrTInstanceBatch : public GrVertexBatch {
public:
    typedef typename Impl::Geometry Geometry;

    static GrTInstanceBatch* Create() { return new GrTInstanceBatch; }

    const char* name() const override { return Impl::Name(); }

    void getInvariantOutputColor(GrInitInvariantOutput* out) const override {
        // When this is called on a batch, there is only one geometry bundle
        out->setKnownFourComponents(fGeoData[0].fColor);
    }

    void getInvariantOutputCoverage(GrInitInvariantOutput* out) const override {
        Impl::InitInvariantOutputCoverage(out);
    }

    void initBatchTracker(const GrPipelineOptimizations& opt) override {
        opt.getOverrideColorIfSet(&fGeoData[0].fColor);
        fOpts = opt;
    }

    SkSTArray<1, Geometry, true>* geoData() { return &fGeoData; }

    // to avoid even the initial copy of the struct, we have a getter for the first item which
    // is used to seed the batch with its initial geometry.  After seeding, the client should call
    // init() so the Batch can initialize itself
    Geometry* geometry() { return &fGeoData[0]; }
    void init() {
        const Geometry& geo = fGeoData[0];
        Impl::SetBounds(geo, &fBounds);
    }

private:
    GrTInstanceBatch() {
        this->initClassID<GrTInstanceBatch<Impl>>();

        // Push back an initial geometry
        fGeoData.push_back();
    }

    void onPrepareDraws(Target* target) override {
        SkAutoTUnref<const GrGeometryProcessor> gp(Impl::CreateGP(this->seedGeometry(), fOpts));
        if (!gp) {
            SkDebugf("Couldn't create GrGeometryProcessor\n");
            return;
        }

        target->initDraw(gp, this->pipeline());

        size_t vertexStride = gp->getVertexStride();
        int instanceCount = fGeoData.count();

        SkAutoTUnref<const GrIndexBuffer> indexBuffer(
                Impl::GetIndexBuffer(target->resourceProvider()));
        InstancedHelper helper;
        void* vertices = helper.init(target, kTriangles_GrPrimitiveType, vertexStride,
                                     indexBuffer, Impl::kVertsPerInstance,
                                     Impl::kIndicesPerInstance, instanceCount);
        if (!vertices || !indexBuffer) {
            SkDebugf("Could not allocate vertices\n");
            return;
        }

        for (int i = 0; i < instanceCount; i++) {
            intptr_t verts = reinterpret_cast<intptr_t>(vertices) +
                             i * Impl::kVertsPerInstance * vertexStride;
            Impl::Tesselate(verts, vertexStride, fGeoData[i], fOpts);
        }
        helper.recordDraw(target);
    }

    const Geometry& seedGeometry() const { return fGeoData[0]; }

    bool onCombineIfPossible(GrBatch* t, const GrCaps& caps) override {
        GrTInstanceBatch* that = t->cast<GrTInstanceBatch>();
        if (!GrPipeline::CanCombine(*this->pipeline(), this->bounds(), *that->pipeline(),
                                    that->bounds(), caps)) {
            return false;
        }

        if (!Impl::CanCombine(this->seedGeometry(), that->seedGeometry(), fOpts)) {
            return false;
        }

        // In the event of two batches, one who can tweak, one who cannot, we just fall back to
        // not tweaking
        if (fOpts.canTweakAlphaForCoverage() && !that->fOpts.canTweakAlphaForCoverage()) {
            fOpts = that->fOpts;
        }

        fGeoData.push_back_n(that->geoData()->count(), that->geoData()->begin());
        this->joinBounds(that->bounds());
        return true;
    }

    GrPipelineOptimizations fOpts;
    SkSTArray<1, Geometry, true> fGeoData;
};

#endif
