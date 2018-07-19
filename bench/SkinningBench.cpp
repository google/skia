/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkCanvas.h"
#include "SkMatrix.h"
#include "SkPaint.h"
#include "SkString.h"
#include "SkVertices.h"

#include <vector>

static constexpr int kCellSize = 64;

static constexpr SkColor kColor = 0xFFFF0000;
static constexpr SkVertices::BoneIndices kBoneIndices = {{
    1, 1, 1, 1
}};
static constexpr SkVertices::BoneWeights kBoneWeights = {{
    0.25f, 0.25f, 0.25f, 0.25f
}};

class SkinningBench : public Benchmark {

public:
    SkinningBench(int tessellations, int tiles, int boneCount, bool useCPU)
            : fName("skinning")
            , fPaint()
            , fVertices(nullptr)
            , fTessellations(tessellations)
            , fTiles(tiles)
            , fBones(boneCount, SkMatrix::I())
            , fUseCPU(useCPU)
            , fPositions()
            , fVertexCount((fTessellations + 1) * (fTessellations + 1))
            , fIndexCount(fTessellations * fTessellations * 6) {
        SkASSERT(boneCount > 1);

        // Generate the tessellations.
        std::vector<SkColor> colors;
        std::vector<SkVertices::BoneIndices> boneIndices;
        std::vector<SkVertices::BoneWeights> boneWeights;
        std::vector<uint16_t> indices;
        fPositions.reserve(fVertexCount);
        colors.reserve(fVertexCount);
        indices.reserve(fIndexCount);
        for (int i = 0; i <= fTessellations; i ++) {
            int y = kCellSize * i / fTessellations;
            for (int j = 0; j <= fTessellations; j ++) {
                int x = kCellSize * j / fTessellations;

                // Position.
                fPositions.push_back(SkPoint::Make(x, y));

                // Color.
                colors.push_back(kColor);

                // Bone indices and weights.
                boneIndices.push_back(kBoneIndices);
                boneWeights.push_back(kBoneWeights);

                // Indices.
                if (i < fTessellations && j < fTessellations) {
                    indices.push_back(getIndex(j, i));
                    indices.push_back(getIndex(j, i + 1));
                    indices.push_back(getIndex(j + 1, i));
                    indices.push_back(getIndex(j + 1, i));
                    indices.push_back(getIndex(j, i + 1));
                    indices.push_back(getIndex(j + 1, i + 1));
                }
            }
        }

        // Generate the vertices object.
        fVertices = SkVertices::MakeCopy(SkVertices::kTriangles_VertexMode,
                                         fPositions.size(),
                                         fPositions.data(),
                                         nullptr,
                                         colors.data(),
                                         boneIndices.data(),
                                         boneWeights.data(),
                                         indices.size(),
                                         indices.data());

        // Generate the name.
        fName.appendf("_%d", fTessellations);
        fName.appendf("_%d", fTiles);
        fName.appendf("_%d", static_cast<int>(fBones.size()));
        if (fUseCPU) {
            fName.append("_cpu");
        }
    }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }
    const char* onGetUniqueName() override {
        return onGetName();
    }

    SkIPoint onGetSize() override {
        return SkIPoint::Make(kCellSize * fTiles, kCellSize * fTiles);
    }

    bool isSuitableFor(Backend backend) override {
        return backend != kNonRendering_Backend;
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        for (int loop = 0; loop < loops; loop ++) {
            for (int i = 0; i < fTiles; i ++) {
                for (int j = 0; j < fTiles; j ++) {
                    canvas->save();

                    canvas->translate(i * kCellSize, j * kCellSize);

                    if (fUseCPU) {
                        // Transform the vertices.
                        std::vector<SkPoint> positions(fVertexCount);
                        for (int i = 0; i < fVertexCount; i ++) {
                            // Apply deformations.
                            SkPoint& result = positions[i];
                            SkPoint transformed;
                            for (uint32_t j = 0; j < 4; j ++) {
                                // Get the bone attachment data.
                                uint32_t index = kBoneIndices.indices[j];
                                float weight = kBoneWeights.weights[j];

                                // Skip the bone is there is no weight.
                                if (weight == 0.0f) {
                                    continue;
                                }
                                SkASSERT(index != 0);

                                // transformed = M * v
                                fBones[index].mapPoints(&transformed, &fPositions[i], 1);

                                // result += transformed * w
                                result += transformed * weight;
                            }
                        }

                        // Reset the vertices object.
                        fVertices = SkVertices::MakeCopy(SkVertices::kTriangles_VertexMode,
                                                         positions.size(),
                                                         positions.data(),
                                                         nullptr,
                                                         fVertices->colors(),
                                                         fVertices->boneIndices(),
                                                         fVertices->boneWeights(),
                                                         fVertices->indexCount(),
                                                         fVertices->indices());

                        // Draw the vertices without bones.
                        canvas->drawVertices(fVertices.get(),
                                             SkBlendMode::kSrc,
                                             fPaint);
                    } else {
                        // Draw the vertices with bones.
                        canvas->drawVertices(fVertices.get(),
                                             fBones.data(),
                                             fBones.size(),
                                             SkBlendMode::kSrc,
                                             fPaint);
                    }


                    canvas->restore();
                }
            }
        }
    }

private:
    int getIndex(int x, int y) const {
        return x + y * (fTessellations + 1);
    }

private:
    SkString fName;

    SkPaint fPaint;
    sk_sp<SkVertices> fVertices;
    int fTessellations;
    int fTiles;
    std::vector<SkMatrix> fBones;
    bool fUseCPU;

    std::vector<SkPoint> fPositions;

    int fVertexCount;
    int fIndexCount;

    typedef Benchmark INHERITED;

};

///////////////////////////////////////////////////////////////////////////////

// Recursive bench defines.

// Tessellations.
#define DEF_TESSELLATIONS(tiles, bones, cpu) \
    DEF_BENCH(return new SkinningBench(1, tiles, bones, cpu);) \
    DEF_BENCH(return new SkinningBench(2, tiles, bones, cpu);) \
    DEF_BENCH(return new SkinningBench(4, tiles, bones, cpu);) \
    DEF_BENCH(return new SkinningBench(8, tiles, bones, cpu);)

// Tiles.
#define DEF_TILES(bones, cpu) \
    DEF_TESSELLATIONS(1, bones, cpu) \
    DEF_TESSELLATIONS(2, bones, cpu) \
    DEF_TESSELLATIONS(4, bones, cpu) \
    DEF_TESSELLATIONS(8, bones, cpu)

// Bones.
#define DEF_BONES(cpu) \
    DEF_TILES(4, cpu) \
    DEF_TILES(8, cpu) \
    DEF_TILES(16, cpu) \
    DEF_TILES(32, cpu) \

// CPU.
DEF_BONES(true)
DEF_BONES(false)
