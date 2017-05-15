/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkArenaAlloc.h"
#include "SkBlendModePriv.h"
#include "SkComposeShader.h"
#include "SkColorFilter.h"
#include "SkColorPriv.h"
#include "SkColorShader.h"
#include "SkRasterPipeline.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkString.h"
#include "../jumper/SkJumper.h"

sk_sp<SkShader> SkShader::MakeComposeShader(sk_sp<SkShader> dst, sk_sp<SkShader> src,
                                            SkBlendMode mode) {
    if (!src || !dst) {
        return nullptr;
    }
    if (SkBlendMode::kSrc == mode) {
        return src;
    }
    if (SkBlendMode::kDst == mode) {
        return dst;
    }
    return sk_sp<SkShader>(new SkComposeShader(std::move(dst), std::move(src), mode));
}

///////////////////////////////////////////////////////////////////////////////

class SkAutoAlphaRestore {
public:
    SkAutoAlphaRestore(SkPaint* paint, uint8_t newAlpha) {
        fAlpha = paint->getAlpha();
        fPaint = paint;
        paint->setAlpha(newAlpha);
    }

    ~SkAutoAlphaRestore() {
        fPaint->setAlpha(fAlpha);
    }
private:
    SkPaint*    fPaint;
    uint8_t     fAlpha;
};
#define SkAutoAlphaRestore(...) SK_REQUIRE_LOCAL_VAR(SkAutoAlphaRestore)

sk_sp<SkFlattenable> SkComposeShader::CreateProc(SkReadBuffer& buffer) {
    sk_sp<SkShader> shaderA(buffer.readShader());
    sk_sp<SkShader> shaderB(buffer.readShader());
    SkBlendMode mode;
    if (buffer.isVersionLT(SkReadBuffer::kXfermodeToBlendMode2_Version)) {
        sk_sp<SkXfermode> xfer = buffer.readXfermode();
        mode = xfer ? xfer->blend() : SkBlendMode::kSrcOver;
    } else {
        mode = (SkBlendMode)buffer.read32();
    }
    if (!shaderA || !shaderB) {
        return nullptr;
    }
    return sk_make_sp<SkComposeShader>(std::move(shaderA), std::move(shaderB), mode);
}

void SkComposeShader::flatten(SkWriteBuffer& buffer) const {
    buffer.writeFlattenable(fShaderA.get());
    buffer.writeFlattenable(fShaderB.get());
    buffer.write32((int)fMode);
}

SkShader::Context* SkComposeShader::onMakeContext(
    const ContextRec& rec, SkArenaAlloc* alloc) const
{
    // we preconcat our localMatrix (if any) with the device matrix
    // before calling our sub-shaders
    SkMatrix tmpM;
    tmpM.setConcat(*rec.fMatrix, this->getLocalMatrix());

    // Our sub-shaders need to see opaque, so by combining them we don't double-alphatize the
    // result. ComposeShader itself will respect the alpha, and post-apply it after calling the
    // sub-shaders.
    SkPaint opaquePaint(*rec.fPaint);
    opaquePaint.setAlpha(0xFF);

    ContextRec newRec(rec);
    newRec.fMatrix = &tmpM;
    newRec.fPaint = &opaquePaint;

    SkShader::Context* contextA = fShaderA->makeContext(newRec, alloc);
    SkShader::Context* contextB = fShaderB->makeContext(newRec, alloc);
    if (!contextA || !contextB) {
        return nullptr;
    }

    return alloc->make<ComposeShaderContext>(*this, rec, contextA, contextB);
}

sk_sp<SkShader> SkComposeShader::onMakeColorSpace(SkColorSpaceXformer* xformer) const {
    return SkShader::MakeComposeShader(xformer->apply(fShaderA.get()),
                                       xformer->apply(fShaderB.get()), fMode);
}

