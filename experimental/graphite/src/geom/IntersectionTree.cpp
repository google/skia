/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/geom/IntersectionTree.h"

#include "include/private/SkTPin.h"
#include <algorithm>
#include <limits>

namespace skgpu {

// BSP node. Space is partitioned by an either vertical or horizontal line. Note that if a rect
// straddles the partition line, it will need to go on both sides of the tree.
template<IntersectionTree::SplitType kSplitType>
class IntersectionTree::TreeNode final : public Node {
public:
    TreeNode(float splitCoord, Node* lo, Node* hi)
            : fSplitCoord(splitCoord), fLo(lo), fHi(hi) {
    }

    bool intersects(Rect rect) override {
        if (GetLoVal(rect) < fSplitCoord && fLo->intersects(rect)) {
            return true;
        }
        if (GetHiVal(rect) > fSplitCoord && fHi->intersects(rect)) {
            return true;
        }
        return false;
    }

    Node* addNonIntersecting(Rect rect, SkArenaAlloc* arena) override {
        if (GetLoVal(rect) < fSplitCoord) {
            fLo = fLo->addNonIntersecting(rect, arena);
        }
        if (GetHiVal(rect) > fSplitCoord) {
            fHi = fHi->addNonIntersecting(rect, arena);
        }
        return this;
    }

private:
    SK_ALWAYS_INLINE static float GetLoVal(const Rect& rect) {
        return (kSplitType == SplitType::kX) ? rect.left() : rect.top();
    }
    SK_ALWAYS_INLINE static float GetHiVal(const Rect& rect) {
        return (kSplitType == SplitType::kX) ? rect.right() : rect.bot();
    }

    float fSplitCoord;
    Node* fLo;
    Node* fHi;
};

// Leaf node. Rects are kept in a simple list and intersection testing is performed by brute force.
class IntersectionTree::LeafNode final : public Node {
public:
    // Max number of rects to store in this node before splitting. With SSE/NEON optimizations, ~64
    // brute force rect comparisons seems to be the optimal number.
    constexpr static int kMaxRectsInList = 64;

    LeafNode() {
        this->popAll();
        // Initialize our arrays with maximally negative rects. These have the advantage of always
        // failing intersection tests, thus allowing us to test for intersection beyond fNumRects
        // without failing.
        constexpr static float infinity = std::numeric_limits<float>::infinity();
        std::fill_n(fLefts, kMaxRectsInList, infinity);
        std::fill_n(fTops, kMaxRectsInList, infinity);
        std::fill_n(fNegRights, kMaxRectsInList, infinity);
        std::fill_n(fNegBots, kMaxRectsInList, infinity);
    }

    void popAll() {
        fNumRects = 0;
        fSplittableBounds = -std::numeric_limits<float>::infinity();
        fRectValsSum = 0;
        // Leave the rect arrays untouched. Since we know they are either already valid in the tree,
        // or else maximally negative, this allows the future list to check for intersection beyond
        // fNumRects without failing.
    }

    bool intersects(Rect rect) override {
        // Test for intersection in sets of 4. Since all the data in our rect arrays is either
        // maximally negative, or valid from somewhere else in the tree, we can test beyond
        // fNumRects without failing.
        static_assert(kMaxRectsInList % 4 == 0);
        SkASSERT(fNumRects <= kMaxRectsInList);
        float4 comp = Rect::ComplementRect(rect).fVals;
        for (int i = 0; i < fNumRects; i += 4) {
            float4 l = float4::Load(fLefts + i);
            float4 t = float4::Load(fTops + i);
            float4 nr = float4::Load(fNegRights + i);
            float4 nb = float4::Load(fNegBots + i);
            if (any((l < comp[0]) &
                    (t < comp[1]) &
                    (nr < comp[2]) &
                    (nb < comp[3]))) {
                return true;
            }
        }
        return false;
    }

    Node* addNonIntersecting(Rect rect, SkArenaAlloc* arena) override {
        if (fNumRects == kMaxRectsInList) {
            // The new rect doesn't fit. Split our rect list first and then add.
            return this->split(arena)->addNonIntersecting(rect, arena);
        }
        this->appendToList(rect);
        return this;
    }

private:
    void appendToList(Rect rect) {
        SkASSERT(fNumRects < kMaxRectsInList);
        int i = fNumRects++;
        // [maxLeft, maxTop, -minRight, -minBot]
        fSplittableBounds = max(fSplittableBounds, rect.vals());
        fRectValsSum += rect.vals();  // [sum(left), sum(top), -sum(right), -sum(bot)]
        fLefts[i] = rect.vals()[0];
        fTops[i] = rect.vals()[1];
        fNegRights[i] = rect.vals()[2];
        fNegBots[i] = rect.vals()[3];
    }

