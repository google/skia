/* include/graphics/SkScalerContext.h
**
** Copyright 2006, Google Inc.
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
*/

#ifndef SkScalerContext_DEFINED
#define SkScalerContext_DEFINED

#include "SkMatrix.h"
#include "SkPath.h"
#include "SkPoint.h"

class SkDescriptor;
class SkMaskFilter;
class SkPaint;
class SkPathEffect;
class SkRasterizer;

struct SkGlyph {
    void*       fImage;
    SkPath*     fPath;
    SkFixed     fAdvanceX, fAdvanceY;

    uint16_t    f_GlyphID;
    uint16_t    fWidth, fHeight, fRowBytes;
    int16_t     fTop, fLeft;

    uint8_t     fMaskFormat;

    unsigned    getGlyphID(unsigned baseGlyphCount) const
    {
        SkASSERT(f_GlyphID >= baseGlyphCount);
        return f_GlyphID - baseGlyphCount;
    }

    size_t computeImageSize() const;
};

class SkScalerContext {
public:
    enum Hints {
        kNo_Hints,
        kAuto_Hints,
        kNative_Hints
    };
    struct Rec {
        SkScalar    fTextSize, fPreScaleX, fPreSkewX;
        SkScalar    fPost2x2[2][2];
        SkScalar    fFrameWidth, fMiterLimit;
        uint8_t     fHints;
        SkBool8     fFrameAndFill;
        uint8_t     fMaskFormat;
        uint8_t     fStrokeJoin;

        void    getMatrixFrom2x2(SkMatrix*) const;
        void    getLocalMatrix(SkMatrix*) const;
        void    getSingleMatrix(SkMatrix*) const;
    };

    SkScalerContext(const SkDescriptor* desc);
    virtual ~SkScalerContext();

    void setBaseGlyphCount(unsigned baseGlyphCount) { fBaseGlyphCount = baseGlyphCount; }

    uint16_t    charToGlyphID(SkUnichar uni);

    unsigned    getGlyphCount() const { return this->generateGlyphCount(); }
    void        getMetrics(SkGlyph*);
    void        getImage(const SkGlyph&);
    void        getPath(const SkGlyph&, SkPath*);
    void        getLineHeight(SkPoint* above, SkPoint* below);

    static inline void MakeRec(const SkPaint&, const SkMatrix*, Rec* rec);
    static SkScalerContext* Create(const SkDescriptor*);

protected:
    Rec         fRec;
    unsigned    fBaseGlyphCount;

    virtual unsigned generateGlyphCount() const = 0;
    virtual uint16_t generateCharToGlyph(SkUnichar) = 0;
    virtual void generateMetrics(SkGlyph*) = 0;
    virtual void generateImage(const SkGlyph&) = 0;
    virtual void generatePath(const SkGlyph&, SkPath*) = 0;
    virtual void generateLineHeight(SkPoint* above, SkPoint* below) = 0;

private:
    SkPathEffect*   fPathEffect;
    SkMaskFilter*   fMaskFilter;
    SkRasterizer*   fRasterizer;
    SkScalar        fDevFrameWidth;

    void internalGetPath(const SkGlyph& glyph, SkPath* fillPath, SkPath* devPath, SkMatrix* fillToDevMatrix);

    mutable SkScalerContext* fAuxScalerContext;

    SkScalerContext* getGlyphContext(const SkGlyph& glyph) const;
    SkScalerContext* loadAuxContext() const;  // return loaded fAuxScalerContext or NULL
};

#define kRec_SkDescriptorTag            SkSetFourByteTag('s', 'r', 'e', 'c')
#define kTypeface_SkDescriptorTag       SkSetFourByteTag('t', 'p', 'f', 'c')
#define kPathEffect_SkDescriptorTag     SkSetFourByteTag('p', 't', 'h', 'e')
#define kMaskFilter_SkDescriptorTag     SkSetFourByteTag('m', 's', 'k', 'f')
#define kRasterizer_SkDescriptorTag     SkSetFourByteTag('r', 'a', 's', 't')

#endif

