/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_geom_IntersectionTree_DEFINED
#define skgpu_graphite_geom_IntersectionTree_DEFINED

#include "include/private/base/SkAlign.h"
#include "src/base/SkArenaAlloc.h"
#include "src/gpu/graphite/geom/Rect.h"

namespace skgpu::graphite {

/**
 * Maintains a collection of non-overlapping rectangles.
 *
 * add() either adds the given rect to the collection, or returns false if it intersected with a
 * rect already in the collection.
 */
class IntersectionTree {
public:
    enum class SplitType : bool {
        kX,
        kY
    };

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

    template<SplitType kSplitType> class TreeNode;
    class LeafNode;

    // The TreeNode size is made of a vtable (i.e. sizeof(void*)), float, and two Node* pointers.
    // We also align between the Node* and the float which may add some extra padding.
    constexpr static int kTreeNodeSize = SkAlignTo(sizeof(void*) + sizeof(float), alignof(void*)) +
                                         2 * sizeof(Node*);
    constexpr static int kLeafNodeSize = 16 + (2 + 64) * sizeof(Rect);
    constexpr static int kPadSize = 256;  // For footers and alignment.
    SkArenaAlloc fArena{kLeafNodeSize + kTreeNodeSize + kPadSize*2};
    Node* fRoot;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_geom_IntersectionTree_DEFINED
