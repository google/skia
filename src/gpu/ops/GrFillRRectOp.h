/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrFillRRectOp_DEFINED
#define GrFillRRectOp_DEFINED

#include "src/gpu/GrProgramInfo.h"
#include "src/gpu/ops/GrDrawOp.h"

class GrRecordingContext;

class GrFillRRectOp : public GrDrawOp {
public:
    DEFINE_OP_CLASS_ID

    static std::unique_ptr<GrFillRRectOp> Make(
            GrRecordingContext*, GrAAType, const SkMatrix& viewMatrix, const SkRRect&,
            const GrCaps&, GrPaint&&);

    const char* name() const final { return "GrFillRRectOp"; }

    FixedFunctionFlags fixedFunctionFlags() const final {
        return (GrAAType::kMSAA == fAAType) ? FixedFunctionFlags::kUsesHWAA
                                            : FixedFunctionFlags::kNone;
    }
    GrProcessorSet::Analysis finalize(const GrCaps&, const GrAppliedClip*,
                                      bool hasMixedSampledCoverage, GrClampType) final;
    CombineResult onCombineIfPossible(GrOp*, GrRecordingContext::Arenas*, const GrCaps&) final;
    void visitProxies(const VisitProxyFunc& fn) const override {
        if (fProgramInfo) {
            fProgramInfo->visitProxies(fn);
        } else {
            fProcessors.visitProxies(fn);
        }
    }

    void onPrePrepare(GrRecordingContext*, const GrSurfaceProxyView*, GrAppliedClip*,
                      const GrXferProcessor::DstProxyView&) final;

    void onPrepare(GrOpFlushState*) final;

    void onExecute(GrOpFlushState*, const SkRect& chainBounds) final;

private:
    enum class Flags {
        kNone = 0,
        kUseHWDerivatives = 1 << 0,
        kHasPerspective = 1 << 1,
        kHasLocalCoords = 1 << 2,
        kWideColor = 1 << 3
    };

    GR_DECL_BITFIELD_CLASS_OPS_FRIENDS(Flags);

    class Processor;

    GrFillRRectOp(GrAAType, const SkRRect&, Flags, const SkMatrix& totalShapeMatrix,
                  GrPaint&&, const SkRect& devBounds);

    // These methods are used to append data of various POD types to our internal array of instance
    // data. The actual layout of the instance buffer can vary from Op to Op.
    template <typename T> inline T* appendInstanceData(int count) {
        static_assert(std::is_pod<T>::value, "");
        static_assert(4 == alignof(T), "");
        return reinterpret_cast<T*>(fInstanceData.push_back_n(sizeof(T) * count));
    }

    template <typename T, typename... Args>
    inline void writeInstanceData(const T& val, const Args&... remainder) {
        memcpy(this->appendInstanceData<T>(1), &val, sizeof(T));
        this->writeInstanceData(remainder...);
    }

    void writeInstanceData() {}  // Halt condition.

    // Create a GrProgramInfo object in the provided arena
    GrProgramInfo* createProgramInfo(const GrCaps*,
                                     SkArenaAlloc*,
                                     const GrSurfaceProxyView* dstView,
                                     GrAppliedClip&&,
                                     const GrXferProcessor::DstProxyView&);

    const GrAAType fAAType;
    const SkPMColor4f fOriginalColor;
    const SkRect fLocalRect;
    Flags fFlags;
    GrProcessorSet fProcessors;

    SkSTArray<sizeof(float) * 16 * 4, char, /*MEM_MOVE=*/ true> fInstanceData;
    int fInstanceCount = 1;
    int fInstanceStride = 0;

    sk_sp<const GrBuffer> fInstanceBuffer;
    sk_sp<const GrBuffer> fVertexBuffer;
    sk_sp<const GrBuffer> fIndexBuffer;
    int fBaseInstance = 0;
    int fIndexCount = 0;

    // If this op is prePrepared the created programInfo will be stored here from use in
    // onExecute. In the prePrepared case it will have been stored in the record-time arena.
    GrProgramInfo* fProgramInfo = nullptr;

    friend class GrOpMemoryPool;
};

GR_MAKE_BITFIELD_CLASS_OPS(GrFillRRectOp::Flags)

#endif
