/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gl/builders/GrGLProgramBuilder.h"
#include "GrAARectRenderer.h"
#include "GrGpu.h"
#include "gl/GrGLEffect.h"
#include "gl/GrGLVertexEffect.h"
#include "GrTBackendEffectFactory.h"
#include "SkColorPriv.h"
#include "effects/GrVertexEffect.h"

///////////////////////////////////////////////////////////////////////////////
class GrGLAlignedRectEffect;

// Axis Aligned special case
class GrAlignedRectEffect : public GrVertexEffect {
public:
    static GrEffect* Create() {
        GR_CREATE_STATIC_EFFECT(gAlignedRectEffect, GrAlignedRectEffect, ());
        gAlignedRectEffect->ref();
        return gAlignedRectEffect;
    }

    virtual ~GrAlignedRectEffect() {}

    static const char* Name() { return "AlignedRectEdge"; }

    virtual void getConstantColorComponents(GrColor* color,
                                            uint32_t* validFlags) const SK_OVERRIDE {
        *validFlags = 0;
    }

    virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE {
        return GrTBackendEffectFactory<GrAlignedRectEffect>::getInstance();
    }

    class GLEffect : public GrGLVertexEffect {
    public:
        GLEffect(const GrBackendEffectFactory& factory, const GrDrawEffect&)
        : INHERITED (factory) {}

        virtual void emitCode(GrGLFullProgramBuilder* builder,
                              const GrDrawEffect& drawEffect,
                              const GrEffectKey& key,
                              const char* outputColor,
                              const char* inputColor,
                              const TransformedCoordsArray&,
                              const TextureSamplerArray& samplers) SK_OVERRIDE {
            // setup the varying for the Axis aligned rect effect
            //      xy -> interpolated offset
            //      zw -> w/2+0.5, h/2+0.5
            const char *vsRectName, *fsRectName;
            builder->addVarying(kVec4f_GrSLType, "Rect", &vsRectName, &fsRectName);

            GrGLVertexShaderBuilder* vsBuilder = builder->getVertexShaderBuilder();
            const SkString* attr0Name =
                vsBuilder->getEffectAttributeName(drawEffect.getVertexAttribIndices()[0]);
            vsBuilder->codeAppendf("\t%s = %s;\n", vsRectName, attr0Name->c_str());

            GrGLFragmentShaderBuilder* fsBuilder = builder->getFragmentShaderBuilder();
            // TODO: compute all these offsets, spans, and scales in the VS
            fsBuilder->codeAppendf("\tfloat insetW = min(1.0, %s.z) - 0.5;\n", fsRectName);
            fsBuilder->codeAppendf("\tfloat insetH = min(1.0, %s.w) - 0.5;\n", fsRectName);
            fsBuilder->codeAppend("\tfloat outset = 0.5;\n");
            // For rects > 1 pixel wide and tall the span's are noops (i.e., 1.0). For rects
            // < 1 pixel wide or tall they serve to normalize the < 1 ramp to a 0 .. 1 range.
            fsBuilder->codeAppend("\tfloat spanW = insetW + outset;\n");
            fsBuilder->codeAppend("\tfloat spanH = insetH + outset;\n");
            // For rects < 1 pixel wide or tall, these scale factors are used to cap the maximum
            // value of coverage that is used. In other words it is the coverage that is
            // used in the interior of the rect after the ramp.
            fsBuilder->codeAppend("\tfloat scaleW = min(1.0, 2.0*insetW/spanW);\n");
            fsBuilder->codeAppend("\tfloat scaleH = min(1.0, 2.0*insetH/spanH);\n");

            // Compute the coverage for the rect's width
            fsBuilder->codeAppendf(
                "\tfloat coverage = scaleW*clamp((%s.z-abs(%s.x))/spanW, 0.0, 1.0);\n", fsRectName,
                fsRectName);
            // Compute the coverage for the rect's height and merge with the width
            fsBuilder->codeAppendf(
                "\tcoverage = coverage*scaleH*clamp((%s.w-abs(%s.y))/spanH, 0.0, 1.0);\n",
                fsRectName, fsRectName);


            fsBuilder->codeAppendf("\t%s = %s;\n", outputColor,
                                   (GrGLSLExpr4(inputColor) * GrGLSLExpr1("coverage")).c_str());
        }

        static void GenKey(const GrDrawEffect&, const GrGLCaps&, GrEffectKeyBuilder*) {}

        virtual void setData(const GrGLProgramDataManager& pdman, const GrDrawEffect&) SK_OVERRIDE {}

    private:
        typedef GrGLVertexEffect INHERITED;
    };


private:
    GrAlignedRectEffect() : GrVertexEffect() {
        this->addVertexAttrib(kVec4f_GrSLType);
    }

    virtual bool onIsEqual(const GrEffect&) const SK_OVERRIDE { return true; }

    GR_DECLARE_EFFECT_TEST;

    typedef GrVertexEffect INHERITED;
};


GR_DEFINE_EFFECT_TEST(GrAlignedRectEffect);

GrEffect* GrAlignedRectEffect::TestCreate(SkRandom* random,
                                          GrContext* context,
                                          const GrDrawTargetCaps&,
                                          GrTexture* textures[]) {
    return GrAlignedRectEffect::Create();
}

///////////////////////////////////////////////////////////////////////////////
class GrGLRectEffect;

/**
 * The output of this effect is a modulation of the input color and coverage
 * for an arbitrarily oriented rect. The rect is specified as:
 *      Center of the rect
 *      Unit vector point down the height of the rect
 *      Half width + 0.5
 *      Half height + 0.5
 * The center and vector are stored in a vec4 varying ("RectEdge") with the
 * center in the xy components and the vector in the zw components.
 * The munged width and height are stored in a vec2 varying ("WidthHeight")
 * with the width in x and the height in y.
 */
