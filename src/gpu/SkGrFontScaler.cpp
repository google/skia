/*
    Copyright 2010 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

         http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */


#include "SkGr.h"
#include "SkDescriptor.h"
#include "SkGlyphCache.h"

class SkGrDescKey : public GrKey {
public:
    explicit SkGrDescKey(const SkDescriptor& desc);
    virtual ~SkGrDescKey();

protected:
    // overrides
    virtual bool lt(const GrKey& rh) const;
    virtual bool eq(const GrKey& rh) const;

private:
    SkDescriptor* fDesc;
    enum {
        kMaxStorageInts = 16
    };
    uint32_t fStorage[kMaxStorageInts];
};

///////////////////////////////////////////////////////////////////////////////

SkGrDescKey::SkGrDescKey(const SkDescriptor& desc) : GrKey(desc.getChecksum()) {
    size_t size = desc.getLength();
    if (size <= sizeof(fStorage)) {
        fDesc = GrTCast<SkDescriptor*>(fStorage);
    } else {
        fDesc = SkDescriptor::Alloc(size);
    }
    memcpy(fDesc, &desc, size);
}

SkGrDescKey::~SkGrDescKey() {
    if (fDesc != GrTCast<SkDescriptor*>(fStorage)) {
        SkDescriptor::Free(fDesc);
    }
}

bool SkGrDescKey::lt(const GrKey& rh) const {
    const SkDescriptor* srcDesc = ((const SkGrDescKey*)&rh)->fDesc;
    size_t lenLH = fDesc->getLength();
    size_t lenRH = srcDesc->getLength();
    int cmp = memcmp(fDesc, srcDesc, SkMin32(lenLH, lenRH));
    if (0 == cmp) {
        return lenLH < lenRH;
    } else {
        return cmp < 0;
    }
}

bool SkGrDescKey::eq(const GrKey& rh) const {
    const SkDescriptor* srcDesc = ((const SkGrDescKey*)&rh)->fDesc;
    return fDesc->equals(*srcDesc);
}

///////////////////////////////////////////////////////////////////////////////

SkGrFontScaler::SkGrFontScaler(SkGlyphCache* strike) {
    fStrike = strike;
    fKey = NULL;
}

SkGrFontScaler::~SkGrFontScaler() {
    GrSafeUnref(fKey);
}

const GrKey* SkGrFontScaler::getKey() {
    if (NULL == fKey) {
        fKey = new SkGrDescKey(fStrike->getDescriptor());
    }
    return fKey;
}

bool SkGrFontScaler::getPackedGlyphBounds(GrGlyph::PackedID packed,
                                          GrIRect* bounds) {
    const SkGlyph& glyph = fStrike->getGlyphIDMetrics(GrGlyph::UnpackID(packed),
                                              GrGlyph::UnpackFixedX(packed),
                                              GrGlyph::UnpackFixedY(packed));
    bounds->setXYWH(glyph.fLeft, glyph.fTop, glyph.fWidth, glyph.fHeight);
    return true;

}

bool SkGrFontScaler::getPackedGlyphImage(GrGlyph::PackedID packed,
                                         int width, int height,
                                         int dstRB, void* dst) {
    const SkGlyph& glyph = fStrike->getGlyphIDMetrics(GrGlyph::UnpackID(packed),
                                              GrGlyph::UnpackFixedX(packed),
                                              GrGlyph::UnpackFixedY(packed));
    GrAssert(glyph.fWidth == width);
    GrAssert(glyph.fHeight == height);
    const void* src = fStrike->findImage(glyph);
    if (NULL == src) {
        return false;
    }

    int srcRB = glyph.rowBytes();
    if (srcRB == dstRB) {
        memcpy(dst, src, dstRB * height);
    } else {
        for (int y = 0; y < height; y++) {
            memcpy(dst, src, width);
            src = (const char*)src + srcRB;
            dst = (char*)dst + dstRB;
        }
    }
    return true;
}

bool SkGrFontScaler::getGlyphPath(uint16_t glyphID, GrPath* path) {

    const SkGlyph& glyph = fStrike->getGlyphIDMetrics(glyphID);
    const SkPath* skPath = fStrike->findPath(glyph);
    if (skPath) {
        SkGrPathIter iter(*skPath);
        path->resetFromIter(&iter);
        return true;
    }
    return false;
}



