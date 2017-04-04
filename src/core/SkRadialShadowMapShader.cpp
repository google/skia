/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkLights.h"
#include "SkPoint3.h"
#include "SkRadialShadowMapShader.h"

////////////////////////////////////////////////////////////////////////////
#ifdef SK_EXPERIMENTAL_SHADOWING


/** \class SkRadialShadowMapShaderImpl
    This subclass of shader applies shadowing radially around a light
*/
class SkRadialShadowMapShaderImpl : public SkShader {
public:
    /** Create a new shadowing shader that shadows radially around a light
    */
    SkRadialShadowMapShaderImpl(sk_sp<SkShader> occluderShader,
                                sk_sp<SkLights> lights,
                                int diffuseWidth, int diffuseHeight)
        : fOccluderShader(std::move(occluderShader))
        , fLight(std::move(lights))
        , fWidth(diffuseWidth)
        , fHeight(diffuseHeight) { }

    bool isOpaque() const override;

#if SK_SUPPORT_GPU
    sk_sp<GrFragmentProcessor> asFragmentProcessor(const AsFPArgs&) const override;
#endif

    class ShadowMapRadialShaderContext : public SkShader::Context {
    public:
        // The context takes ownership of the states. It will call their destructors
        // but will NOT free the memory.
        ShadowMapRadialShaderContext(const SkRadialShadowMapShaderImpl&, const ContextRec&,
                                 SkShader::Context* occluderContext,
                                 void* heapAllocated);

        ~ShadowMapRadialShaderContext() override;

        void shadeSpan(int x, int y, SkPMColor[], int count) override;

        uint32_t getFlags() const override { return fFlags; }

    private:
        SkShader::Context*        fOccluderContext;
        uint32_t                  fFlags;

        void* fHeapAllocated;

        typedef SkShader::Context INHERITED;
    };

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkRadialShadowMapShaderImpl)

protected:
    void flatten(SkWriteBuffer&) const override;
    size_t onContextSize(const ContextRec&) const override;
    Context* onCreateContext(const ContextRec&, void*) const override;

private:
    sk_sp<SkShader> fOccluderShader;
    sk_sp<SkLights> fLight;

    int fWidth;
    int fHeight;

    friend class SkRadialShadowMapShader;

    typedef SkShader INHERITED;
};

////////////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU

#include "GrContext.h"
#include "GrCoordTransform.h"
#include "GrFragmentProcessor.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "SkGr.h"
#include "SkImage_Base.h"
#include "GrInvariantOutput.h"
#include "SkSpecialImage.h"

class RadialShadowMapFP : public GrFragmentProcessor {
public:
    RadialShadowMapFP(sk_sp<GrFragmentProcessor> occluder,
                      sk_sp<SkLights> light,
                      int diffuseWidth, int diffuseHeight,
                      GrContext* context) {
        fLightPos = light->light(0).pos();

        fWidth = diffuseWidth;
        fHeight = diffuseHeight;

        this->registerChildProcessor(std::move(occluder));
        this->initClassID<RadialShadowMapFP>();
    }

    class GLSLRadialShadowMapFP : public GrGLSLFragmentProcessor {
    public:
        GLSLRadialShadowMapFP() { }

        void emitCode(EmitArgs& args) override {

            GrGLSLFragmentBuilder* fragBuilder = args.fFragBuilder;
            GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;

            const char* lightPosUniName = nullptr;

            fLightPosUni = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                      kVec3f_GrSLType,
                                                      kDefault_GrSLPrecision,
                                                      "lightPos",
                                                      &lightPosUniName);

            const char* widthUniName = nullptr;
            const char* heightUniName = nullptr;