class GrRectEffect : public GrVertexEffect {
public:
    static GrEffect* Create() {
        GR_CREATE_STATIC_EFFECT(gRectEffect, GrRectEffect, ());
        gRectEffect->ref();
        return gRectEffect;
    }

    virtual ~GrRectEffect() {}

    static const char* Name() { return "RectEdge"; }

    virtual void getConstantColorComponents(GrColor* color,
                                            uint32_t* validFlags) const SK_OVERRIDE {
        *validFlags = 0;
    }

    virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE {
        return GrTBackendEffectFactory<GrRectEffect>::getInstance();
    }

    class GLEffect : public GrGLVertexEffect {
    public:
        GLEffect(const GrBackendEffectFactory& factory, const GrDrawEffect&)
        : INHERITED (factory) {}

        virtual void emitCode(GrGLFullProgramBuilder* builder,
                              const GrDrawEffect& drawEffect,
                              const GrEffectKey& key,
                              const char* outputColor,
                              const char* inputColor,
                              const TransformedCoordsArray&,
                              const TextureSamplerArray& samplers) SK_OVERRIDE {
            // setup the varying for the center point and the unit vector
            // that points down the height of the rect
            const char *vsRectEdgeName, *fsRectEdgeName;
            builder->addVarying(kVec4f_GrSLType, "RectEdge",
                                &vsRectEdgeName, &fsRectEdgeName);

            GrGLVertexShaderBuilder* vsBuilder = builder->getVertexShaderBuilder();
            const SkString* attr0Name =
                vsBuilder->getEffectAttributeName(drawEffect.getVertexAttribIndices()[0]);
            vsBuilder->codeAppendf("\t%s = %s;\n", vsRectEdgeName, attr0Name->c_str());

            // setup the varying for width/2+.5 and height/2+.5
            const char *vsWidthHeightName, *fsWidthHeightName;
            builder->addVarying(kVec2f_GrSLType, "WidthHeight",
                                &vsWidthHeightName, &fsWidthHeightName);
            const SkString* attr1Name =
                vsBuilder->getEffectAttributeName(drawEffect.getVertexAttribIndices()[1]);
            vsBuilder->codeAppendf("\t%s = %s;\n", vsWidthHeightName, attr1Name->c_str());

            GrGLFragmentShaderBuilder* fsBuilder = builder->getFragmentShaderBuilder();
            // TODO: compute all these offsets, spans, and scales in the VS
            fsBuilder->codeAppendf("\tfloat insetW = min(1.0, %s.x) - 0.5;\n", fsWidthHeightName);
            fsBuilder->codeAppendf("\tfloat insetH = min(1.0, %s.y) - 0.5;\n", fsWidthHeightName);
            fsBuilder->codeAppend("\tfloat outset = 0.5;\n");
            // For rects > 1 pixel wide and tall the span's are noops (i.e., 1.0). For rects
            // < 1 pixel wide or tall they serve to normalize the < 1 ramp to a 0 .. 1 range.
            fsBuilder->codeAppend("\tfloat spanW = insetW + outset;\n");
            fsBuilder->codeAppend("\tfloat spanH = insetH + outset;\n");
            // For rects < 1 pixel wide or tall, these scale factors are used to cap the maximum
            // value of coverage that is used. In other words it is the coverage that is
            // used in the interior of the rect after the ramp.
            fsBuilder->codeAppend("\tfloat scaleW = min(1.0, 2.0*insetW/spanW);\n");
            fsBuilder->codeAppend("\tfloat scaleH = min(1.0, 2.0*insetH/spanH);\n");

            // Compute the coverage for the rect's width
            fsBuilder->codeAppendf("\tvec2 offset = %s.xy - %s.xy;\n",
                                   fsBuilder->fragmentPosition(), fsRectEdgeName);
            fsBuilder->codeAppendf("\tfloat perpDot = abs(offset.x * %s.w - offset.y * %s.z);\n",
                                   fsRectEdgeName, fsRectEdgeName);
            fsBuilder->codeAppendf(
                "\tfloat coverage = scaleW*clamp((%s.x-perpDot)/spanW, 0.0, 1.0);\n",
                fsWidthHeightName);

            // Compute the coverage for the rect's height and merge with the width
            fsBuilder->codeAppendf("\tperpDot = abs(dot(offset, %s.zw));\n",
                                   fsRectEdgeName);
            fsBuilder->codeAppendf(
                    "\tcoverage = coverage*scaleH*clamp((%s.y-perpDot)/spanH, 0.0, 1.0);\n",
                    fsWidthHeightName);


            fsBuilder->codeAppendf("\t%s = %s;\n", outputColor,
                                   (GrGLSLExpr4(inputColor) * GrGLSLExpr1("coverage")).c_str());
        }

        static void GenKey(const GrDrawEffect&, const GrGLCaps&, GrEffectKeyBuilder*) {}

        virtual void setData(const GrGLProgramDataManager& pdman, const GrDrawEffect&) SK_OVERRIDE {}

    private:
        typedef GrGLVertexEffect INHERITED;
    };


private:
    GrRectEffect() : GrVertexEffect() {
        this->addVertexAttrib(kVec4f_GrSLType);
        this->addVertexAttrib(kVec2f_GrSLType);
        this->setWillReadFragmentPosition();
    }

    virtual bool onIsEqual(const GrEffect&) const SK_OVERRIDE { return true; }

    GR_DECLARE_EFFECT_TEST;

    typedef GrVertexEffect INHERITED;
};


