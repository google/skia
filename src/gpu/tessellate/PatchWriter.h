/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef tessellate_PatchWriter_DEFINED
#define tessellate_PatchWriter_DEFINED

#include "include/private/SkColorData.h"
#include "src/gpu/GrVertexChunkArray.h"
#include "src/gpu/tessellate/MiddleOutPolygonTriangulator.h"
#include "src/gpu/tessellate/Tessellation.h"

namespace skgpu {

#if SK_GPU_V1
class PathTessellator;
class StrokeTessellator;
#endif

// Writes out tessellation patches, formatted with their specific attribs, to a GPU buffer.
class PatchWriter {
public:
    PatchWriter(GrMeshDrawTarget* target,
                GrVertexChunkArray* vertexChunkArray,
                PatchAttribs attribs,
                size_t patchStride,
                int initialAllocCount)
            : fAttribs(attribs)
            , fChunker(target, vertexChunkArray, patchStride, initialAllocCount) {
    }

#if SK_GPU_V1
    // Create PatchWriters that write directly to the GrVertexChunkArrays stored on the provided
    // tessellators.
    PatchWriter(GrMeshDrawTarget*, PathTessellator*, int initialPatchAllocCount);
    PatchWriter(GrMeshDrawTarget*, StrokeTessellator*, int initialPatchAllocCount);
#endif

    PatchAttribs attribs() const { return fAttribs; }

    // Updates the fan point that will be written out with each patch (i.e., the point that wedges
    // fan around).
    // PathPatchAttrib::kFanPoint must be enabled.
    void updateFanPointAttrib(SkPoint fanPoint) {
        SkASSERT(fAttribs & PatchAttribs::kFanPoint);
        fFanPointAttrib = fanPoint;
    }

    // Updates the stroke params that are written out with each patch.
    // PathPatchAttrib::kStrokeParams must be enabled.
    void updateStrokeParamsAttrib(StrokeParams strokeParams) {
        SkASSERT(fAttribs & PatchAttribs::kStrokeParams);
        fStrokeParamsAttrib = strokeParams;
    }

    // Updates the color that will be written out with each patch.
    // PathPatchAttrib::kColor must be enabled.
    void updateColorAttrib(const SkPMColor4f& color) {
        SkASSERT(fAttribs & PatchAttribs::kColor);
        fColorAttrib.set(color, fAttribs & PatchAttribs::kWideColorIfEnabled);
    }

    // RAII. Appends a patch during construction and writes the attribs during destruction.
    //
    //    Patch(patchWriter, explicitCurveType) << p0 << p1 << ...;
    //
    struct Patch {
        Patch(PatchWriter& w, float explicitCurveType)
                : fPatchWriter(w)
                , fVertexWriter(w.appendPatch())
                , fExplicitCurveType(explicitCurveType) {}
        ~Patch() {
            fPatchWriter.emitPatchAttribs(std::move(fVertexWriter), fExplicitCurveType);
        }
        operator VertexWriter&() { return fVertexWriter; }
        PatchWriter& fPatchWriter;
        VertexWriter fVertexWriter;
        const float fExplicitCurveType;
    };

    // RAII. Appends a patch during construction and writes the remaining data for a cubic during
    // destruction. The caller outputs p0,p1,p2,p3 (8 floats):
    //
    //    CubicPatch(patchWriter) << p0 << p1 << p2 << p3;
    //
    struct CubicPatch : public Patch {
        CubicPatch(PatchWriter& w) : Patch(w, kCubicCurveType) {}
    };

    // RAII. Appends a patch during construction and writes the remaining data for a conic during
    // destruction. The caller outputs p0,p1,p2,w (7 floats):
    //
    //     ConicPatch(patchWriter) << p0 << p1 << p2 << w;
    //
    struct ConicPatch : public Patch {
        ConicPatch(PatchWriter& w) : Patch(w, kConicCurveType) {}
        ~ConicPatch() {
            fVertexWriter << VertexWriter::kIEEE_32_infinity;  // p3.y=Inf indicates a conic.
        }
    };

    // RAII. Appends a patch during construction and writes the remaining data for a triangle during
    // destruction. The caller outputs p0,p1,p2 (6 floats):
    //
    //     TrianglePatch(patchWriter) << p0 << p1 << p2;
    //
    struct TrianglePatch : public Patch {
        TrianglePatch(PatchWriter& w) : Patch(w, kTriangularConicCurveType) {}
        ~TrianglePatch() {
            // Mark this patch as a triangle by setting it to a conic with w=Inf.
            fVertexWriter << VertexWriter::Repeat<2>(VertexWriter::kIEEE_32_infinity);
        }
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
            vertexWriter = {fFallbackPatchStorage.data(), fChunker.stride()};
        }
        return vertexWriter;
    }

    template <typename T>
    static VertexWriter::Conditional<T> If(bool c, const T& v) { return VertexWriter::If(c,v); }

    void emitPatchAttribs(VertexWriter vertexWriter, float explicitCurveType) {
        vertexWriter << If((fAttribs & PatchAttribs::kFanPoint), fFanPointAttrib)
                     << If((fAttribs & PatchAttribs::kStrokeParams), fStrokeParamsAttrib)
                     << If((fAttribs & PatchAttribs::kColor), fColorAttrib)
                     << If((fAttribs & PatchAttribs::kExplicitCurveType), explicitCurveType);
    }

    const PatchAttribs fAttribs;
    SkPoint fFanPointAttrib;
    StrokeParams fStrokeParamsAttrib;
    VertexColor fColorAttrib;

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
    QuadToCubic(float2 p0, float2 p1, float2 p2) : fP0(p0), fP1(p1), fP2(p2) {}
    QuadToCubic(const SkPoint p[3])
            : QuadToCubic(float2::Load(p), float2::Load(p+1), float2::Load(p+2)) {}
    float2 fP0, fP1, fP2;
};

SK_MAYBE_UNUSED SK_ALWAYS_INLINE VertexWriter& operator<<(VertexWriter& vertexWriter,
                                                          const QuadToCubic& quadratic) {
    auto [p0, p1, p2] = quadratic;
    return vertexWriter << p0 << mix(float4(p0,p2), p1.xyxy(), 2/3.f) << p2;
}

SK_MAYBE_UNUSED SK_ALWAYS_INLINE VertexWriter& operator<<(VertexWriter&& vertexWriter,
                                                          const QuadToCubic& quadratic) {
    return vertexWriter << quadratic;
}

SK_MAYBE_UNUSED SK_ALWAYS_INLINE void operator<<(
        PatchWriter& w, MiddleOutPolygonTriangulator::PoppedTriangleStack&& stack) {
    for (auto [p0, p1, p2] : stack) {
        PatchWriter::TrianglePatch(w) << p0 << p1 << p2;
    }
}

}  // namespace skgpu

#endif  // tessellate_PatchWriter_DEFINED
