/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef AtlasInstancedHelper_DEFINED
#define AtlasInstancedHelper_DEFINED

#include "src/core/SkIPoint16.h"
#include "src/gpu/GrGeometryProcessor.h"
#include "src/gpu/GrSurfaceProxyView.h"
#include "src/gpu/glsl/GrGLSLUniformHandler.h"

namespace skgpu::v1 {

// This class encapsulates all the necessary steps for an instanced GrGeometryProcessor to clip
// against a path mask from an atlas.
class AtlasInstancedHelper {
public:
    enum class ShaderFlags {
        kNone = 0,
        kInvertCoverage = 1 << 0,
        kCheckBounds = 1 << 1
    };

    GR_DECL_BITFIELD_CLASS_OPS_FRIENDS(ShaderFlags);

    constexpr static int kNumShaderFlags = 2;

    AtlasInstancedHelper(GrSurfaceProxyView atlasView, ShaderFlags shaderFlags)
            : fAtlasProxy(atlasView.detachProxy())
            , fAtlasSwizzle(atlasView.swizzle())
            , fShaderFlags(shaderFlags) {
        // Bottom left origin is not supported.
        SkASSERT(atlasView.origin() == kTopLeft_GrSurfaceOrigin);
    }

    GrSurfaceProxy* proxy() const { return fAtlasProxy.get(); }
    const GrSwizzle& atlasSwizzle() const { return fAtlasSwizzle; }

    // Returns whether the two helpers can be batched together in a single draw.
    bool isCompatible(const AtlasInstancedHelper& helper) {
        // TODO: We may want to consider two helpers compatible if they only differ in the
        // kCheckBounds flag -- we can always promote one to checking its bounds.
        SkASSERT(fAtlasProxy != helper.fAtlasProxy || fAtlasSwizzle == helper.fAtlasSwizzle);
        return fAtlasProxy == helper.fAtlasProxy && fShaderFlags == helper.fShaderFlags;
    }

    // Adds bits to the shader key that uniquely identify this specific helper's shader code.
    void getKeyBits(KeyBuilder* b) const;

    // Appends the instanced input attribs to the back of the array that we will need in order to
    // locate our path in the atlas.
    void appendInstanceAttribs(SkTArray<GrGeometryProcessor::Attribute>* instanceAttribs) const;

    struct Instance {
        Instance(SkIPoint16 locationInAtlas, const SkIRect& pathDevIBounds, bool transposedInAtlas)
                : fLocationInAtlas(locationInAtlas)
                , fPathDevIBounds(pathDevIBounds)
                , fTransposedInAtlas(transposedInAtlas) {
            SkASSERT(fLocationInAtlas.x() >= 0);
            SkASSERT(fLocationInAtlas.y() >= 0);
        }
        SkIPoint16 fLocationInAtlas;
        SkIRect fPathDevIBounds;
        bool fTransposedInAtlas;
    };

    // Writes out the given instance data, formatted for the specific attribs that we added during
    // appendInstanceAttribs().
    void writeInstanceData(VertexWriter* instanceWriter, const Instance*) const;

    // Injects vertex code, fragment code, varyings, and uniforms to ultimately multiply
    // "args.fOutputCoverage" in the fragment shader by the atlas coverage.
    //
    // The caller is responsible to store "atlasAdjustUniformHandle" and pass it to
    // setUniformData().
    void injectShaderCode(const GrGeometryProcessor::ProgramImpl::EmitArgs&,
                          const GrShaderVar& devCoord,
                          GrGLSLUniformHandler::UniformHandle* atlasAdjustUniformHandle) const;

    // The atlas clip requires one uniform value -- "atlasAdjustUniform". The caller should have
    // stored this handle after its call to injectShaderCode(). This method sets its value prior to
    // drawing.
    void setUniformData(const GrGLSLProgramDataManager&,
                        const GrGLSLUniformHandler::UniformHandle& atlasAdjustUniformHandle) const;

private:
    const sk_sp<GrSurfaceProxy> fAtlasProxy;
    const GrSwizzle fAtlasSwizzle;
    const ShaderFlags fShaderFlags;
};

GR_MAKE_BITFIELD_CLASS_OPS(AtlasInstancedHelper::ShaderFlags);

} // namespace skgpu::v1

#endif // AtlasInstancedHelper_DEFINED