GR_DEFINE_EFFECT_TEST(GrRectEffect);

GrEffect* GrRectEffect::TestCreate(SkRandom* random,
                                   GrContext* context,
                                   const GrDrawTargetCaps&,
                                   GrTexture* textures[]) {
    return GrRectEffect::Create();
}

///////////////////////////////////////////////////////////////////////////////

namespace {
extern const GrVertexAttrib gAARectAttribs[] = {
    {kVec2f_GrVertexAttribType,  0,                                 kPosition_GrVertexAttribBinding},
    {kVec4ub_GrVertexAttribType, sizeof(SkPoint),                   kColor_GrVertexAttribBinding},
    {kVec4ub_GrVertexAttribType, sizeof(SkPoint) + sizeof(SkColor), kCoverage_GrVertexAttribBinding},
};

// Should the coverage be multiplied into the color attrib or use a separate attrib.
enum CoverageAttribType {
    kUseColor_CoverageAttribType,
    kUseCoverage_CoverageAttribType,
};
}

static CoverageAttribType set_rect_attribs(GrDrawState* drawState) {
    if (drawState->canTweakAlphaForCoverage()) {
        drawState->setVertexAttribs<gAARectAttribs>(2);
        return kUseColor_CoverageAttribType;
    } else {
        drawState->setVertexAttribs<gAARectAttribs>(3);
        return kUseCoverage_CoverageAttribType;
    }
}

static void set_inset_fan(SkPoint* pts, size_t stride,
                          const SkRect& r, SkScalar dx, SkScalar dy) {
    pts->setRectFan(r.fLeft + dx, r.fTop + dy,
                    r.fRight - dx, r.fBottom - dy, stride);
}

void GrAARectRenderer::reset() {
    SkSafeSetNull(fAAFillRectIndexBuffer);
    SkSafeSetNull(fAAMiterStrokeRectIndexBuffer);
    SkSafeSetNull(fAABevelStrokeRectIndexBuffer);
}

static const uint16_t gFillAARectIdx[] = {
    0, 1, 5, 5, 4, 0,
    1, 2, 6, 6, 5, 1,
    2, 3, 7, 7, 6, 2,
    3, 0, 4, 4, 7, 3,
    4, 5, 6, 6, 7, 4,
};

static const int kIndicesPerAAFillRect = SK_ARRAY_COUNT(gFillAARectIdx);
static const int kVertsPerAAFillRect = 8;
static const int kNumAAFillRectsInIndexBuffer = 256;

GrIndexBuffer* GrAARectRenderer::aaFillRectIndexBuffer(GrGpu* gpu) {
    static const size_t kAAFillRectIndexBufferSize = kIndicesPerAAFillRect *
                                                     sizeof(uint16_t) *
                                                     kNumAAFillRectsInIndexBuffer;

    if (NULL == fAAFillRectIndexBuffer) {
        fAAFillRectIndexBuffer = gpu->createIndexBuffer(kAAFillRectIndexBufferSize, false);
        if (NULL != fAAFillRectIndexBuffer) {
            uint16_t* data = (uint16_t*) fAAFillRectIndexBuffer->map();
            bool useTempData = (NULL == data);
            if (useTempData) {
                data = SkNEW_ARRAY(uint16_t, kNumAAFillRectsInIndexBuffer * kIndicesPerAAFillRect);
            }
            for (int i = 0; i < kNumAAFillRectsInIndexBuffer; ++i) {
                // Each AA filled rect is drawn with 8 vertices and 10 triangles (8 around
                // the inner rect (for AA) and 2 for the inner rect.
                int baseIdx = i * kIndicesPerAAFillRect;
                uint16_t baseVert = (uint16_t)(i * kVertsPerAAFillRect);
                for (int j = 0; j < kIndicesPerAAFillRect; ++j) {
                    data[baseIdx+j] = baseVert + gFillAARectIdx[j];
                }
            }
            if (useTempData) {
                if (!fAAFillRectIndexBuffer->updateData(data, kAAFillRectIndexBufferSize)) {
                    SkFAIL("Can't get AA Fill Rect indices into buffer!");
                }
                SkDELETE_ARRAY(data);
            } else {
                fAAFillRectIndexBuffer->unmap();
            }
        }
    }

    return fAAFillRectIndexBuffer;
}

static const uint16_t gMiterStrokeAARectIdx[] = {
    0 + 0, 1 + 0, 5 + 0, 5 + 0, 4 + 0, 0 + 0,
    1 + 0, 2 + 0, 6 + 0, 6 + 0, 5 + 0, 1 + 0,
    2 + 0, 3 + 0, 7 + 0, 7 + 0, 6 + 0, 2 + 0,
    3 + 0, 0 + 0, 4 + 0, 4 + 0, 7 + 0, 3 + 0,

    0 + 4, 1 + 4, 5 + 4, 5 + 4, 4 + 4, 0 + 4,
    1 + 4, 2 + 4, 6 + 4, 6 + 4, 5 + 4, 1 + 4,
    2 + 4, 3 + 4, 7 + 4, 7 + 4, 6 + 4, 2 + 4,
    3 + 4, 0 + 4, 4 + 4, 4 + 4, 7 + 4, 3 + 4,

    0 + 8, 1 + 8, 5 + 8, 5 + 8, 4 + 8, 0 + 8,
    1 + 8, 2 + 8, 6 + 8, 6 + 8, 5 + 8, 1 + 8,
    2 + 8, 3 + 8, 7 + 8, 7 + 8, 6 + 8, 2 + 8,
    3 + 8, 0 + 8, 4 + 8, 4 + 8, 7 + 8, 3 + 8,
};

