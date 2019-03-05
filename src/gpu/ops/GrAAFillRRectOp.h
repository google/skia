/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrAAFillRRectOp_DEFINED
#define GrAAFillRRectOp_DEFINED

#include "GrDrawOp.h"

class GrRecordingContext;

class GrAAFillRRectOp : public GrDrawOp {
public:
    DEFINE_OP_CLASS_ID

    static std::unique_ptr<GrAAFillRRectOp> Make(GrRecordingContext*, const SkMatrix&,
                                                 const SkRRect&, const GrCaps&, GrPaint&&);

    const char* name() const override { return "GrAAFillRRectOp"; }
    FixedFunctionFlags fixedFunctionFlags() const override { return FixedFunctionFlags::kNone; }
    GrProcessorSet::Analysis finalize(const GrCaps&, const GrAppliedClip*, GrFSAAType) override;
    CombineResult onCombineIfPossible(GrOp*, const GrCaps&) override;
    void visitProxies(const VisitProxyFunc& fn, VisitorType) const override {
        fProcessors.visitProxies(fn);
    }
    void onPrepare(GrOpFlushState*) override;

    void onExecute(GrOpFlushState*, const SkRect& chainBounds) override;

private:
    enum class Flags {
        kNone = 0,
        kUseHWDerivatives = 1 << 0,
        kHasLocalCoords = 1 << 1,
        kWideColor = 1 << 2
    };

    GR_DECL_BITFIELD_CLASS_OPS_FRIENDS(Flags);

    class Processor;

    GrAAFillRRectOp(const GrShaderCaps&, const SkMatrix&, const SkRRect&, GrPaint&&);

    // These methods are used to append data of various POD types to our internal array of instance
    // data. The actual layout of the instance buffer can vary from Op to Op.
    template <typename T> inline void* appendInstanceData(int count) {
        static_assert(std::is_pod<T>::value, "");
        static_assert(4 == alignof(T), "");
        return fInstanceData.push_back_n(sizeof(T) * count);
    }

    template <typename T, typename... Args>
    inline void writeInstanceData(const T& val, const Args&... remainder) {
        memcpy(this->appendInstanceData<T>(1), &val, sizeof(T));
        this->writeInstanceData(remainder...);
    }

    void writeInstanceData() {}  // Halt condition.

    const SkPMColor4f fOriginalColor;
    const SkRect fLocalRect;
    Flags fFlags = Flags::kNone;
    GrProcessorSet fProcessors;

    SkSTArray<sizeof(float) * 16 * 4, char, /*MEM_MOVE=*/ true> fInstanceData;
    int fInstanceCount = 1;
    int fInstanceStride = 0;

    sk_sp<const GrBuffer> fInstanceBuffer;
    int fBaseInstance;

    friend class GrOpMemoryPool;
};

GR_MAKE_BITFIELD_CLASS_OPS(GrAAFillRRectOp::Flags)

#endif
