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
    // field:  |  transparent (false) | sortZ    | material    | draw order |
    // bits:   |      1               |  4       |     4       |      4     |
    //
    // Transparent Order
    // field:  |  transparent (true)  | sortZ    | draw order  |  material  |
    // bits:   |      1               |  4       |     4       |      4     |
    // Note:
    // For opaque draws, the material dominates the draw order to improve material-based batching
    // and the 'draw order' is inverted to yield front-to-back drawing.
    // For transparent draws, material-based batching is less important so the material comes
    // last. Additionally, the 'draw order' is setup to yield back-to-front drawing.

    static const uint32_t kNumMaterialBits = 4;
    static const uint32_t kMaterialMask = (0x1 << kNumMaterialBits) - 1;

    static const uint32_t kNumSortZBits = 4;
    static const uint32_t kSortZMask = (0x1 << kNumSortZBits) - 1;

    static const uint32_t kNumOrderBits = 4;
    static const uint32_t kOrderMask = (0x1 << kNumOrderBits) - 1;
    static const uint32_t kMaxOrder = kOrderMask;  // for inversion in opaque case

    static const uint32_t kNumTransparentBits = 1;
    static const uint32_t kTransparentMask = (0x1 << kNumTransparentBits) - 1;
    //---

    static const uint32_t kTransparentMaterialShift = 0;
    static const uint32_t kTransparentOrderShift = kNumMaterialBits;

    static const uint32_t kOpaqueOrderShift = 0;
    static const uint32_t kOpaqueMaterialShift = kNumOrderBits;

    static const uint32_t kSortZShift = kNumMaterialBits + kNumOrderBits;
    static const uint32_t kTransparentShift = kNumMaterialBits + kNumOrderBits + kNumSortZBits;

    // Since the default key is opaque, its order (of zero) needs to be inverted
    SortKey() : fKey(Order2Bits(false, 0)) {}
    explicit SortKey(bool transparent, uint32_t sortZ, uint32_t order, uint32_t material)
        : fKey(Transparency2Bits(transparent) |
               SortZ2Bits(transparent, sortZ) |
               Order2Bits(transparent, order) |
               Material2Bits(transparent, material)) {
    }

    bool transparent() const { return Bits2Transparency(fKey); }
    uint32_t sortZ() const { return Bits2SortZ(this->transparent(), fKey); }
    uint32_t order() const { return Bits2Order(this->transparent(), fKey); }
    uint32_t material() const { return Bits2Material(this->transparent(), fKey); }

    void dump() const {
        SkDebugf("%s sortZ: %d order: %d mat: %d - raw: %d",
                 this->transparent() ? "trans" : "opaque",
                 this->sortZ(),
                 this->order(),
                 this->material(),
                 fKey);
    }

    bool operator>(const SortKey& other) const { return fKey > other.fKey; }
    bool operator<(const SortKey& other) const { return fKey < other.fKey; }

private:
    static uint32_t Transparency2Bits(bool transparent) {
        return (int)transparent << kTransparentShift;
    }

    static bool Bits2Transparency(uint32_t bits) {
        return (bits >> kTransparentShift) & kTransparentMask;
    }

    static uint32_t SortZ2Bits(bool transparent, uint32_t sortZ) {
        SkASSERT(!(sortZ & ~kSortZMask));
        return sortZ << kSortZShift;
    }

    static uint32_t Bits2SortZ(bool transparent, uint32_t bits) {
        return (bits >> kSortZShift) & kSortZMask;
    }

    static uint32_t Order2Bits(bool transparent, uint32_t order) {
        SkASSERT(!(order & ~kOrderMask));

        if (transparent) {
            return order << kTransparentOrderShift;
        } else {
            // We want the opaque draws to be sorted front to back
            order = kMaxOrder - order;
            SkASSERT(!(order & ~kOrderMask));
            return order << kOpaqueOrderShift;
        }
    }

    static uint32_t Bits2Order(bool transparent, uint32_t bits) {
        if (transparent) {
            return (bits >> kTransparentOrderShift) & kOrderMask;
        } else {
            uint32_t tmp = (bits >> kOpaqueOrderShift) & kOrderMask;
            return kMaxOrder - tmp;
        }
    }

    static uint32_t Material2Bits(bool transparent, uint32_t material) {
        SkASSERT(!(material & ~kMaterialMask));
        return material << (transparent ? kTransparentMaterialShift : kOpaqueMaterialShift);
    }

    static uint32_t Bits2Material(bool transparent, uint32_t bits) {
        return (bits >> (transparent ? kTransparentMaterialShift : kOpaqueMaterialShift)) &
               kMaterialMask;
    }

    uint32_t fKey;
};

#endif // Key_DEFINED
