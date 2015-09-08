
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkComposeShader.h"
#include "SkColorFilter.h"
#include "SkColorPriv.h"
#include "SkColorShader.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkXfermode.h"
#include "SkString.h"

///////////////////////////////////////////////////////////////////////////////

SkComposeShader::SkComposeShader(SkShader* sA, SkShader* sB, SkXfermode* mode) {
    fShaderA = sA;  sA->ref();
    fShaderB = sB;  sB->ref();
    // mode may be null
    fMode = mode;
    SkSafeRef(mode);
}

SkComposeShader::~SkComposeShader() {
    SkSafeUnref(fMode);
    fShaderB->unref();
    fShaderA->unref();
}

size_t SkComposeShader::contextSize() const {
    return sizeof(ComposeShaderContext) + fShaderA->contextSize() + fShaderB->contextSize();
}

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

SkFlattenable* SkComposeShader::CreateProc(SkReadBuffer& buffer) {
    SkAutoTUnref<SkShader> shaderA(buffer.readShader());
    SkAutoTUnref<SkShader> shaderB(buffer.readShader());
    SkAutoTUnref<SkXfermode> mode(buffer.readXfermode());
    if (!shaderA.get() || !shaderB.get()) {
        return nullptr;
    }
    return new SkComposeShader(shaderA, shaderB, mode);
}

void SkComposeShader::flatten(SkWriteBuffer& buffer) const {
    buffer.writeFlattenable(fShaderA);
    buffer.writeFlattenable(fShaderB);
    buffer.writeFlattenable(fMode);
}

template <typename T> void safe_call_destructor(T* obj) {
    if (obj) {
        obj->~T();
    }
}

SkShader::Context* SkComposeShader::onCreateContext(const ContextRec& rec, void* storage) const {
    char* aStorage = (char*) storage + sizeof(ComposeShaderContext);
    char* bStorage = aStorage + fShaderA->contextSize();

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

    SkShader::Context* contextA = fShaderA->createContext(newRec, aStorage);
    SkShader::Context* contextB = fShaderB->createContext(newRec, bStorage);
    if (!contextA || !contextB) {
        safe_call_destructor(contextA);
        safe_call_destructor(contextB);
        return nullptr;
    }

    return new (storage) ComposeShaderContext(*this, rec, contextA, contextB);
}

SkComposeShader::ComposeShaderContext::ComposeShaderContext(
        const SkComposeShader& shader, const ContextRec& rec,
        SkShader::Context* contextA, SkShader::Context* contextB)
    : INHERITED(shader, rec)
    , fShaderContextA(contextA)
    , fShaderContextB(contextB) {}

SkComposeShader::ComposeShaderContext::~ComposeShaderContext() {
    fShaderContextA->~Context();
    fShaderContextB->~Context();
}

bool SkComposeShader::asACompose(ComposeRec* rec) const {
    if (rec) {
        rec->fShaderA = fShaderA;
        rec->fShaderB = fShaderB;
        rec->fMode = fMode;
    }
    return true;
}


// larger is better (fewer times we have to loop), but we shouldn't
// take up too much stack-space (each element is 4 bytes)
#define TMP_COLOR_COUNT     64