/**
 * As in miter-stroke, index = a + b, and a is the current index, b is the shift
 * from the first index. The index layout:
 * outer AA line: 0~3, 4~7
 * outer edge:    8~11, 12~15
 * inner edge:    16~19
 * inner AA line: 20~23
 * Following comes a bevel-stroke rect and its indices:
 *
 *           4                                 7
 *            *********************************
 *          *   ______________________________  *
 *         *  / 12                          15 \  *
 *        *  /                                  \  *
 *     0 *  |8     16_____________________19  11 |  * 3
 *       *  |       |                    |       |  *
 *       *  |       |  ****************  |       |  *
 *       *  |       |  * 20        23 *  |       |  *
 *       *  |       |  *              *  |       |  *
 *       *  |       |  * 21        22 *  |       |  *
 *       *  |       |  ****************  |       |  *
 *       *  |       |____________________|       |  *
 *     1 *  |9    17                      18   10|  * 2
 *        *  \                                  /  *
 *         *  \13 __________________________14/  *
 *          *                                   *
 *           **********************************
 *          5                                  6
 */
static const uint16_t gBevelStrokeAARectIdx[] = {
    // Draw outer AA, from outer AA line to outer edge, shift is 0.
    0 + 0, 1 + 0, 9 + 0, 9 + 0, 8 + 0, 0 + 0,
    1 + 0, 5 + 0, 13 + 0, 13 + 0, 9 + 0, 1 + 0,
    5 + 0, 6 + 0, 14 + 0, 14 + 0, 13 + 0, 5 + 0,
    6 + 0, 2 + 0, 10 + 0, 10 + 0, 14 + 0, 6 + 0,
    2 + 0, 3 + 0, 11 + 0, 11 + 0, 10 + 0, 2 + 0,
    3 + 0, 7 + 0, 15 + 0, 15 + 0, 11 + 0, 3 + 0,
    7 + 0, 4 + 0, 12 + 0, 12 + 0, 15 + 0, 7 + 0,
    4 + 0, 0 + 0, 8 + 0, 8 + 0, 12 + 0, 4 + 0,

    // Draw the stroke, from outer edge to inner edge, shift is 8.
    0 + 8, 1 + 8, 9 + 8, 9 + 8, 8 + 8, 0 + 8,
    1 + 8, 5 + 8, 9 + 8,
    5 + 8, 6 + 8, 10 + 8, 10 + 8, 9 + 8, 5 + 8,
    6 + 8, 2 + 8, 10 + 8,
    2 + 8, 3 + 8, 11 + 8, 11 + 8, 10 + 8, 2 + 8,
    3 + 8, 7 + 8, 11 + 8,
    7 + 8, 4 + 8, 8 + 8, 8 + 8, 11 + 8, 7 + 8,
    4 + 8, 0 + 8, 8 + 8,

    // Draw the inner AA, from inner edge to inner AA line, shift is 16.
    0 + 16, 1 + 16, 5 + 16, 5 + 16, 4 + 16, 0 + 16,
    1 + 16, 2 + 16, 6 + 16, 6 + 16, 5 + 16, 1 + 16,
    2 + 16, 3 + 16, 7 + 16, 7 + 16, 6 + 16, 2 + 16,
    3 + 16, 0 + 16, 4 + 16, 4 + 16, 7 + 16, 3 + 16,
};

int GrAARectRenderer::aaStrokeRectIndexCount(bool miterStroke) {
    return miterStroke ? SK_ARRAY_COUNT(gMiterStrokeAARectIdx) :
                         SK_ARRAY_COUNT(gBevelStrokeAARectIdx);
}

GrIndexBuffer* GrAARectRenderer::aaStrokeRectIndexBuffer(GrGpu* gpu, bool miterStroke) {
    if (miterStroke) {
        if (NULL == fAAMiterStrokeRectIndexBuffer) {
            fAAMiterStrokeRectIndexBuffer =
                gpu->createIndexBuffer(sizeof(gMiterStrokeAARectIdx), false);
            if (NULL != fAAMiterStrokeRectIndexBuffer) {
#ifdef SK_DEBUG
                bool updated =
#endif
                fAAMiterStrokeRectIndexBuffer->updateData(gMiterStrokeAARectIdx,
                                                          sizeof(gMiterStrokeAARectIdx));
                GR_DEBUGASSERT(updated);
            }
        }
        return fAAMiterStrokeRectIndexBuffer;
    } else {
        if (NULL == fAABevelStrokeRectIndexBuffer) {
            fAABevelStrokeRectIndexBuffer =
                gpu->createIndexBuffer(sizeof(gBevelStrokeAARectIdx), false);
            if (NULL != fAABevelStrokeRectIndexBuffer) {
#ifdef SK_DEBUG
                bool updated =
#endif
                fAABevelStrokeRectIndexBuffer->updateData(gBevelStrokeAARectIdx,
                                                          sizeof(gBevelStrokeAARectIdx));
                GR_DEBUGASSERT(updated);
            }
        }
        return fAABevelStrokeRectIndexBuffer;
    }
}