            fWidthUni = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                   kInt_GrSLType,
                                                   kDefault_GrSLPrecision,
                                                   "width", &widthUniName);
            fHeightUni = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                    kInt_GrSLType,
                                                    kDefault_GrSLPrecision,
                                                    "height", &heightUniName);


            SkString occluder("occluder");
            this->emitChild(0, nullptr, &occluder, args);

            // Modify the input texture coordinates to index into our 1D output
            fragBuilder->codeAppend("float distHere;");

            // we use a max shadow distance of 2 times the max of width/height
            fragBuilder->codeAppend("float closestDistHere = 2;");
            fragBuilder->codeAppend("vec2 coords = vMatrixCoord_0_0_Stage0;");
            fragBuilder->codeAppend("coords.y = 0;");
            fragBuilder->codeAppend("vec2 destCoords = vec2(0,0);");
            fragBuilder->codeAppendf("float step = 1.0 / %s;", heightUniName);

            // assume that we are at 0, 0 light pos
            // TODO use correct light positions

            // this goes through each depth value in the final output buffer,
            // basically raycasting outwards, and finding the first collision.
            // we also increment coords.y to 2 instead 1 so our shadows stretch the whole screen.
            fragBuilder->codeAppendf("for (coords.y = 0; coords.y <= 2; coords.y += step) {");

                fragBuilder->codeAppend("float theta = (coords.x * 2.0 - 1.0) * 3.1415;");
                fragBuilder->codeAppend("float r = coords.y;");
                fragBuilder->codeAppend("destCoords = "
                        "vec2(r * cos(theta), - r * sin(theta)) /2.0 + 0.5;");
                fragBuilder->codeAppendf("vec2 lightOffset = (vec2(%s)/vec2(%s,%s) - 0.5)"
                                                            "* vec2(1.0, 1.0);",
                                         lightPosUniName, widthUniName, heightUniName);

                fragBuilder->codeAppend("distHere = texture(uTextureSampler0_Stage1,"
                                                           "destCoords + lightOffset).b;");
                fragBuilder->codeAppend("if (distHere > 0.0) {"
                                            "closestDistHere = coords.y;"
                                        "break;}");
            fragBuilder->codeAppend("}");

            fragBuilder->codeAppendf("%s = vec4(vec3(closestDistHere / 2.0),1);", args.fOutputColor);
        }

        static void GenKey(const GrProcessor& proc, const GrShaderCaps&,
                           GrProcessorKeyBuilder* b) {
            b->add32(0); // nothing to add here
        }

    protected:
        void onSetData(const GrGLSLProgramDataManager& pdman,
                       const GrFragmentProcessor& proc) override {
            const RadialShadowMapFP &radialShadowMapFP = proc.cast<RadialShadowMapFP>();

            const SkVector3& lightPos = radialShadowMapFP.lightPos();
            if (lightPos != fLightPos) {
                pdman.set3fv(fLightPosUni, 1, &lightPos.fX);
                fLightPos = lightPos;
            }

            int width = radialShadowMapFP.width();
            if (width != fWidth) {
                pdman.set1i(fWidthUni, width);
                fWidth = width;
            }
            int height = radialShadowMapFP.height();
            if (height != fHeight) {
                pdman.set1i(fHeightUni, height);
                fHeight = height;
            }
        }

    private:
        SkVector3 fLightPos;
        GrGLSLProgramDataManager::UniformHandle fLightPosUni;

        int fWidth;
        GrGLSLProgramDataManager::UniformHandle fWidthUni;
        int fHeight;
        GrGLSLProgramDataManager::UniformHandle fHeightUni;
    };

    void onGetGLSLProcessorKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const override {
        GLSLRadialShadowMapFP::GenKey(*this, caps, b);
    }

    const char* name() const override { return "RadialShadowMapFP"; }

    const SkVector3& lightPos() const {
        return fLightPos;
    }

    int width() const { return fWidth; }
    int height() const { return fHeight; }

private:
    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override {
        return new GLSLRadialShadowMapFP;
    }

    bool onIsEqual(const GrFragmentProcessor& proc) const override {
        const RadialShadowMapFP& radialShadowMapFP = proc.cast<RadialShadowMapFP>();

        if (fWidth != radialShadowMapFP.fWidth || fHeight != radialShadowMapFP.fHeight) {
            return false;
        }

        if (fLightPos != radialShadowMapFP.fLightPos) {
            return false;
        }

        return true;
    }

    SkVector3        fLightPos;

    int              fHeight;
    int              fWidth;
};

////////////////////////////////////////////////////////////////////////////

sk_sp<GrFragmentProcessor> SkRadialShadowMapShaderImpl::asFragmentProcessor
        (const AsFPArgs& fpargs) const {

    sk_sp<GrFragmentProcessor> occluderFP = fOccluderShader->asFragmentProcessor(fpargs);

    sk_sp<GrFragmentProcessor> shadowFP = sk_make_sp<RadialShadowMapFP>(std::move(occluderFP),
                                                                        fLight, fWidth, fHeight,
                                                                        fpargs.fContext);
    return shadowFP;
}

#endif

////////////////////////////////////////////////////////////////////////////

bool SkRadialShadowMapShaderImpl::isOpaque() const {
    return fOccluderShader->isOpaque();
}

SkRadialShadowMapShaderImpl::ShadowMapRadialShaderContext::ShadowMapRadialShaderContext(
        const SkRadialShadowMapShaderImpl& shader, const ContextRec& rec,
        SkShader::Context* occluderContext,
        void* heapAllocated)
        : INHERITED(shader, rec)
        , fOccluderContext(occluderContext)
        , fHeapAllocated(heapAllocated) {
    bool isOpaque = shader.isOpaque();

    // update fFlags
    uint32_t flags = 0;
    if (isOpaque && (255 == this->getPaintAlpha())) {
        flags |= kOpaqueAlpha_Flag;
    }

    fFlags = flags;
}

