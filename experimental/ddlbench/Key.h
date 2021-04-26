// Copyright 2021 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#ifndef Key_DEFINED
#define Key_DEFINED

#include "include/core/SkTypes.h"

class Key {
public:
    // field:  |  transparent  |  depth   |  material  |
    // bits:   |      1        |   3      |     4      |
    static const uint32_t kMaterialShift = 0;
    static const uint32_t kNumMaterialBits = 4;
    static const uint32_t kMaterialMask = (0x1 << kNumMaterialBits) - 1;

    // This is a pseudo-Z.
    // For opaque objects it decreases from kMaxDepth with each painter's order draw call
    // For transparent objects it increases from 1 with each painter's order draw call but
    // all those keys have the transparent bit set so they will always be larger than any
    // opaque draw's key.
    // The draw calls are then sorted into increasing key order. This will yield the opaque draw
    // calls in front to back order followed by all the transparent draw calls in back to front
    // order.
    static const uint32_t kDepthShift = kNumMaterialBits;
    static const uint32_t kNumDepthBits = 3;
    static const uint32_t kDepthMask = (0x1 << kNumDepthBits) - 1;
    static const uint32_t kMaxDepth = kDepthMask;

    static const uint32_t kTransparentShift = kNumMaterialBits + kNumDepthBits;
    static const uint32_t kNumTransparentBits = 1;
    static const uint32_t kTransparentMask = (0x1 << kNumTransparentBits) - 1;

    Key() : fKey((kMaxDepth - 1) << kDepthShift) {}
    explicit Key(bool transparent, uint32_t depth, uint32_t material) {
        SkASSERT(depth != 0 /* && material != 0*/);
        SkASSERT(!(depth & ~kDepthMask));
        SkASSERT(!(material & ~kMaterialMask));

        uint32_t munged;
        if (transparent) {
            munged = depth;
        } else {
            // We want the opaque draws to be sorted front to back
            munged = kMaxDepth - depth - 1;
        }

        SkASSERT(!(munged & ~kDepthMask));

        fKey = (transparent ? 0x1 : 0x0) << kTransparentShift |
               (munged & kDepthMask) << kDepthShift |
               (material & kMaterialMask) << kMaterialShift;
    }

    bool transparent() const {
        return (fKey >> kTransparentShift) & kTransparentMask;
    }

    uint32_t depth() const {
        uint32_t tmp = (fKey >> kDepthShift) & kDepthMask;
        if (this->transparent()) {
            return tmp;
        }

        return (kMaxDepth - tmp) - 1;
    }

    uint32_t material() const {
        return (fKey >> kMaterialShift) & kMaterialMask;
    }

    void dump() const {
        SkDebugf("transparent: %d depth: %d mat: %d\n",
                 this->transparent(),
                 this->depth(),
                 this->material());
    }

    bool operator>(const Key& other) const { return fKey > other.fKey; }
    bool operator<(const Key& other) const { return fKey < other.fKey; }

private:
    uint64_t fKey;
};

#endif // Key_DEFINED
