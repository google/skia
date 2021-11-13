/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef tessellate_PatchWriter_DEFINED
#define tessellate_PatchWriter_DEFINED

#include "src/gpu/GrVertexChunkArray.h"
#include "src/gpu/tessellate/MiddleOutPolygonTriangulator.h"
#include "src/gpu/tessellate/Tessellation.h"
#include "src/gpu/tessellate/shaders/GrTessellationShader.h"

namespace skgpu {

#if SK_GPU_V1
class PathTessellator;
#endif

// Writes out tessellation patches, formatted with their specific attribs, to a GPU buffer.
class PatchWriter {
public:
    PatchWriter(GrMeshDrawTarget* target,
                GrVertexChunkArray* vertexChunkArray,
                PatchAttribs attribs,
                size_t patchStride,
                int initialAllocCount)
            : fPatchAttribs(attribs)
            , fChunker(target, vertexChunkArray, patchStride, initialAllocCount) {
    }

#if SK_GPU_V1
    // Creates a PatchWriter that writes directly to the GrVertexChunkArray stored on the provided
    // PathTessellator.
    PatchWriter(GrMeshDrawTarget*, PathTessellator* tessellator, int initialPatchAllocCount);
#endif

    // Updates the fan point that will be written out with each patch (i.e., the point that wedges
    // fan around).
    // PathPatchAttrib::kFanPoint must be enabled.
    void updateFanPointAttrib(SkPoint fanPoint) {
        SkASSERT(fPatchAttribs & PatchAttribs::kFanPoint);
        fFanPointAttrib = fanPoint;
    }

    // Updates the color that will be written out with each patch.
    // PathPatchAttrib::kColor must be enabled.
    void updateColorAttrib(const SkPMColor4f& color) {
        SkASSERT(fPatchAttribs & PatchAttribs::kColor);
        fColorAttrib.set(color, fPatchAttribs & PatchAttribs::kWideColorIfEnabled);
    }

    // RAII. Appends a patch during construction and writes the remaining data for a cubic during
    // destruction. The caller outputs p0,p1,p2,p3 (8 floats):
    //
    //    CubicPatch(patchWriter) << p0 << p1 << p2 << p3;
    //
    struct CubicPatch {
        CubicPatch(PatchWriter& w) : fPatchWriter(w), fVertexWriter(w.appendPatch()) {}
        ~CubicPatch() {
            fPatchWriter.outputPatchAttribs(std::move(fVertexWriter),
                                            GrTessellationShader::kCubicCurveType);
        }
        operator VertexWriter&() { return fVertexWriter; }
        PatchWriter& fPatchWriter;
        VertexWriter fVertexWriter;
    };

    // RAII. Appends a patch during construction and writes the remaining data for a conic during
    // destruction. The caller outputs p0,p1,p2,w (7 floats):
    //
    //     ConicPatch(patchWriter) << p0 << p1 << p2 << w;
    //
    struct ConicPatch {
        ConicPatch(PatchWriter& w) : fPatchWriter(w), fVertexWriter(w.appendPatch()) {}
        ~ConicPatch() {
            fVertexWriter << VertexWriter::kIEEE_32_infinity;  // p3.y=Inf indicates a conic.
            fPatchWriter.outputPatchAttribs(std::move(fVertexWriter),
                                            GrTessellationShader::kConicCurveType);
        }
        operator VertexWriter&() { return fVertexWriter; }
        PatchWriter& fPatchWriter;
        VertexWriter fVertexWriter;
    };

    // RAII. Appends a patch during construction and writes the remaining data for a triangle during
    // destruction. The caller outputs p0,p1,p2 (6 floats):
    //
    //     TrianglePatch(patchWriter) << p0 << p1 << p2;
    //
    struct TrianglePatch {
        TrianglePatch(PatchWriter& w) : fPatchWriter(w), fVertexWriter(w.appendPatch()) {}
        ~TrianglePatch() {
            // Mark this patch as a triangle by setting it to a conic with w=Inf.
            fVertexWriter.fill(VertexWriter::kIEEE_32_infinity, 2);
            fPatchWriter.outputPatchAttribs(std::move(fVertexWriter),
                                            GrTessellationShader::kTriangularConicCurveType);
        }
        operator VertexWriter&() { return fVertexWriter; }
        PatchWriter& fPatchWriter;
        VertexWriter fVertexWriter;
    };