void GrAARectRenderer::geometryFillAARect(GrGpu* gpu,
                                          GrDrawTarget* target,
                                          const SkRect& rect,
                                          const SkMatrix& combinedMatrix,
                                          const SkRect& devRect) {
    GrDrawState* drawState = target->drawState();

    GrColor color = drawState->getColor();

    CoverageAttribType covAttribType = set_rect_attribs(drawState);
    if (kUseCoverage_CoverageAttribType == covAttribType && GrColorIsOpaque(color)) {
        drawState->setHint(GrDrawState::kVertexColorsAreOpaque_Hint, true);
    }

    GrDrawTarget::AutoReleaseGeometry geo(target, 8, 0);
    if (!geo.succeeded()) {
        GrPrintf("Failed to get space for vertices!\n");
        return;
    }

    GrIndexBuffer* indexBuffer = this->aaFillRectIndexBuffer(gpu);
    if (NULL == indexBuffer) {
        GrPrintf("Failed to create index buffer!\n");
        return;
    }

    intptr_t verts = reinterpret_cast<intptr_t>(geo.vertices());
    size_t vsize = drawState->getVertexSize();

    SkPoint* fan0Pos = reinterpret_cast<SkPoint*>(verts);
    SkPoint* fan1Pos = reinterpret_cast<SkPoint*>(verts + 4 * vsize);

    SkScalar inset = SkMinScalar(devRect.width(), SK_Scalar1);
    inset = SK_ScalarHalf * SkMinScalar(inset, devRect.height());

    if (combinedMatrix.rectStaysRect()) {
        // Temporarily #if'ed out. We don't want to pass in the devRect but
        // right now it is computed in GrContext::apply_aa_to_rect and we don't
        // want to throw away the work
#if 0
        SkRect devRect;
        combinedMatrix.mapRect(&devRect, rect);
#endif

        set_inset_fan(fan0Pos, vsize, devRect, -SK_ScalarHalf, -SK_ScalarHalf);
        set_inset_fan(fan1Pos, vsize, devRect, inset,  inset);
    } else {
        // compute transformed (1, 0) and (0, 1) vectors
        SkVector vec[2] = {
          { combinedMatrix[SkMatrix::kMScaleX], combinedMatrix[SkMatrix::kMSkewY] },
          { combinedMatrix[SkMatrix::kMSkewX],  combinedMatrix[SkMatrix::kMScaleY] }
        };

        vec[0].normalize();
        vec[0].scale(SK_ScalarHalf);
        vec[1].normalize();
        vec[1].scale(SK_ScalarHalf);

        // create the rotated rect
        fan0Pos->setRectFan(rect.fLeft, rect.fTop,
                            rect.fRight, rect.fBottom, vsize);
        combinedMatrix.mapPointsWithStride(fan0Pos, vsize, 4);

        // Now create the inset points and then outset the original
        // rotated points

        // TL
        *((SkPoint*)((intptr_t)fan1Pos + 0 * vsize)) =
            *((SkPoint*)((intptr_t)fan0Pos + 0 * vsize)) + vec[0] + vec[1];
        *((SkPoint*)((intptr_t)fan0Pos + 0 * vsize)) -= vec[0] + vec[1];
        // BL
        *((SkPoint*)((intptr_t)fan1Pos + 1 * vsize)) =
            *((SkPoint*)((intptr_t)fan0Pos + 1 * vsize)) + vec[0] - vec[1];
        *((SkPoint*)((intptr_t)fan0Pos + 1 * vsize)) -= vec[0] - vec[1];
        // BR
        *((SkPoint*)((intptr_t)fan1Pos + 2 * vsize)) =
            *((SkPoint*)((intptr_t)fan0Pos + 2 * vsize)) - vec[0] - vec[1];
        *((SkPoint*)((intptr_t)fan0Pos + 2 * vsize)) += vec[0] + vec[1];
        // TR
        *((SkPoint*)((intptr_t)fan1Pos + 3 * vsize)) =
            *((SkPoint*)((intptr_t)fan0Pos + 3 * vsize)) - vec[0] + vec[1];
        *((SkPoint*)((intptr_t)fan0Pos + 3 * vsize)) += vec[0] - vec[1];
    }

    // Make verts point to vertex color and then set all the color and coverage vertex attrs values.
    verts += sizeof(SkPoint);
    for (int i = 0; i < 4; ++i) {
        if (kUseCoverage_CoverageAttribType == covAttribType) {
            *reinterpret_cast<GrColor*>(verts + i * vsize) = color;
            *reinterpret_cast<GrColor*>(verts + i * vsize + sizeof(GrColor)) = 0;
        } else {
            *reinterpret_cast<GrColor*>(verts + i * vsize) = 0;
        }
    }

    int scale;
    if (inset < SK_ScalarHalf) {
        scale = SkScalarFloorToInt(512.0f * inset / (inset + SK_ScalarHalf));
        SkASSERT(scale >= 0 && scale <= 255);
    } else {
        scale = 0xff;
    }

    GrColor innerCoverage;
    if (kUseCoverage_CoverageAttribType == covAttribType) {
        innerCoverage = GrColorPackRGBA(scale, scale, scale, scale); 
    } else {
        innerCoverage = (0xff == scale) ? color : SkAlphaMulQ(color, scale);
    }
    verts += 4 * vsize;
    for (int i = 0; i < 4; ++i) {
        if (kUseCoverage_CoverageAttribType == covAttribType) {
            *reinterpret_cast<GrColor*>(verts + i * vsize) = color;
            *reinterpret_cast<GrColor*>(verts + i * vsize + sizeof(GrColor)) = innerCoverage;
        } else {
            *reinterpret_cast<GrColor*>(verts + i * vsize) = innerCoverage;
        }
    }

    target->setIndexSourceToBuffer(indexBuffer);
    target->drawIndexedInstances(kTriangles_GrPrimitiveType, 1,
                                 kVertsPerAAFillRect,
                                 kIndicesPerAAFillRect);
    target->resetIndexSource();
}

