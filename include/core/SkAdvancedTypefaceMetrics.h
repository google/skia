
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkAdvancedTypefaceMetrics_DEFINED
#define SkAdvancedTypefaceMetrics_DEFINED

#include "SkRect.h"
#include "SkRefCnt.h"
#include "SkString.h"
#include "SkTDArray.h"
#include "SkTemplates.h"

/** \class SkAdvancedTypefaceMetrics

    The SkAdvancedTypefaceMetrics class is used by the PDF backend to correctly
    embed typefaces.  This class is filled in with information about a given
    typeface by the SkFontHost class.
*/

class SkAdvancedTypefaceMetrics : public SkRefCnt {
public:
    SK_DECLARE_INST_COUNT(SkAdvancedTypefaceMetrics)

    SkString fFontName;

    enum FontType {
        kType1_Font,
        kType1CID_Font,
        kCFF_Font,
        kTrueType_Font,
        kOther_Font,
    };
    // The type of the underlying font program.  This field determines which
    // of the following fields are valid.  If it is kOther_Font the per glyph
    // information will never be populated.
    FontType fType;

    enum FontFlags {
        kEmpty_FontFlag          = 0x0,  //!<No flags set
        kMultiMaster_FontFlag    = 0x1,  //!<May be true for Type1 or CFF fonts.
        kNotEmbeddable_FontFlag  = 0x2,  //!<May not be embedded.
        kNotSubsettable_FontFlag = 0x4,  //!<May not be subset.
    };
    // Global font flags.
    FontFlags fFlags;

    uint16_t fLastGlyphID; // The last valid glyph ID in the font.
    uint16_t fEmSize;  // The size of the em box (defines font units).

    // These enum values match the values used in the PDF file format.
    enum StyleFlags {
        kFixedPitch_Style  = 0x00001,
        kSerif_Style       = 0x00002,
        kScript_Style      = 0x00008,
        kItalic_Style      = 0x00040,
        kAllCaps_Style     = 0x10000,
        kSmallCaps_Style   = 0x20000,
        kForceBold_Style   = 0x40000
    };
    uint16_t fStyle;        // Font style characteristics.
    int16_t fItalicAngle;   // Counterclockwise degrees from vertical of the
                            // dominant vertical stroke for an Italic face.
    // The following fields are all in font units.
    int16_t fAscent;       // Max height above baseline, not including accents.
    int16_t fDescent;      // Max depth below baseline (negative).
    int16_t fStemV;        // Thickness of dominant vertical stem.
    int16_t fCapHeight;    // Height (from baseline) of top of flat capitals.

    SkIRect fBBox;  // The bounding box of all glyphs (in font units).

    // The type of advance data wanted.
    enum PerGlyphInfo {
      kNo_PerGlyphInfo         = 0x0, // Don't populate any per glyph info.
      kHAdvance_PerGlyphInfo   = 0x1, // Populate horizontal advance data.
      kVAdvance_PerGlyphInfo   = 0x2, // Populate vertical advance data.
      kGlyphNames_PerGlyphInfo = 0x4, // Populate glyph names (Type 1 only).
      kToUnicode_PerGlyphInfo  = 0x8  // Populate ToUnicode table, ignored
                                      // for Type 1 fonts
    };

    template <typename Data>
    struct AdvanceMetric {
        enum MetricType {
            kDefault,  // Default advance: fAdvance.count = 1
            kRange,    // Advances for a range: fAdvance.count = fEndID-fStartID
            kRun       // fStartID-fEndID have same advance: fAdvance.count = 1
        };
        MetricType fType;
        uint16_t fStartId;
        uint16_t fEndId;
        SkTDArray<Data> fAdvance;
        SkAutoTDelete<AdvanceMetric<Data> > fNext;
    };

    struct VerticalMetric {
        int16_t fVerticalAdvance;
        int16_t fOriginXDisp;  // Horiz. displacement of the secondary origin.
        int16_t fOriginYDisp;  // Vert. displacement of the secondary origin.
    };
    typedef AdvanceMetric<int16_t> WidthRange;
    typedef AdvanceMetric<VerticalMetric> VerticalAdvanceRange;

    // This is indexed by glyph id.
    SkAutoTDelete<WidthRange> fGlyphWidths;
    // Only used for Vertical CID fonts.
    SkAutoTDelete<VerticalAdvanceRange> fVerticalMetrics;

    // The names of each glyph, only populated for postscript fonts.
    SkAutoTDelete<SkAutoTArray<SkString> > fGlyphNames;

    // The mapping from glyph to Unicode, only populated if
    // kToUnicode_PerGlyphInfo is passed to GetAdvancedTypefaceMetrics.
    SkTDArray<SkUnichar> fGlyphToUnicode;

private:
    typedef SkRefCnt INHERITED;
};

namespace skia_advanced_typeface_metrics_utils {

template <typename Data>
void resetRange(SkAdvancedTypefaceMetrics::AdvanceMetric<Data>* range,
                       int startId);

template <typename Data>
SkAdvancedTypefaceMetrics::AdvanceMetric<Data>* appendRange(
        SkAutoTDelete<SkAdvancedTypefaceMetrics::AdvanceMetric<Data> >* nextSlot,
        int startId);

template <typename Data>
void finishRange(
        SkAdvancedTypefaceMetrics::AdvanceMetric<Data>* range,
        int endId,
        typename SkAdvancedTypefaceMetrics::AdvanceMetric<Data>::MetricType
                type);

/** Retrieve advance data for glyphs. Used by the PDF backend. It calls
    underlying platform dependent API getAdvance to acquire the data.
    @param num_glyphs    Total number of glyphs in the given font.
    @param glyphIDs      For per-glyph info, specify subset of the font by
                         giving glyph ids.  Each integer represents a glyph
                         id.  Passing NULL means all glyphs in the font.
    @param glyphIDsCount Number of elements in subsetGlyphIds. Ignored if
                         glyphIDs is NULL.
*/
template <typename Data, typename FontHandle>
SkAdvancedTypefaceMetrics::AdvanceMetric<Data>* getAdvanceData(
        FontHandle fontHandle,
        int num_glyphs,
        const uint32_t* glyphIDs,
        uint32_t glyphIDsCount,
        bool (*getAdvance)(FontHandle fontHandle, int gId, Data* data));

} // namespace skia_advanced_typeface_metrics_utils

#endif
