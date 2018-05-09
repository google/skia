/*
 * Copyright 2018 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGlyphRunInfo_DEFINED
#define SkGlyphRunInfo_DEFINED

#include <vector>

#include "SkDescriptor.h"
#include "SkMask.h"
#include "SkPath.h"
#include "SkPoint.h"
#include "SkTypes.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wuninitialized"
class SkGlyphRunInfo {
public:
    static SkGlyphRunInfo Make(uint16_t count, const SkGlyphID[]);

    void populateAdvances(const SkDescriptor& desc) {

    }

    void populateMask(const SkDescriptor& desc) {

    }

    void populatePaths(const SkDescriptor& desc) {

    }

private:
    struct DrawableGlyph {
        explicit DrawableGlyph(SkGlyphID glyphID_) : glyphID{glyphID_} {}
        SkGlyphID glyphID;
        SkPoint   advance;
        SkMask    mask;
        SkPath*   path;
    };

    struct DenseIndex {
        DenseIndex() : index(index) {}
        DenseIndex(uint16_t index_) : index(index_) {}
        operator uint16_t () const { return index; }
        uint16_t index;
    };

    SkGlyphRunInfo(std::vector<DenseIndex>&&, std::vector<DrawableGlyph>&&);
    std::vector<DenseIndex> fDenseIndex;
    std::vector<DrawableGlyph> fUnique;
};



#pragma clang diagnostic pop
#endif  // SkGlyphRunInfo_DEFINED