namespace {

// Rotated
struct RectVertex {
    SkPoint fPos;
    SkPoint fCenter;
    SkPoint fDir;
    SkPoint fWidthHeight;
};

// Rotated
extern const GrVertexAttrib gAARectVertexAttribs[] = {
    { kVec2f_GrVertexAttribType, 0,                 kPosition_GrVertexAttribBinding },
    { kVec4f_GrVertexAttribType, sizeof(SkPoint),   kEffect_GrVertexAttribBinding },
    { kVec2f_GrVertexAttribType, 3*sizeof(SkPoint), kEffect_GrVertexAttribBinding }
};

// Axis Aligned
struct AARectVertex {
    SkPoint fPos;
    SkPoint fOffset;
    SkPoint fWidthHeight;
};

// Axis Aligned
extern const GrVertexAttrib gAAAARectVertexAttribs[] = {
    { kVec2f_GrVertexAttribType, 0,                 kPosition_GrVertexAttribBinding },
    { kVec4f_GrVertexAttribType, sizeof(SkPoint),   kEffect_GrVertexAttribBinding },
};

};

void GrAARectRenderer::shaderFillAARect(GrGpu* gpu,
                                        GrDrawTarget* target,
                                        const SkRect& rect,
                                        const SkMatrix& combinedMatrix) {
    GrDrawState* drawState = target->drawState();

    SkPoint center = SkPoint::Make(rect.centerX(), rect.centerY());
    combinedMatrix.mapPoints(&center, 1);

    // compute transformed (0, 1) vector
    SkVector dir = { combinedMatrix[SkMatrix::kMSkewX], combinedMatrix[SkMatrix::kMScaleY] };
    dir.normalize();

    // compute transformed (width, 0) and (0, height) vectors
    SkVector vec[2] = {
      { combinedMatrix[SkMatrix::kMScaleX], combinedMatrix[SkMatrix::kMSkewY] },
      { combinedMatrix[SkMatrix::kMSkewX],  combinedMatrix[SkMatrix::kMScaleY] }
    };

    SkScalar newWidth = SkScalarHalf(rect.width() * vec[0].length()) + SK_ScalarHalf;
    SkScalar newHeight = SkScalarHalf(rect.height() * vec[1].length()) + SK_ScalarHalf;
    drawState->setVertexAttribs<gAARectVertexAttribs>(SK_ARRAY_COUNT(gAARectVertexAttribs));
    SkASSERT(sizeof(RectVertex) == drawState->getVertexSize());

    GrDrawTarget::AutoReleaseGeometry geo(target, 4, 0);
    if (!geo.succeeded()) {
        GrPrintf("Failed to get space for vertices!\n");
        return;
    }

    RectVertex* verts = reinterpret_cast<RectVertex*>(geo.vertices());

    GrEffect* effect = GrRectEffect::Create();
    static const int kRectAttrIndex = 1;
    static const int kWidthIndex = 2;
    drawState->addCoverageEffect(effect, kRectAttrIndex, kWidthIndex)->unref();

    for (int i = 0; i < 4; ++i) {
        verts[i].fCenter = center;
        verts[i].fDir = dir;
        verts[i].fWidthHeight.fX = newWidth;
        verts[i].fWidthHeight.fY = newHeight;
    }

    SkRect devRect;
    combinedMatrix.mapRect(&devRect, rect);

    SkRect devBounds = {
        devRect.fLeft   - SK_ScalarHalf,
        devRect.fTop    - SK_ScalarHalf,
        devRect.fRight  + SK_ScalarHalf,
        devRect.fBottom + SK_ScalarHalf
    };

    verts[0].fPos = SkPoint::Make(devBounds.fLeft, devBounds.fTop);
    verts[1].fPos = SkPoint::Make(devBounds.fLeft, devBounds.fBottom);
    verts[2].fPos = SkPoint::Make(devBounds.fRight, devBounds.fBottom);
    verts[3].fPos = SkPoint::Make(devBounds.fRight, devBounds.fTop);

    target->setIndexSourceToBuffer(gpu->getContext()->getQuadIndexBuffer());
    target->drawIndexedInstances(kTriangles_GrPrimitiveType, 1, 4, 6);
    target->resetIndexSource();
}

void GrAARectRenderer::shaderFillAlignedAARect(GrGpu* gpu,
                                               GrDrawTarget* target,
                                               const SkRect& rect,
                                               const SkMatrix& combinedMatrix) {
    GrDrawState* drawState = target->drawState();
    SkASSERT(combinedMatrix.rectStaysRect());

    drawState->setVertexAttribs<gAAAARectVertexAttribs>(SK_ARRAY_COUNT(gAAAARectVertexAttribs));
    SkASSERT(sizeof(AARectVertex) == drawState->getVertexSize());

    GrDrawTarget::AutoReleaseGeometry geo(target, 4, 0);
    if (!geo.succeeded()) {
        GrPrintf("Failed to get space for vertices!\n");
        return;
    }

    AARectVertex* verts = reinterpret_cast<AARectVertex*>(geo.vertices());

    GrEffect* effect = GrAlignedRectEffect::Create();
    static const int kOffsetIndex = 1;
    drawState->addCoverageEffect(effect, kOffsetIndex)->unref();

    SkRect devRect;
    combinedMatrix.mapRect(&devRect, rect);

    SkRect devBounds = {
        devRect.fLeft   - SK_ScalarHalf,
        devRect.fTop    - SK_ScalarHalf,
        devRect.fRight  + SK_ScalarHalf,
        devRect.fBottom + SK_ScalarHalf
    };

    SkPoint widthHeight = {
        SkScalarHalf(devRect.width()) + SK_ScalarHalf,
        SkScalarHalf(devRect.height()) + SK_ScalarHalf
    };

    verts[0].fPos = SkPoint::Make(devBounds.fLeft, devBounds.fTop);
    verts[0].fOffset = SkPoint::Make(-widthHeight.fX, -widthHeight.fY);
    verts[0].fWidthHeight = widthHeight;

    verts[1].fPos = SkPoint::Make(devBounds.fLeft, devBounds.fBottom);
    verts[1].fOffset = SkPoint::Make(-widthHeight.fX, widthHeight.fY);
    verts[1].fWidthHeight = widthHeight;

    verts[2].fPos = SkPoint::Make(devBounds.fRight, devBounds.fBottom);
    verts[2].fOffset = widthHeight;
    verts[2].fWidthHeight = widthHeight;

    verts[3].fPos = SkPoint::Make(devBounds.fRight, devBounds.fTop);
    verts[3].fOffset = SkPoint::Make(widthHeight.fX, -widthHeight.fY);
    verts[3].fWidthHeight = widthHeight;

    target->setIndexSourceToBuffer(gpu->getContext()->getQuadIndexBuffer());
    target->drawIndexedInstances(kTriangles_GrPrimitiveType, 1, 4, 6);
    target->resetIndexSource();
}