SkComposeShader::ComposeShaderContext::ComposeShaderContext(
        const SkComposeShader& shader, const ContextRec& rec,
        SkShader::Context* contextA, SkShader::Context* contextB)
    : INHERITED(shader, rec)
    , fShaderContextA(contextA)
    , fShaderContextB(contextB) {}

bool SkComposeShader::asACompose(ComposeRec* rec) const {
    if (rec) {
        rec->fShaderA   = fShaderA.get();
        rec->fShaderB   = fShaderB.get();
        rec->fBlendMode = fMode;
    }
    return true;
}

bool SkComposeShader::isRasterPipelineOnly() const {
    return fShaderA->isRasterPipelineOnly() || fShaderB->isRasterPipelineOnly();
}

bool SkComposeShader::onAppendStages(SkRasterPipeline* pipeline, SkColorSpace* dstCS,
                                     SkArenaAlloc* alloc, const SkMatrix& ctm,
                                     const SkPaint& paint, const SkMatrix* localM) const {
    struct Storage {
        float   fXY[4 * SkJumper_kMaxStride];
        float   fRGBA[4 * SkJumper_kMaxStride];
        float   fAlpha;
    };
    auto storage = alloc->make<Storage>();

    // We need to save off device x,y (inputs to shader), since after calling fShaderA they
    // will be smashed, and I'll need them again for fShaderB. store_rgba saves off 4 registers
    // even though we only need to save r,g.
    pipeline->append(SkRasterPipeline::store_rgba, storage->fXY);
    if (!fShaderB->appendStages(pipeline, dstCS, alloc, ctm, paint, localM)) { // SRC
        return false;
    }
    // This outputs r,g,b,a, which we'll need later when we apply the mode, but we save it off now
    // since fShaderB will overwrite them.
    pipeline->append(SkRasterPipeline::store_rgba, storage->fRGBA);
    // Now we restore the device x,y for the next shader
    pipeline->append(SkRasterPipeline::load_rgba, storage->fXY);
    if (!fShaderA->appendStages(pipeline, dstCS, alloc, ctm, paint, localM)) {  // DST
        return false;
    }
    // We now have our logical 'dst' in r,g,b,a, but we need it in dr,dg,db,da for the mode
    // so we have to shuttle them. If we had a stage the would load_into_dst, then we could
    // reverse the two shader invocations, and avoid this move...
    pipeline->append(SkRasterPipeline::move_src_dst);
    pipeline->append(SkRasterPipeline::load_rgba, storage->fRGBA);

    // Idea: should time this, and see if it helps to have custom versions of the overflow modes
    //       that do their own clamping, avoiding the overhead of an extra stage.
    SkBlendMode_AppendStages(fMode, pipeline);
    if (SkBlendMode_CanOverflow(fMode)) {
        pipeline->append(SkRasterPipeline::clamp_a);
    }
    return true;
}

// larger is better (fewer times we have to loop), but we shouldn't
// take up too much stack-space (each element is 4 bytes)
#define TMP_COLOR_COUNT     64

void SkComposeShader::ComposeShaderContext::shadeSpan(int x, int y, SkPMColor result[], int count) {
    SkShader::Context* shaderContextA = fShaderContextA;
    SkShader::Context* shaderContextB = fShaderContextB;
    SkBlendMode        mode = static_cast<const SkComposeShader&>(fShader).fMode;
    unsigned           scale = SkAlpha255To256(this->getPaintAlpha());

    SkPMColor   tmp[TMP_COLOR_COUNT];

    SkXfermode* xfer = SkXfermode::Peek(mode);
    if (nullptr == xfer) {   // implied SRC_OVER
        // TODO: when we have a good test-case, should use SkBlitRow::Proc32
        // for these loops
        do {
            int n = count;
            if (n > TMP_COLOR_COUNT) {
                n = TMP_COLOR_COUNT;
            }

            shaderContextA->shadeSpan(x, y, result, n);
            shaderContextB->shadeSpan(x, y, tmp, n);

            if (256 == scale) {
                for (int i = 0; i < n; i++) {
                    result[i] = SkPMSrcOver(tmp[i], result[i]);
                }
            } else {
                for (int i = 0; i < n; i++) {
                    result[i] = SkAlphaMulQ(SkPMSrcOver(tmp[i], result[i]),
                                            scale);
                }
            }

            result += n;
            x += n;
            count -= n;
        } while (count > 0);
    } else {    // use mode for the composition
        do {
            int n = count;
            if (n > TMP_COLOR_COUNT) {
                n = TMP_COLOR_COUNT;
            }

            shaderContextA->shadeSpan(x, y, result, n);
            shaderContextB->shadeSpan(x, y, tmp, n);
            xfer->xfer32(result, tmp, n, nullptr);

            if (256 != scale) {
                for (int i = 0; i < n; i++) {
                    result[i] = SkAlphaMulQ(result[i], scale);
                }
            }

            result += n;
            x += n;
            count -= n;
        } while (count > 0);
    }
}

