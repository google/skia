/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "InstanceProcessor.h"

#include "GrContext.h"
#include "GrRenderTargetPriv.h"
#include "GrResourceCache.h"
#include "GrResourceProvider.h"
#include "GrShaderCaps.h"
#include "glsl/GrGLSLGeometryProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramBuilder.h"
#include "glsl/GrGLSLVarying.h"

namespace gr_instanced {

GrCaps::InstancedSupport InstanceProcessor::CheckSupport(const GrShaderCaps& shaderCaps,
                                                         const GrCaps& caps) {
    if (!shaderCaps.canUseAnyFunctionInShader() ||
        !shaderCaps.flatInterpolationSupport() ||
        !shaderCaps.integerSupport() ||
        0 == shaderCaps.maxVertexSamplers() ||
        !caps.shaderCaps()->texelBufferSupport() ||
        caps.maxVertexAttributes() < kNumAttribs) {
        return GrCaps::InstancedSupport::kNone;
    }
    if (!caps.sampleLocationsSupport() ||
        !shaderCaps.sampleVariablesSupport() ||
        !shaderCaps.shaderDerivativeSupport()) {
        return GrCaps::InstancedSupport::kBasic;
    }
    if (0 == caps.maxRasterSamples() ||
        !shaderCaps.sampleMaskOverrideCoverageSupport()) {
        return GrCaps::InstancedSupport::kMultisampled;
    }
    return GrCaps::InstancedSupport::kMixedSampled;
}

InstanceProcessor::InstanceProcessor(OpInfo opInfo, GrBuffer* paramsBuffer) : fOpInfo(opInfo) {
    this->initClassID<InstanceProcessor>();

    this->addVertexAttrib("shapeCoords", kVec2f_GrVertexAttribType, kHigh_GrSLPrecision);
    this->addVertexAttrib("vertexAttrs", kInt_GrVertexAttribType);
    this->addVertexAttrib("instanceInfo", kUint_GrVertexAttribType);
    this->addVertexAttrib("shapeMatrixX", kVec3f_GrVertexAttribType, kHigh_GrSLPrecision);
    this->addVertexAttrib("shapeMatrixY", kVec3f_GrVertexAttribType, kHigh_GrSLPrecision);
    this->addVertexAttrib("color", kVec4f_GrVertexAttribType, kLow_GrSLPrecision);
    this->addVertexAttrib("localRect", kVec4f_GrVertexAttribType, kHigh_GrSLPrecision);

    GR_STATIC_ASSERT(0 == (int)Attrib::kShapeCoords);
    GR_STATIC_ASSERT(1 == (int)Attrib::kVertexAttrs);
    GR_STATIC_ASSERT(2 == (int)Attrib::kInstanceInfo);
    GR_STATIC_ASSERT(3 == (int)Attrib::kShapeMatrixX);
    GR_STATIC_ASSERT(4 == (int)Attrib::kShapeMatrixY);
    GR_STATIC_ASSERT(5 == (int)Attrib::kColor);
    GR_STATIC_ASSERT(6 == (int)Attrib::kLocalRect);
    GR_STATIC_ASSERT(7 == kNumAttribs);

    if (fOpInfo.fHasParams) {
        SkASSERT(paramsBuffer);
        fParamsAccess.reset(kRGBA_float_GrPixelConfig, paramsBuffer, kVertex_GrShaderFlag);
        this->addBufferAccess(&fParamsAccess);
    }

    if (GrAATypeIsHW(fOpInfo.aaType())) {
        if (!fOpInfo.isSimpleRects() || GrAAType::kMixedSamples == fOpInfo.aaType()) {
            this->setWillUseSampleLocations();
        }
    }
}

class GLSLInstanceProcessor : public GrGLSLGeometryProcessor {
public:
    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override;

private:
    void setData(const GrGLSLProgramDataManager& pdman, const GrPrimitiveProcessor&,
                 FPCoordTransformIter&& transformIter) override {
        this->setTransformDataHelper(SkMatrix::I(), pdman, &transformIter);
    }

    class VertexInputs;
    class Backend;
    class BackendNonAA;
    class BackendCoverage;
    class BackendMultisample;

    typedef GrGLSLGeometryProcessor INHERITED;
};

GrGLSLPrimitiveProcessor* InstanceProcessor::createGLSLInstance(const GrShaderCaps&) const {
    return new GLSLInstanceProcessor();
}

class GLSLInstanceProcessor::VertexInputs {
public:
    VertexInputs(const InstanceProcessor& instProc, GrGLSLVertexBuilder* vertexBuilder)
        : fInstProc(instProc),
          fVertexBuilder(vertexBuilder) {
    }

    void initParams(const SamplerHandle paramsBuffer) {
        fParamsBuffer = paramsBuffer;
        fVertexBuilder->codeAppendf("highp int paramsIdx = int(%s & 0x%x);",
                                    this->attr(Attrib::kInstanceInfo),
                                    kParamsIdx_InfoMask);
    }

    const char* attr(Attrib attr) const { return fInstProc.getAttrib((int)attr).fName; }

    void fetchNextParam(GrSLType type = kVec4f_GrSLType) const {
        SkASSERT(fParamsBuffer.isValid());
        switch (type) {
            case kVec2f_GrSLType: // fall through
            case kVec3f_GrSLType: // fall through
            case kVec4f_GrSLType:
                break;
            default:
                fVertexBuilder->codeAppendf("%s(", GrGLSLTypeString(type));
        }
        fVertexBuilder->appendTexelFetch(fParamsBuffer, "paramsIdx++");
        switch (type) {
            case kVec2f_GrSLType:
                fVertexBuilder->codeAppend(".xy");
                break;
            case kVec3f_GrSLType:
                fVertexBuilder->codeAppend(".xyz");
                break;
            case kVec4f_GrSLType:
                break;
            default:
                fVertexBuilder->codeAppend(")");
        }
    }

    void skipParams(unsigned n) const {
        SkASSERT(fParamsBuffer.isValid());
        fVertexBuilder->codeAppendf("paramsIdx += %u;", n);
    }

private:
    const InstanceProcessor&     fInstProc;
    GrGLSLVertexBuilder*         fVertexBuilder;
    SamplerHandle                fParamsBuffer;
};

class GLSLInstanceProcessor::Backend {
public:
    static Backend* SK_WARN_UNUSED_RESULT Create(const GrPipeline&, OpInfo, const VertexInputs&);
    virtual ~Backend() {}

    void init(GrGLSLVaryingHandler*, GrGLSLVertexBuilder*);
    virtual void setupRect(GrGLSLVertexBuilder*) = 0;
    virtual void setupOval(GrGLSLVertexBuilder*) = 0;
    void setupRRect(GrGLSLVertexBuilder*, int* usedShapeDefinitions);

    void initInnerShape(GrGLSLVaryingHandler*, GrGLSLVertexBuilder*);
    virtual void setupInnerRect(GrGLSLVertexBuilder*) = 0;
    virtual void setupInnerOval(GrGLSLVertexBuilder*) = 0;
    void setupInnerSimpleRRect(GrGLSLVertexBuilder*);

    const char* outShapeCoords() {
        return fModifiedShapeCoords ? fModifiedShapeCoords : fInputs.attr(Attrib::kShapeCoords);
    }

    void emitCode(GrGLSLVertexBuilder*, GrGLSLPPFragmentBuilder*, const char* outCoverage,
                  const char* outColor);

protected:
    Backend(OpInfo opInfo, const VertexInputs& inputs)
            : fOpInfo(opInfo)
            , fInputs(inputs)
            , fModifiesCoverage(false)
            , fModifiesColor(false)
            , fNeedsNeighborRadii(false)
            , fColor(kVec4f_GrSLType)
            , fTriangleIsArc(kInt_GrSLType)
            , fArcCoords(kVec2f_GrSLType)
            , fInnerShapeCoords(kVec2f_GrSLType)
            , fInnerRRect(kVec4f_GrSLType)
            , fModifiedShapeCoords(nullptr) {
        if (fOpInfo.fShapeTypes & kRRect_ShapesMask) {
            fModifiedShapeCoords = "adjustedShapeCoords";
        }
    }

    virtual void onInit(GrGLSLVaryingHandler*, GrGLSLVertexBuilder*) = 0;
    virtual void adjustRRectVertices(GrGLSLVertexBuilder*);
    virtual void onSetupRRect(GrGLSLVertexBuilder*) {}

    virtual void onInitInnerShape(GrGLSLVaryingHandler*, GrGLSLVertexBuilder*) = 0;
    virtual void onSetupInnerSimpleRRect(GrGLSLVertexBuilder*) = 0;

    virtual void onEmitCode(GrGLSLVertexBuilder*, GrGLSLPPFragmentBuilder*,
                            const char* outCoverage, const char* outColor) = 0;

    void setupSimpleRadii(GrGLSLVertexBuilder*);
    void setupNinePatchRadii(GrGLSLVertexBuilder*);
    void setupComplexRadii(GrGLSLVertexBuilder*);