void GrAARectRenderer::strokeAARect(GrGpu* gpu,
                                    GrDrawTarget* target,
                                    const SkRect& rect,
                                    const SkMatrix& combinedMatrix,
                                    const SkRect& devRect,
                                    const SkStrokeRec& stroke) {
    SkVector devStrokeSize;
    SkScalar width = stroke.getWidth();
    if (width > 0) {
        devStrokeSize.set(width, width);
        combinedMatrix.mapVectors(&devStrokeSize, 1);
        devStrokeSize.setAbs(devStrokeSize);
    } else {
        devStrokeSize.set(SK_Scalar1, SK_Scalar1);
    }

    const SkScalar dx = devStrokeSize.fX;
    const SkScalar dy = devStrokeSize.fY;
    const SkScalar rx = SkScalarMul(dx, SK_ScalarHalf);
    const SkScalar ry = SkScalarMul(dy, SK_ScalarHalf);

    // Temporarily #if'ed out. We don't want to pass in the devRect but
    // right now it is computed in GrContext::apply_aa_to_rect and we don't
    // want to throw away the work
#if 0
    SkRect devRect;
    combinedMatrix.mapRect(&devRect, rect);
#endif

    SkScalar spare;
    {
        SkScalar w = devRect.width() - dx;
        SkScalar h = devRect.height() - dy;
        spare = SkTMin(w, h);
    }

    SkRect devOutside(devRect);
    devOutside.outset(rx, ry);

    bool miterStroke = true;
    // For hairlines, make bevel and round joins appear the same as mitered ones.
    // small miter limit means right angles show bevel...
    if ((width > 0) && (stroke.getJoin() != SkPaint::kMiter_Join ||
                        stroke.getMiter() < SK_ScalarSqrt2)) {
        miterStroke = false;
    }

    if (spare <= 0 && miterStroke) {
        this->fillAARect(gpu, target, devOutside, SkMatrix::I(), devOutside);
        return;
    }

    SkRect devInside(devRect);
    devInside.inset(rx, ry);

    SkRect devOutsideAssist(devRect);

    // For bevel-stroke, use 2 SkRect instances(devOutside and devOutsideAssist)
    // to draw the outer of the rect. Because there are 8 vertices on the outer
    // edge, while vertex number of inner edge is 4, the same as miter-stroke.
    if (!miterStroke) {
        devOutside.inset(0, ry);
        devOutsideAssist.outset(0, ry);
    }

    this->geometryStrokeAARect(gpu, target, devOutside, devOutsideAssist, devInside, miterStroke);
}

