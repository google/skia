/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGeometryProcessor_DEFINED
#define GrGeometryProcessor_DEFINED

#include "GrPrimitiveProcessor.h"

/**
 * A GrGeometryProcessor is a flexible method for rendering a primitive.  The GrGeometryProcessor
 * has complete control over vertex attributes and uniforms(aside from the render target) but it
 * must obey the same contract as any GrPrimitiveProcessor, specifically it must emit a color and
 * coverage into the fragment shader.  Where this color and coverage come from is completely the
 * responsibility of the GrGeometryProcessor.
 */
class GrGeometryProcessor : public GrPrimitiveProcessor {
public:
    GrGeometryProcessor(ClassID classID)
        : INHERITED(classID)
        , fWillUseGeoShader(false)
        , fLocalCoordsType(kUnused_LocalCoordsType)
        , fSampleShading(0.0) {}

    bool willUseGeoShader() const final { return fWillUseGeoShader; }

    bool hasExplicitLocalCoords() const final {
        return kHasExplicit_LocalCoordsType == fLocalCoordsType;
    }

    /**
     * Returns the minimum fraction of samples for which the fragment shader will be run. For
     * instance, if sampleShading is 0.5 in MSAA16 mode, the fragment shader will run a minimum of
     * 8 times per pixel. The default value is zero.
     */
    float getSampleShading() const final { return fSampleShading; }

protected:
    void setWillUseGeoShader() { fWillUseGeoShader = true; }

    /**
     * If a GrFragmentProcessor in the GrPipeline needs localCoods, we will provide them in one of
     * three ways
     * 1) LocalCoordTransform * Position - in Shader
     * 2) LocalCoordTransform * ExplicitLocalCoords- in Shader
     * 3) A transformation on the CPU uploaded via vertex attribute
     */
    enum LocalCoordsType {
        kUnused_LocalCoordsType,
        kHasExplicit_LocalCoordsType,
        kHasTransformed_LocalCoordsType
    };

    void setHasExplicitLocalCoords() {
        SkASSERT(kUnused_LocalCoordsType == fLocalCoordsType);
        fLocalCoordsType = kHasExplicit_LocalCoordsType;
    }
    void setHasTransformedLocalCoords() {
        SkASSERT(kUnused_LocalCoordsType == fLocalCoordsType);
        fLocalCoordsType = kHasTransformed_LocalCoordsType;
    }

    void setSampleShading(float sampleShading) {
        fSampleShading = sampleShading;
    }

    /**
     * Recursive helpers for implementating onVertexAttribute or onInstanceAttribute.
     */

    template<typename... Args>
    static const Attribute& IthAttribute(int i, const Attribute &attr0, const Args&... attrs) {
        SkASSERT(attr0.isInitialized());
        return (0 == i) ? attr0 : IthAttribute(i - 1, attrs...);
    }

    static const Attribute& IthAttribute(int i) {
        SK_ABORT("Illegal attribute Index");
        static constexpr Attribute kBogus;
        return kBogus;
    }

    template<typename... Args>
    static const Attribute& IthInitializedAttribute(int i, const Attribute &attr0, const Args&... attrs) {
        if (attr0.isInitialized()) {
            if (0 == i) {
                return attr0;
            }
            i -= 1;
        }
        return IthInitializedAttribute(i, attrs...);
    }

    static const Attribute& IthInitializedAttribute(int i) { return IthAttribute(i); }

private:
    const Attribute& onInstanceAttribute(int i) const override {
        SK_ABORT("No instanced attributes");
        static constexpr Attribute kBogus;
        return kBogus;
    }

    bool fWillUseGeoShader;
    LocalCoordsType fLocalCoordsType;
    float fSampleShading;

    typedef GrPrimitiveProcessor INHERITED;
};

#endif
