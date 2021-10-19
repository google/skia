/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_geom_IntersectionTree_DEFINED
#define skgpu_geom_IntersectionTree_DEFINED

#include "experimental/graphite/src/geom/Rect.h"
#include "src/core/SkArenaAlloc.h"

namespace skgpu {

// Maintains a collection of non-overlapping rectangles.
//
// add() either adds the given rect to the collection, or returns false if it intersected with a
// rect already in the collection.
class IntersectionTree {
public:
    IntersectionTree();

    bool add(Rect rect) {
        if (rect.isEmptyNegativeOrNaN()) {
            // Empty and undefined rects can simply pass without modifying the tree.
            return true;
        }
        if (!fRoot->intersects(rect)) {
            fRoot = fRoot->addNonIntersecting(rect, &fArena);
            return true;
        }
        return false;
    }

private:
    class Node {
    public:
        virtual ~Node() = default;

        virtual bool intersects(Rect) = 0;
        virtual Node* addNonIntersecting(Rect, SkArenaAlloc*) = 0;
    };

    enum class SplitType : bool {
        kX,
        kY
    };

    template<SplitType kSplitType> class TreeNode;
    class LeafNode;

    constexpr static int kTreeNodeSize = 16 + sizeof(Node*) * 2;
    constexpr static int kLeafNodeSize = 16 + (2 + 64) * sizeof(float4);
    constexpr static int kPadSize = 256;  // For footers and alignment.
    SkArenaAlloc fArena{kLeafNodeSize + kTreeNodeSize + kPadSize*2};
    Node* fRoot;
};


} // namespace skgpu

#endif // skgpu_geom_IntersectionTree_DEFINED
