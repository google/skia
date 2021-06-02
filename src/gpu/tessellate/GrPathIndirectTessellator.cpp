/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/GrPathIndirectTessellator.h"

#include "src/gpu/GrEagerVertexAllocator.h"
#include "src/gpu/geometry/GrPathUtils.h"
#include "src/gpu/geometry/GrWangsFormula.h"
#include "src/gpu/tessellate/GrMiddleOutPolygonTriangulator.h"
#include "src/gpu/tessellate/shaders/GrPathTessellationShader.h"

constexpr static float kPrecision = GrTessellationPathRenderer::kLinearizationPrecision;

GrPathTessellator* GrPathIndirectTessellator::Make(SkArenaAlloc* arena, const SkPath& path,
                                                   const SkMatrix& viewMatrix,
                                                   const SkPMColor4f& color,
                                                   DrawInnerFan drawInnerFan) {
    auto shader = GrPathTessellationShader::MakeMiddleOutInstancedShader(arena, viewMatrix, color);
    return arena->make<GrPathIndirectTessellator>(shader, path, drawInnerFan);
}

GrPathIndirectTessellator::GrPathIndirectTessellator(GrPathTessellationShader* shader,
                                                     const SkPath& path, DrawInnerFan drawInnerFan)
        : GrPathTessellator(shader)
        , fDrawInnerFan(drawInnerFan != DrawInnerFan::kNo) {
    // Count the number of instances at each resolveLevel.
    GrVectorXform xform(fShader->viewMatrix());
    for (auto [verb, pts, w] : SkPathPriv::Iterate(path)) {
        int level;
        switch (verb) {
            case SkPathVerb::kQuad:
                level = GrWangsFormula::quadratic_log2(kPrecision, pts, xform);
                break;
            case SkPathVerb::kConic:
                level = GrWangsFormula::conic_log2(kPrecision, pts, *w, xform);
                break;
            case SkPathVerb::kCubic:
                level = GrWangsFormula::cubic_log2(kPrecision, pts, xform);
                break;
            default:
                continue;
        }
        SkASSERT(level >= 0);
        // Instances with 2^0=1 segments are empty (zero area). We ignore them completely.
        if (level > 0) {
            level = std::min(level, kMaxResolveLevel);
            ++fResolveLevelCounts[level];
            ++fOuterCurveInstanceCount;
        }
    }
}

// How many vertices do we need to draw in order to triangulate a curve with 2^resolveLevel line
// segments?
constexpr static int num_vertices_at_resolve_level(int resolveLevel) {
    // resolveLevel=0 -> 0 line segments -> 0 triangles -> 0 vertices
    // resolveLevel=1 -> 2 line segments -> 1 triangle -> 3 vertices
    // resolveLevel=2 -> 4 line segments -> 3 triangles -> 9 vertices
    // resolveLevel=3 -> 8 line segments -> 7 triangles -> 21 vertices
    // ...
    return ((1 << resolveLevel) - 1) * 3;
}