SkRadialShadowMapShaderImpl::ShadowMapRadialShaderContext::~ShadowMapRadialShaderContext() {
    // The dependencies have been created outside of the context on memory that was allocated by
    // the onCreateContext() method. Call the destructors and free the memory.
    fOccluderContext->~Context();

    sk_free(fHeapAllocated);
}

static inline SkPMColor convert(SkColor3f color, U8CPU a) {
    if (color.fX <= 0.0f) {
        color.fX = 0.0f;
    } else if (color.fX >= 255.0f) {
        color.fX = 255.0f;
    }

    if (color.fY <= 0.0f) {
        color.fY = 0.0f;
    } else if (color.fY >= 255.0f) {
        color.fY = 255.0f;
    }

    if (color.fZ <= 0.0f) {
        color.fZ = 0.0f;
    } else if (color.fZ >= 255.0f) {
        color.fZ = 255.0f;
    }

    return SkPreMultiplyARGB(a, (int) color.fX,  (int) color.fY, (int) color.fZ);
}

// larger is better (fewer times we have to loop), but we shouldn't
// take up too much stack-space (each one here costs 16 bytes)
#define BUFFER_MAX 16
void SkRadialShadowMapShaderImpl::ShadowMapRadialShaderContext::shadeSpan
        (int x, int y, SkPMColor result[], int count) {
    do {
        int n = SkTMin(count, BUFFER_MAX);

        // just fill with white for now
        SkPMColor accum = convert(SkColor3f::Make(1.0f, 1.0f, 1.0f), 0xFF);

        for (int i = 0; i < n; ++i) {
            result[i] = accum;
        }

        result += n;
        x += n;
        count -= n;
    } while (count > 0);
}

////////////////////////////////////////////////////////////////////////////

#ifndef SK_IGNORE_TO_STRING
void SkRadialShadowMapShaderImpl::toString(SkString* str) const {
    str->appendf("RadialShadowMapShader: ()");
}
#endif

sk_sp<SkFlattenable> SkRadialShadowMapShaderImpl::CreateProc(SkReadBuffer& buf) {

    // Discarding SkShader flattenable params
    bool hasLocalMatrix = buf.readBool();
    SkAssertResult(!hasLocalMatrix);

    sk_sp<SkLights> light = SkLights::MakeFromBuffer(buf);

    int diffuseWidth = buf.readInt();
    int diffuseHeight = buf.readInt();

    sk_sp<SkShader> occluderShader(buf.readFlattenable<SkShader>());

    return sk_make_sp<SkRadialShadowMapShaderImpl>(std::move(occluderShader),
                                                   std::move(light),
                                                   diffuseWidth, diffuseHeight);
}

void SkRadialShadowMapShaderImpl::flatten(SkWriteBuffer& buf) const {
    this->INHERITED::flatten(buf);

    fLight->flatten(buf);

    buf.writeInt(fWidth);
    buf.writeInt(fHeight);

    buf.writeFlattenable(fOccluderShader.get());
}

size_t SkRadialShadowMapShaderImpl::onContextSize(const ContextRec& rec) const {
    return sizeof(ShadowMapRadialShaderContext);
}

SkShader::Context* SkRadialShadowMapShaderImpl::onCreateContext(const ContextRec& rec,
                                                                void* storage) const {
    size_t heapRequired = fOccluderShader->contextSize(rec);

    void* heapAllocated = sk_malloc_throw(heapRequired);

    void* occluderContextStorage = heapAllocated;

    SkShader::Context* occluderContext =
            fOccluderShader->createContext(rec, occluderContextStorage);

    if (!occluderContext) {
        sk_free(heapAllocated);
        return nullptr;
    }

    return new (storage) ShadowMapRadialShaderContext(*this, rec, occluderContext, heapAllocated);
}

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkShader> SkRadialShadowMapShader::Make(sk_sp<SkShader> occluderShader,
                                              sk_sp<SkLights> light,
                                              int diffuseWidth, int diffuseHeight) {
    if (!occluderShader) {
        // TODO: Use paint's color in absence of a diffuseShader
        // TODO: Use a default implementation of normalSource instead
        return nullptr;
    }

    return sk_make_sp<SkRadialShadowMapShaderImpl>(std::move(occluderShader),
                                                   std::move(light),
                                                   diffuseWidth, diffuseHeight);
}

///////////////////////////////////////////////////////////////////////////////

SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(SkRadialShadowMapShader)
SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkRadialShadowMapShaderImpl)
SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END

///////////////////////////////////////////////////////////////////////////////

#endif
