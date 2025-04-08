/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_ganesh_AtlasTextOp_DEFINED
#define skgpu_ganesh_AtlasTextOp_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkString.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkPoint_impl.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/core/SkColorData.h"
#include "src/gpu/AtlasTypes.h"
#include "src/gpu/ganesh/GrAppliedClip.h"
#include "src/gpu/ganesh/GrBuffer.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrColorInfo.h"
#include "src/gpu/ganesh/GrColorSpaceXform.h"
#include "src/gpu/ganesh/GrProcessorSet.h"
#include "src/gpu/ganesh/effects/GrDistanceFieldGeoProc.h"
#include "src/gpu/ganesh/ops/GrMeshDrawOp.h"
#include "src/gpu/ganesh/ops/GrOp.h"

#include <cstddef>
#include <cstdint>
#include <tuple>
#include <utility>

class GrClip;
class GrDstProxyView;
class GrGeometryProcessor;
class GrMeshDrawTarget;
class GrOpFlushState;
class GrPaint;
class GrProgramInfo;
class GrRecordingContext;
class GrSurfaceProxy;
class GrSurfaceProxyView;
class SkArenaAlloc;
class SkPaint;
enum class GrXferBarrierFlags;
struct GrShaderCaps;

namespace skgpu { namespace ganesh { class SurfaceDrawContext; } }
namespace sktext { namespace gpu { class AtlasSubRun; } }

namespace skgpu::ganesh {

class AtlasTextOp final : public GrMeshDrawOp {
public:
    DEFINE_OP_CLASS_ID

    static std::tuple<const GrClip*, GrOp::Owner> Make(SurfaceDrawContext*,
                                                       const sktext::gpu::AtlasSubRun*,
                                                       const GrClip*,
                                                       const SkMatrix& viewMatrix,
                                                       SkPoint drawOrigin,
                                                       const SkPaint&,
                                                       sk_sp<SkRefCnt>&& subRunStorage);

    ~AtlasTextOp() override {
        for (const Geometry* g = fHead; g != nullptr;) {
            const Geometry* next = g->fNext;
            g->~Geometry();
            g = next;
        }
    }

    void* operator new(size_t s);
    void operator delete(void* b) noexcept;
    static void ClearCache();

    struct Geometry {
        Geometry(const sktext::gpu::AtlasSubRun& subRun,
                 const SkMatrix& drawMatrix,
                 SkPoint drawOrigin,
                 SkIRect clipRect,
                 sk_sp<SkRefCnt>&& supportData,
                 const SkPMColor4f& color)
            : fSubRun{subRun}
            , fSupportDataKeepAlive{std::move(supportData)}
            , fDrawMatrix{drawMatrix}
            , fDrawOrigin{drawOrigin}
            , fClipRect{clipRect}
            , fColor{color} {
                SkASSERT(fSupportDataKeepAlive != nullptr);
        }

        static Geometry* Make(const sktext::gpu::AtlasSubRun& subRun,
                              const SkMatrix& drawMatrix,
                              SkPoint drawOrigin,
                              SkIRect clipRect,
                              sk_sp<SkRefCnt>&& supportData,
                              const SkPMColor4f& color,
                              SkArenaAlloc* alloc);

        void fillVertexData(void* dst, int offset, int count) const;

        const sktext::gpu::AtlasSubRun& fSubRun;

        // Keep the TextBlob or Slug alive until the op is deleted.
        sk_sp<SkRefCnt> fSupportDataKeepAlive;

        const SkMatrix fDrawMatrix;
        const SkPoint fDrawOrigin;

        // fClipRect is only used in the DirectMaskSubRun case to do geometric clipping.
        // TransformedMaskSubRun, and SDFTSubRun don't use this field, and expect an empty rect.
        const SkIRect fClipRect;

        // Color is updated after processor analysis if it was determined the shader resolves to
        // a constant color that we then evaluate on the CPU.
        // TODO: This can be made const once processor analysis is separated from op creation.
        SkPMColor4f fColor;
        Geometry* fNext{nullptr};
    };

    const char* name() const override { return "AtlasTextOp"; }

    void visitProxies(const GrVisitProxyFunc&) const override;

    FixedFunctionFlags fixedFunctionFlags() const override;

    GrProcessorSet::Analysis finalize(const GrCaps&, const GrAppliedClip*, GrClampType) override;

    enum class MaskType : uint32_t {
        kGrayscaleCoverage,
        kLCDCoverage,
        kColorBitmap,
#if !defined(SK_DISABLE_SDF_TEXT)
        kAliasedDistanceField,
        kGrayscaleDistanceField,
        kLCDDistanceField,

        kLast = kLCDDistanceField
#else
        kLast = kColorBitmap
#endif
    };
    inline static constexpr int kMaskTypeCount = static_cast<int>(MaskType::kLast) + 1;

private:
    friend class GrOp; // for ctor

    struct FlushInfo {
        sk_sp<const GrBuffer> fVertexBuffer;
        sk_sp<const GrBuffer> fIndexBuffer;
        GrGeometryProcessor*  fGeometryProcessor;
        const GrSurfaceProxy** fPrimProcProxies;
        int fGlyphsToFlush = 0;
        int fVertexOffset = 0;
        int fNumDraws = 0;
    };