void GrAARectRenderer::geometryStrokeAARect(GrGpu* gpu,
                                            GrDrawTarget* target,
                                            const SkRect& devOutside,
                                            const SkRect& devOutsideAssist,
                                            const SkRect& devInside,
                                            bool miterStroke) {
    GrDrawState* drawState = target->drawState();

    CoverageAttribType covAttribType = set_rect_attribs(drawState);

    GrColor color = drawState->getColor();
    if (kUseCoverage_CoverageAttribType == covAttribType && GrColorIsOpaque(color)) {
        drawState->setHint(GrDrawState::kVertexColorsAreOpaque_Hint, true);
    }

    int innerVertexNum = 4;
    int outerVertexNum = miterStroke ? 4 : 8;
    int totalVertexNum = (outerVertexNum + innerVertexNum) * 2;

    GrDrawTarget::AutoReleaseGeometry geo(target, totalVertexNum, 0);
    if (!geo.succeeded()) {
        GrPrintf("Failed to get space for vertices!\n");
        return;
    }
    GrIndexBuffer* indexBuffer = this->aaStrokeRectIndexBuffer(gpu, miterStroke);
    if (NULL == indexBuffer) {
        GrPrintf("Failed to create index buffer!\n");
        return;
    }

    intptr_t verts = reinterpret_cast<intptr_t>(geo.vertices());
    size_t vsize = drawState->getVertexSize();

    // We create vertices for four nested rectangles. There are two ramps from 0 to full
    // coverage, one on the exterior of the stroke and the other on the interior.
    // The following pointers refer to the four rects, from outermost to innermost.
    SkPoint* fan0Pos = reinterpret_cast<SkPoint*>(verts);
    SkPoint* fan1Pos = reinterpret_cast<SkPoint*>(verts + outerVertexNum * vsize);
    SkPoint* fan2Pos = reinterpret_cast<SkPoint*>(verts + 2 * outerVertexNum * vsize);
    SkPoint* fan3Pos = reinterpret_cast<SkPoint*>(verts + (2 * outerVertexNum + innerVertexNum) * vsize);

#ifndef SK_IGNORE_THIN_STROKED_RECT_FIX
    // TODO: this only really works if the X & Y margins are the same all around
    // the rect
    SkScalar inset = SkMinScalar(SK_Scalar1, devOutside.fRight - devInside.fRight);
    inset = SkMinScalar(inset, devInside.fLeft - devOutside.fLeft);
    inset = SkMinScalar(inset, devInside.fTop - devOutside.fTop);
    if (miterStroke) {
        inset = SK_ScalarHalf * SkMinScalar(inset, devOutside.fBottom - devInside.fBottom);
    } else {
        inset = SK_ScalarHalf * SkMinScalar(inset, devOutsideAssist.fBottom - devInside.fBottom);
    }
    SkASSERT(inset >= 0);
#else
    SkScalar inset = SK_ScalarHalf;
#endif

    if (miterStroke) {
        // outermost
        set_inset_fan(fan0Pos, vsize, devOutside, -SK_ScalarHalf, -SK_ScalarHalf);
        // inner two
        set_inset_fan(fan1Pos, vsize, devOutside,  inset,  inset);
        set_inset_fan(fan2Pos, vsize, devInside,  -inset, -inset);
        // innermost
        set_inset_fan(fan3Pos, vsize, devInside,   SK_ScalarHalf,  SK_ScalarHalf);
    } else {
        SkPoint* fan0AssistPos = reinterpret_cast<SkPoint*>(verts + 4 * vsize);
        SkPoint* fan1AssistPos = reinterpret_cast<SkPoint*>(verts + (outerVertexNum + 4) * vsize);
        // outermost
        set_inset_fan(fan0Pos, vsize, devOutside, -SK_ScalarHalf, -SK_ScalarHalf);
        set_inset_fan(fan0AssistPos, vsize, devOutsideAssist, -SK_ScalarHalf, -SK_ScalarHalf);
        // outer one of the inner two
        set_inset_fan(fan1Pos, vsize, devOutside,  inset,  inset);
        set_inset_fan(fan1AssistPos, vsize, devOutsideAssist,  inset,  inset);
        // inner one of the inner two
        set_inset_fan(fan2Pos, vsize, devInside,  -inset, -inset);
        // innermost
        set_inset_fan(fan3Pos, vsize, devInside,   SK_ScalarHalf,  SK_ScalarHalf);
    }

    // Make verts point to vertex color and then set all the color and coverage vertex attrs values.
    // The outermost rect has 0 coverage
    verts += sizeof(SkPoint);
    for (int i = 0; i < outerVertexNum; ++i) {
        if (kUseCoverage_CoverageAttribType == covAttribType) {
            *reinterpret_cast<GrColor*>(verts + i * vsize) = color;
            *reinterpret_cast<GrColor*>(verts + i * vsize + sizeof(GrColor)) = 0;
        } else {
            *reinterpret_cast<GrColor*>(verts + i * vsize) = 0;
        }
    }

    // scale is the coverage for the the inner two rects.
    int scale;
    if (inset < SK_ScalarHalf) {
        scale = SkScalarFloorToInt(512.0f * inset / (inset + SK_ScalarHalf));
        SkASSERT(scale >= 0 && scale <= 255);
    } else {
        scale = 0xff;
    }

    verts += outerVertexNum * vsize;
    GrColor innerCoverage;
    if (kUseCoverage_CoverageAttribType == covAttribType) {
        innerCoverage = GrColorPackRGBA(scale, scale, scale, scale);
    } else {
        innerCoverage = (0xff == scale) ? color : SkAlphaMulQ(color, scale);
    }

    for (int i = 0; i < outerVertexNum + innerVertexNum; ++i) {
        if (kUseCoverage_CoverageAttribType == covAttribType) {
            *reinterpret_cast<GrColor*>(verts + i * vsize) = color;
            *reinterpret_cast<GrColor*>(verts + i * vsize + sizeof(GrColor)) = innerCoverage;
        } else {
            *reinterpret_cast<GrColor*>(verts + i * vsize) = innerCoverage;
        }
    }

    // The innermost rect has 0 coverage
    verts += (outerVertexNum + innerVertexNum) * vsize;
    for (int i = 0; i < innerVertexNum; ++i) {
        if (kUseCoverage_CoverageAttribType == covAttribType) {
            *reinterpret_cast<GrColor*>(verts + i * vsize) = color;
            *reinterpret_cast<GrColor*>(verts + i * vsize + sizeof(GrColor)) = 0;
        } else {
            *reinterpret_cast<GrColor*>(verts + i * vsize) = 0;
        }
    }

    target->setIndexSourceToBuffer(indexBuffer);
    target->drawIndexed(kTriangles_GrPrimitiveType, 0, 0,
                        totalVertexNum, aaStrokeRectIndexCount(miterStroke));
}

void GrAARectRenderer::fillAANestedRects(GrGpu* gpu,
                                         GrDrawTarget* target,
                                         const SkRect rects[2],
                                         const SkMatrix& combinedMatrix) {
    SkASSERT(combinedMatrix.rectStaysRect());
    SkASSERT(!rects[1].isEmpty());

    SkRect devOutside, devOutsideAssist, devInside;
    combinedMatrix.mapRect(&devOutside, rects[0]);
    // can't call mapRect for devInside since it calls sort
    combinedMatrix.mapPoints((SkPoint*)&devInside, (const SkPoint*)&rects[1], 2);

    if (devInside.isEmpty()) {
        this->fillAARect(gpu, target, devOutside, SkMatrix::I(), devOutside);
        return;
    }

    this->geometryStrokeAARect(gpu, target, devOutside, devOutsideAssist, devInside, true);
}