void SkComposeShader::ComposeShaderContext::shadeSpan(int x, int y, SkPMColor result[], int count) {
    SkShader::Context* shaderContextA = fShaderContextA;
    SkShader::Context* shaderContextB = fShaderContextB;
    SkXfermode*        mode = static_cast<const SkComposeShader&>(fShader).fMode;
    unsigned           scale = SkAlpha255To256(this->getPaintAlpha());

#ifdef SK_BUILD_FOR_ANDROID
    scale = 256;    // ugh -- maintain old bug/behavior for now
#endif

    SkPMColor   tmp[TMP_COLOR_COUNT];

    if (nullptr == mode) {   // implied SRC_OVER
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
            mode->xfer32(result, tmp, n, nullptr);

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

#if SK_SUPPORT_GPU

#include "SkGr.h"
#include "GrProcessor.h"
#include "effects/GrConstColorProcessor.h"
#include "gl/GrGLBlend.h"
#include "gl/builders/GrGLProgramBuilder.h"

/////////////////////////////////////////////////////////////////////

class GrComposeEffect : public GrFragmentProcessor {
public:

    static GrFragmentProcessor* Create(const GrFragmentProcessor* fpA,
                                       const GrFragmentProcessor* fpB, SkXfermode::Mode mode) {
        return SkNEW_ARGS(GrComposeEffect, (fpA, fpB, mode));
    }
    const char* name() const override { return "ComposeShader"; }
    void onGetGLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const override;

    SkXfermode::Mode getMode() const { return fMode; }

protected:
    bool onIsEqual(const GrFragmentProcessor&) const override;
    void onComputeInvariantOutput(GrInvariantOutput* inout) const override;

private:
    GrComposeEffect(const GrFragmentProcessor* fpA, const GrFragmentProcessor* fpB,
                    SkXfermode::Mode mode)
        : fMode(mode) {
        this->initClassID<GrComposeEffect>();
        SkDEBUGCODE(int shaderAChildIndex = )this->registerChildProcessor(fpA);
        SkDEBUGCODE(int shaderBChildIndex = )this->registerChildProcessor(fpB);
        SkASSERT(0 == shaderAChildIndex);
        SkASSERT(1 == shaderBChildIndex);
    }

    GrGLFragmentProcessor* onCreateGLInstance() const override;

    SkXfermode::Mode fMode;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

    typedef GrFragmentProcessor INHERITED;
};

/////////////////////////////////////////////////////////////////////

class GrGLComposeEffect : public GrGLFragmentProcessor {
public:
    GrGLComposeEffect(const GrProcessor& processor) {}

    void emitCode(EmitArgs&) override;

private:
    typedef GrGLFragmentProcessor INHERITED;
};

/////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrComposeEffect);

const GrFragmentProcessor* GrComposeEffect::TestCreate(GrProcessorTestData* d) {
#if SK_ALLOW_STATIC_GLOBAL_INITIALIZERS
    // Create two random frag procs.
    // For now, we'll prevent either children from being a shader with children to prevent the
    // possibility of an arbitrarily large tree of procs.
    SkAutoTUnref<const GrFragmentProcessor> fpA;
    do {
        fpA.reset(GrProcessorTestFactory<GrFragmentProcessor>::CreateStage(d));
        SkASSERT(fpA);
    } while (fpA->numChildProcessors() != 0);
    SkAutoTUnref<const GrFragmentProcessor> fpB;
    do {
        fpB.reset(GrProcessorTestFactory<GrFragmentProcessor>::CreateStage(d));
        SkASSERT(fpB);
    } while (fpB->numChildProcessors() != 0);

    SkXfermode::Mode mode = static_cast<SkXfermode::Mode>(
            d->fRandom->nextRangeU(0, SkXfermode::kLastCoeffMode));
    return SkNEW_ARGS(GrComposeEffect, (fpA, fpB, mode));
#else
    SkFAIL("Should not be called if !SK_ALLOW_STATIC_GLOBAL_INITIALIZERS");
    return nullptr;
#endif
}

bool GrComposeEffect::onIsEqual(const GrFragmentProcessor& other) const {
    const GrComposeEffect& cs = other.cast<GrComposeEffect>();
    return fMode == cs.fMode;
}

void GrComposeEffect::onComputeInvariantOutput(GrInvariantOutput* inout) const {
    inout->setToUnknown(GrInvariantOutput::kWill_ReadInput);
}

void GrComposeEffect::onGetGLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const {
    b->add32(fMode);
}

GrGLFragmentProcessor* GrComposeEffect::onCreateGLInstance() const{
    return SkNEW_ARGS(GrGLComposeEffect, (*this));
}

/////////////////////////////////////////////////////////////////////

void GrGLComposeEffect::emitCode(EmitArgs& args) {

    GrGLFragmentBuilder* fsBuilder = args.fBuilder->getFragmentShaderBuilder();
    const GrComposeEffect& cs = args.fFp.cast<GrComposeEffect>();

    // Store alpha of input color and un-premultiply the input color by its alpha. We will
    // re-multiply by this alpha after blending the output colors of the two child procs.
    // This is because we don't want the paint's alpha to affect either child proc's output
    // before the blend; we want to apply the paint's alpha AFTER the blend. This mirrors the
    // software implementation of SkComposeShader.
    SkString inputAlpha("inputAlpha");
    fsBuilder->codeAppendf("float %s = %s.a;", inputAlpha.c_str(), args.fInputColor);
    fsBuilder->codeAppendf("%s /= %s.a;", args.fInputColor, args.fInputColor);

    // declare outputColor and emit the code for each of the two children
    SkString outputColorA(args.fOutputColor);
    outputColorA.append("_dst");
    fsBuilder->codeAppendf("vec4 %s;\n", outputColorA.c_str());
    this->emitChild(0, args.fInputColor, outputColorA.c_str(), args);

    SkString outputColorB(args.fOutputColor);
    outputColorB.append("_src");
    fsBuilder->codeAppendf("vec4 %s;\n", outputColorB.c_str());
    this->emitChild(1, args.fInputColor, outputColorB.c_str(), args);

    // emit blend code
    SkXfermode::Mode mode = cs.getMode();
    fsBuilder->codeAppend("{");
    fsBuilder->codeAppendf("// Compose Xfer Mode: %s\n", SkXfermode::ModeName(mode));
    GrGLBlend::AppendPorterDuffBlend(fsBuilder, outputColorB.c_str(),
                                     outputColorA.c_str(), args.fOutputColor, mode);
    fsBuilder->codeAppend("}");

    // re-multiply the output color by the input color's alpha
    fsBuilder->codeAppendf("%s *= %s;", args.fOutputColor, inputAlpha.c_str());
}

/////////////////////////////////////////////////////////////////////

const GrFragmentProcessor* SkComposeShader::asFragmentProcessor(GrContext* context,
                                                            const SkMatrix& viewM,
                                                            const SkMatrix* localMatrix,
                                                            SkFilterQuality fq,
                                                            GrProcessorDataManager* procDataManager
                                                            ) const {
    // Fragment processor will only support coefficient modes. This is because
    // GrGLBlend::AppendPorterDuffBlend(), which emits the blend code in the shader,
    // only supports those modes.
    SkXfermode::Mode mode;
    if (!(SkXfermode::AsMode(fMode, &mode) && mode <= SkXfermode::kLastCoeffMode)) {
        return nullptr;
    }

    switch (mode) {
        case SkXfermode::kClear_Mode:
            return GrConstColorProcessor::Create(GrColor_TRANS_BLACK,
                                                 GrConstColorProcessor::kIgnore_InputMode);
            break;
        case SkXfermode::kSrc_Mode:
            return fShaderB->asFragmentProcessor(context, viewM, localMatrix, fq, procDataManager);
            break;
        case SkXfermode::kDst_Mode:
            return fShaderA->asFragmentProcessor(context, viewM, localMatrix, fq, procDataManager);
            break;
        default:
            SkAutoTUnref<const GrFragmentProcessor> fpA(fShaderA->asFragmentProcessor(context,
                                                        viewM, localMatrix, fq, procDataManager));
            if (!fpA.get()) {
                return nullptr;
            }
            SkAutoTUnref<const GrFragmentProcessor> fpB(fShaderB->asFragmentProcessor(context,
                                                        viewM, localMatrix, fq, procDataManager));
            if (!fpB.get()) {
                return nullptr;
            }
            return GrComposeEffect::Create(fpA, fpB, mode);
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
    if (fMode) {
        str->append(" Xfermode: ");
        fMode->toString(str);
    }

    this->INHERITED::toString(str);

    str->append(")");
}
#endif