    // DirectMask and TransformedMask constructor
    AtlasTextOp(MaskType maskType,
                bool needsTransform,
                int glyphCount,
                SkRect deviceRect,
                Geometry* geo,
                const GrColorInfo& dstColorInfo,
                GrPaint&& paint);

    // SDF constructor
    AtlasTextOp(MaskType maskType,
                bool needsTransform,
                int glyphCount,
                SkRect deviceRect,
                SkColor luminanceColor,
                bool useGammaCorrectDistanceTable,
                uint32_t DFGPFlags,
                Geometry* geo,
                GrPaint&& paint);

    GrProgramInfo* programInfo() override {
        // TODO [PI]: implement
        return nullptr;
    }

    void addGeometry(Geometry* geometry) {
        *fTail = geometry;
        // The geometry may have many entries. Find the end.
        do {
            fTail = &(*fTail)->fNext;
        } while (*fTail != nullptr);
    }

    void onCreateProgramInfo(const GrCaps*,
                             SkArenaAlloc*,
                             const GrSurfaceProxyView& writeView,
                             bool usesMSAASurface,
                             GrAppliedClip&&,
                             const GrDstProxyView&,
                             GrXferBarrierFlags renderPassXferBarriers,
                             GrLoadOp colorLoadOp) override {
        // We cannot surface the AtlasTextOp's programInfo at record time. As currently
        // implemented, the GP is modified at flush time based on the number of pages in the
        // atlas.
    }

    void onPrePrepareDraws(GrRecordingContext*,
                           const GrSurfaceProxyView& writeView,
                           GrAppliedClip*,
                           const GrDstProxyView&,
                           GrXferBarrierFlags renderPassXferBarriers,
                           GrLoadOp colorLoadOp) override {
        // TODO [PI]: implement
    }

    void onPrepareDraws(GrMeshDrawTarget*) override;
    void onExecute(GrOpFlushState*, const SkRect& chainBounds) override;

#if defined(GPU_TEST_UTILS)
    SkString onDumpInfo() const override;
#endif

    skgpu::MaskFormat maskFormat() const {
        switch (this->maskType()) {
            case MaskType::kLCDCoverage:
                return skgpu::MaskFormat::kA565;
            case MaskType::kColorBitmap:
                return skgpu::MaskFormat::kARGB;
            case MaskType::kGrayscaleCoverage:
#if !defined(SK_DISABLE_SDF_TEXT)
            case MaskType::kAliasedDistanceField:
            case MaskType::kGrayscaleDistanceField:
            case MaskType::kLCDDistanceField:
#endif
                return skgpu::MaskFormat::kA8;
        }
        // SkUNREACHABLE;
        return skgpu::MaskFormat::kA8;
    }

#if !defined(SK_DISABLE_SDF_TEXT)
    bool usesDistanceFields() const {
        return MaskType::kAliasedDistanceField == this->maskType() ||
               MaskType::kGrayscaleDistanceField == this->maskType() ||
               MaskType::kLCDDistanceField == this->maskType();
    }

    bool isLCD() const {
        return MaskType::kLCDCoverage == this->maskType() ||
               MaskType::kLCDDistanceField == this->maskType();
    }
#else
    bool isLCD() const {
        return MaskType::kLCDCoverage == this->maskType();
    }
#endif

    inline void createDrawForGeneratedGlyphs(
            GrMeshDrawTarget* target, FlushInfo* flushInfo) const;

    MaskType maskType() const { return static_cast<MaskType>(fMaskType); }

    CombineResult onCombineIfPossible(GrOp* t, SkArenaAlloc*, const GrCaps& caps) override;

#if !defined(SK_DISABLE_SDF_TEXT)
    GrGeometryProcessor* setupDfProcessor(SkArenaAlloc*,
                                          const GrShaderCaps&,
                                          const SkMatrix& localMatrix,
                                          const GrSurfaceProxyView* views,
                                          unsigned int numActiveViews) const;
#endif

    GrProcessorSet fProcessors;
    int fNumGlyphs; // Sum of glyphs in each geometry's subrun

    // All combinable atlas ops have equal bit field values
    uint32_t fDFGPFlags                    : 10; // Distance field properties
    uint32_t fMaskType                     : 3;  // MaskType
    uint32_t fUsesLocalCoords              : 1;  // Filled in post processor analysis
    uint32_t fNeedsGlyphTransform          : 1;
    uint32_t fHasPerspective               : 1;  // True if perspective affects draw
    uint32_t fUseGammaCorrectDistanceTable : 1;
    static_assert(kMaskTypeCount <= 8, "MaskType does not fit in 3 bits");
#if !defined(SK_DISABLE_SDF_TEXT)
    static_assert(kInvalid_DistanceFieldEffectFlag <= (1 << 9), "DFGP Flags do not fit in 10 bits");
#endif

    // Only needed for color emoji
    sk_sp<GrColorSpaceXform> fColorSpaceXform;

    // Only used for distance fields; per-channel luminance for LCD, or gamma-corrected luminance
    // for single-channel distance fields.
    const SkColor fLuminanceColor{0};

    Geometry* fHead{nullptr};
    Geometry** fTail{&fHead};

    using INHERITED = GrMeshDrawOp;
};

} // namespace skgpu::ganesh

#endif // skgpu_ganesh_AtlasTextOp_DEFINED
