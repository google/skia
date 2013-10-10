/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrAARectRenderer.h"
#include "GrGpu.h"
#include "gl/GrGLEffect.h"
#include "gl/GrGLVertexEffect.h"
#include "GrTBackendEffectFactory.h"
#include "SkColorPriv.h"
#include "effects/GrVertexEffect.h"

SK_DEFINE_INST_COUNT(GrAARectRenderer)

///////////////////////////////////////////////////////////////////////////////
class GrGLAlignedRectEffect;

// Axis Aligned special case
class GrAlignedRectEffect : public GrVertexEffect {
public:
    static GrEffectRef* Create() {
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

        virtual void emitCode(GrGLFullShaderBuilder* builder,
                              const GrDrawEffect& drawEffect,
                              EffectKey key,
                              const char* outputColor,
                              const char* inputColor,
                              const TransformedCoordsArray&,
                              const TextureSamplerArray& samplers) SK_OVERRIDE {
            // setup the varying for the Axis aligned rect effect
            //      xy -> interpolated offset
            //      zw -> w/2+0.5, h/2+0.5
            const char *vsRectName, *fsRectName;
            builder->addVarying(kVec4f_GrSLType, "Rect", &vsRectName, &fsRectName);
            const SkString* attr0Name =
                builder->getEffectAttributeName(drawEffect.getVertexAttribIndices()[0]);
            builder->vsCodeAppendf("\t%s = %s;\n", vsRectName, attr0Name->c_str());

            // TODO: compute all these offsets, spans, and scales in the VS
            builder->fsCodeAppendf("\tfloat insetW = min(1.0, %s.z) - 0.5;\n", fsRectName);
            builder->fsCodeAppendf("\tfloat insetH = min(1.0, %s.w) - 0.5;\n", fsRectName);
            builder->fsCodeAppend("\tfloat outset = 0.5;\n");
            // For rects > 1 pixel wide and tall the span's are noops (i.e., 1.0). For rects
            // < 1 pixel wide or tall they serve to normalize the < 1 ramp to a 0 .. 1 range.
            builder->fsCodeAppend("\tfloat spanW = insetW + outset;\n");
            builder->fsCodeAppend("\tfloat spanH = insetH + outset;\n");
            // For rects < 1 pixel wide or tall, these scale factors are used to cap the maximum
            // value of coverage that is used. In other words it is the coverage that is
            // used in the interior of the rect after the ramp.
            builder->fsCodeAppend("\tfloat scaleW = min(1.0, 2.0*insetW/spanW);\n");
            builder->fsCodeAppend("\tfloat scaleH = min(1.0, 2.0*insetH/spanH);\n");

            // Compute the coverage for the rect's width
            builder->fsCodeAppendf(
                "\tfloat coverage = scaleW*clamp((%s.z-abs(%s.x))/spanW, 0.0, 1.0);\n", fsRectName,
                fsRectName);
            // Compute the coverage for the rect's height and merge with the width
            builder->fsCodeAppendf(
                "\tcoverage = coverage*scaleH*clamp((%s.w-abs(%s.y))/spanH, 0.0, 1.0);\n",
                fsRectName, fsRectName);


            builder->fsCodeAppendf("\t%s = %s;\n", outputColor,
                                   (GrGLSLExpr<4>(inputColor) * GrGLSLExpr<1>("coverage")).c_str());
        }

        static inline EffectKey GenKey(const GrDrawEffect& drawEffect, const GrGLCaps&) {
            return 0;
        }

        virtual void setData(const GrGLUniformManager& uman, const GrDrawEffect&) SK_OVERRIDE {}

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

GrEffectRef* GrAlignedRectEffect::TestCreate(SkRandom* random,
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
    static GrEffectRef* Create() {
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

        virtual void emitCode(GrGLFullShaderBuilder* builder,
                              const GrDrawEffect& drawEffect,
                              EffectKey key,
                              const char* outputColor,
                              const char* inputColor,
                              const TransformedCoordsArray&,
                              const TextureSamplerArray& samplers) SK_OVERRIDE {
            // setup the varying for the center point and the unit vector
            // that points down the height of the rect
            const char *vsRectEdgeName, *fsRectEdgeName;
            builder->addVarying(kVec4f_GrSLType, "RectEdge",
                                &vsRectEdgeName, &fsRectEdgeName);
            const SkString* attr0Name =
                builder->getEffectAttributeName(drawEffect.getVertexAttribIndices()[0]);
            builder->vsCodeAppendf("\t%s = %s;\n", vsRectEdgeName, attr0Name->c_str());

            // setup the varying for width/2+.5 and height/2+.5
            const char *vsWidthHeightName, *fsWidthHeightName;
            builder->addVarying(kVec2f_GrSLType, "WidthHeight",
                                &vsWidthHeightName, &fsWidthHeightName);
            const SkString* attr1Name =
                builder->getEffectAttributeName(drawEffect.getVertexAttribIndices()[1]);
            builder->vsCodeAppendf("\t%s = %s;\n", vsWidthHeightName, attr1Name->c_str());

            // TODO: compute all these offsets, spans, and scales in the VS
            builder->fsCodeAppendf("\tfloat insetW = min(1.0, %s.x) - 0.5;\n", fsWidthHeightName);
            builder->fsCodeAppendf("\tfloat insetH = min(1.0, %s.y) - 0.5;\n", fsWidthHeightName);
            builder->fsCodeAppend("\tfloat outset = 0.5;\n");
            // For rects > 1 pixel wide and tall the span's are noops (i.e., 1.0). For rects
            // < 1 pixel wide or tall they serve to normalize the < 1 ramp to a 0 .. 1 range.
            builder->fsCodeAppend("\tfloat spanW = insetW + outset;\n");
            builder->fsCodeAppend("\tfloat spanH = insetH + outset;\n");
            // For rects < 1 pixel wide or tall, these scale factors are used to cap the maximum
            // value of coverage that is used. In other words it is the coverage that is
            // used in the interior of the rect after the ramp.
            builder->fsCodeAppend("\tfloat scaleW = min(1.0, 2.0*insetW/spanW);\n");
            builder->fsCodeAppend("\tfloat scaleH = min(1.0, 2.0*insetH/spanH);\n");

            // Compute the coverage for the rect's width
            builder->fsCodeAppendf("\tvec2 offset = %s.xy - %s.xy;\n",
                                   builder->fragmentPosition(), fsRectEdgeName);
            builder->fsCodeAppendf("\tfloat perpDot = abs(offset.x * %s.w - offset.y * %s.z);\n",
                                   fsRectEdgeName, fsRectEdgeName);
            builder->fsCodeAppendf(
                "\tfloat coverage = scaleW*clamp((%s.x-perpDot)/spanW, 0.0, 1.0);\n",
                fsWidthHeightName);

            // Compute the coverage for the rect's height and merge with the width
            builder->fsCodeAppendf("\tperpDot = abs(dot(offset, %s.zw));\n",
                                   fsRectEdgeName);
            builder->fsCodeAppendf(
                    "\tcoverage = coverage*scaleH*clamp((%s.y-perpDot)/spanH, 0.0, 1.0);\n",
                    fsWidthHeightName);


            builder->fsCodeAppendf("\t%s = %s;\n", outputColor,
                                   (GrGLSLExpr<4>(inputColor) * GrGLSLExpr<1>("coverage")).c_str());
        }

        static inline EffectKey GenKey(const GrDrawEffect& drawEffect, const GrGLCaps&) {
            return 0;
        }

        virtual void setData(const GrGLUniformManager& uman, const GrDrawEffect&) SK_OVERRIDE {}

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

GrEffectRef* GrRectEffect::TestCreate(SkRandom* random,
                                      GrContext* context,
                                      const GrDrawTargetCaps&,
                                      GrTexture* textures[]) {
    return GrRectEffect::Create();
}

///////////////////////////////////////////////////////////////////////////////

namespace {

extern const GrVertexAttrib gAARectCoverageAttribs[] = {
    {kVec2f_GrVertexAttribType,  0,               kPosition_GrVertexAttribBinding},
    {kVec4ub_GrVertexAttribType, sizeof(GrPoint), kCoverage_GrVertexAttribBinding},
};

extern const GrVertexAttrib gAARectColorAttribs[] = {
    {kVec2f_GrVertexAttribType,  0,               kPosition_GrVertexAttribBinding},
    {kVec4ub_GrVertexAttribType, sizeof(GrPoint), kColor_GrVertexAttribBinding},
};

static void set_aa_rect_vertex_attributes(GrDrawState* drawState, bool useCoverage) {
    if (useCoverage) {
        drawState->setVertexAttribs<gAARectCoverageAttribs>(SK_ARRAY_COUNT(gAARectCoverageAttribs));
    } else {
        drawState->setVertexAttribs<gAARectColorAttribs>(SK_ARRAY_COUNT(gAARectColorAttribs));
    }
}

static void set_inset_fan(GrPoint* pts, size_t stride,
                          const SkRect& r, SkScalar dx, SkScalar dy) {
    pts->setRectFan(r.fLeft + dx, r.fTop + dy,
                    r.fRight - dx, r.fBottom - dy, stride);
}

};

void GrAARectRenderer::reset() {
    SkSafeSetNull(fAAFillRectIndexBuffer);
    SkSafeSetNull(fAAStrokeRectIndexBuffer);
}

static const uint16_t gFillAARectIdx[] = {
    0, 1, 5, 5, 4, 0,
    1, 2, 6, 6, 5, 1,
    2, 3, 7, 7, 6, 2,
    3, 0, 4, 4, 7, 3,
    4, 5, 6, 6, 7, 4,
};

static const int kIndicesPerAAFillRect = GR_ARRAY_COUNT(gFillAARectIdx);
static const int kVertsPerAAFillRect = 8;
static const int kNumAAFillRectsInIndexBuffer = 256;

GrIndexBuffer* GrAARectRenderer::aaFillRectIndexBuffer(GrGpu* gpu) {
    static const size_t kAAFillRectIndexBufferSize = kIndicesPerAAFillRect *
                                                     sizeof(uint16_t) *
                                                     kNumAAFillRectsInIndexBuffer;

    if (NULL == fAAFillRectIndexBuffer) {
        fAAFillRectIndexBuffer = gpu->createIndexBuffer(kAAFillRectIndexBufferSize, false);
        if (NULL != fAAFillRectIndexBuffer) {
            uint16_t* data = (uint16_t*) fAAFillRectIndexBuffer->lock();
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
                    GrCrash("Can't get AA Fill Rect indices into buffer!");
                }
                SkDELETE_ARRAY(data);
            } else {
                fAAFillRectIndexBuffer->unlock();
            }
        }
    }

    return fAAFillRectIndexBuffer;
}

static const uint16_t gStrokeAARectIdx[] = {
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

int GrAARectRenderer::aaStrokeRectIndexCount() {
    return GR_ARRAY_COUNT(gStrokeAARectIdx);
}

GrIndexBuffer* GrAARectRenderer::aaStrokeRectIndexBuffer(GrGpu* gpu) {
    if (NULL == fAAStrokeRectIndexBuffer) {
        fAAStrokeRectIndexBuffer =
                  gpu->createIndexBuffer(sizeof(gStrokeAARectIdx), false);
        if (NULL != fAAStrokeRectIndexBuffer) {
#ifdef SK_DEBUG
            bool updated =
#endif
            fAAStrokeRectIndexBuffer->updateData(gStrokeAARectIdx,
                                                 sizeof(gStrokeAARectIdx));
            GR_DEBUGASSERT(updated);
        }
    }
    return fAAStrokeRectIndexBuffer;
}

void GrAARectRenderer::geometryFillAARect(GrGpu* gpu,
                                          GrDrawTarget* target,
                                          const SkRect& rect,
                                          const SkMatrix& combinedMatrix,
                                          const SkRect& devRect,
                                          bool useVertexCoverage) {
    GrDrawState* drawState = target->drawState();

    set_aa_rect_vertex_attributes(drawState, useVertexCoverage);

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
    SkASSERT(sizeof(GrPoint) + sizeof(GrColor) == vsize);

    GrPoint* fan0Pos = reinterpret_cast<GrPoint*>(verts);
    GrPoint* fan1Pos = reinterpret_cast<GrPoint*>(verts + 4 * vsize);

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

    verts += sizeof(GrPoint);
    for (int i = 0; i < 4; ++i) {
        *reinterpret_cast<GrColor*>(verts + i * vsize) = 0;
    }

    int scale;
    if (inset < SK_ScalarHalf) {
        scale = SkScalarFloorToInt(512.0f * inset / (inset + SK_ScalarHalf));
        SkASSERT(scale >= 0 && scale <= 255);
    } else {
        scale = 0xff;
    }

    GrColor innerColor;
    if (useVertexCoverage) {
        innerColor = GrColorPackRGBA(scale, scale, scale, scale);
    } else {
        if (0xff == scale) {
            innerColor = target->getDrawState().getColor();
        } else {
            innerColor = SkAlphaMulQ(target->getDrawState().getColor(), scale);
        }
    }

    verts += 4 * vsize;
    for (int i = 0; i < 4; ++i) {
        *reinterpret_cast<GrColor*>(verts + i * vsize) = innerColor;
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
    GrPoint fPos;
    GrPoint fCenter;
    GrPoint fDir;
    GrPoint fWidthHeight;
};

// Rotated
extern const GrVertexAttrib gAARectVertexAttribs[] = {
    { kVec2f_GrVertexAttribType, 0,                 kPosition_GrVertexAttribBinding },
    { kVec4f_GrVertexAttribType, sizeof(GrPoint),   kEffect_GrVertexAttribBinding },
    { kVec2f_GrVertexAttribType, 3*sizeof(GrPoint), kEffect_GrVertexAttribBinding }
};

// Axis Aligned
struct AARectVertex {
    GrPoint fPos;
    GrPoint fOffset;
    GrPoint fWidthHeight;
};

// Axis Aligned
extern const GrVertexAttrib gAAAARectVertexAttribs[] = {
    { kVec2f_GrVertexAttribType, 0,                 kPosition_GrVertexAttribBinding },
    { kVec4f_GrVertexAttribType, sizeof(GrPoint),   kEffect_GrVertexAttribBinding },
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

    GrEffectRef* effect = GrRectEffect::Create();
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

    GrEffectRef* effect = GrAlignedRectEffect::Create();
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

    GrPoint widthHeight = {
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
                                    SkScalar width,
                                    bool useVertexCoverage) {
    GrVec devStrokeSize;
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
        spare = GrMin(w, h);
    }

    SkRect devOutside(devRect);
    devOutside.outset(rx, ry);

    if (spare <= 0) {
        this->fillAARect(gpu, target, devOutside, SkMatrix::I(),
                         devOutside, useVertexCoverage);
        return;
    }

    SkRect devInside(devRect);
    devInside.inset(rx, ry);

    this->geometryStrokeAARect(gpu, target, devOutside, devInside, useVertexCoverage);
}

void GrAARectRenderer::geometryStrokeAARect(GrGpu* gpu,
                                            GrDrawTarget* target,
                                            const SkRect& devOutside,
                                            const SkRect& devInside,
                                            bool useVertexCoverage) {
    GrDrawState* drawState = target->drawState();

    set_aa_rect_vertex_attributes(drawState, useVertexCoverage);

    GrDrawTarget::AutoReleaseGeometry geo(target, 16, 0);
    if (!geo.succeeded()) {
        GrPrintf("Failed to get space for vertices!\n");
        return;
    }
    GrIndexBuffer* indexBuffer = this->aaStrokeRectIndexBuffer(gpu);
    if (NULL == indexBuffer) {
        GrPrintf("Failed to create index buffer!\n");
        return;
    }

    intptr_t verts = reinterpret_cast<intptr_t>(geo.vertices());
    size_t vsize = drawState->getVertexSize();
    SkASSERT(sizeof(GrPoint) + sizeof(GrColor) == vsize);

    // We create vertices for four nested rectangles. There are two ramps from 0 to full
    // coverage, one on the exterior of the stroke and the other on the interior.
    // The following pointers refer to the four rects, from outermost to innermost.
    GrPoint* fan0Pos = reinterpret_cast<GrPoint*>(verts);
    GrPoint* fan1Pos = reinterpret_cast<GrPoint*>(verts + 4 * vsize);
    GrPoint* fan2Pos = reinterpret_cast<GrPoint*>(verts + 8 * vsize);
    GrPoint* fan3Pos = reinterpret_cast<GrPoint*>(verts + 12 * vsize);

#ifndef SK_IGNORE_THIN_STROKED_RECT_FIX
    // TODO: this only really works if the X & Y margins are the same all around
    // the rect
    SkScalar inset = SkMinScalar(SK_Scalar1, devOutside.fRight - devInside.fRight);
    inset = SkMinScalar(inset, devInside.fLeft - devOutside.fLeft);
    inset = SkMinScalar(inset, devInside.fTop - devOutside.fTop);
    inset = SK_ScalarHalf * SkMinScalar(inset, devOutside.fBottom - devInside.fBottom);
    SkASSERT(inset >= 0);
#else
    SkScalar inset = SK_ScalarHalf;
#endif

    // outermost
    set_inset_fan(fan0Pos, vsize, devOutside, -SK_ScalarHalf, -SK_ScalarHalf);
    // inner two
    set_inset_fan(fan1Pos, vsize, devOutside,  inset,  inset);
    set_inset_fan(fan2Pos, vsize, devInside,  -inset, -inset);
    // innermost
    set_inset_fan(fan3Pos, vsize, devInside,   SK_ScalarHalf,  SK_ScalarHalf);

    // The outermost rect has 0 coverage
    verts += sizeof(GrPoint);
    for (int i = 0; i < 4; ++i) {
        *reinterpret_cast<GrColor*>(verts + i * vsize) = 0;
    }

    int scale;
    if (inset < SK_ScalarHalf) {
        scale = SkScalarFloorToInt(512.0f * inset / (inset + SK_ScalarHalf));
        SkASSERT(scale >= 0 && scale <= 255);
    } else {
        scale = 0xff;
    }

    // The inner two rects have full coverage
    GrColor innerColor;
    if (useVertexCoverage) {
        innerColor = GrColorPackRGBA(scale, scale, scale, scale);
    } else {
        if (0xff == scale) {
            innerColor = target->getDrawState().getColor();
        } else {
            innerColor = SkAlphaMulQ(target->getDrawState().getColor(), scale);
        }
    }

    verts += 4 * vsize;
    for (int i = 0; i < 8; ++i) {
        *reinterpret_cast<GrColor*>(verts + i * vsize) = innerColor;
    }

    // The innermost rect has 0 coverage
    verts += 8 * vsize;
    for (int i = 0; i < 4; ++i) {
        *reinterpret_cast<GrColor*>(verts + i * vsize) = 0;
    }

    target->setIndexSourceToBuffer(indexBuffer);
    target->drawIndexed(kTriangles_GrPrimitiveType,
                        0, 0, 16, aaStrokeRectIndexCount());
}

void GrAARectRenderer::fillAANestedRects(GrGpu* gpu,
                                         GrDrawTarget* target,
                                         const SkRect rects[2],
                                         const SkMatrix& combinedMatrix,
                                         bool useVertexCoverage) {
    SkASSERT(combinedMatrix.rectStaysRect());
    SkASSERT(!rects[1].isEmpty());

    SkRect devOutside, devInside;
    combinedMatrix.mapRect(&devOutside, rects[0]);
    // can't call mapRect for devInside since it calls sort
    combinedMatrix.mapPoints((SkPoint*)&devInside, (const SkPoint*)&rects[1], 2);

    if (devInside.isEmpty()) {
        this->fillAARect(gpu, target, devOutside, SkMatrix::I(), devOutside, useVertexCoverage);
        return;
    }

    this->geometryStrokeAARect(gpu, target, devOutside, devInside, useVertexCoverage);
}