void GrPathIndirectTessellator::prepare(GrMeshDrawOp::Target* target, const SkRect& /*cullBounds*/,
                                        const SkPath& path,
                                        const BreadcrumbTriangleList* breadcrumbTriangleList) {
    SkASSERT(fTotalInstanceCount == 0);
    SkASSERT(fIndirectDrawCount == 0);
    SkASSERT(target->caps().drawInstancedSupport());

    int instanceLockCount = fOuterCurveInstanceCount;
    if (fDrawInnerFan) {
        instanceLockCount += GrPathTessellator::MaxTrianglesInInnerFan(path);
    }
    if (breadcrumbTriangleList) {
        instanceLockCount += breadcrumbTriangleList->count();
    }
    if (instanceLockCount == 0) {
        return;
    }

    // Allocate a buffer to store the instance data.
    GrEagerDynamicVertexAllocator vertexAlloc(target, &fInstanceBuffer, &fBaseInstance);
    GrVertexWriter instanceWriter = static_cast<SkPoint*>(vertexAlloc.lock(sizeof(SkPoint) * 4,
                                                                           instanceLockCount));
    if (!instanceWriter) {
        return;
    }

    // Write out any triangles at the beginning of the cubic data. Since this shader draws curves,
    // output the triangles as conics with w=infinity (which is equivalent to a triangle).
    int numTrianglesAtBeginningOfData = 0;
    if (fDrawInnerFan) {
        numTrianglesAtBeginningOfData = GrMiddleOutPolygonTriangulator::WritePathInnerFan(
                &instanceWriter,
                GrMiddleOutPolygonTriangulator::OutputType::kConicsWithInfiniteWeight, path);
    }
    if (breadcrumbTriangleList) {
        numTrianglesAtBeginningOfData += GrPathTessellator::WriteBreadcrumbTriangles(
                &instanceWriter, breadcrumbTriangleList);
    }

    // Allocate space for the GrDrawIndexedIndirectCommand structs. Allocate enough for each
    // possible resolve level (kMaxResolveLevel; resolveLevel=0 never has any instances), plus one
    // more for the optional inner fan triangles.
    int indirectLockCnt = kMaxResolveLevel + 1;
    GrDrawIndirectWriter indirectWriter = target->makeDrawIndirectSpace(indirectLockCnt,
                                                                        &fIndirectDrawBuffer,
                                                                        &fIndirectDrawOffset);
    if (!indirectWriter) {
        SkASSERT(!fIndirectDrawBuffer);
        vertexAlloc.unlock(0);
        return;
    }

    // Fill out the GrDrawIndexedIndirectCommand structs and determine the starting instance data
    // location at each resolve level.
    GrVertexWriter instanceLocations[kMaxResolveLevel + 1];
    int currentBaseInstance = fBaseInstance;
    SkASSERT(fResolveLevelCounts[0] == 0);
    for (int resolveLevel=1, numExtraInstances=numTrianglesAtBeginningOfData;
         resolveLevel <= kMaxResolveLevel;
         ++resolveLevel, numExtraInstances=0) {
        int instanceCountAtCurrLevel = fResolveLevelCounts[resolveLevel];
        if (!(instanceCountAtCurrLevel + numExtraInstances)) {
            SkDEBUGCODE(instanceLocations[resolveLevel] = nullptr;)
            continue;
        }
        instanceLocations[resolveLevel] = instanceWriter.makeOffset(0);
        SkASSERT(fIndirectDrawCount < indirectLockCnt);
        // The vertex shader determines the T value at which to draw each vertex. Since the
        // triangles are arranged in "middle-out" order, we can conveniently control the
        // resolveLevel by changing only the vertexCount.
        indirectWriter.write(instanceCountAtCurrLevel + numExtraInstances, currentBaseInstance,
                             num_vertices_at_resolve_level(resolveLevel), 0);
        ++fIndirectDrawCount;
        currentBaseInstance += instanceCountAtCurrLevel + numExtraInstances;
        instanceWriter = instanceWriter.makeOffset(instanceCountAtCurrLevel * 4 * sizeof(SkPoint));
    }

    target->putBackIndirectDraws(indirectLockCnt - fIndirectDrawCount);

#ifdef SK_DEBUG
    SkASSERT(currentBaseInstance ==
             fBaseInstance + numTrianglesAtBeginningOfData + fOuterCurveInstanceCount);

    GrVertexWriter endLocations[kMaxResolveLevel + 1];
    int lastResolveLevel = 0;
    for (int resolveLevel = 1; resolveLevel <= kMaxResolveLevel; ++resolveLevel) {
        if (!instanceLocations[resolveLevel]) {
            endLocations[resolveLevel] = nullptr;
            continue;
        }
        endLocations[lastResolveLevel] = instanceLocations[resolveLevel].makeOffset(0);
        lastResolveLevel = resolveLevel;
    }
    endLocations[lastResolveLevel] = instanceWriter.makeOffset(0);
#endif

    fTotalInstanceCount = numTrianglesAtBeginningOfData;

    // Write out the cubic instances.
    if (fOuterCurveInstanceCount) {
        GrVectorXform xform(fShader->viewMatrix());
        for (auto [verb, pts, w] : SkPathPriv::Iterate(path)) {
            int level;
            switch (verb) {
                default:
                    continue;
                case SkPathVerb::kQuad:
                    level = GrWangsFormula::quadratic_log2(kPrecision, pts, xform);
                    break;
                case SkPathVerb::kConic:
                    level = GrWangsFormula::conic_log2(kPrecision, pts, *w, xform);
                    break;
                case SkPathVerb::kCubic:
                    level = GrWangsFormula::cubic_log2(kPrecision, pts, xform);
                    break;
            }
            if (level == 0) {
                continue;
            }
            level = std::min(level, kMaxResolveLevel);
            switch (verb) {
                case SkPathVerb::kQuad:
                    GrPathUtils::writeQuadAsCubic(pts, &instanceLocations[level]);
                    break;
                case SkPathVerb::kCubic:
                    instanceLocations[level].writeArray(pts, 4);
                    break;
                case SkPathVerb::kConic:
                    GrTessellationShader::WriteConicPatch(pts, *w, &instanceLocations[level]);
                    break;
                default:
                    SkUNREACHABLE;
            }
            ++fTotalInstanceCount;
        }
    }

#ifdef SK_DEBUG
    for (int i = 1; i <= kMaxResolveLevel; ++i) {
        SkASSERT(instanceLocations[i] == endLocations[i]);
    }
    SkASSERT(fTotalInstanceCount == numTrianglesAtBeginningOfData + fOuterCurveInstanceCount);
#endif

    vertexAlloc.unlock(fTotalInstanceCount);
}

void GrPathIndirectTessellator::draw(GrOpFlushState* flushState) const {
    if (fIndirectDrawCount) {
        flushState->bindBuffers(nullptr, fInstanceBuffer, nullptr);
        flushState->drawIndirect(fIndirectDrawBuffer.get(), fIndirectDrawOffset,
                                 fIndirectDrawCount);
    }
}

void GrPathIndirectTessellator::drawHullInstances(GrOpFlushState* flushState) const {
    if (fTotalInstanceCount) {
        flushState->bindBuffers(nullptr, fInstanceBuffer, nullptr);
        flushState->drawInstanced(fTotalInstanceCount, fBaseInstance, 4, 0);
    }
}
