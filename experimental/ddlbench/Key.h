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

    static const uint32_t kDepthShift = kNumMaterialBits;
    static const uint32_t kNumDepthBits = 3;
    static const uint32_t kDepthMask = (0x1 << kNumDepthBits) - 1;

    static const uint32_t kTransparentShift = kNumMaterialBits + kNumDepthBits;
    static const uint32_t kNumTransparentBits = 1;
    static const uint32_t kTransparentMask = (0x1 << kNumTransparentBits) - 1;

    Key() : fKey(0) {}
    explicit Key(bool transparent, uint32_t depth, uint32_t material) {
        SkASSERT(depth != 0 /* && material != 0*/);
        SkASSERT(!(depth & ~kDepthMask));
        SkASSERT(!(material & ~kMaterialMask));

        fKey = (transparent ? 0x1 : 0x0) << kTransparentShift |
               (depth & kDepthMask) << kDepthShift |
               (material & kMaterialMask) << kMaterialShift;
    }

    bool transparent() const {
        return (fKey >> kTransparentShift) & kTransparentMask;
    }

    uint32_t depth() const {
        return (fKey >> kDepthShift) & kDepthMask;
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
