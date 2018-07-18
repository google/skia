/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkVertices.h"
#include "SkPoint.h"
#include <iostream>
#include <vector>

using namespace skiagm;

static const int kCellSize = 60;
static const int kColumnSize = 36;

static const int kBoneCount = 7;
static const SkMatrix kBones[] = {
    SkMatrix::I(),
    SkMatrix::MakeTrans(10, 0),
    SkMatrix::MakeTrans(0, 10),
    SkMatrix::MakeTrans(-10, 0),
    SkMatrix::MakeTrans(0, -10),
    SkMatrix::MakeScale(0.5f),
    SkMatrix::MakeScale(1.5f),
};

static const int kVertexCount = 4;
static const SkPoint kPositions[] = {
    { 0, 0 },
    { 0, 30 },
    { 30, 30 },
    { 30, 0 },
};
static const SkColor kColors[] = {
    0xFFFF0000,
    0xFF00FF00,
    0xFF0000FF,
    0xFFFFFF00,
};
static const SkVertices::BoneIndices kBoneIndices[] = {
    {{ 1, 0, 0, 0 }},
    {{ 2, 1, 0, 0 }},
    {{ 3, 2, 1, 0 }},
    {{ 4, 3, 2, 1 }},
};
static const SkVertices::BoneWeights kBoneWeights[] = {
    {{ 1.0f,  0.0f,  0.0f,  0.0f  }},
    {{ 0.5f,  0.5f,  0.0f,  0.0f  }},
    {{ 0.34f, 0.33f, 0.33f, 0.0f  }},
    {{ 0.25f, 0.25f, 0.25f, 0.25f }},
};

static const int kIndexCount = 6;
static const uint16_t kIndices[] = {
    0, 1, 2,
    2, 3, 0,
};

// Swap two SkMatrix pointers in place.
static void swap(const SkMatrix** x, const SkMatrix** y) {
    const SkMatrix* temp = *x;
    *x = *y;
    *y = temp;
}

class SkinningGM : public GM {

public:
    SkinningGM(bool deformUsingCPU, bool cache)
            : fPaint()
            , fVertices(nullptr)
            , fDeformUsingCPU(deformUsingCPU)
            , fCache(cache)
    {}

protected:
    bool runAsBench() const override {
        return true;
    }

    SkString onShortName() override {
        SkString name("skinning");
        if (fDeformUsingCPU) {
            name.append("_cpu");
        }
        if (fCache) {
            name.append("_cached");
        }
        return name;
    }

    SkISize onISize() override {
        return SkISize::Make(2400, 2400);
    }

    void onOnceBeforeDraw() override {
        fVertices = SkVertices::MakeCopy(SkVertices::kTriangles_VertexMode,
                                         kVertexCount,
                                         kPositions,
                                         nullptr,
                                         kColors,
                                         kBoneIndices,
                                         kBoneWeights,
                                         kIndexCount,
                                         kIndices,
                                         !fCache);
    }

    void onDraw(SkCanvas* canvas) override {
        // Set the initial position.
        int xpos = kCellSize;
        int ypos = kCellSize;

        // Create the mutable set of bones.
        const SkMatrix* bones[kBoneCount];
        for (int i = 0; i < kBoneCount; i ++) {
            bones[i] = &kBones[i];
        }

        // Draw the vertices.
        drawPermutations(canvas, xpos, ypos, bones, 1);
    }

private:
    void drawPermutations(SkCanvas* canvas,
                          int& xpos,
                          int& ypos,
                          const SkMatrix** bones,
                          int start) {
        if (start == kBoneCount) {
            // Reached the end of the permutations, so draw.
            canvas->save();

            // Copy the bones.
            SkMatrix copiedBones[kBoneCount];
            for (int i = 0; i < kBoneCount; i ++) {
                copiedBones[i] = *bones[i];
            }

            // Set the position.
            canvas->translate(xpos, ypos);

            // Draw the vertices.
            if (fDeformUsingCPU) {
                // Deform with CPU.
                std::vector<SkPoint> positions(kVertexCount);
                for (int i = 0; i < kVertexCount; i ++) {
                    const SkVertices::BoneIndices& indices = kBoneIndices[i];
                    const SkVertices::BoneWeights& weights = kBoneWeights[i];

                    // Apply deformations.
                    SkPoint& result = positions[i];
                    SkPoint transformed;
                    for (uint32_t j = 0; j < 4; j ++) {
                        // Get the bone attachment data.
                        uint32_t index = indices.indices[j];
                        float weight = weights.weights[j];

                        // Skip the bone is there is no weight.
                        if (weight == 0.0f) {
                            continue;
                        }
                        SkASSERT(index != 0);

                        // transformed = M * v
                        copiedBones[index].mapPoints(&transformed, &kPositions[i], 1);

                        // result += transformed * w
                        result += transformed * weight;
                    }
                }

                sk_sp<SkVertices> vertices = SkVertices::MakeCopy(SkVertices::kTriangles_VertexMode,
                                                                  kVertexCount,
                                                                  positions.data(),
                                                                  nullptr,
                                                                  kColors,
                                                                  kIndexCount,
                                                                  kIndices,
                                                                  !fCache);
                canvas->drawVertices(vertices.get(),
                                     SkBlendMode::kSrc,
                                     fPaint);
            } else {
                // Deform with GPU.
                canvas->drawVertices(fVertices.get(),
                                     copiedBones,
                                     kBoneCount,
                                     SkBlendMode::kSrc,
                                     fPaint);
            }

            canvas->restore();

            // Get a new position to draw the vertices.
            xpos += kCellSize;
            if (xpos > kCellSize * kColumnSize) {
                xpos = kCellSize;
                ypos += kCellSize;
            }

            return;
        }

        // Find all possible permutations within the given range.
        for (int i = start; i < kBoneCount; i ++) {
            // Swap the start and i-th elements.
            swap(bones + start, bones + i);

            // Find permutations of the sub array.
            drawPermutations(canvas, xpos, ypos, bones, start + 1);

            // Swap the elements back.
            swap(bones + i, bones + start);
        }
    }

private:
    SkPaint fPaint;
    sk_sp<SkVertices> fVertices;
    bool fDeformUsingCPU;
    bool fCache;

    typedef GM INHERITED;
};

/////////////////////////////////////////////////////////////////////////////////////

DEF_GM(return new SkinningGM(true, true);)
DEF_GM(return new SkinningGM(false, true);)
DEF_GM(return new SkinningGM(true, false);)
DEF_GM(return new SkinningGM(false, false);)
