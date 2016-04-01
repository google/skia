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
 *     void UpdateBoundsAfterAppend(const Geometry& lastGeometry, SkRect* currentBounds)
 *
 *     bool CanCombine(const Geometry& mine, const Geometry& theirs,
 *                     const GrXPOverridesForBatch&)
 *
 *     const GrGeometryProcessor* CreateGP(const Geometry& seedGeometry,
 *                                         const GrXPOverridesForBatch& overrides)
 *
 *     const GrBuffer* GetIndexBuffer(GrResourceProvider*)
 *
 *     Tesselate(intptr_t vertices, size_t vertexStride, const Geometry& geo,
 *               const GrXPOverridesForBatch& overrides)
 */
template <typename Impl>
class GrTInstanceBatch : public GrVertexBatch {
public:
    DEFINE_BATCH_CLASS_ID

    typedef typename Impl::Geometry Geometry;

    static GrTInstanceBatch* Create() { return new GrTInstanceBatch; }

    const char* name() const override { return Impl::Name(); }

    SkString dumpInfo() const override {
        SkString str;
        for (int i = 0; i < fGeoData.count(); ++i) {
            str.append(Impl::DumpInfo(fGeoData[i], i));
        }
        str.append(INHERITED::dumpInfo());
        return str;
    }

    void computePipelineOptimizations(GrInitInvariantOutput* color,
                                      GrInitInvariantOutput* coverage,
                                      GrBatchToXPOverrides* overrides) const override {
        // When this is called on a batch, there is only one geometry bundle
        color->setKnownFourComponents(fGeoData[0].fColor);
        Impl::InitInvariantOutputCoverage(coverage);
    }

    void initBatchTracker(const GrXPOverridesForBatch& overrides) override {
        overrides.getOverrideColorIfSet(&fGeoData[0].fColor);
        fOverrides = overrides;
    }

    SkSTArray<1, Geometry, true>* geoData() { return &fGeoData; }

    // After seeding, the client should call init() so the Batch can initialize itself
    void init() {
        const Geometry& geo = fGeoData[0];
        Impl::SetBounds(geo, &fBounds);
    }

    void updateBoundsAfterAppend() {
        const Geometry& geo = fGeoData.back();
        Impl::UpdateBoundsAfterAppend(geo, &fBounds);
    }

private:
    GrTInstanceBatch() : INHERITED(ClassID()) {}

    void onPrepareDraws(Target* target) const override {
        SkAutoTUnref<const GrGeometryProcessor> gp(Impl::CreateGP(this->seedGeometry(),
                                                                  fOverrides));
        if (!gp) {
            SkDebugf("Couldn't create GrGeometryProcessor\n");
            return;
        }

        size_t vertexStride = gp->getVertexStride();
        int instanceCount = fGeoData.count();

        SkAutoTUnref<const GrBuffer> indexBuffer(
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
            Impl::Tesselate(verts, vertexStride, fGeoData[i], fOverrides);
        }
        helper.recordDraw(target, gp);
    }

    const Geometry& seedGeometry() const { return fGeoData[0]; }

    bool onCombineIfPossible(GrBatch* t, const GrCaps& caps) override {
        GrTInstanceBatch* that = t->cast<GrTInstanceBatch>();
        if (!GrPipeline::CanCombine(*this->pipeline(), this->bounds(), *that->pipeline(),
                                    that->bounds(), caps)) {
            return false;
        }

        if (!Impl::CanCombine(this->seedGeometry(), that->seedGeometry(), fOverrides)) {
            return false;
        }

        // In the event of two batches, one who can tweak, one who cannot, we just fall back to
        // not tweaking
        if (fOverrides.canTweakAlphaForCoverage() && !that->fOverrides.canTweakAlphaForCoverage()) {
            fOverrides = that->fOverrides;
        }

        fGeoData.push_back_n(that->geoData()->count(), that->geoData()->begin());
        this->joinBounds(that->bounds());
        return true;
    }

    GrXPOverridesForBatch fOverrides;
    SkSTArray<1, Geometry, true> fGeoData;

    typedef GrVertexBatch INHERITED;
};

#endif