void SkComposeShader::ComposeShaderContext::shadeSpan4f(int x, int y, SkPM4f result[], int count) {
    SkShader::Context* shaderContextA = fShaderContextA;
    SkShader::Context* shaderContextB = fShaderContextB;
    SkBlendMode        mode = static_cast<const SkComposeShader&>(fShader).fMode;
    unsigned           alpha = this->getPaintAlpha();
    Sk4f               scale(alpha * (1.0f / 255));

    SkPM4f  tmp[TMP_COLOR_COUNT];

    SkXfermodeProc4f xfer = SkXfermode::GetProc4f(mode);
    do {
        int n = SkTMin(count, TMP_COLOR_COUNT);

        shaderContextA->shadeSpan4f(x, y, result, n);
        shaderContextB->shadeSpan4f(x, y, tmp, n);
        if (255 == alpha) {
            for (int i = 0; i < n; ++i) {
                result[i] = xfer(tmp[i], result[i]);
            }
        } else {
            for (int i = 0; i < n; ++i) {
                (xfer(tmp[i], result[i]).to4f() * scale).store(result + i);
            }
        }
        result += n;
        x += n;
        count -= n;
    } while (count > 0);
}

#if SK_SUPPORT_GPU

#include "effects/GrConstColorProcessor.h"
#include "effects/GrXfermodeFragmentProcessor.h"

/////////////////////////////////////////////////////////////////////

sk_sp<GrFragmentProcessor> SkComposeShader::asFragmentProcessor(const AsFPArgs& args) const {
    switch (fMode) {
        case SkBlendMode::kClear:
            return GrConstColorProcessor::Make(GrColor4f::TransparentBlack(),
                                               GrConstColorProcessor::kIgnore_InputMode);
            break;
        case SkBlendMode::kSrc:
            return fShaderB->asFragmentProcessor(args);
            break;
        case SkBlendMode::kDst:
            return fShaderA->asFragmentProcessor(args);
            break;
        default:
            sk_sp<GrFragmentProcessor> fpA(fShaderA->asFragmentProcessor(args));
            if (!fpA) {
                return nullptr;
            }
            sk_sp<GrFragmentProcessor> fpB(fShaderB->asFragmentProcessor(args));
            if (!fpB) {
                return nullptr;
            }
            return GrXfermodeFragmentProcessor::MakeFromTwoProcessors(std::move(fpB),
                                                                      std::move(fpA), fMode);
    }
}
#endif

#ifndef SK_IGNORE_TO_STRING
void SkComposeShader::toString(SkString* str) const {
    str->append("SkComposeShader: (");

    str->append("ShaderA: ");
    fShaderA->toString(str);
    str->append(" ShaderB: ");
    fShaderB->toString(str);
    if (SkBlendMode::kSrcOver != fMode) {
        str->appendf(" Xfermode: %s", SkXfermode::ModeName(fMode));
    }

    this->INHERITED::toString(str);

    str->append(")");
}
#endif
