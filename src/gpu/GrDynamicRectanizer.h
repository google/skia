/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDynamicRectanizer_DEFINED
#define GrDynamicRectanizer_DEFINED

#include "include/core/SkSize.h"
#include "src/core/SkArenaAlloc.h"

struct SkIPoint16;
struct SkIRect;

/**
 * This class implements a dynamic size GrRectanizer that grows until it reaches the implementation-
 * dependent max texture size.
 */
class GrDynamicRectanizer {
public:
    inline static constexpr int kPadding = 1;  // Amount of padding below and to the right of each
                                               // path.

    enum class RectanizerAlgorithm { kSkyline, kPow2 };

    GrDynamicRectanizer(SkISize initialSize,
                        int maxAtlasSize,
                        RectanizerAlgorithm algorithm = RectanizerAlgorithm::kSkyline)
            : fMaxAtlasSize(maxAtlasSize), fRectanizerAlgorithm(algorithm) {
        this->reset(initialSize);
    }

    void reset(SkISize initialSize);

    int maxAtlasSize() const { return fMaxAtlasSize; }
    int width() const { return fWidth; }
    int height() const { return fHeight; }
    const SkISize& drawBounds() const { return fDrawBounds; }

    // Attempts to add a rect to the atlas. Returns true if successful, along with the rect's
    // top-left location in the atlas.
    bool addRect(int width, int height, SkIPoint16* location);

private:
    class Node;

    Node* makeNode(Node* previous, int l, int t, int r, int b);
    bool internalPlaceRect(int w, int h, SkIPoint16* loc);

    const int fMaxAtlasSize;
    const RectanizerAlgorithm fRectanizerAlgorithm;
    int fWidth;
    int fHeight;
    SkISize fDrawBounds;

    SkSTArenaAllocWithReset<512> fNodeAllocator;
    Node* fTopNode = nullptr;
};

#endif