    Rect loadRect(int i) const {
        return Rect::FromVals(float4(fLefts[i], fTops[i], fNegRights[i], fNegBots[i]));
    }

    // Splits this node with a new LeafNode, then returns a TreeNode that reuses our "this" pointer
    // along with the new node.
    IntersectionTree::Node* split(SkArenaAlloc* arena) {
        // This should only get called when our list is full.
        SkASSERT(fNumRects == kMaxRectsInList);

        // Since rects cannot overlap, there will always be a split that places at least one pairing
        // of rects on opposite sides. The region:
        //
        //     fSplittableBounds == [maxLeft, maxTop, -minRight, -minBot] == [r, b, -l, -t]
        //
        // Represents the region of splits that guarantee a strict subdivision of our rect list.
        float2 splittableSize = fSplittableBounds.xy() + fSplittableBounds.zw();  // == [r-l, b-t]
        SkASSERT(max(splittableSize) >= 0);
        SplitType splitType = (splittableSize.x() > splittableSize.y()) ? SplitType::kX
                                                                        : SplitType::kY;

        float splitCoord;
        const float *loVals, *negHiVals;
        if (splitType == SplitType::kX) {
            // Split horizontally, at the geometric midpoint if it falls within the splittable
            // bounds.
            splitCoord = (fRectValsSum.x() - fRectValsSum.z()) * (.5f/kMaxRectsInList);
            splitCoord = SkTPin(splitCoord, -fSplittableBounds.z(), fSplittableBounds.x());
            loVals = fLefts;
            negHiVals = fNegRights;
        } else {
            // Split vertically, at the geometric midpoint if it falls within the splittable bounds.
            splitCoord = (fRectValsSum.y() - fRectValsSum.w()) * (.5f/kMaxRectsInList);
            splitCoord = SkTPin(splitCoord, -fSplittableBounds.w(), fSplittableBounds.y());
            loVals = fTops;
            negHiVals = fNegBots;
        }

        // Split "this", leaving all rects below "splitCoord" in this, and placing all rects above
        // splitCoord in "hiNode". There may be some reduncancy between lists, but we made sure to
        // select a split that would leave both lists strictly smaller than the original.
        LeafNode* hiNode = arena->make<LeafNode>();
        int numCombinedRects = fNumRects;
        float negSplitCoord = -splitCoord;
        this->popAll();
        for (int i = 0; i < numCombinedRects; ++i) {
            Rect rect = this->loadRect(i);
            if (loVals[i] < splitCoord) {
                this->appendToList(rect);
            }
            if (negHiVals[i] < negSplitCoord) {
                hiNode->appendToList(rect);
            }
        }

        SkASSERT(0 < fNumRects && fNumRects < numCombinedRects);
        SkASSERT(0 < hiNode->fNumRects && hiNode->fNumRects < numCombinedRects);

        return (splitType == SplitType::kX)
                ? (Node*)arena->make<TreeNode<SplitType::kX>>(splitCoord, this, hiNode)
                : (Node*)arena->make<TreeNode<SplitType::kY>>(splitCoord, this, hiNode);
    }

    int fNumRects;
    float4 fSplittableBounds;  // [maxLeft, maxTop, -minRight, -minBot]
    float4 fRectValsSum;  // [sum(left), sum(top), -sum(right), -sum(bot)]
    alignas(float4) float fLefts[kMaxRectsInList];
    alignas(float4) float fTops[kMaxRectsInList];
    alignas(float4) float fNegRights[kMaxRectsInList];
    alignas(float4) float fNegBots[kMaxRectsInList];
    static_assert((kMaxRectsInList * sizeof(float)) % sizeof(float4) == 0);
};

IntersectionTree::IntersectionTree()
        : fRoot(fArena.make<LeafNode>()) {
    static_assert(kTreeNodeSize == sizeof(TreeNode<SplitType::kX>));
    static_assert(kTreeNodeSize == sizeof(TreeNode<SplitType::kY>));
    static_assert(kLeafNodeSize == sizeof(LeafNode));
}

} // namespace skgpu