    const OpInfo fOpInfo;
    const VertexInputs& fInputs;
    bool fModifiesCoverage;
    bool fModifiesColor;
    bool fNeedsNeighborRadii;
    GrGLSLVertToFrag fColor;
    GrGLSLVertToFrag fTriangleIsArc;
    GrGLSLVertToFrag fArcCoords;
    GrGLSLVertToFrag fInnerShapeCoords;
    GrGLSLVertToFrag fInnerRRect;
    const char* fModifiedShapeCoords;
};

void GLSLInstanceProcessor::onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) {
    const GrPipeline& pipeline = args.fVertBuilder->getProgramBuilder()->pipeline();
    const InstanceProcessor& ip = args.fGP.cast<InstanceProcessor>();
    GrGLSLUniformHandler* uniHandler = args.fUniformHandler;
    GrGLSLVaryingHandler* varyingHandler = args.fVaryingHandler;
    GrGLSLVertexBuilder* v = args.fVertBuilder;
    GrGLSLPPFragmentBuilder* f = args.fFragBuilder;

    varyingHandler->emitAttributes(ip);

    VertexInputs inputs(ip, v);
    if (ip.opInfo().fHasParams) {
        SkASSERT(1 == ip.numBuffers());
        inputs.initParams(args.fBufferSamplers[0]);
    }

    if (!ip.opInfo().fHasPerspective) {
        v->codeAppendf("mat2x3 shapeMatrix = mat2x3(%s, %s);",
                       inputs.attr(Attrib::kShapeMatrixX), inputs.attr(Attrib::kShapeMatrixY));
    } else {
        v->defineConstantf("int", "PERSPECTIVE_FLAG", "0x%x", kPerspective_InfoFlag);
        v->codeAppendf("mat3 shapeMatrix = mat3(%s, %s, vec3(0, 0, 1));",
                       inputs.attr(Attrib::kShapeMatrixX), inputs.attr(Attrib::kShapeMatrixY));
        v->codeAppendf("if (0 != (%s & PERSPECTIVE_FLAG)) {",
                       inputs.attr(Attrib::kInstanceInfo));
        v->codeAppend (    "shapeMatrix[2] = ");
        inputs.fetchNextParam(kVec3f_GrSLType);
        v->codeAppend (    ";");
        v->codeAppend ("}");
    }

    bool hasSingleShapeType = SkIsPow2(ip.opInfo().fShapeTypes);
    if (!hasSingleShapeType) {
        v->defineConstant("SHAPE_TYPE_BIT", kShapeType_InfoBit);
        v->codeAppendf("uint shapeType = %s >> SHAPE_TYPE_BIT;",
                       inputs.attr(Attrib::kInstanceInfo));
    }

    std::unique_ptr<Backend> backend(Backend::Create(pipeline, ip.opInfo(), inputs));
    backend->init(varyingHandler, v);

    int usedShapeDefinitions = 0;

    if (hasSingleShapeType || !(ip.opInfo().fShapeTypes & ~kRRect_ShapesMask)) {
        if (kRect_ShapeFlag == ip.opInfo().fShapeTypes) {
            backend->setupRect(v);
        } else if (kOval_ShapeFlag == ip.opInfo().fShapeTypes) {
            backend->setupOval(v);
        } else {
            backend->setupRRect(v, &usedShapeDefinitions);
        }
    } else {
        if (ip.opInfo().fShapeTypes & kRRect_ShapesMask) {
            v->codeAppend ("if (shapeType >= SIMPLE_R_RECT_SHAPE_TYPE) {");
            backend->setupRRect(v, &usedShapeDefinitions);
            v->codeAppend ("}");
            usedShapeDefinitions |= kSimpleRRect_ShapeFlag;
        }
        if (ip.opInfo().fShapeTypes & kOval_ShapeFlag) {
            if (ip.opInfo().fShapeTypes & kRect_ShapeFlag) {
                if (ip.opInfo().fShapeTypes & kRRect_ShapesMask) {
                    v->codeAppend ("else ");
                }
                v->codeAppend ("if (OVAL_SHAPE_TYPE == shapeType) {");
                usedShapeDefinitions |= kOval_ShapeFlag;
            } else {
                v->codeAppend ("else {");
            }
            backend->setupOval(v);
            v->codeAppend ("}");
        }
        if (ip.opInfo().fShapeTypes & kRect_ShapeFlag) {
            v->codeAppend ("else {");
            backend->setupRect(v);
            v->codeAppend ("}");
        }
    }

    if (ip.opInfo().fInnerShapeTypes) {
        bool hasSingleInnerShapeType = SkIsPow2(ip.opInfo().fInnerShapeTypes);
        if (!hasSingleInnerShapeType) {
            v->defineConstantf("int", "INNER_SHAPE_TYPE_MASK", "0x%x", kInnerShapeType_InfoMask);
            v->defineConstant("INNER_SHAPE_TYPE_BIT", kInnerShapeType_InfoBit);
            v->codeAppendf("uint innerShapeType = ((%s & INNER_SHAPE_TYPE_MASK) >> "
                                                  "INNER_SHAPE_TYPE_BIT);",
                           inputs.attr(Attrib::kInstanceInfo));
        }
        // Here we take advantage of the fact that outerRect == localRect in recordDRRect.
        v->codeAppendf("vec4 outer = %s;", inputs.attr(Attrib::kLocalRect));
        v->codeAppend ("vec4 inner = ");
        inputs.fetchNextParam();
        v->codeAppend (";");
        // outer2Inner is a transform from shape coords to inner shape coords:
        // e.g. innerShapeCoords = shapeCoords * outer2Inner.xy + outer2Inner.zw
        v->codeAppend ("vec4 outer2Inner = vec4(outer.zw - outer.xy, "
                                               "outer.xy + outer.zw - inner.xy - inner.zw) / "
                                               "(inner.zw - inner.xy).xyxy;");
        v->codeAppendf("vec2 innerShapeCoords = %s * outer2Inner.xy + outer2Inner.zw;",
                       backend->outShapeCoords());

        backend->initInnerShape(varyingHandler, v);

        SkASSERT(0 == (ip.opInfo().fInnerShapeTypes & kRRect_ShapesMask) ||
                 kSimpleRRect_ShapeFlag == (ip.opInfo().fInnerShapeTypes & kRRect_ShapesMask));

        if (hasSingleInnerShapeType) {
            if (kRect_ShapeFlag == ip.opInfo().fInnerShapeTypes) {
                backend->setupInnerRect(v);
            } else if (kOval_ShapeFlag == ip.opInfo().fInnerShapeTypes) {
                backend->setupInnerOval(v);
            } else {
                backend->setupInnerSimpleRRect(v);
            }
        } else {
            if (ip.opInfo().fInnerShapeTypes & kSimpleRRect_ShapeFlag) {
                v->codeAppend ("if (SIMPLE_R_RECT_SHAPE_TYPE == innerShapeType) {");
                backend->setupInnerSimpleRRect(v);
                v->codeAppend("}");
                usedShapeDefinitions |= kSimpleRRect_ShapeFlag;
            }
            if (ip.opInfo().fInnerShapeTypes & kOval_ShapeFlag) {
                if (ip.opInfo().fInnerShapeTypes & kRect_ShapeFlag) {
                    if (ip.opInfo().fInnerShapeTypes & kSimpleRRect_ShapeFlag) {
                        v->codeAppend ("else ");
                    }
                    v->codeAppend ("if (OVAL_SHAPE_TYPE == innerShapeType) {");
                    usedShapeDefinitions |= kOval_ShapeFlag;
                } else {
                    v->codeAppend ("else {");
                }
                backend->setupInnerOval(v);
                v->codeAppend("}");
            }
            if (ip.opInfo().fInnerShapeTypes & kRect_ShapeFlag) {
                v->codeAppend("else {");
                backend->setupInnerRect(v);
                v->codeAppend("}");
            }
        }
    }

    if (usedShapeDefinitions & kOval_ShapeFlag) {
        v->defineConstant("OVAL_SHAPE_TYPE", (int)ShapeType::kOval);
    }
    if (usedShapeDefinitions & kSimpleRRect_ShapeFlag) {
        v->defineConstant("SIMPLE_R_RECT_SHAPE_TYPE", (int)ShapeType::kSimpleRRect);
    }
    if (usedShapeDefinitions & kNinePatch_ShapeFlag) {
        v->defineConstant("NINE_PATCH_SHAPE_TYPE", (int)ShapeType::kNinePatch);
    }
    SkASSERT(!(usedShapeDefinitions & (kRect_ShapeFlag | kComplexRRect_ShapeFlag)));

    backend->emitCode(v, f, args.fOutputCoverage, args.fOutputColor);

    const char* localCoords = nullptr;
    if (ip.opInfo().fUsesLocalCoords) {
        localCoords = "localCoords";
        v->codeAppendf("vec2 t = 0.5 * (%s + vec2(1));", backend->outShapeCoords());
        v->codeAppendf("vec2 localCoords = (1.0 - t) * %s.xy + t * %s.zw;",
                       inputs.attr(Attrib::kLocalRect), inputs.attr(Attrib::kLocalRect));
    }
    if (ip.opInfo().fHasLocalMatrix && ip.opInfo().fHasParams) {
        v->defineConstantf("int", "LOCAL_MATRIX_FLAG", "0x%x", kLocalMatrix_InfoFlag);
        v->codeAppendf("if (0 != (%s & LOCAL_MATRIX_FLAG)) {",
                       inputs.attr(Attrib::kInstanceInfo));
        if (!ip.opInfo().fUsesLocalCoords) {
            inputs.skipParams(2);
        } else {
            v->codeAppendf(    "mat2x3 localMatrix;");
            v->codeAppend (    "localMatrix[0] = ");
            inputs.fetchNextParam(kVec3f_GrSLType);
            v->codeAppend (    ";");
            v->codeAppend (    "localMatrix[1] = ");
            inputs.fetchNextParam(kVec3f_GrSLType);
            v->codeAppend (    ";");
            v->codeAppend (    "localCoords = (vec3(localCoords, 1) * localMatrix).xy;");
        }
        v->codeAppend("}");
    }

    GrSLType positionType = ip.opInfo().fHasPerspective ? kVec3f_GrSLType : kVec2f_GrSLType;
    v->codeAppendf("%s deviceCoords = vec3(%s, 1) * shapeMatrix;",
                   GrGLSLTypeString(positionType), backend->outShapeCoords());
    gpArgs->fPositionVar.set(positionType, "deviceCoords");

    this->emitTransforms(v, varyingHandler, uniHandler, gpArgs->fPositionVar, localCoords,
                         args.fFPCoordTransformHandler);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void GLSLInstanceProcessor::Backend::init(GrGLSLVaryingHandler* varyingHandler,
                                          GrGLSLVertexBuilder* v) {
    if (fModifiedShapeCoords) {
        v->codeAppendf("vec2 %s = %s;", fModifiedShapeCoords, fInputs.attr(Attrib::kShapeCoords));
    }

    this->onInit(varyingHandler, v);

    if (!fColor.vsOut()) {
        varyingHandler->addFlatVarying("color", &fColor, kLow_GrSLPrecision);
        v->codeAppendf("%s = %s;", fColor.vsOut(), fInputs.attr(Attrib::kColor));
    }
}

void GLSLInstanceProcessor::Backend::setupRRect(GrGLSLVertexBuilder* v, int* usedShapeDefinitions) {
    v->codeAppendf("uvec2 corner = uvec2(%s & 1, (%s >> 1) & 1);",
                   fInputs.attr(Attrib::kVertexAttrs), fInputs.attr(Attrib::kVertexAttrs));
    v->codeAppend ("vec2 cornerSign = vec2(corner) * 2.0 - 1.0;");
    v->codeAppendf("vec2 radii%s;", fNeedsNeighborRadii ? ", neighborRadii" : "");
    v->codeAppend ("mat2 p = ");
    fInputs.fetchNextParam(kMat22f_GrSLType);
    v->codeAppend (";");
    uint8_t types = fOpInfo.fShapeTypes & kRRect_ShapesMask;
    if (0 == (types & (types - 1))) {
        if (kSimpleRRect_ShapeFlag == types) {
            this->setupSimpleRadii(v);
        } else if (kNinePatch_ShapeFlag == types) {
            this->setupNinePatchRadii(v);
        } else if (kComplexRRect_ShapeFlag == types) {
            this->setupComplexRadii(v);
        }
    } else {
        if (types & kSimpleRRect_ShapeFlag) {
            v->codeAppend ("if (SIMPLE_R_RECT_SHAPE_TYPE == shapeType) {");
            this->setupSimpleRadii(v);
            v->codeAppend ("}");
            *usedShapeDefinitions |= kSimpleRRect_ShapeFlag;
        }
        if (types & kNinePatch_ShapeFlag) {
            if (types & kComplexRRect_ShapeFlag) {
                if (types & kSimpleRRect_ShapeFlag) {
                    v->codeAppend ("else ");
                }
                v->codeAppend ("if (NINE_PATCH_SHAPE_TYPE == shapeType) {");
                *usedShapeDefinitions |= kNinePatch_ShapeFlag;
            } else {
                v->codeAppend ("else {");
            }
            this->setupNinePatchRadii(v);
            v->codeAppend ("}");
        }
        if (types & kComplexRRect_ShapeFlag) {
            v->codeAppend ("else {");
            this->setupComplexRadii(v);
            v->codeAppend ("}");
        }
    }

    this->adjustRRectVertices(v);

    if (fArcCoords.vsOut()) {
        v->codeAppendf("%s = (cornerSign * %s + radii - vec2(1)) / radii;",
                       fArcCoords.vsOut(), fModifiedShapeCoords);
    }
    if (fTriangleIsArc.vsOut()) {
        v->codeAppendf("%s = int(all(equal(vec2(1), abs(%s))));",
                       fTriangleIsArc.vsOut(), fInputs.attr(Attrib::kShapeCoords));
    }

    this->onSetupRRect(v);
}

void GLSLInstanceProcessor::Backend::setupSimpleRadii(GrGLSLVertexBuilder* v) {
    if (fNeedsNeighborRadii) {
        v->codeAppend ("neighborRadii = ");
    }
    v->codeAppend("radii = p[0] * 2.0 / p[1];");
}

void GLSLInstanceProcessor::Backend::setupNinePatchRadii(GrGLSLVertexBuilder* v) {
    v->codeAppend("radii = vec2(p[0][corner.x], p[1][corner.y]);");
    if (fNeedsNeighborRadii) {
        v->codeAppend("neighborRadii = vec2(p[0][1u - corner.x], p[1][1u - corner.y]);");
    }
}

void GLSLInstanceProcessor::Backend::setupComplexRadii(GrGLSLVertexBuilder* v) {
    /**
     * The x and y radii of each arc are stored in separate vectors,
     * in the following order:
     *
     *        __x1 _ _ _ x3__
     *
     *    y1 |               | y2
     *
     *       |               |
     *
     *    y3 |__   _ _ _   __| y4
     *          x2       x4
     *
     */
    v->codeAppend("mat2 p2 = ");
    fInputs.fetchNextParam(kMat22f_GrSLType);
    v->codeAppend(";");
    v->codeAppend("radii = vec2(p[corner.x][corner.y], p2[corner.y][corner.x]);");
    if (fNeedsNeighborRadii) {
        v->codeAppend("neighborRadii = vec2(p[1u - corner.x][corner.y], "
                                           "p2[1u - corner.y][corner.x]);");
    }
}

void GLSLInstanceProcessor::Backend::adjustRRectVertices(GrGLSLVertexBuilder* v) {
    // Resize the 4 triangles that arcs are drawn into so they match their corresponding radii.
    // 0.5 is a special value that indicates the edge of an arc triangle.
    v->codeAppendf("if (abs(%s.x) == 0.5)"
                       "%s.x = cornerSign.x * (1.0 - radii.x);",
                       fInputs.attr(Attrib::kShapeCoords), fModifiedShapeCoords);
    v->codeAppendf("if (abs(%s.y) == 0.5) "
                       "%s.y = cornerSign.y * (1.0 - radii.y);",
                       fInputs.attr(Attrib::kShapeCoords), fModifiedShapeCoords);
}

void GLSLInstanceProcessor::Backend::initInnerShape(GrGLSLVaryingHandler* varyingHandler,
                                                    GrGLSLVertexBuilder* v) {
    SkASSERT(!(fOpInfo.fInnerShapeTypes & (kNinePatch_ShapeFlag | kComplexRRect_ShapeFlag)));

    this->onInitInnerShape(varyingHandler, v);

    if (fInnerShapeCoords.vsOut()) {
        v->codeAppendf("%s = innerShapeCoords;", fInnerShapeCoords.vsOut());
    }
}

void GLSLInstanceProcessor::Backend::setupInnerSimpleRRect(GrGLSLVertexBuilder* v) {
    v->codeAppend("mat2 innerP = ");
    fInputs.fetchNextParam(kMat22f_GrSLType);
    v->codeAppend(";");
    v->codeAppend("vec2 innerRadii = innerP[0] * 2.0 / innerP[1];");
    this->onSetupInnerSimpleRRect(v);
}

void GLSLInstanceProcessor::Backend::emitCode(GrGLSLVertexBuilder* v, GrGLSLPPFragmentBuilder* f,
                                              const char* outCoverage, const char* outColor) {
    SkASSERT(!fModifiesCoverage || outCoverage);
    this->onEmitCode(v, f, fModifiesCoverage ? outCoverage : nullptr,
                     fModifiesColor ? outColor : nullptr);
    if (outCoverage && !fModifiesCoverage) {
        // Even though the subclass doesn't use coverage, we are expected to assign some value.
        f->codeAppendf("%s = vec4(1);", outCoverage);
    }
    if (!fModifiesColor) {
        // The subclass didn't assign a value to the output color.
        f->codeAppendf("%s = %s;", outColor, fColor.fsIn());
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

class GLSLInstanceProcessor::BackendNonAA : public Backend {
public:
    BackendNonAA(OpInfo opInfo, const VertexInputs& inputs) : INHERITED(opInfo, inputs) {
        if (fOpInfo.fCannotDiscard && !fOpInfo.isSimpleRects()) {
            fModifiesColor = !fOpInfo.fCannotTweakAlphaForCoverage;
            fModifiesCoverage = !fModifiesColor;
        }
    }

private:
    void onInit(GrGLSLVaryingHandler*, GrGLSLVertexBuilder*) override;
    void setupRect(GrGLSLVertexBuilder*) override;
    void setupOval(GrGLSLVertexBuilder*) override;

    void onInitInnerShape(GrGLSLVaryingHandler*, GrGLSLVertexBuilder*) override;
    void setupInnerRect(GrGLSLVertexBuilder*) override;
    void setupInnerOval(GrGLSLVertexBuilder*) override;
    void onSetupInnerSimpleRRect(GrGLSLVertexBuilder*) override;

    void onEmitCode(GrGLSLVertexBuilder*, GrGLSLPPFragmentBuilder*, const char*,
                    const char*) override;

    typedef Backend INHERITED;
};

void GLSLInstanceProcessor::BackendNonAA::onInit(GrGLSLVaryingHandler* varyingHandler,
                                                 GrGLSLVertexBuilder*) {
    if (kRect_ShapeFlag != fOpInfo.fShapeTypes) {
        varyingHandler->addFlatVarying("triangleIsArc", &fTriangleIsArc, kLow_GrSLPrecision);
        varyingHandler->addVarying("arcCoords", &fArcCoords, kMedium_GrSLPrecision);
    }
}

void GLSLInstanceProcessor::BackendNonAA::setupRect(GrGLSLVertexBuilder* v) {
    if (fTriangleIsArc.vsOut()) {
        v->codeAppendf("%s = 0;", fTriangleIsArc.vsOut());
    }
}

void GLSLInstanceProcessor::BackendNonAA::setupOval(GrGLSLVertexBuilder* v) {
    SkASSERT(fArcCoords.vsOut());
    SkASSERT(fTriangleIsArc.vsOut());
    v->codeAppendf("%s = %s;", fArcCoords.vsOut(), this->outShapeCoords());
    v->codeAppendf("%s = %s & 1;", fTriangleIsArc.vsOut(), fInputs.attr(Attrib::kVertexAttrs));
}

void GLSLInstanceProcessor::BackendNonAA::onInitInnerShape(GrGLSLVaryingHandler* varyingHandler,
                                                           GrGLSLVertexBuilder*) {
    varyingHandler->addVarying("innerShapeCoords", &fInnerShapeCoords, kMedium_GrSLPrecision);
    if (kRect_ShapeFlag != fOpInfo.fInnerShapeTypes &&
        kOval_ShapeFlag != fOpInfo.fInnerShapeTypes) {
        varyingHandler->addFlatVarying("innerRRect", &fInnerRRect, kMedium_GrSLPrecision);
    }
}

void GLSLInstanceProcessor::BackendNonAA::setupInnerRect(GrGLSLVertexBuilder* v) {
    if (fInnerRRect.vsOut()) {
        v->codeAppendf("%s = vec4(1);", fInnerRRect.vsOut());
    }
}

void GLSLInstanceProcessor::BackendNonAA::setupInnerOval(GrGLSLVertexBuilder* v) {
    if (fInnerRRect.vsOut()) {
        v->codeAppendf("%s = vec4(0, 0, 1, 1);", fInnerRRect.vsOut());
    }
}

void GLSLInstanceProcessor::BackendNonAA::onSetupInnerSimpleRRect(GrGLSLVertexBuilder* v) {
    v->codeAppendf("%s = vec4(1.0 - innerRadii, 1.0 / innerRadii);", fInnerRRect.vsOut());
}

void GLSLInstanceProcessor::BackendNonAA::onEmitCode(GrGLSLVertexBuilder*,
                                                     GrGLSLPPFragmentBuilder* f,
                                                     const char* outCoverage,
                                                     const char* outColor) {
    const char* dropFragment = nullptr;
    if (!fOpInfo.fCannotDiscard) {
        dropFragment = "discard";
    } else if (fModifiesCoverage) {
        f->codeAppend ("lowp float covered = 1.0;");
        dropFragment = "covered = 0.0";
    } else if (fModifiesColor) {
        f->codeAppendf("lowp vec4 color = %s;", fColor.fsIn());
        dropFragment = "color = vec4(0)";
    }
    if (fTriangleIsArc.fsIn()) {
        SkASSERT(dropFragment);
        f->codeAppendf("if (%s != 0 && dot(%s, %s) > 1.0) %s;",
                       fTriangleIsArc.fsIn(), fArcCoords.fsIn(), fArcCoords.fsIn(), dropFragment);
    }
    if (fOpInfo.fInnerShapeTypes) {
        SkASSERT(dropFragment);
        f->codeAppendf("// Inner shape.\n");
        if (kRect_ShapeFlag == fOpInfo.fInnerShapeTypes) {
            f->codeAppendf("if (all(lessThanEqual(abs(%s), vec2(1)))) %s;",
                           fInnerShapeCoords.fsIn(), dropFragment);
        } else if (kOval_ShapeFlag == fOpInfo.fInnerShapeTypes) {
            f->codeAppendf("if ((dot(%s, %s) <= 1.0)) %s;",
                           fInnerShapeCoords.fsIn(), fInnerShapeCoords.fsIn(), dropFragment);
        } else {
            f->codeAppendf("if (all(lessThan(abs(%s), vec2(1)))) {", fInnerShapeCoords.fsIn());
            f->codeAppendf(    "vec2 distanceToArcEdge = abs(%s) - %s.xy;",
                               fInnerShapeCoords.fsIn(), fInnerRRect.fsIn());
            f->codeAppend (    "if (any(lessThan(distanceToArcEdge, vec2(0)))) {");
            f->codeAppendf(        "%s;", dropFragment);
            f->codeAppend (    "} else {");
            f->codeAppendf(        "vec2 rrectCoords = distanceToArcEdge * %s.zw;",
                                   fInnerRRect.fsIn());
            f->codeAppend (        "if (dot(rrectCoords, rrectCoords) <= 1.0) {");
            f->codeAppendf(            "%s;", dropFragment);
            f->codeAppend (        "}");
            f->codeAppend (    "}");
            f->codeAppend ("}");
        }
    }
    if (fModifiesCoverage) {
        f->codeAppendf("%s = vec4(covered);", outCoverage);
    } else if (fModifiesColor) {
        f->codeAppendf("%s = color;", outColor);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

class GLSLInstanceProcessor::BackendCoverage : public Backend {
public:
    BackendCoverage(OpInfo opInfo, const VertexInputs& inputs)
            : INHERITED(opInfo, inputs)
            , fColorTimesRectCoverage(kVec4f_GrSLType)
            , fRectCoverage(kFloat_GrSLType)
            , fEllipseCoords(kVec2f_GrSLType)
            , fEllipseName(kVec2f_GrSLType)
            , fBloatedRadius(kFloat_GrSLType)
            , fDistanceToInnerEdge(kVec2f_GrSLType)
            , fInnerShapeBloatedHalfSize(kVec2f_GrSLType)
            , fInnerEllipseCoords(kVec2f_GrSLType)
            , fInnerEllipseName(kVec2f_GrSLType) {
        fShapeIsCircle = !fOpInfo.fNonSquare && !(fOpInfo.fShapeTypes & kRRect_ShapesMask);
        fTweakAlphaForCoverage = !fOpInfo.fCannotTweakAlphaForCoverage && !fOpInfo.fInnerShapeTypes;
        fModifiesCoverage = !fTweakAlphaForCoverage;
        fModifiesColor = fTweakAlphaForCoverage;
        fModifiedShapeCoords = "bloatedShapeCoords";
    }

private:
    void onInit(GrGLSLVaryingHandler*, GrGLSLVertexBuilder*) override;
    void setupRect(GrGLSLVertexBuilder*) override;
    void setupOval(GrGLSLVertexBuilder*) override;
    void adjustRRectVertices(GrGLSLVertexBuilder*) override;
    void onSetupRRect(GrGLSLVertexBuilder*) override;

    void onInitInnerShape(GrGLSLVaryingHandler*, GrGLSLVertexBuilder*) override;
    void setupInnerRect(GrGLSLVertexBuilder*) override;
    void setupInnerOval(GrGLSLVertexBuilder*) override;
    void onSetupInnerSimpleRRect(GrGLSLVertexBuilder*) override;

    void onEmitCode(GrGLSLVertexBuilder*, GrGLSLPPFragmentBuilder*, const char* outCoverage,
                    const char* outColor) override;

    void emitRect(GrGLSLPPFragmentBuilder*, const char* outCoverage, const char* outColor);
    void emitCircle(GrGLSLPPFragmentBuilder*, const char* outCoverage);
    void emitArc(GrGLSLPPFragmentBuilder* f, const char* ellipseCoords, const char* ellipseName,
                 bool ellipseCoordsNeedClamp, bool ellipseCoordsMayBeNegative,
                 const char* outCoverage);
    void emitInnerRect(GrGLSLPPFragmentBuilder*, const char* outCoverage);

    GrGLSLVertToFrag   fColorTimesRectCoverage;
    GrGLSLVertToFrag   fRectCoverage;
    GrGLSLVertToFrag   fEllipseCoords;
    GrGLSLVertToFrag   fEllipseName;
    GrGLSLVertToFrag   fBloatedRadius;
    GrGLSLVertToFrag   fDistanceToInnerEdge;
    GrGLSLVertToFrag   fInnerShapeBloatedHalfSize;
    GrGLSLVertToFrag   fInnerEllipseCoords;
    GrGLSLVertToFrag   fInnerEllipseName;
    bool               fShapeIsCircle;
    bool               fTweakAlphaForCoverage;

    typedef Backend INHERITED;
};

void GLSLInstanceProcessor::BackendCoverage::onInit(GrGLSLVaryingHandler* varyingHandler,
                                                    GrGLSLVertexBuilder* v) {
    v->codeAppend ("mat2 shapeTransposeMatrix = transpose(mat2(shapeMatrix));");
    v->codeAppend ("vec2 shapeHalfSize = vec2(length(shapeTransposeMatrix[0]), "
                                             "length(shapeTransposeMatrix[1]));");
    v->codeAppend ("vec2 bloat = 0.5 / shapeHalfSize;");
    v->codeAppendf("bloatedShapeCoords = %s * (1.0 + bloat);", fInputs.attr(Attrib::kShapeCoords));

    if (kOval_ShapeFlag != fOpInfo.fShapeTypes) {
        if (fTweakAlphaForCoverage) {
            varyingHandler->addVarying("colorTimesRectCoverage", &fColorTimesRectCoverage,
                                       kLow_GrSLPrecision);
            if (kRect_ShapeFlag == fOpInfo.fShapeTypes) {
                fColor = fColorTimesRectCoverage;
            }
        } else {
            varyingHandler->addVarying("rectCoverage", &fRectCoverage, kLow_GrSLPrecision);
        }
        v->codeAppend("float rectCoverage = 0.0;");
    }
    if (kRect_ShapeFlag != fOpInfo.fShapeTypes) {
        varyingHandler->addFlatVarying("triangleIsArc", &fTriangleIsArc, kLow_GrSLPrecision);
        if (!fShapeIsCircle) {
            varyingHandler->addVarying("ellipseCoords", &fEllipseCoords, kMedium_GrSLPrecision);
            varyingHandler->addFlatVarying("ellipseName", &fEllipseName, kHigh_GrSLPrecision);
        } else {
            varyingHandler->addVarying("circleCoords", &fEllipseCoords, kHigh_GrSLPrecision);
            varyingHandler->addFlatVarying("bloatedRadius", &fBloatedRadius, kHigh_GrSLPrecision);
        }
    }
}

void GLSLInstanceProcessor::BackendCoverage::setupRect(GrGLSLVertexBuilder* v) {
    // Make the border one pixel wide. Inner vs outer is indicated by coordAttrs.
    v->codeAppendf("vec2 rectBloat = (%s != 0) ? bloat : -bloat;",
                   fInputs.attr(Attrib::kVertexAttrs));
    // Here we use the absolute value, because when the rect is thinner than a pixel, this makes it
    // mark the spot where pixel center is within half a pixel of the *opposite* edge. This,
    // combined with the "maxCoverage" logic below gives us mathematically correct coverage even for
    // subpixel rectangles.
    v->codeAppendf("bloatedShapeCoords = %s * abs(vec2(1.0 + rectBloat));",
                   fInputs.attr(Attrib::kShapeCoords));

    // Determine coverage at the vertex. Coverage naturally ramps from 0 to 1 unless the rect is
    // narrower than a pixel.
    v->codeAppend ("float maxCoverage = 4.0 * min(0.5, shapeHalfSize.x) *"
                                             "min(0.5, shapeHalfSize.y);");
    v->codeAppendf("rectCoverage = (%s != 0) ? 0.0 : maxCoverage;",
                   fInputs.attr(Attrib::kVertexAttrs));

    if (fTriangleIsArc.vsOut()) {
        v->codeAppendf("%s = 0;", fTriangleIsArc.vsOut());
    }
}

void GLSLInstanceProcessor::BackendCoverage::setupOval(GrGLSLVertexBuilder* v) {
    // Offset the inner and outer octagons by one pixel. Inner vs outer is indicated by coordAttrs.
    v->codeAppendf("vec2 ovalBloat = (%s != 0) ? bloat : -bloat;",
                   fInputs.attr(Attrib::kVertexAttrs));
    v->codeAppendf("bloatedShapeCoords = %s * max(vec2(1.0 + ovalBloat), vec2(0));",
                   fInputs.attr(Attrib::kShapeCoords));
    v->codeAppendf("%s = bloatedShapeCoords * shapeHalfSize;", fEllipseCoords.vsOut());
    if (fEllipseName.vsOut()) {
        v->codeAppendf("%s = 1.0 / (shapeHalfSize * shapeHalfSize);", fEllipseName.vsOut());
    }
    if (fBloatedRadius.vsOut()) {
        SkASSERT(fShapeIsCircle);
        v->codeAppendf("%s = shapeHalfSize.x + 0.5;", fBloatedRadius.vsOut());
    }
    if (fTriangleIsArc.vsOut()) {
        v->codeAppendf("%s = int(%s != 0);",
                       fTriangleIsArc.vsOut(), fInputs.attr(Attrib::kVertexAttrs));
    }
    if (fColorTimesRectCoverage.vsOut() || fRectCoverage.vsOut()) {
        v->codeAppendf("rectCoverage = 1.0;");
    }
}

void GLSLInstanceProcessor::BackendCoverage::adjustRRectVertices(GrGLSLVertexBuilder* v) {
    // We try to let the AA borders line up with the arc edges on their particular side, but we
    // can't allow them to get closer than one half pixel to the edge or they might overlap with
    // their neighboring border.
    v->codeAppend("vec2 innerEdge = max(1.0 - bloat, vec2(0));");
    v->codeAppend ("vec2 borderEdge = cornerSign * clamp(1.0 - radii, -innerEdge, innerEdge);");
    // 0.5 is a special value that indicates this vertex is an arc edge.
    v->codeAppendf("if (abs(%s.x) == 0.5)"
                       "bloatedShapeCoords.x = borderEdge.x;", fInputs.attr(Attrib::kShapeCoords));
    v->codeAppendf("if (abs(%s.y) == 0.5)"
                       "bloatedShapeCoords.y = borderEdge.y;", fInputs.attr(Attrib::kShapeCoords));

    // Adjust the interior border vertices to make the border one pixel wide. 0.75 is a special
    // value to indicate these points.
    v->codeAppendf("if (abs(%s.x) == 0.75) "
                       "bloatedShapeCoords.x = cornerSign.x * innerEdge.x;",
                       fInputs.attr(Attrib::kShapeCoords));
    v->codeAppendf("if (abs(%s.y) == 0.75) "
                       "bloatedShapeCoords.y = cornerSign.y * innerEdge.y;",
                       fInputs.attr(Attrib::kShapeCoords));
}

void GLSLInstanceProcessor::BackendCoverage::onSetupRRect(GrGLSLVertexBuilder* v) {
    // The geometry is laid out in such a way that rectCoverage will be 0 and 1 on the vertices, but
    // we still need to recompute this value because when the rrect gets thinner than one pixel, the
    // interior edge of the border will necessarily clamp, and we need to match the AA behavior of
    // the arc segments (i.e. distance from bloated edge only; ignoring the fact that the pixel
    // actully has less coverage because it's not completely inside the opposite edge.)
    v->codeAppend("vec2 d = shapeHalfSize + 0.5 - abs(bloatedShapeCoords) * shapeHalfSize;");
    v->codeAppend("rectCoverage = min(d.x, d.y);");

    SkASSERT(!fShapeIsCircle);
    // The AA border does not get closer than one half pixel to the edge of the rect, so to get a
    // smooth transition from flat edge to arc, we don't allow the radii to be smaller than one half
    // pixel. (We don't worry about the transition on the opposite side when a radius is so large
    // that the border clamped on that side.)
    v->codeAppendf("vec2 clampedRadii = max(radii, bloat);");
    v->codeAppendf("%s = (cornerSign * bloatedShapeCoords + clampedRadii - vec2(1)) * "
                        "shapeHalfSize;", fEllipseCoords.vsOut());
    v->codeAppendf("%s = 1.0 / (clampedRadii * clampedRadii * shapeHalfSize * shapeHalfSize);",
                   fEllipseName.vsOut());
}

void GLSLInstanceProcessor::BackendCoverage::onInitInnerShape(GrGLSLVaryingHandler* varyingHandler,
                                                              GrGLSLVertexBuilder* v) {
    v->codeAppend("vec2 innerShapeHalfSize = shapeHalfSize / outer2Inner.xy;");

    if (kOval_ShapeFlag == fOpInfo.fInnerShapeTypes) {
        varyingHandler->addVarying("innerEllipseCoords", &fInnerEllipseCoords,
                                   kMedium_GrSLPrecision);
        varyingHandler->addFlatVarying("innerEllipseName", &fInnerEllipseName, kHigh_GrSLPrecision);
    } else {
        varyingHandler->addVarying("distanceToInnerEdge", &fDistanceToInnerEdge,
                                   kMedium_GrSLPrecision);
        varyingHandler->addFlatVarying("innerShapeBloatedHalfSize", &fInnerShapeBloatedHalfSize,
                                       kMedium_GrSLPrecision);
        if (kRect_ShapeFlag != fOpInfo.fInnerShapeTypes) {
            varyingHandler->addVarying("innerShapeCoords", &fInnerShapeCoords,
                                       kMedium_GrSLPrecision);
            varyingHandler->addFlatVarying("innerEllipseName", &fInnerEllipseName,
                                           kHigh_GrSLPrecision);
            varyingHandler->addFlatVarying("innerRRect", &fInnerRRect, kMedium_GrSLPrecision);
        }
    }
}

void GLSLInstanceProcessor::BackendCoverage::setupInnerRect(GrGLSLVertexBuilder* v) {
    if (fInnerRRect.vsOut()) {
        // The fragment shader will generalize every inner shape as a round rect. Since this one
        // is a rect, we simply emit bogus parameters for the round rect (effectively negative
        // radii) that ensure the fragment shader always takes the "emitRect" codepath.
        v->codeAppendf("%s.xy = abs(outer2Inner.xy) * (1.0 + bloat) + abs(outer2Inner.zw);",
                       fInnerRRect.vsOut());
    }
}

void GLSLInstanceProcessor::BackendCoverage::setupInnerOval(GrGLSLVertexBuilder* v) {
    v->codeAppendf("%s = 1.0 / (innerShapeHalfSize * innerShapeHalfSize);",
                   fInnerEllipseName.vsOut());
    if (fInnerEllipseCoords.vsOut()) {
        v->codeAppendf("%s = innerShapeCoords * innerShapeHalfSize;", fInnerEllipseCoords.vsOut());
    }
    if (fInnerRRect.vsOut()) {
        v->codeAppendf("%s = vec4(0, 0, innerShapeHalfSize);", fInnerRRect.vsOut());
    }
}

void GLSLInstanceProcessor::BackendCoverage::onSetupInnerSimpleRRect(GrGLSLVertexBuilder* v) {
    // The distance to ellipse formula doesn't work well when the radii are less than half a pixel.
    v->codeAppend ("innerRadii = max(innerRadii, bloat);");
    v->codeAppendf("%s = 1.0 / (innerRadii * innerRadii * innerShapeHalfSize * "
                               "innerShapeHalfSize);",
                   fInnerEllipseName.vsOut());
    v->codeAppendf("%s = vec4(1.0 - innerRadii, innerShapeHalfSize);", fInnerRRect.vsOut());
}

void GLSLInstanceProcessor::BackendCoverage::onEmitCode(GrGLSLVertexBuilder* v,
                                                        GrGLSLPPFragmentBuilder* f,
                                                        const char* outCoverage,
                                                        const char* outColor) {
    if (fColorTimesRectCoverage.vsOut()) {
        SkASSERT(!fRectCoverage.vsOut());
        v->codeAppendf("%s = %s * rectCoverage;",
                       fColorTimesRectCoverage.vsOut(), fInputs.attr(Attrib::kColor));
    }
    if (fRectCoverage.vsOut()) {
        SkASSERT(!fColorTimesRectCoverage.vsOut());
        v->codeAppendf("%s = rectCoverage;", fRectCoverage.vsOut());
    }

    SkString coverage("lowp float coverage");
    if (fOpInfo.fInnerShapeTypes || (!fTweakAlphaForCoverage && fTriangleIsArc.fsIn())) {
        f->codeAppendf("%s;", coverage.c_str());
        coverage = "coverage";
    }
    if (fTriangleIsArc.fsIn()) {
        f->codeAppendf("if (%s == 0) {", fTriangleIsArc.fsIn());
        this->emitRect(f, coverage.c_str(), outColor);
        f->codeAppend ("} else {");
        if (fShapeIsCircle) {
            this->emitCircle(f, coverage.c_str());
        } else {
            bool ellipseCoordsMayBeNegative = SkToBool(fOpInfo.fShapeTypes & kOval_ShapeFlag);
            this->emitArc(f, fEllipseCoords.fsIn(), fEllipseName.fsIn(),
                          true /*ellipseCoordsNeedClamp*/, ellipseCoordsMayBeNegative,
                          coverage.c_str());
        }
        if (fTweakAlphaForCoverage) {
            f->codeAppendf("%s = %s * coverage;", outColor, fColor.fsIn());
        }
        f->codeAppend ("}");
    } else {
        this->emitRect(f, coverage.c_str(), outColor);
    }

    if (fOpInfo.fInnerShapeTypes) {
        f->codeAppendf("// Inner shape.\n");
        SkString innerCoverageDecl("lowp float innerCoverage");
        if (kOval_ShapeFlag == fOpInfo.fInnerShapeTypes) {
            this->emitArc(f, fInnerEllipseCoords.fsIn(), fInnerEllipseName.fsIn(),
                          true /*ellipseCoordsNeedClamp*/, true /*ellipseCoordsMayBeNegative*/,
                          innerCoverageDecl.c_str());
        } else {
            v->codeAppendf("%s = innerShapeCoords * innerShapeHalfSize;",
                           fDistanceToInnerEdge.vsOut());
            v->codeAppendf("%s = innerShapeHalfSize + 0.5;", fInnerShapeBloatedHalfSize.vsOut());

            if (kRect_ShapeFlag == fOpInfo.fInnerShapeTypes) {
                this->emitInnerRect(f, innerCoverageDecl.c_str());
            } else {
                f->codeAppendf("%s = 0.0;", innerCoverageDecl.c_str());
                f->codeAppendf("mediump vec2 distanceToArcEdge = abs(%s) - %s.xy;",
                               fInnerShapeCoords.fsIn(), fInnerRRect.fsIn());
                f->codeAppend ("if (any(lessThan(distanceToArcEdge, vec2(1e-5)))) {");
                this->emitInnerRect(f, "innerCoverage");
                f->codeAppend ("} else {");
                f->codeAppendf(    "mediump vec2 ellipseCoords = distanceToArcEdge * %s.zw;",
                                   fInnerRRect.fsIn());
                this->emitArc(f, "ellipseCoords", fInnerEllipseName.fsIn(),
                              false /*ellipseCoordsNeedClamp*/,
                              false /*ellipseCoordsMayBeNegative*/, "innerCoverage");
                f->codeAppend ("}");
            }
        }
        f->codeAppendf("%s = vec4(max(coverage - innerCoverage, 0.0));", outCoverage);
    } else if (!fTweakAlphaForCoverage) {
        f->codeAppendf("%s = vec4(coverage);", outCoverage);
    }
}

void GLSLInstanceProcessor::BackendCoverage::emitRect(GrGLSLPPFragmentBuilder* f,
                                                      const char* outCoverage,
                                                      const char* outColor) {
    if (fColorTimesRectCoverage.fsIn()) {
        f->codeAppendf("%s = %s;", outColor, fColorTimesRectCoverage.fsIn());
    } else if (fTweakAlphaForCoverage) {
        // We are drawing just ovals. The interior rect always has 100% coverage.
        f->codeAppendf("%s = %s;", outColor, fColor.fsIn());
    } else if (fRectCoverage.fsIn()) {
        f->codeAppendf("%s = %s;", outCoverage, fRectCoverage.fsIn());
    } else {
        f->codeAppendf("%s = 1.0;", outCoverage);
    }
}

void GLSLInstanceProcessor::BackendCoverage::emitCircle(GrGLSLPPFragmentBuilder* f,
                                                        const char* outCoverage) {
    // TODO: circleCoords = max(circleCoords, 0) if we decide to do this optimization on rrects.
    SkASSERT(!(kRRect_ShapesMask & fOpInfo.fShapeTypes));
    f->codeAppendf("mediump float distanceToEdge = %s - length(%s);",
                   fBloatedRadius.fsIn(), fEllipseCoords.fsIn());
    f->codeAppendf("%s = clamp(distanceToEdge, 0.0, 1.0);", outCoverage);
}

void GLSLInstanceProcessor::BackendCoverage::emitArc(GrGLSLPPFragmentBuilder* f,
                                                     const char* ellipseCoords,
                                                     const char* ellipseName,
                                                     bool ellipseCoordsNeedClamp,
                                                     bool ellipseCoordsMayBeNegative,
                                                     const char* outCoverage) {
    SkASSERT(!ellipseCoordsMayBeNegative || ellipseCoordsNeedClamp);
    if (ellipseCoordsNeedClamp) {
        // This serves two purposes:
        //  - To restrict the arcs of rounded rects to their positive quadrants.
        //  - To avoid inversesqrt(0) in the ellipse formula.
        if (ellipseCoordsMayBeNegative) {
            f->codeAppendf("mediump vec2 ellipseClampedCoords = max(abs(%s), vec2(1e-4));", 
                           ellipseCoords);
        } else {
            f->codeAppendf("mediump vec2 ellipseClampedCoords = max(%s, vec2(1e-4));", 
                           ellipseCoords);
        }
        ellipseCoords = "ellipseClampedCoords";
    }
    // ellipseCoords are in pixel space and ellipseName is 1 / rx^2, 1 / ry^2.
    f->codeAppendf("highp vec2 Z = %s * %s;", ellipseCoords, ellipseName);
    // implicit is the evaluation of (x/rx)^2 + (y/ry)^2 - 1.
    f->codeAppendf("highp float implicit = dot(Z, %s) - 1.0;", ellipseCoords);
    // gradDot is the squared length of the gradient of the implicit.
    f->codeAppendf("highp float gradDot = 4.0 * dot(Z, Z);");
    f->codeAppend ("mediump float approxDist = implicit * inversesqrt(gradDot);");
    f->codeAppendf("%s = clamp(0.5 - approxDist, 0.0, 1.0);", outCoverage);
}

void GLSLInstanceProcessor::BackendCoverage::emitInnerRect(GrGLSLPPFragmentBuilder* f,
                                                           const char* outCoverage) {
    f->codeAppendf("lowp vec2 c = %s - abs(%s);",
                   fInnerShapeBloatedHalfSize.fsIn(), fDistanceToInnerEdge.fsIn());
    f->codeAppendf("%s = clamp(min(c.x, c.y), 0.0, 1.0);", outCoverage);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

class GLSLInstanceProcessor::BackendMultisample : public Backend {
public:
    BackendMultisample(OpInfo opInfo, const VertexInputs& inputs, int effectiveSampleCnt)
            : INHERITED(opInfo, inputs)
            , fEffectiveSampleCnt(effectiveSampleCnt)
            , fShapeCoords(kVec2f_GrSLType)
            , fShapeInverseMatrix(kMat22f_GrSLType)
            , fFragShapeHalfSpan(kVec2f_GrSLType)
            , fArcTest(kVec2f_GrSLType)
            , fArcInverseMatrix(kMat22f_GrSLType)
            , fFragArcHalfSpan(kVec2f_GrSLType)
            , fEarlyAccept(kInt_GrSLType)
            , fInnerShapeInverseMatrix(kMat22f_GrSLType)
            , fFragInnerShapeHalfSpan(kVec2f_GrSLType) {
        fRectTrianglesMaySplit = fOpInfo.fHasPerspective;
        fNeedsNeighborRadii = this->isMixedSampled() && !fOpInfo.fHasPerspective;
    }

private:
    bool isMixedSampled() const { return GrAAType::kMixedSamples == fOpInfo.aaType(); }

    void onInit(GrGLSLVaryingHandler*, GrGLSLVertexBuilder*) override;
    void setupRect(GrGLSLVertexBuilder*) override;
    void setupOval(GrGLSLVertexBuilder*) override;
    void adjustRRectVertices(GrGLSLVertexBuilder*) override;
    void onSetupRRect(GrGLSLVertexBuilder*) override;

    void onInitInnerShape(GrGLSLVaryingHandler*, GrGLSLVertexBuilder*) override;
    void setupInnerRect(GrGLSLVertexBuilder*) override;
    void setupInnerOval(GrGLSLVertexBuilder*) override;
    void onSetupInnerSimpleRRect(GrGLSLVertexBuilder*) override;

    void onEmitCode(GrGLSLVertexBuilder*, GrGLSLPPFragmentBuilder*, const char*,
                    const char*) override;

    struct EmitShapeCoords {
        const GrGLSLVarying*   fVarying;
        const char*            fInverseMatrix;
        const char*            fFragHalfSpan;
    };

    struct EmitShapeOpts {
        bool fIsTightGeometry;
        bool fResolveMixedSamples;
        bool fInvertCoverage;
    };

    void emitRect(GrGLSLPPFragmentBuilder*, const EmitShapeCoords&, const EmitShapeOpts&);
    void emitArc(GrGLSLPPFragmentBuilder*, const EmitShapeCoords&, bool coordsMayBeNegative,
                 bool clampCoords, const EmitShapeOpts&);
    void emitSimpleRRect(GrGLSLPPFragmentBuilder*, const EmitShapeCoords&, const char* rrect,
                         const EmitShapeOpts&);
    void interpolateAtSample(GrGLSLPPFragmentBuilder*, const GrGLSLVarying&, const char* sampleIdx,
                             const char* interpolationMatrix);
    void acceptOrRejectWholeFragment(GrGLSLPPFragmentBuilder*, bool inside, const EmitShapeOpts&);
    void acceptCoverageMask(GrGLSLPPFragmentBuilder*, const char* shapeMask, const EmitShapeOpts&,
                            bool maybeSharedEdge = true);

    int                fEffectiveSampleCnt;
    bool               fRectTrianglesMaySplit;
    GrGLSLVertToFrag   fShapeCoords;
    GrGLSLVertToFrag   fShapeInverseMatrix;
    GrGLSLVertToFrag   fFragShapeHalfSpan;
    GrGLSLVertToFrag   fArcTest;
    GrGLSLVertToFrag   fArcInverseMatrix;
    GrGLSLVertToFrag   fFragArcHalfSpan;
    GrGLSLVertToFrag   fEarlyAccept;
    GrGLSLVertToFrag   fInnerShapeInverseMatrix;
    GrGLSLVertToFrag   fFragInnerShapeHalfSpan;
    SkString           fSquareFun;

    typedef Backend INHERITED;
};

void GLSLInstanceProcessor::BackendMultisample::onInit(GrGLSLVaryingHandler* varyingHandler,
                                                       GrGLSLVertexBuilder* v) {
    if (!this->isMixedSampled()) {
        if (kRect_ShapeFlag != fOpInfo.fShapeTypes) {
            varyingHandler->addFlatVarying("triangleIsArc", &fTriangleIsArc, kLow_GrSLPrecision);
            varyingHandler->addVarying("arcCoords", &fArcCoords, kHigh_GrSLPrecision);
            if (!fOpInfo.fHasPerspective) {
                varyingHandler->addFlatVarying("arcInverseMatrix", &fArcInverseMatrix,
                                               kHigh_GrSLPrecision);
                varyingHandler->addFlatVarying("fragArcHalfSpan", &fFragArcHalfSpan,
                                               kHigh_GrSLPrecision);
            }
        } else if (!fOpInfo.fInnerShapeTypes) {
            return;
        }
    } else {
        varyingHandler->addVarying("shapeCoords", &fShapeCoords, kHigh_GrSLPrecision);
        if (!fOpInfo.fHasPerspective) {
            varyingHandler->addFlatVarying("shapeInverseMatrix", &fShapeInverseMatrix,
                                           kHigh_GrSLPrecision);
            varyingHandler->addFlatVarying("fragShapeHalfSpan", &fFragShapeHalfSpan,
                                           kHigh_GrSLPrecision);
        }
        if (fOpInfo.fShapeTypes & kRRect_ShapesMask) {
            varyingHandler->addVarying("arcCoords", &fArcCoords, kHigh_GrSLPrecision);
            varyingHandler->addVarying("arcTest", &fArcTest, kHigh_GrSLPrecision);
            if (!fOpInfo.fHasPerspective) {
                varyingHandler->addFlatVarying("arcInverseMatrix", &fArcInverseMatrix,
                                               kHigh_GrSLPrecision);
                varyingHandler->addFlatVarying("fragArcHalfSpan", &fFragArcHalfSpan,
                                               kHigh_GrSLPrecision);
            }
        } else if (fOpInfo.fShapeTypes & kOval_ShapeFlag) {
            fArcCoords = fShapeCoords;
            fArcInverseMatrix = fShapeInverseMatrix;
            fFragArcHalfSpan = fFragShapeHalfSpan;
            if (fOpInfo.fShapeTypes & kRect_ShapeFlag) {
                varyingHandler->addFlatVarying("triangleIsArc", &fTriangleIsArc,
                                               kLow_GrSLPrecision);
            }
        }
        if (kRect_ShapeFlag != fOpInfo.fShapeTypes) {
            v->defineConstantf("int", "SAMPLE_MASK_ALL", "0x%x", (1 << fEffectiveSampleCnt) - 1);
            varyingHandler->addFlatVarying("earlyAccept", &fEarlyAccept, kHigh_GrSLPrecision);
        }
    }
    if (!fOpInfo.fHasPerspective) {
        v->codeAppend("mat2 shapeInverseMatrix = inverse(mat2(shapeMatrix));");
        v->codeAppend("vec2 fragShapeSpan = abs(vec4(shapeInverseMatrix).xz) + "
                                           "abs(vec4(shapeInverseMatrix).yw);");
    }
}

void GLSLInstanceProcessor::BackendMultisample::setupRect(GrGLSLVertexBuilder* v) {
    if (fShapeCoords.vsOut()) {
        v->codeAppendf("%s = %s;", fShapeCoords.vsOut(), this->outShapeCoords());
    }
    if (fShapeInverseMatrix.vsOut()) {
        v->codeAppendf("%s = shapeInverseMatrix;", fShapeInverseMatrix.vsOut());
    }
    if (fFragShapeHalfSpan.vsOut()) {
        v->codeAppendf("%s = 0.5 * fragShapeSpan;", fFragShapeHalfSpan.vsOut());
    }
    if (fArcTest.vsOut()) {
        // Pick a value that is not > 0.
        v->codeAppendf("%s = vec2(0);", fArcTest.vsOut());
    }
    if (fTriangleIsArc.vsOut()) {
        v->codeAppendf("%s = 0;", fTriangleIsArc.vsOut());
    }
    if (fEarlyAccept.vsOut()) {
        v->codeAppendf("%s = SAMPLE_MASK_ALL;", fEarlyAccept.vsOut());
    }
}

void GLSLInstanceProcessor::BackendMultisample::setupOval(GrGLSLVertexBuilder* v) {
    v->codeAppendf("%s = abs(%s);", fArcCoords.vsOut(), this->outShapeCoords());
    if (fArcInverseMatrix.vsOut()) {
        v->codeAppendf("vec2 s = sign(%s);", this->outShapeCoords());
        v->codeAppendf("%s = shapeInverseMatrix * mat2(s.x, 0, 0 , s.y);",
                       fArcInverseMatrix.vsOut());
    }
    if (fFragArcHalfSpan.vsOut()) {
        v->codeAppendf("%s = 0.5 * fragShapeSpan;", fFragArcHalfSpan.vsOut());
    }
    if (fArcTest.vsOut()) {
        // Pick a value that is > 0.
        v->codeAppendf("%s = vec2(1);", fArcTest.vsOut());
    }
    if (fTriangleIsArc.vsOut()) {
        if (!this->isMixedSampled()) {
            v->codeAppendf("%s = %s & 1;",
                           fTriangleIsArc.vsOut(), fInputs.attr(Attrib::kVertexAttrs));
        } else {
            v->codeAppendf("%s = 1;", fTriangleIsArc.vsOut());
        }
    }
    if (fEarlyAccept.vsOut()) {
        v->codeAppendf("%s = ~%s & SAMPLE_MASK_ALL;",
                       fEarlyAccept.vsOut(), fInputs.attr(Attrib::kVertexAttrs));
    }
}

void GLSLInstanceProcessor::BackendMultisample::adjustRRectVertices(GrGLSLVertexBuilder* v) {
    if (!this->isMixedSampled()) {
        INHERITED::adjustRRectVertices(v);
        return;
    }

    if (!fOpInfo.fHasPerspective) {
        // For the mixed samples algorithm it's best to bloat the corner triangles a bit so that
        // more of the pixels that cross into the arc region are completely inside the shared edges.
        // We also snap to a regular rect if the radii shrink smaller than a pixel.
        v->codeAppend ("vec2 midpt = 0.5 * (neighborRadii - radii);");
        v->codeAppend ("vec2 cornerSize = any(lessThan(radii, fragShapeSpan)) ? "
                           "vec2(0) : min(radii + 0.5 * fragShapeSpan, 1.0 - midpt);");
    } else {
        // TODO: We could still bloat the corner triangle in the perspective case; we would just
        // need to find the screen-space derivative of shape coords at this particular point.
        v->codeAppend ("vec2 cornerSize = any(lessThan(radii, vec2(1e-3))) ? vec2(0) : radii;");
    }

    v->codeAppendf("if (abs(%s.x) == 0.5)"
                       "%s.x = cornerSign.x * (1.0 - cornerSize.x);",
                       fInputs.attr(Attrib::kShapeCoords), fModifiedShapeCoords);
    v->codeAppendf("if (abs(%s.y) == 0.5)"
                       "%s.y = cornerSign.y * (1.0 - cornerSize.y);",
                       fInputs.attr(Attrib::kShapeCoords), fModifiedShapeCoords);
}

void GLSLInstanceProcessor::BackendMultisample::onSetupRRect(GrGLSLVertexBuilder* v) {
    if (fShapeCoords.vsOut()) {
        v->codeAppendf("%s = %s;", fShapeCoords.vsOut(), this->outShapeCoords());
    }
    if (fShapeInverseMatrix.vsOut()) {
        v->codeAppendf("%s = shapeInverseMatrix;", fShapeInverseMatrix.vsOut());
    }
    if (fFragShapeHalfSpan.vsOut()) {
        v->codeAppendf("%s = 0.5 * fragShapeSpan;", fFragShapeHalfSpan.vsOut());
    }
    if (fArcInverseMatrix.vsOut()) {
        v->codeAppend ("vec2 s = cornerSign / radii;");
        v->codeAppendf("%s = shapeInverseMatrix * mat2(s.x, 0, 0, s.y);",
                       fArcInverseMatrix.vsOut());
    }
    if (fFragArcHalfSpan.vsOut()) {
        v->codeAppendf("%s = 0.5 * (abs(vec4(%s).xz) + abs(vec4(%s).yw));",
                       fFragArcHalfSpan.vsOut(), fArcInverseMatrix.vsOut(),
                       fArcInverseMatrix.vsOut());
    }
    if (fArcTest.vsOut()) {
        // The interior triangles are laid out as a fan. fArcTest is both distances from shared
        // edges of a fan triangle to a point within that triangle. fArcTest is used to check if a
        // fragment is too close to either shared edge, in which case we point sample the shape as a
        // rect at that point in order to guarantee the mixed samples discard logic works correctly.
        v->codeAppendf("%s = (cornerSize == vec2(0)) ? vec2(0) : "
                       "cornerSign * %s * mat2(1, cornerSize.x - 1.0, cornerSize.y - 1.0, 1);",
                       fArcTest.vsOut(), fModifiedShapeCoords);
        if (!fOpInfo.fHasPerspective) {
            // Shift the point at which distances to edges are measured from the center of the pixel
            // to the corner. This way the sign of fArcTest will quickly tell us whether a pixel
            // is completely inside the shared edge. Perspective mode will accomplish this same task
            // by finding the derivatives in the fragment shader.
            v->codeAppendf("%s -= 0.5 * (fragShapeSpan.yx * abs(radii - 1.0) + fragShapeSpan);",
                           fArcTest.vsOut());
        }
    }
    if (fEarlyAccept.vsOut()) {
        SkASSERT(this->isMixedSampled());
        v->codeAppendf("%s = all(equal(vec2(1), abs(%s))) ? 0 : SAMPLE_MASK_ALL;",
                       fEarlyAccept.vsOut(), fInputs.attr(Attrib::kShapeCoords));
    }
}

void
GLSLInstanceProcessor::BackendMultisample::onInitInnerShape(GrGLSLVaryingHandler* varyingHandler,
                                                            GrGLSLVertexBuilder* v) {
    varyingHandler->addVarying("innerShapeCoords", &fInnerShapeCoords, kHigh_GrSLPrecision);
    if (kOval_ShapeFlag != fOpInfo.fInnerShapeTypes &&
        kRect_ShapeFlag != fOpInfo.fInnerShapeTypes) {
        varyingHandler->addFlatVarying("innerRRect", &fInnerRRect, kHigh_GrSLPrecision);
    }
    if (!fOpInfo.fHasPerspective) {
        varyingHandler->addFlatVarying("innerShapeInverseMatrix", &fInnerShapeInverseMatrix,
                                       kHigh_GrSLPrecision);
        v->codeAppendf("%s = shapeInverseMatrix * mat2(outer2Inner.x, 0, 0, outer2Inner.y);",
                       fInnerShapeInverseMatrix.vsOut());
        varyingHandler->addFlatVarying("fragInnerShapeHalfSpan", &fFragInnerShapeHalfSpan,
                                       kHigh_GrSLPrecision);
        v->codeAppendf("%s = 0.5 * fragShapeSpan * outer2Inner.xy;",
                       fFragInnerShapeHalfSpan.vsOut());
    }
}

void GLSLInstanceProcessor::BackendMultisample::setupInnerRect(GrGLSLVertexBuilder* v) {
    if (fInnerRRect.vsOut()) {
        // The fragment shader will generalize every inner shape as a round rect. Since this one
        // is a rect, we simply emit bogus parameters for the round rect (negative radii) that
        // ensure the fragment shader always takes the "sample as rect" codepath.
        v->codeAppendf("%s = vec4(2.0 * (inner.zw - inner.xy) / (outer.zw - outer.xy), vec2(0));",
                       fInnerRRect.vsOut());
    }
}

void GLSLInstanceProcessor::BackendMultisample::setupInnerOval(GrGLSLVertexBuilder* v) {
    if (fInnerRRect.vsOut()) {
        v->codeAppendf("%s = vec4(0, 0, 1, 1);", fInnerRRect.vsOut());
    }
}

void GLSLInstanceProcessor::BackendMultisample::onSetupInnerSimpleRRect(GrGLSLVertexBuilder* v) {
    // Avoid numeric instability by not allowing the inner radii to get smaller than 1/10th pixel.
    if (fFragInnerShapeHalfSpan.vsOut()) {
        v->codeAppendf("innerRadii = max(innerRadii, 2e-1 * %s);", fFragInnerShapeHalfSpan.vsOut());
    } else {
        v->codeAppend ("innerRadii = max(innerRadii, vec2(1e-4));");
    }
    v->codeAppendf("%s = vec4(1.0 - innerRadii, 1.0 / innerRadii);", fInnerRRect.vsOut());
}

void GLSLInstanceProcessor::BackendMultisample::onEmitCode(GrGLSLVertexBuilder*,
                                                           GrGLSLPPFragmentBuilder* f,
                                                           const char*, const char*) {
    f->defineConstant("SAMPLE_COUNT", fEffectiveSampleCnt);
    if (this->isMixedSampled()) {
        f->defineConstantf("int", "SAMPLE_MASK_ALL", "0x%x", (1 << fEffectiveSampleCnt) - 1);
        f->defineConstantf("int", "SAMPLE_MASK_MSB", "0x%x", 1 << (fEffectiveSampleCnt - 1));
    }

    if (kRect_ShapeFlag != (fOpInfo.fShapeTypes | fOpInfo.fInnerShapeTypes)) {
        GrShaderVar x("x", kVec2f_GrSLType, GrShaderVar::kNonArray, kHigh_GrSLPrecision);
        f->emitFunction(kFloat_GrSLType, "square", 1, &x, "return dot(x, x);", &fSquareFun);
    }

    EmitShapeCoords shapeCoords;
    shapeCoords.fVarying = &fShapeCoords;
    shapeCoords.fInverseMatrix = fShapeInverseMatrix.fsIn();
    shapeCoords.fFragHalfSpan = fFragShapeHalfSpan.fsIn();

    EmitShapeCoords arcCoords;
    arcCoords.fVarying = &fArcCoords;
    arcCoords.fInverseMatrix = fArcInverseMatrix.fsIn();
    arcCoords.fFragHalfSpan = fFragArcHalfSpan.fsIn();
    bool clampArcCoords = this->isMixedSampled() && (fOpInfo.fShapeTypes & kRRect_ShapesMask);

    EmitShapeOpts opts;
    opts.fIsTightGeometry = true;
    opts.fResolveMixedSamples = this->isMixedSampled();
    opts.fInvertCoverage = false;

    if (fOpInfo.fHasPerspective && fOpInfo.fInnerShapeTypes) {
        // This determines if the fragment should consider the inner shape in its sample mask.
        // We take the derivative early in case discards may occur before we get to the inner shape.
        f->codeAppendf("highp vec2 fragInnerShapeApproxHalfSpan = 0.5 * fwidth(%s);",
                       fInnerShapeCoords.fsIn());
    }

    if (!this->isMixedSampled()) {
        SkASSERT(!fArcTest.fsIn());
        if (fTriangleIsArc.fsIn()) {
            f->codeAppendf("if (%s != 0) {", fTriangleIsArc.fsIn());
            this->emitArc(f, arcCoords, false, clampArcCoords, opts);

            f->codeAppend ("}");
        }
    } else {
        const char* arcTest = fArcTest.fsIn();
        if (arcTest && fOpInfo.fHasPerspective) {
            // The non-perspective version accounts for fwidth() in the vertex shader.
            // We make sure to take the derivative here, before a neighbor pixel may early accept.
            f->codeAppendf("highp vec2 arcTest = %s - 0.5 * fwidth(%s);",
                           fArcTest.fsIn(), fArcTest.fsIn());
            arcTest = "arcTest";
        }
        const char* earlyAccept = fEarlyAccept.fsIn() ? fEarlyAccept.fsIn() : "SAMPLE_MASK_ALL";
        f->codeAppendf("if (gl_SampleMaskIn[0] == %s) {", earlyAccept);
        f->overrideSampleCoverage(earlyAccept);
        f->codeAppend ("} else {");
        if (arcTest) {
            // At this point, if the sample mask is all set it means we are inside an arc triangle.
            f->codeAppendf("if (gl_SampleMaskIn[0] == SAMPLE_MASK_ALL || "
                               "all(greaterThan(%s, vec2(0)))) {", arcTest);
            this->emitArc(f, arcCoords, false, clampArcCoords, opts);
            f->codeAppend ("} else {");
            this->emitRect(f, shapeCoords, opts);
            f->codeAppend ("}");
        } else if (fTriangleIsArc.fsIn()) {
            f->codeAppendf("if (%s == 0) {", fTriangleIsArc.fsIn());
            this->emitRect(f, shapeCoords, opts);
            f->codeAppend ("} else {");
            this->emitArc(f, arcCoords, false, clampArcCoords, opts);
            f->codeAppend ("}");
        } else if (fOpInfo.fShapeTypes == kOval_ShapeFlag) {
            this->emitArc(f, arcCoords, false, clampArcCoords, opts);
        } else {
            SkASSERT(fOpInfo.fShapeTypes == kRect_ShapeFlag);
            this->emitRect(f, shapeCoords, opts);
        }
        f->codeAppend ("}");
    }

    if (fOpInfo.fInnerShapeTypes) {
        f->codeAppendf("// Inner shape.\n");

        EmitShapeCoords innerShapeCoords;
        innerShapeCoords.fVarying = &fInnerShapeCoords;
        if (!fOpInfo.fHasPerspective) {
            innerShapeCoords.fInverseMatrix = fInnerShapeInverseMatrix.fsIn();
            innerShapeCoords.fFragHalfSpan = fFragInnerShapeHalfSpan.fsIn();
        }

        EmitShapeOpts innerOpts;
        innerOpts.fIsTightGeometry = false;
        innerOpts.fResolveMixedSamples = false; // Mixed samples are resolved in the outer shape.
        innerOpts.fInvertCoverage = true;

        if (kOval_ShapeFlag == fOpInfo.fInnerShapeTypes) {
            this->emitArc(f, innerShapeCoords, true, false, innerOpts);
        } else {
            f->codeAppendf("if (all(lessThan(abs(%s), 1.0 + %s))) {", fInnerShapeCoords.fsIn(),
                           !fOpInfo.fHasPerspective ? innerShapeCoords.fFragHalfSpan
                                                    : "fragInnerShapeApproxHalfSpan");  // Above.
            if (kRect_ShapeFlag == fOpInfo.fInnerShapeTypes) {
                this->emitRect(f, innerShapeCoords, innerOpts);
            } else {
                this->emitSimpleRRect(f, innerShapeCoords, fInnerRRect.fsIn(), innerOpts);
            }
            f->codeAppend ("}");
        }
    }
}

void GLSLInstanceProcessor::BackendMultisample::emitRect(GrGLSLPPFragmentBuilder* f,
                                                         const EmitShapeCoords& coords,
                                                         const EmitShapeOpts& opts) {
    // Full MSAA doesn't need to do anything to draw a rect.
    SkASSERT(!opts.fIsTightGeometry || opts.fResolveMixedSamples);
    if (coords.fFragHalfSpan) {
        f->codeAppendf("if (all(lessThanEqual(abs(%s), 1.0 - %s))) {",
                       coords.fVarying->fsIn(), coords.fFragHalfSpan);
        // The entire pixel is inside the rect.
        this->acceptOrRejectWholeFragment(f, true, opts);
        f->codeAppend ("} else ");
        if (opts.fIsTightGeometry && !fRectTrianglesMaySplit) {
            f->codeAppendf("if (any(lessThan(abs(%s), 1.0 - %s))) {",
                           coords.fVarying->fsIn(), coords.fFragHalfSpan);
            // The pixel falls on an edge of the rectangle and is known to not be on a shared edge.
            this->acceptCoverageMask(f, "gl_SampleMaskIn[0]", opts, false);
            f->codeAppend ("} else");
        }
        f->codeAppend ("{");
    }
    f->codeAppend ("int rectMask = 0;");
    f->codeAppend ("for (int i = 0; i < SAMPLE_COUNT; i++) {");
    f->codeAppend (    "highp vec2 pt = ");
    this->interpolateAtSample(f, *coords.fVarying, "i", coords.fInverseMatrix);
    f->codeAppend (    ";");
    f->codeAppend (    "if (all(lessThan(abs(pt), vec2(1)))) rectMask |= (1 << i);");
    f->codeAppend ("}");
    this->acceptCoverageMask(f, "rectMask", opts);
    if (coords.fFragHalfSpan) {
        f->codeAppend ("}");
    }
}

void GLSLInstanceProcessor::BackendMultisample::emitArc(GrGLSLPPFragmentBuilder* f,
                                                        const EmitShapeCoords& coords,
                                                        bool coordsMayBeNegative, bool clampCoords,
                                                        const EmitShapeOpts& opts) {
    if (coords.fFragHalfSpan) {
        SkString absArcCoords;
        absArcCoords.printf(coordsMayBeNegative ? "abs(%s)" : "%s", coords.fVarying->fsIn());
        if (clampCoords) {
            f->codeAppendf("if (%s(max(%s + %s, vec2(0))) < 1.0) {",
                           fSquareFun.c_str(), absArcCoords.c_str(), coords.fFragHalfSpan);
        } else {
            f->codeAppendf("if (%s(%s + %s) < 1.0) {",
                           fSquareFun.c_str(), absArcCoords.c_str(), coords.fFragHalfSpan);
        }
        // The entire pixel is inside the arc.
        this->acceptOrRejectWholeFragment(f, true, opts);
        f->codeAppendf("} else if (%s(max(%s - %s, vec2(0))) >= 1.0) {",
                       fSquareFun.c_str(), absArcCoords.c_str(), coords.fFragHalfSpan);
        // The entire pixel is outside the arc.
        this->acceptOrRejectWholeFragment(f, false, opts);
        f->codeAppend ("} else {");
    }
    f->codeAppend (    "int arcMask = 0;");
    f->codeAppend (    "for (int i = 0; i < SAMPLE_COUNT; i++) {");
    f->codeAppend (        "highp vec2 pt = ");
    this->interpolateAtSample(f, *coords.fVarying, "i", coords.fInverseMatrix);
    f->codeAppend (        ";");
    if (clampCoords) {
        SkASSERT(!coordsMayBeNegative);
        f->codeAppend (    "pt = max(pt, vec2(0));");
    }
    f->codeAppendf(        "if (%s(pt) < 1.0) arcMask |= (1 << i);", fSquareFun.c_str());
    f->codeAppend (    "}");
    this->acceptCoverageMask(f, "arcMask", opts);
    if (coords.fFragHalfSpan) {
        f->codeAppend ("}");
    }
}

void GLSLInstanceProcessor::BackendMultisample::emitSimpleRRect(GrGLSLPPFragmentBuilder* f,
                                                                const EmitShapeCoords& coords,
                                                                const char* rrect,
                                                                const EmitShapeOpts& opts) {
    f->codeAppendf("highp vec2 distanceToArcEdge = abs(%s) - %s.xy;", coords.fVarying->fsIn(), 
                   rrect);
    f->codeAppend ("if (any(lessThan(distanceToArcEdge, vec2(0)))) {");
    this->emitRect(f, coords, opts);
    f->codeAppend ("} else {");
    if (coords.fInverseMatrix && coords.fFragHalfSpan) {
        f->codeAppendf("highp vec2 rrectCoords = distanceToArcEdge * %s.zw;", rrect);
        f->codeAppendf("highp vec2 fragRRectHalfSpan = %s * %s.zw;", coords.fFragHalfSpan, rrect);
        f->codeAppendf("if (%s(rrectCoords + fragRRectHalfSpan) <= 1.0) {", fSquareFun.c_str());
        // The entire pixel is inside the round rect.
        this->acceptOrRejectWholeFragment(f, true, opts);
        f->codeAppendf("} else if (%s(max(rrectCoords - fragRRectHalfSpan, vec2(0))) >= 1.0) {",
                       fSquareFun.c_str());
        // The entire pixel is outside the round rect.
        this->acceptOrRejectWholeFragment(f, false, opts);
        f->codeAppend ("} else {");
        f->codeAppendf(    "highp vec2 s = %s.zw * sign(%s);", rrect, coords.fVarying->fsIn());
        f->codeAppendf(    "highp mat2 innerRRectInverseMatrix = %s * mat2(s.x, 0, 0, s.y);",
                           coords.fInverseMatrix);
        f->codeAppend (    "highp int rrectMask = 0;");
        f->codeAppend (    "for (int i = 0; i < SAMPLE_COUNT; i++) {");
        f->codeAppend (        "highp vec2 pt = rrectCoords + ");
        f->appendOffsetToSample("i", GrGLSLFPFragmentBuilder::kSkiaDevice_Coordinates);
        f->codeAppend (                  "* innerRRectInverseMatrix;");
        f->codeAppendf(        "if (%s(max(pt, vec2(0))) < 1.0) rrectMask |= (1 << i);",
                               fSquareFun.c_str());
        f->codeAppend (    "}");
        this->acceptCoverageMask(f, "rrectMask", opts);
        f->codeAppend ("}");
    } else {
        f->codeAppend ("int rrectMask = 0;");
        f->codeAppend ("for (int i = 0; i < SAMPLE_COUNT; i++) {");
        f->codeAppend (    "highp vec2 shapePt = ");
        this->interpolateAtSample(f, *coords.fVarying, "i", nullptr);
        f->codeAppend (    ";");
        f->codeAppendf(    "highp vec2 rrectPt = max(abs(shapePt) - %s.xy, vec2(0)) * %s.zw;",
                           rrect, rrect);
        f->codeAppendf(    "if (%s(rrectPt) < 1.0) rrectMask |= (1 << i);", fSquareFun.c_str());
        f->codeAppend ("}");
        this->acceptCoverageMask(f, "rrectMask", opts);
    }
    f->codeAppend ("}");
}

void GLSLInstanceProcessor::BackendMultisample::interpolateAtSample(GrGLSLPPFragmentBuilder* f,
                                                                  const GrGLSLVarying& varying,
                                                                  const char* sampleIdx,
                                                                  const char* interpolationMatrix) {
    if (interpolationMatrix) {
        f->codeAppendf("(%s + ", varying.fsIn());
        f->appendOffsetToSample(sampleIdx, GrGLSLFPFragmentBuilder::kSkiaDevice_Coordinates);
        f->codeAppendf(" * %s)", interpolationMatrix);
    } else {
        SkAssertResult(
            f->enableFeature(GrGLSLFragmentBuilder::kMultisampleInterpolation_GLSLFeature));
        f->codeAppendf("interpolateAtOffset(%s, ", varying.fsIn());
        f->appendOffsetToSample(sampleIdx, GrGLSLFPFragmentBuilder::kGLSLWindow_Coordinates);
        f->codeAppend(")");
    }
}

void
GLSLInstanceProcessor::BackendMultisample::acceptOrRejectWholeFragment(GrGLSLPPFragmentBuilder* f,
                                                                       bool inside,
                                                                       const EmitShapeOpts& opts) {
    if (inside != opts.fInvertCoverage) { // Accept the entire fragment.
        if (opts.fResolveMixedSamples) {
            // This is a mixed sampled fragment in the interior of the shape. Reassign 100% coverage
            // to one fragment, and drop all other fragments that may fall on this same pixel. Since
            // our geometry is water tight and non-overlapping, we can take advantage of the
            // properties that (1) the incoming sample masks will be disjoint across fragments that
            // fall on a common pixel, and (2) since the entire fragment is inside the shape, each
            // sample's corresponding bit will be set in the incoming sample mask of exactly one
            // fragment.
            f->codeAppend("if ((gl_SampleMaskIn[0] & SAMPLE_MASK_MSB) == 0) {");
            // Drop this fragment.
            if (!fOpInfo.fCannotDiscard) {
                f->codeAppend("discard;");
            } else {
                f->overrideSampleCoverage("0");
            }
            f->codeAppend("} else {");
            // Override the lone surviving fragment to full coverage.
            f->overrideSampleCoverage("-1");
            f->codeAppend("}");
        }
    } else { // Reject the entire fragment.
        if (!fOpInfo.fCannotDiscard) {
            f->codeAppend("discard;");
        } else if (opts.fResolveMixedSamples) {
            f->overrideSampleCoverage("0");
        } else {
            f->maskSampleCoverage("0");
        }
    }
}

void GLSLInstanceProcessor::BackendMultisample::acceptCoverageMask(GrGLSLPPFragmentBuilder* f,
                                                                   const char* shapeMask,
                                                                   const EmitShapeOpts& opts,
                                                                   bool maybeSharedEdge) {
    if (opts.fResolveMixedSamples) {
        if (maybeSharedEdge) {
            // This is a mixed sampled fragment, potentially on the outer edge of the shape, with
            // only partial shape coverage. Override the coverage of one fragment to "shapeMask",
            // and drop all other fragments that may fall on this same pixel. Since our geometry is
            // water tight, non-overlapping, and completely contains the shape, this means that each
            // "on" bit from shapeMask is guaranteed to be set in the incoming sample mask of one,
            // and only one, fragment that falls on this same pixel.
            SkASSERT(!opts.fInvertCoverage);
            f->codeAppendf("if ((gl_SampleMaskIn[0] & (1 << findMSB(%s))) == 0) {", shapeMask);
            // Drop this fragment.
            if (!fOpInfo.fCannotDiscard) {
                f->codeAppend ("discard;");
            } else {
                f->overrideSampleCoverage("0");
            }
            f->codeAppend ("} else {");
            // Override the coverage of the lone surviving fragment to "shapeMask".
            f->overrideSampleCoverage(shapeMask);
            f->codeAppend ("}");
        } else {
            f->overrideSampleCoverage(shapeMask);
        }
    } else {
        f->maskSampleCoverage(shapeMask, opts.fInvertCoverage);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

GLSLInstanceProcessor::Backend* GLSLInstanceProcessor::Backend::Create(const GrPipeline& pipeline,
                                                                       OpInfo opInfo,
                                                                       const VertexInputs& inputs) {
    switch (opInfo.aaType()) {
        default:
            SkFAIL("Unexpected antialias mode.");
        case GrAAType::kNone:
            return new BackendNonAA(opInfo, inputs);
        case GrAAType::kCoverage:
            return new BackendCoverage(opInfo, inputs);
        case GrAAType::kMSAA:
        case GrAAType::kMixedSamples: {
            const GrRenderTargetPriv& rtp = pipeline.getRenderTarget()->renderTargetPriv();
            const GrGpu::MultisampleSpecs& specs = rtp.getMultisampleSpecs(pipeline);
            return new BackendMultisample(opInfo, inputs, specs.fEffectiveSampleCnt);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

const ShapeVertex kVertexData[] = {
    // Rectangle.
    {+1, +1, ~0},   /*0*/
    {-1, +1, ~0},   /*1*/
    {-1, -1, ~0},   /*2*/
    {+1, -1, ~0},   /*3*/
    // The next 4 are for the bordered version.
    {+1, +1,  0},   /*4*/
    {-1, +1,  0},   /*5*/
    {-1, -1,  0},   /*6*/
    {+1, -1,  0},   /*7*/

    // Octagon that inscribes the unit circle, cut by an interior unit octagon.
    {+1.000000f,  0.000000f,  0},   /* 8*/
    {+1.000000f, +0.414214f, ~0},   /* 9*/
    {+0.707106f, +0.707106f,  0},   /*10*/
    {+0.414214f, +1.000000f, ~0},   /*11*/
    { 0.000000f, +1.000000f,  0},   /*12*/
    {-0.414214f, +1.000000f, ~0},   /*13*/
    {-0.707106f, +0.707106f,  0},   /*14*/
    {-1.000000f, +0.414214f, ~0},   /*15*/
    {-1.000000f,  0.000000f,  0},   /*16*/
    {-1.000000f, -0.414214f, ~0},   /*17*/
    {-0.707106f, -0.707106f,  0},   /*18*/
    {-0.414214f, -1.000000f, ~0},   /*19*/
    { 0.000000f, -1.000000f,  0},   /*20*/
    {+0.414214f, -1.000000f, ~0},   /*21*/
    {+0.707106f, -0.707106f,  0},   /*22*/
    {+1.000000f, -0.414214f, ~0},   /*23*/
    // This vertex is for the fanned versions.
    { 0.000000f,  0.000000f, ~0},   /*24*/

    // Rectangle with disjoint corner segments.
    {+1.0, +0.5,  0x3},   /*25*/
    {+1.0, +1.0,  0x3},   /*26*/
    {+0.5, +1.0,  0x3},   /*27*/
    {-0.5, +1.0,  0x2},   /*28*/
    {-1.0, +1.0,  0x2},   /*29*/
    {-1.0, +0.5,  0x2},   /*30*/
    {-1.0, -0.5,  0x0},   /*31*/
    {-1.0, -1.0,  0x0},   /*32*/
    {-0.5, -1.0,  0x0},   /*33*/
    {+0.5, -1.0,  0x1},   /*34*/
    {+1.0, -1.0,  0x1},   /*35*/
    {+1.0, -0.5,  0x1},   /*36*/
    // The next 4 are for the fanned version.
    { 0.0,  0.0,  0x3},   /*37*/
    { 0.0,  0.0,  0x2},   /*38*/
    { 0.0,  0.0,  0x0},   /*39*/
    { 0.0,  0.0,  0x1},   /*40*/
    // The next 8 are for the bordered version.
    {+0.75, +0.50,  0x3},   /*41*/
    {+0.50, +0.75,  0x3},   /*42*/
    {-0.50, +0.75,  0x2},   /*43*/
    {-0.75, +0.50,  0x2},   /*44*/
    {-0.75, -0.50,  0x0},   /*45*/
    {-0.50, -0.75,  0x0},   /*46*/
    {+0.50, -0.75,  0x1},   /*47*/
    {+0.75, -0.50,  0x1},   /*48*/

    // 16-gon that inscribes the unit circle, cut by an interior unit 16-gon.
    {+1.000000f, +0.000000f,  0},   /*49*/
    {+1.000000f, +0.198913f, ~0},   /*50*/
    {+0.923879f, +0.382683f,  0},   /*51*/
    {+0.847760f, +0.566455f, ~0},   /*52*/
    {+0.707106f, +0.707106f,  0},   /*53*/
    {+0.566455f, +0.847760f, ~0},   /*54*/
    {+0.382683f, +0.923879f,  0},   /*55*/
    {+0.198913f, +1.000000f, ~0},   /*56*/
    {+0.000000f, +1.000000f,  0},   /*57*/
    {-0.198913f, +1.000000f, ~0},   /*58*/
    {-0.382683f, +0.923879f,  0},   /*59*/
    {-0.566455f, +0.847760f, ~0},   /*60*/
    {-0.707106f, +0.707106f,  0},   /*61*/
    {-0.847760f, +0.566455f, ~0},   /*62*/
    {-0.923879f, +0.382683f,  0},   /*63*/
    {-1.000000f, +0.198913f, ~0},   /*64*/
    {-1.000000f, +0.000000f,  0},   /*65*/
    {-1.000000f, -0.198913f, ~0},   /*66*/
    {-0.923879f, -0.382683f,  0},   /*67*/
    {-0.847760f, -0.566455f, ~0},   /*68*/
    {-0.707106f, -0.707106f,  0},   /*69*/
    {-0.566455f, -0.847760f, ~0},   /*70*/
    {-0.382683f, -0.923879f,  0},   /*71*/
    {-0.198913f, -1.000000f, ~0},   /*72*/
    {-0.000000f, -1.000000f,  0},   /*73*/
    {+0.198913f, -1.000000f, ~0},   /*74*/
    {+0.382683f, -0.923879f,  0},   /*75*/
    {+0.566455f, -0.847760f, ~0},   /*76*/
    {+0.707106f, -0.707106f,  0},   /*77*/
    {+0.847760f, -0.566455f, ~0},   /*78*/
    {+0.923879f, -0.382683f,  0},   /*79*/
    {+1.000000f, -0.198913f, ~0},   /*80*/
};

const uint8_t kIndexData[] = {
    // Rectangle.
    0, 1, 2,
    0, 2, 3,

    // Rectangle with a border.
    0, 1, 5,
    5, 4, 0,
    1, 2, 6,
    6, 5, 1,
    2, 3, 7,
    7, 6, 2,
    3, 0, 4,
    4, 7, 3,
    4, 5, 6,
    6, 7, 4,

    // Octagon that inscribes the unit circle, cut by an interior unit octagon.
    10,  8,  9,
    12, 10, 11,
    14, 12, 13,
    16, 14, 15,
    18, 16, 17,
    20, 18, 19,
    22, 20, 21,
     8, 22, 23,
     8, 10, 12,
    12, 14, 16,
    16, 18, 20,
    20, 22,  8,
     8, 12, 16,
    16, 20,  8,

    // Same octagons, but with the interior arranged as a fan. Used by mixed samples.
    10,  8,  9,
    12, 10, 11,
    14, 12, 13,
    16, 14, 15,
    18, 16, 17,
    20, 18, 19,
    22, 20, 21,
     8, 22, 23,
    24,  8, 10,
    12, 24, 10,
    24, 12, 14,
    16, 24, 14,
    24, 16, 18,
    20, 24, 18,
    24, 20, 22,
     8, 24, 22,

    // Same octagons, but with the inner and outer disjoint. Used by coverage AA.
     8, 22, 23,
     9,  8, 23,
    10,  8,  9,
    11, 10,  9,
    12, 10, 11,
    13, 12, 11,
    14, 12, 13,
    15, 14, 13,
    16, 14, 15,
    17, 16, 15,
    18, 16, 17,
    19, 18, 17,
    20, 18, 19,
    21, 20, 19,
    22, 20, 21,
    23, 22, 21,
    22,  8, 10,
    10, 12, 14,
    14, 16, 18,
    18, 20, 22,
    22, 10, 14,
    14, 18, 22,

    // Rectangle with disjoint corner segments.
    27, 25, 26,
    30, 28, 29,
    33, 31, 32,
    36, 34, 35,
    25, 27, 28,
    28, 30, 31,
    31, 33, 34,
    34, 36, 25,
    25, 28, 31,
    31, 34, 25,

    // Same rectangle with disjoint corners, but with the interior arranged as a fan. Used by
    // mixed samples.
    27, 25, 26,
    30, 28, 29,
    33, 31, 32,
    36, 34, 35,
    27, 37, 25,
    28, 37, 27,
    30, 38, 28,
    31, 38, 30,
    33, 39, 31,
    34, 39, 33,
    36, 40, 34,
    25, 40, 36,

    // Same rectangle with disjoint corners, with a border as well. Used by coverage AA.
    41, 25, 26,
    42, 41, 26,
    27, 42, 26,
    43, 28, 29,
    44, 43, 29,
    30, 44, 29,
    45, 31, 32,
    46, 45, 32,
    33, 46, 32,
    47, 34, 35,
    48, 47, 35,
    36, 48, 35,
    27, 28, 42,
    42, 28, 43,
    30, 31, 44,
    44, 31, 45,
    33, 34, 46,
    46, 34, 47,
    36, 25, 48,
    48, 25, 41,
    41, 42, 43,
    43, 44, 45,
    45, 46, 47,
    47, 48, 41,
    41, 43, 45,
    45, 47, 41,

    // Same as the disjoint octagons, but with 16-gons instead. Used by coverage AA when the oval is
    // sufficiently large.
    49, 79, 80,
    50, 49, 80,
    51, 49, 50,
    52, 51, 50,
    53, 51, 52,
    54, 53, 52,
    55, 53, 54,
    56, 55, 54,
    57, 55, 56,
    58, 57, 56,
    59, 57, 58,
    60, 59, 58,
    61, 59, 60,
    62, 61, 60,
    63, 61, 62,
    64, 63, 62,
    65, 63, 64,
    66, 65, 64,
    67, 65, 66,
    68, 67, 66,
    69, 67, 68,
    70, 69, 68,
    71, 69, 70,
    72, 71, 70,
    73, 71, 72,
    74, 73, 72,
    75, 73, 74,
    76, 75, 74,
    77, 75, 76,
    78, 77, 76,
    79, 77, 78,
    80, 79, 78,
    49, 51, 53,
    53, 55, 57,
    57, 59, 61,
    61, 63, 65,
    65, 67, 69,
    69, 71, 73,
    73, 75, 77,
    77, 79, 49,
    49, 53, 57,
    57, 61, 65,
    65, 69, 73,
    73, 77, 49,
    49, 57, 65,
    65, 73, 49,
};

enum {
    kRect_FirstIndex = 0,
    kRect_TriCount = 2,

    kFramedRect_FirstIndex = 6,
    kFramedRect_TriCount = 10,

    kOctagons_FirstIndex = 36,
    kOctagons_TriCount = 14,

    kOctagonsFanned_FirstIndex = 78,
    kOctagonsFanned_TriCount = 16,

    kDisjointOctagons_FirstIndex = 126,
    kDisjointOctagons_TriCount = 22,

    kCorneredRect_FirstIndex = 192,
    kCorneredRect_TriCount = 10,

    kCorneredRectFanned_FirstIndex = 222,
    kCorneredRectFanned_TriCount = 12,

    kCorneredFramedRect_FirstIndex = 258,
    kCorneredFramedRect_TriCount = 26,

    kDisjoint16Gons_FirstIndex = 336,
    kDisjoint16Gons_TriCount = 46,
};

GR_DECLARE_STATIC_UNIQUE_KEY(gShapeVertexBufferKey);

const GrBuffer* InstanceProcessor::FindOrCreateVertexBuffer(GrGpu* gpu) {
    GR_DEFINE_STATIC_UNIQUE_KEY(gShapeVertexBufferKey);
    GrResourceCache* cache = gpu->getContext()->getResourceCache();
    if (GrGpuResource* cached = cache->findAndRefUniqueResource(gShapeVertexBufferKey)) {
        return static_cast<GrBuffer*>(cached);
    }
    if (GrBuffer* buffer = gpu->createBuffer(sizeof(kVertexData), kVertex_GrBufferType,
                                             kStatic_GrAccessPattern, kVertexData)) {
        buffer->resourcePriv().setUniqueKey(gShapeVertexBufferKey);
        return buffer;
    }
    return nullptr;
}

GR_DECLARE_STATIC_UNIQUE_KEY(gShapeIndexBufferKey);

const GrBuffer* InstanceProcessor::FindOrCreateIndex8Buffer(GrGpu* gpu) {
    GR_DEFINE_STATIC_UNIQUE_KEY(gShapeIndexBufferKey);
    GrResourceCache* cache = gpu->getContext()->getResourceCache();
    if (GrGpuResource* cached = cache->findAndRefUniqueResource(gShapeIndexBufferKey)) {
        return static_cast<GrBuffer*>(cached);
    }
    if (GrBuffer* buffer = gpu->createBuffer(sizeof(kIndexData), kIndex_GrBufferType,
                                             kStatic_GrAccessPattern, kIndexData)) {
        buffer->resourcePriv().setUniqueKey(gShapeIndexBufferKey);
        return buffer;
    }
    return nullptr;
}

IndexRange InstanceProcessor::GetIndexRangeForRect(GrAAType aaType) {
    switch (aaType) {
        case GrAAType::kCoverage:
            return {kFramedRect_FirstIndex, 3 * kFramedRect_TriCount};
        case GrAAType::kNone:
        case GrAAType::kMSAA:
        case GrAAType::kMixedSamples:
            return {kRect_FirstIndex, 3 * kRect_TriCount};
    }
    SkFAIL("Unexpected aa type!");
    return {0, 0};
}

IndexRange InstanceProcessor::GetIndexRangeForOval(GrAAType aaType, const SkRect& devBounds) {
    if (GrAAType::kCoverage == aaType && devBounds.height() * devBounds.width() >= 256 * 256) {
        // This threshold was chosen quasi-scientifically on Tegra X1.
        return {kDisjoint16Gons_FirstIndex, 3 * kDisjoint16Gons_TriCount};
    }

    switch (aaType) {
        case GrAAType::kNone:
        case GrAAType::kMSAA:
            return {kOctagons_FirstIndex, 3 * kOctagons_TriCount};
        case GrAAType::kCoverage:
            return {kDisjointOctagons_FirstIndex, 3 * kDisjointOctagons_TriCount};
        case GrAAType::kMixedSamples:
            return {kOctagonsFanned_FirstIndex, 3 * kOctagonsFanned_TriCount};
    }
    SkFAIL("Unexpected aa type!");
    return {0, 0};
}

IndexRange InstanceProcessor::GetIndexRangeForRRect(GrAAType aaType) {
    switch (aaType) {
        case GrAAType::kNone:
        case GrAAType::kMSAA:
            return {kCorneredRect_FirstIndex, 3 * kCorneredRect_TriCount};
        case GrAAType::kCoverage:
            return {kCorneredFramedRect_FirstIndex, 3 * kCorneredFramedRect_TriCount};
        case GrAAType::kMixedSamples:
            return {kCorneredRectFanned_FirstIndex, 3 * kCorneredRectFanned_TriCount};
    }
    SkFAIL("Unexpected aa type!");
    return {0, 0};
}

const char* InstanceProcessor::GetNameOfIndexRange(IndexRange range) {
    switch (range.fStart) {
        case kRect_FirstIndex: return "basic_rect";
        case kFramedRect_FirstIndex: return "coverage_rect";

        case kOctagons_FirstIndex: return "basic_oval";
        case kDisjointOctagons_FirstIndex: return "coverage_oval";
        case kDisjoint16Gons_FirstIndex: return "coverage_large_oval";
        case kOctagonsFanned_FirstIndex: return "mixed_samples_oval";

        case kCorneredRect_FirstIndex: return "basic_round_rect";
        case kCorneredFramedRect_FirstIndex: return "coverage_round_rect";
        case kCorneredRectFanned_FirstIndex: return "mixed_samples_round_rect";

        default: return "unknown";
    }
}

}
