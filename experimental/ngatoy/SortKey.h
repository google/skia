// Copyright 2021 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#ifndef SortKey_DEFINED
#define SortKey_DEFINED

#include "include/core/SkTypes.h"

// These are the material IDs that are stored in the sort key for each class of material
constexpr int kInvalidMat = 0;
constexpr int kSolidMat   = 1;
constexpr int kLinearMat  = 2;
constexpr int kRadialMat  = 3;

class SortKey {
public:
    // Opaque Order
    // field:  |  transparent (false) | material | sortZ       | draw order |
    // bits:   |      1               |  4       |     4       |      4     |
    //
    // Transparent Order
    // field:  |  transparent (true)  | sortZ    | draw order  |  material  |
    // bits:   |      1               |  4       |     4       |      4     |
    // Note:
    // For opaque draws, the material dominates to improve material-based batching and the
    // 'draw order' is inverted to yield front-to-back drawing.
    // For transparent draws, material-based batching is less important so the material comes
    // last. Additionally, the 'draw order' is setup to yield back-to-front drawing.

    static const uint32_t kNumMaterialBits = 4;
    static const uint32_t kMaterialMask = (0x1 << kNumMaterialBits) - 1;

    static const uint32_t kNumOrderBits = 4;
    static const uint32_t kOrderMask = (0x1 << kNumOrderBits) - 1;
    static const uint32_t kMaxOrder = kOrderMask;  // for inversion in opaque case

    static const uint32_t kNumSortZBits = 4;
    static const uint32_t kSortZMask = (0x1 << kNumSortZBits) - 1;

    static const uint32_t kNumTransparentBits = 1;
    static const uint32_t kTransparentMask = (0x1 << kNumTransparentBits) - 1;
    //---

    static const uint32_t kTransparentMaterialShift = 0;
    static const uint32_t kTransparentOrderShift = kNumMaterialBits;
    static const uint32_t kTransparentSortZShift = kNumMaterialBits + kNumOrderBits;

    static const uint32_t kOpaqueOrderShift = 0;
    static const uint32_t kOpaqueSortZShift = kNumOrderBits;
    static const uint32_t kOpaqueMaterialShift = kNumOrderBits + kNumSortZBits;

    static const uint32_t kTransparentShift = kNumMaterialBits + kNumOrderBits + kNumSortZBits;

    // Since the default key is opaque, its order (of zero) needs to be inverted
    SortKey() : fKey(Order2Bits(false, 0)) {}
    explicit SortKey(bool transparent, uint32_t sortZ, uint32_t order, uint32_t material) {
        SkASSERT(sortZ != 0 && order != 0 /* && material != 0*/);

        fKey = ((int)transparent) << kTransparentShift |
                SortZ2Bits(transparent, sortZ) |
                Order2Bits(transparent, order) |
                Material2Bits(transparent, material);
        SkDebugf("Key: %d\n", fKey);
    }

    bool transparent() const {
        return (fKey >> kTransparentShift) & kTransparentMask;
    }

    uint32_t sortZ() const {
        return (fKey >> this->transparent() ? kTransparentSortZShift : kOpaqueSortZShift) &
               kSortZMask;
    }

    uint32_t order() const {
        if (this->transparent()) {
            return (fKey >> kDepthShift) & kDepthMask;
        }

        // TODO: better encapsulate the reversal of the depth & material when the key is opaque
        uint32_t tmp = (fKey >> kMaterialShift) & kDepthMask;
        return (kMaxDepth - tmp) - 1;
    }

    uint32_t material() const {
        return (fKey >> this->transparent() ? kTransparentMaterialShift : kOpaqueMaterialShift) &
               kMaterialMask;
    }

    void dump() const {
        SkDebugf("transparent: %d sortZ: %d order: %d mat: %d\n",
                 this->transparent(),
                 this->sortZ(),
                 this->order(),
                 this->material());
    }

    bool operator>(const SortKey& other) const { return fKey > other.fKey; }
    bool operator<(const SortKey& other) const { return fKey < other.fKey; }

//private:
    static uint32_t SortZ2Bits(bool transparent, uint32_t sortZ) {
        SkASSERT(!(sortZ & ~kSortZMask));
        return sortZ << transparent ? kTransparentSortZShift : kOpaqueSortZShift;
    }

    static uint32_t Order2Bits(bool transparent, uint32_t order) {
        SkASSERT(!(order & ~kOrderMask));

        if (transparent) {
            return order << kTransparentOrderShift;
        } else {
            // We want the opaque draws to be sorted front to back
            order = kMaxOrder - order - 1;
            SkASSERT(!(order & ~kOrderMask));
            return order << kOpaqueOrderShift;
        }
    }

    static uint32_t Bits2Order(bool transparent, uint32_t bits) {

    }

    static uint32_t Material2Bits(bool transparent, uint32_t material) {
        SkASSERT(!(material & ~kMaterialMask));
        return material << transparent ? kTransparentMaterialShift : kOpaqueMaterialShift;
    }

    uint32_t fKey;
};

#endif // Key_DEFINED