    // Chops the given quadratic into 'numPatches' equal segments (in the parametric sense) and
    // writes them to the GPU buffer.
    //
    // Fills space between chops with triangles if PathPatchAttrib::kFanPoint is not enabled.
    void chopAndWriteQuads(float2 p0, float2 p1, float2 p2, int numPatches);

    // Chops the given conic into 'numPatches' equal segments (in the parametric sense) and
    // writes them to the GPU buffer.
    //
    // Fills space between chops with triangles if PathPatchAttrib::kFanPoint is not enabled.
    void chopAndWriteConics(float2 p0, float2 p1, float2 p2, float w, int numPatches);

    // Chops the given cubic into 'numPatches' equal segments (in the parametric sense) and
    // writes them to the GPU buffer.
    //
    // Fills space between chops with triangles if PathPatchAttrib::kFanPoint is not enabled.
    void chopAndWriteCubics(float2 p0, float2 p1, float2 p2, float2 p3, int numPatches);

private:
    VertexWriter appendPatch() {
        VertexWriter vertexWriter = fChunker.appendVertex();
        if (!vertexWriter) {
            // Failed to allocate GPU storage for the patch. Write to a throwaway location so the
            // callsites don't have to do null checks.
            if (!fFallbackPatchStorage) {
                fFallbackPatchStorage.reset(fChunker.stride());
            }
            vertexWriter = fFallbackPatchStorage.data();
        }
        return vertexWriter;
    }

    template <typename T>
    static VertexWriter::Conditional<T> If(bool c, const T& v) { return VertexWriter::If(c,v); }

    void outputPatchAttribs(VertexWriter vertexWriter, float explicitCurveType) {
        vertexWriter << If((fPatchAttribs & PatchAttribs::kFanPoint), fFanPointAttrib)
                     << If((fPatchAttribs & PatchAttribs::kColor), fColorAttrib)
                     << If((fPatchAttribs & PatchAttribs::kExplicitCurveType), explicitCurveType);
    }

    const PatchAttribs fPatchAttribs;
    SkPoint fFanPointAttrib;
    GrVertexColor fColorAttrib;

    GrVertexChunkBuilder fChunker;

    // For when fChunker fails to allocate a patch in GPU memory.
    SkAutoTMalloc<char> fFallbackPatchStorage;
};

// Converts a line to a cubic when being output via '<<' to a VertexWriter.
struct LineToCubic {
    float4 fP0P1;
};

SK_MAYBE_UNUSED SK_ALWAYS_INLINE VertexWriter& operator<<(VertexWriter& vertexWriter,
                                                          const LineToCubic& line) {
    float4 p0p1 = line.fP0P1;
    float4 v = p0p1.zwxy() - p0p1;
    return vertexWriter << p0p1.lo << (v * (1/3.f) + p0p1) << p0p1.hi;
}

// Converts a quadratic to a cubic when being output via '<<' to a VertexWriter.
struct QuadToCubic {
    float2 fP0, fP1, fP2;
};

SK_MAYBE_UNUSED SK_ALWAYS_INLINE VertexWriter& operator<<(VertexWriter& vertexWriter,
                                                          const QuadToCubic& quadratic) {
    auto [p0, p1, p2] = quadratic;
    return vertexWriter << p0 << mix(float4(p0,p2), p1.xyxy(), 2/3.f) << p2;
}

SK_MAYBE_UNUSED SK_ALWAYS_INLINE void operator<<(
        PatchWriter& w, MiddleOutPolygonTriangulator::PoppedTriangleStack&& stack) {
    for (auto [p0, p1, p2] : stack) {
        PatchWriter::TrianglePatch(w) << p0 << p1 << p2;
    }
}

}  // namespace skgpu

#endif  // tessellate_PatchWriter_DEFINED
