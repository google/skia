/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGradientShaderPriv_DEFINED
#define SkGradientShaderPriv_DEFINED

#include "SkGradientShader.h"

#include "SkArenaAlloc.h"
#include "SkMatrix.h"
#include "SkShaderBase.h"
#include "SkTArray.h"
#include "SkTemplates.h"

class SkColorSpace;
class SkColorSpaceXformer;
class SkRasterPipeline;
class SkReadBuffer;
class SkWriteBuffer;

class SkGradientShaderBase : public SkShaderBase {
public:
    struct Descriptor {
        Descriptor() {
            sk_bzero(this, sizeof(*this));
            fTileMode = SkShader::kClamp_TileMode;
        }

        const SkMatrix*     fLocalMatrix;
        const SkColor4f*    fColors;
        sk_sp<SkColorSpace> fColorSpace;
        const SkScalar*     fPos;
        int                 fCount;
        SkShader::TileMode  fTileMode;
        uint32_t            fGradFlags;

        void flatten(SkWriteBuffer&) const;
    };

    class DescriptorScope : public Descriptor {
    public:
        DescriptorScope() {}

        bool unflatten(SkReadBuffer&);

        // fColors and fPos always point into local memory, so they can be safely mutated
        //
        SkColor4f* mutableColors() { return const_cast<SkColor4f*>(fColors); }
        SkScalar* mutablePos() { return const_cast<SkScalar*>(fPos); }

    private:
        SkSTArray<16, SkColor4f, true> fColorStorage;
        SkSTArray<16, SkScalar , true> fPosStorage;
        SkMatrix                       fLocalMatrixStorage;
    };

    SkGradientShaderBase(const Descriptor& desc, const SkMatrix& ptsToUnit);
    ~SkGradientShaderBase() override;

    bool isOpaque() const override;

    uint32_t getGradFlags() const { return fGradFlags; }

    const SkMatrix& getGradientMatrix() const { return fPtsToUnit; }

protected:
    class GradientShaderBase4fContext;

    SkGradientShaderBase(SkReadBuffer& );
    void flatten(SkWriteBuffer&) const override;

    void commonAsAGradient(GradientInfo*) const;

    bool onAsLuminanceColor(SkColor*) const override;

    bool onAppendStages(const StageRec&) const override;

    virtual void appendGradientStages(SkArenaAlloc* alloc, SkRasterPipeline* tPipeline,
                                      SkRasterPipeline* postPipeline) const = 0;

    template <typename T, typename... Args>
    static Context* CheckedMakeContext(SkArenaAlloc* alloc, Args&&... args) {
        auto* ctx = alloc->make<T>(std::forward<Args>(args)...);
        if (!ctx->isValid()) {
            return nullptr;
        }
        return ctx;
    }

    struct AutoXformColors {
        AutoXformColors(const SkGradientShaderBase&, SkColorSpaceXformer*);

        SkAutoSTMalloc<8, SkColor> fColors;
    };

    const SkMatrix fPtsToUnit;
    TileMode       fTileMode;
    uint8_t        fGradFlags;

public:
    SkScalar getPos(int i) const {
        SkASSERT(i < fColorCount);
        return fOrigPos ? fOrigPos[i] : SkIntToScalar(i) / (fColorCount - 1);
    }

    SkColor getLegacyColor(int i) const {
        SkASSERT(i < fColorCount);
        return fOrigColors4f[i].toSkColor();
    }

    bool colorsCanConvertToSkColor() const {
        bool canConvert = true;
        for (int i = 0; i < fColorCount; ++i) {
            canConvert &= fOrigColors4f[i].fitsInBytes();
        }
        return canConvert;
    }

    SkColor4f*          fOrigColors4f; // original colors, as floats
    SkScalar*           fOrigPos;      // original positions
    int                 fColorCount;
    sk_sp<SkColorSpace> fColorSpace;   // color space of gradient stops

    bool colorsAreOpaque() const { return fColorsAreOpaque; }

    TileMode getTileMode() const { return fTileMode; }

private:
    // Reserve inline space for up to 4 stops.
    static constexpr size_t kInlineStopCount   = 4;
    static constexpr size_t kInlineStorageSize = (sizeof(SkColor4f) + sizeof(SkScalar))
                                               * kInlineStopCount;
    SkAutoSTMalloc<kInlineStorageSize, uint8_t> fStorage;

    bool                                        fColorsAreOpaque;

    typedef SkShaderBase INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

struct SkColor4fXformer {
    SkColor4fXformer(const SkColor4f* colors, int colorCount, SkColorSpace* src, SkColorSpace* dst);

    const SkColor4f*              fColors;
    SkSTArray<4, SkColor4f, true> fStorage;
};

#endif
