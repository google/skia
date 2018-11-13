/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrAARoundRectOp_DEFINED
#define GrAARoundRectOp_DEFINED

#include "GrDrawOp.h"


#include "GrPaint.h"

class GrAARoundRectOp : public GrDrawOp {
public:
    DEFINE_OP_CLASS_ID

    static std::unique_ptr<GrAARoundRectOp> Make(GrContext*, const SkMatrix&, const SkRRect&,
                                             const GrCaps&, GrPaint&&);

    const char* name() const override { return "GrAARoundRectOp"; }
    FixedFunctionFlags fixedFunctionFlags() const override { return FixedFunctionFlags::kNone; }
    RequiresDstTexture finalize(const GrCaps&, const GrAppliedClip*) override;
    CombineResult onCombineIfPossible(GrOp*, const GrCaps&) override;
    void visitProxies(const VisitProxyFunc& fn, VisitorType) const override {
        fProcessors.visitProxies(fn);
    }
    void onPrepare(GrOpFlushState*) override;

    void onExecute(GrOpFlushState*) override;

private:
    enum class Flags {
        kNone = 0,
        kUseHWDerivatives = 1,
        kHasLocalCoords = 1 << 1
    };

    GR_DECL_BITFIELD_CLASS_OPS_FRIENDS(Flags)

    class RoundRectProc;

    GrAARoundRectOp(const GrShaderCaps&, const SkMatrix&, const SkRRect&, GrPaint&&);

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
    Flags fFlags = Flags::kNone;
    SkRect fLocalRect;
    GrProcessorSet fProcessors;

    SkSTArray<sizeof(float) * 16 * 4, char, /*MEM_MOVE=*/ true> fInstanceData;
    int fInstanceCount = 1;
    int fInstanceStride = 0;

    const GrBuffer* fInstanceBuffer = nullptr;
    int fBaseInstance;

    friend class GrOpMemoryPool;
};

GR_MAKE_BITFIELD_CLASS_OPS(GrAARoundRectOp::Flags)

#endif
