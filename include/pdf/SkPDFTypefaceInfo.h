/*
 * Copyright (C) 2011 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SkPDFTypefaceInfo_DEFINED
#define SkPDFTypefaceInfo_DEFINED

#include "SkRect.h"
#include "SkRefCnt.h"
#include "SkString.h"
#include "SkTDArray.h"
#include "SkTemplates.h"
#include "SkTScopedPtr.h"

/** \class SkPDFTypefaceInfo

    The SkPDFTypefaceInfo is used by the PDF backend to correctly embed
    typefaces.  This class is filled in with information about a given typeface
    by the SkFontHost class.
*/

class SkPDFTypefaceInfo : public SkRefCnt {
public:
    enum FontType {
        kType1_Font,
        kType1CID_Font,
        kCFF_Font,
        kTrueType_Font,
        kOther_Font,
        kNotEmbeddable_Font,
    };
    // The type of the underlying font program.  This field determines which
    // of the following fields are valid.  If it is kOther_Font or
    // kNotEmbeddable_Font, fFontName may be valid, but no other fields are
    // valid.
    FontType fType;

    // fMultiMaster may be true for Type1_Font or CFF_Font.
    bool fMultiMaster;
    SkString fFontName;
    SkIRect fBBox;  // The bounding box of all glyphs (in font units).

    uint16_t fLastGlyphID;

    template <typename Data>
    struct AdvanceMetric {
        enum MetricType {
            kDefault,  // Default advance: fAdvance.count = 1
            kRange,    // Advances for a range: fAdvance.count = fEndID-fStartID
            kRun,      // fStartID-fEndID have same advance: fAdvance.count = 1
        };
        MetricType fType;
        int fStartId;
        int fEndId;
        SkTDArray<Data> fAdvance;
        SkTScopedPtr<AdvanceMetric<Data> > fNext;
    };

    struct VerticalMetric {
        int fVerticalAdvance;
        int fOriginXDisp;  // Horizontal displacement of the secondary origin.
        int fOriginYDisp;  // Vertical displacement of the secondary origin.
    };
    typedef struct VerticalMetric VerticalMetric;
    typedef AdvanceMetric<int> WidthRange;
    typedef AdvanceMetric<VerticalMetric> VerticalAdvanceRange;

    // This is indexed by glyph id.
    SkTScopedPtr<WidthRange> fGlyphWidths;
    // Only used for Vertical CID fonts.
    SkTScopedPtr<VerticalAdvanceRange> fVerticalMetrics;

    // The names of each glyph, only populated for postscript fonts.
    SkTScopedPtr<SkAutoTArray<SkString> > fGlyphNames;

    // Metrics: probably used by the pdf renderer for substitution, which
    // shouldn't be needed with embedding fonts.  Optional fields with a value
    // of 0 will be ignored.

    // The enum values match the values used in the PDF file format.
    enum StyleFlags {
        kFixedPitch_Style  = 0x00001,
        kSerif_Style       = 0x00002,
        kSymbolic_Style    = 0x00004,
        kScript_Style      = 0x00008,
        kNonsymbolic_Style = 0x00020,
        kItalic_Style      = 0x00040,
        kAllCaps_Style     = 0x10000,
        kSmallCaps_Style   = 0x20000,
        kForceBold_Style   = 0x40000,
    };
    uint16_t fStyle;        // Font style characteristics.
    long fItalicAngle;      // Counterclockwise degrees from vertical of the
                            // dominant vertical stroke for an Italic face.
    SkScalar fAscent;       // Max height above baseline, not including accents.
    SkScalar fDescent;      // Max depth below baseline (negative).
    SkScalar fStemV;        // Thickness of dominant vertical stem.
    SkScalar fCapHeight;    // Height (from baseline) of top of flat capitals.

    /* Omit the optional entries. Better to let the viewer compute them, since
     * it has to be able to anyway.
    SkScalar fLeading;      // Space between lines. Optional.
    SkScalar fXHeight;      // Height of top of 'x'. Optional.
    SkScalar fStemH;        // Thickness of dominant horizontal stem. Optional.
    SkScalar fAvgWidth;     // Average width of glyphs. Optional.
    SkScalar fMaxWidth;     // Max width of a glyph. Optional.
    */
};

#endif
