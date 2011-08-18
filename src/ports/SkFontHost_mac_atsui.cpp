
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include <Carbon/Carbon.h>
#include "SkFontHost.h"
#include "SkDescriptor.h"
#include "SkEndian.h"
#include "SkFloatingPoint.h"
#include "SkPaint.h"
#include "SkPoint.h"

// Give 1MB font cache budget
#define FONT_CACHE_MEMORY_BUDGET    (1024 * 1024)

const char* gDefaultfont = "Arial"; // hard code for now
static SkMutex      gFTMutex;

static inline SkPoint F32PtToSkPoint(const Float32Point p) {
    SkPoint sp = { SkFloatToScalar(p.x), SkFloatToScalar(p.y) };
    return sp;
}

static inline uint32_t _rotl(uint32_t v, uint32_t r) {
    return (v << r | v >> (32 - r));
}

class SkTypeface_Mac : public SkTypeface {
public:
    SkTypeface_Mac(SkTypeface::Style style, uint32_t id)
        : SkTypeface(style, id) {}
};

#pragma mark -

static uint32_t find_from_name(const char name[]) {
    CFStringRef str = CFStringCreateWithCString(NULL, name,
                                                kCFStringEncodingUTF8);
    uint32_t fontID = ::ATSFontFindFromName(str, kATSOptionFlagsDefault);
    CFRelease(str);
    return fontID;
}

static uint32_t find_default_fontID() {
    static const char* gDefaultNames[] = { "Arial", "Tahoma", "Helvetica" };

    uint32_t fontID;
    for (size_t i = 0; i < SK_ARRAY_COUNT(gDefaultNames); i++) {
        fontID = find_from_name(gDefaultNames[i]);
        if (fontID) {
            return fontID;
        }
    }
    sk_throw();
    return 0;
}

static SkTypeface* CreateTypeface_(const char name[], SkTypeface::Style style) {
    uint32_t fontID = 0;
    if (NULL != name) {
        fontID = find_from_name(name);
    }
    if (0 == fontID) {
        fontID = find_default_fontID();
    }
    // we lie (for now) and report that we found the exact style bits
    return new SkTypeface_Mac(style, fontID);
}

#pragma mark -

class SkScalerContext_Mac : public SkScalerContext {
public:
    SkScalerContext_Mac(const SkDescriptor* desc);
    virtual ~SkScalerContext_Mac();

protected:
    virtual unsigned generateGlyphCount();
    virtual uint16_t generateCharToGlyph(SkUnichar uni);
    virtual void generateAdvance(SkGlyph* glyph);
    virtual void generateMetrics(SkGlyph* glyph);
    virtual void generateImage(const SkGlyph& glyph);
    virtual void generatePath(const SkGlyph& glyph, SkPath* path);
    virtual void generateFontMetrics(SkPaint::FontMetrics* mX, SkPaint::FontMetrics* mY);

private:
    ATSUTextLayout  fLayout;
    ATSUStyle       fStyle;
    CGColorSpaceRef fGrayColorSpace;
    CGAffineTransform   fTransform;

    static OSStatus MoveTo(const Float32Point *pt, void *cb);
    static OSStatus Line(const Float32Point *pt, void *cb);
    static OSStatus Curve(const Float32Point *pt1, const Float32Point *pt2, const Float32Point *pt3, void *cb);
    static OSStatus Close(void *cb);
};

void SkFontHost::FilterRec(SkScalerContext::Rec* rec) {
    // we only support 2 levels of hinting
    SkPaint::Hinting h = rec->getHinting();
    if (SkPaint::kSlight_Hinting == h) {
        h = SkPaint::kNo_Hinting;
    } else if (SkPaint::kFull_Hinting == h) {
        h = SkPaint::kNormal_Hinting;
    }
    rec->setHinting(h);

    // we don't support LCD text
    if (SkMask::kLCD16_Format == rec->fMaskFormat ||
        SkMask::kLCD32_Format == rec->fMaskFormat) {
        rec->fMaskFormat = SkMask::kA8_Format;
    }
}

SkScalerContext_Mac::SkScalerContext_Mac(const SkDescriptor* desc)
    : SkScalerContext(desc), fLayout(0), fStyle(0)
{
    SkAutoMutexAcquire  ac(gFTMutex);
    OSStatus err;

    err = ::ATSUCreateStyle(&fStyle);
    SkASSERT(0 == err);

    SkMatrix    m;
    fRec.getSingleMatrix(&m);

    fTransform = CGAffineTransformMake(SkScalarToFloat(m[SkMatrix::kMScaleX]),
                                       SkScalarToFloat(m[SkMatrix::kMSkewX]),
                                       SkScalarToFloat(m[SkMatrix::kMSkewY]),
                                       SkScalarToFloat(m[SkMatrix::kMScaleY]),
                                       SkScalarToFloat(m[SkMatrix::kMTransX]),
                                       SkScalarToFloat(m[SkMatrix::kMTransY]));

    ATSStyleRenderingOptions renderOpts = kATSStyleApplyAntiAliasing;
    switch (fRec.getHinting()) {
        case SkPaint::kNo_Hinting:
        case SkPaint::kSlight_Hinting:
            renderOpts |= kATSStyleNoHinting;
            break;
        case SkPaint::kNormal_Hinting:
        case SkPaint::kFull_Hinting:
            renderOpts |= kATSStyleApplyHints;
            break;
    }

    ATSUFontID fontID = FMGetFontFromATSFontRef(fRec.fFontID);
    // we put everything in the matrix, so our pt size is just 1.0
    Fixed fixedSize = SK_Fixed1;
    static const ATSUAttributeTag tags[] = {
        kATSUFontTag, kATSUSizeTag, kATSUFontMatrixTag, kATSUStyleRenderingOptionsTag
    };
    static const ByteCount sizes[] = {
        sizeof(fontID), sizeof(fixedSize), sizeof(fTransform), sizeof(renderOpts)
    };
    const ATSUAttributeValuePtr values[] = {
        &fontID, &fixedSize, &fTransform, &renderOpts
    };
    err = ::ATSUSetAttributes(fStyle, SK_ARRAY_COUNT(tags),
                              tags, sizes, values);
    SkASSERT(0 == err);

    err = ::ATSUCreateTextLayout(&fLayout);
    SkASSERT(0 == err);

    fGrayColorSpace = ::CGColorSpaceCreateDeviceGray();
}

SkScalerContext_Mac::~SkScalerContext_Mac() {
    ::CGColorSpaceRelease(fGrayColorSpace);
    ::ATSUDisposeTextLayout(fLayout);
    ::ATSUDisposeStyle(fStyle);
}

// man, we need to consider caching this, since it is just dependent on
// fFontID, and not on any of the other settings like matrix or flags
unsigned SkScalerContext_Mac::generateGlyphCount() {
    // The 'maxp' table stores the number of glyphs a offset 4, in 2 bytes
    uint16_t numGlyphs;
    if (SkFontHost::GetTableData(fRec.fFontID,
                                 SkSetFourByteTag('m', 'a', 'x', 'p'),
                                 4, 2, &numGlyphs) != 2) {
        return 0xFFFF;
    }
    return SkEndian_SwapBE16(numGlyphs);
}

uint16_t SkScalerContext_Mac::generateCharToGlyph(SkUnichar uni)
{
    SkAutoMutexAcquire  ac(gFTMutex);

    OSStatus err;
    UniChar achar = uni;
    err = ::ATSUSetTextPointerLocation(fLayout,&achar,0,1,1);
    err = ::ATSUSetRunStyle(fLayout,fStyle,kATSUFromTextBeginning,kATSUToTextEnd);

    ATSLayoutRecord *layoutPtr;
    ItemCount count;
    ATSGlyphRef glyph;

    err = ::ATSUDirectGetLayoutDataArrayPtrFromTextLayout(fLayout,0,kATSUDirectDataLayoutRecordATSLayoutRecordCurrent,(void**)&layoutPtr,&count);
    glyph = layoutPtr->glyphID;
    ::ATSUDirectReleaseLayoutDataArrayPtr(NULL,kATSUDirectDataLayoutRecordATSLayoutRecordCurrent,(void**)&layoutPtr);
    return glyph;
}

static void set_glyph_metrics_on_error(SkGlyph* glyph) {
    glyph->fRsbDelta = 0;
    glyph->fLsbDelta = 0;
    glyph->fWidth    = 0;
    glyph->fHeight   = 0;
    glyph->fTop      = 0;
    glyph->fLeft     = 0;
    glyph->fAdvanceX = 0;
    glyph->fAdvanceY = 0;
}

void SkScalerContext_Mac::generateAdvance(SkGlyph* glyph) {
    this->generateMetrics(glyph);
}

void SkScalerContext_Mac::generateMetrics(SkGlyph* glyph) {
    GlyphID glyphID = glyph->getGlyphID(fBaseGlyphCount);
    ATSGlyphScreenMetrics screenMetrics;
    ATSGlyphIdealMetrics idealMetrics;

    OSStatus err = ATSUGlyphGetScreenMetrics(fStyle, 1, &glyphID, 0, true, true,
                                             &screenMetrics);
    if (noErr != err) {
        set_glyph_metrics_on_error(glyph);
        return;
    }
    err = ATSUGlyphGetIdealMetrics(fStyle, 1, &glyphID, 0, &idealMetrics);
    if (noErr != err) {
        set_glyph_metrics_on_error(glyph);
        return;
    }

    if ((fRec.fFlags & SkScalerContext::kSubpixelPositioning_Flag) == 0) {
        glyph->fAdvanceX = SkFloatToFixed(screenMetrics.deviceAdvance.x);
        glyph->fAdvanceY = -SkFloatToFixed(screenMetrics.deviceAdvance.y);
    } else {
        glyph->fAdvanceX = SkFloatToFixed(idealMetrics.advance.x);
        glyph->fAdvanceY = -SkFloatToFixed(idealMetrics.advance.y);
    }

    // specify an extra 1-pixel border, go tive CG room for its antialiasing
    // i.e. without this, I was seeing some edges chopped off!
    glyph->fWidth = screenMetrics.width + 2;
    glyph->fHeight = screenMetrics.height + 2;
    glyph->fLeft = sk_float_round2int(screenMetrics.topLeft.x) - 1;
    glyph->fTop = -sk_float_round2int(screenMetrics.topLeft.y) - 1;
}

void SkScalerContext_Mac::generateImage(const SkGlyph& glyph)
{
    SkAutoMutexAcquire  ac(gFTMutex);
    SkASSERT(fLayout);

    sk_bzero(glyph.fImage, glyph.fHeight * glyph.rowBytes());
    CGContextRef contextRef = ::CGBitmapContextCreate(glyph.fImage,
                                              glyph.fWidth, glyph.fHeight, 8,
                                              glyph.rowBytes(), fGrayColorSpace,
                                              kCGImageAlphaNone);
    if (!contextRef) {
        SkASSERT(false);
        return;
    }

    ::CGContextSetGrayFillColor(contextRef, 1.0, 1.0);
    ::CGContextSetTextDrawingMode(contextRef, kCGTextFill);

    CGGlyph glyphID = glyph.getGlyphID(fBaseGlyphCount);
    CGFontRef fontRef = CGFontCreateWithPlatformFont(&fRec.fFontID);
    CGContextSetFont(contextRef, fontRef);
    CGContextSetFontSize(contextRef, 1);
    CGContextSetTextMatrix(contextRef, fTransform);
    CGContextShowGlyphsAtPoint(contextRef, -glyph.fLeft,
                               glyph.fTop + glyph.fHeight, &glyphID, 1);

    ::CGContextRelease(contextRef);
}

#if 0
static void convert_metrics(SkPaint::FontMetrics* dst,
                            const ATSFontMetrics& src) {
    dst->fTop     = -SkFloatToScalar(src.ascent);
    dst->fAscent  = -SkFloatToScalar(src.ascent);
    dst->fDescent = SkFloatToScalar(src.descent);
    dst->fBottom  = SkFloatToScalar(src.descent);
    dst->fLeading = SkFloatToScalar(src.leading);
}
#endif

static void* get_font_table(ATSFontRef fontID, uint32_t tag) {
    ByteCount size;
    OSStatus err = ATSFontGetTable(fontID, tag, 0, 0, NULL, &size);
    if (err) {
        return NULL;
    }
    void* data = sk_malloc_throw(size);
    err = ATSFontGetTable(fontID, tag, 0, size, data, &size);
    if (err) {
        sk_free(data);
        data = NULL;
    }
    return data;
}

static int get_be16(const void* data, size_t offset) {
    const char* ptr = reinterpret_cast<const char*>(data);
    uint16_t value = *reinterpret_cast<const uint16_t*>(ptr + offset);
    int n = SkEndian_SwapBE16(value);
    // now force it to be signed
    return n << 16 >> 16;
}

#define SFNT_HEAD_UPEM_OFFSET       18
#define SFNT_HEAD_YMIN_OFFSET       38
#define SFNT_HEAD_YMAX_OFFSET       42
#define SFNT_HEAD_STYLE_OFFSET      44

#define SFNT_HHEA_ASCENT_OFFSET     4
#define SFNT_HHEA_DESCENT_OFFSET    6
#define SFNT_HHEA_LEADING_OFFSET    8

static bool init_vertical_metrics(ATSFontRef font, SkPoint pts[5]) {
    void* head = get_font_table(font, 'head');
    if (NULL == head) {
        return false;
    }
    void* hhea = get_font_table(font, 'hhea');
    if (NULL == hhea) {
        sk_free(head);
        return false;
    }

    int upem = get_be16(head, SFNT_HEAD_UPEM_OFFSET);
    int ys[5];

    ys[0] = -get_be16(head, SFNT_HEAD_YMAX_OFFSET);
    ys[1] = -get_be16(hhea, SFNT_HHEA_ASCENT_OFFSET);
    ys[2] = -get_be16(hhea, SFNT_HHEA_DESCENT_OFFSET);
    ys[3] = -get_be16(head, SFNT_HEAD_YMIN_OFFSET);
    ys[4] =  get_be16(hhea, SFNT_HHEA_LEADING_OFFSET);

    // now do some cleanup, to ensure y[max,min] are really that
    if (ys[0] > ys[1]) {
        ys[0] = ys[1];
    }
    if (ys[3] < ys[2]) {
        ys[3] = ys[2];
    }

    for (int i = 0; i < 5; i++) {
        pts[i].set(0, SkIntToScalar(ys[i]) / upem);
    }

    sk_free(hhea);
    sk_free(head);
    return true;
}

void SkScalerContext_Mac::generateFontMetrics(SkPaint::FontMetrics* mx,
                                              SkPaint::FontMetrics* my) {
    SkPoint pts[5];

    if (!init_vertical_metrics(fRec.fFontID, pts)) {
        // these are not as accurate as init_vertical_metrics :(
        ATSFontMetrics metrics;
        ATSFontGetVerticalMetrics(fRec.fFontID, kATSOptionFlagsDefault,
                                  &metrics);
        pts[0].set(0, -SkFloatToScalar(metrics.ascent));
        pts[1].set(0, -SkFloatToScalar(metrics.ascent));
        pts[2].set(0, -SkFloatToScalar(metrics.descent));
        pts[3].set(0, -SkFloatToScalar(metrics.descent));
        pts[4].set(0, SkFloatToScalar(metrics.leading));    //+ or -?
    }

    SkMatrix m;
    fRec.getSingleMatrix(&m);
    m.mapPoints(pts, 5);

    if (mx) {
        mx->fTop = pts[0].fX;
        mx->fAscent = pts[1].fX;
        mx->fDescent = pts[2].fX;
        mx->fBottom = pts[3].fX;
        mx->fLeading = pts[4].fX;
        // FIXME:
        mx->fAvgCharWidth = 0;
        mx->fXMin = 0;
        mx->fXMax = 0;
        mx->fXHeight = 0;
    }
    if (my) {
        my->fTop = pts[0].fY;
        my->fAscent = pts[1].fY;
        my->fDescent = pts[2].fY;
        my->fBottom = pts[3].fY;
        my->fLeading = pts[4].fY;
        // FIXME:
        my->fAvgCharWidth = 0;
        my->fXMin = 0;
        my->fXMax = 0;
        my->fXHeight = 0;
    }
}

void SkScalerContext_Mac::generatePath(const SkGlyph& glyph, SkPath* path)
{
    SkAutoMutexAcquire  ac(gFTMutex);
    OSStatus err,result;

    err = ::ATSUGlyphGetCubicPaths(
            fStyle,glyph.fID,
            &SkScalerContext_Mac::MoveTo,
            &SkScalerContext_Mac::Line,
            &SkScalerContext_Mac::Curve,
            &SkScalerContext_Mac::Close,
            path,&result);
    SkASSERT(err == noErr);
}

OSStatus SkScalerContext_Mac::MoveTo(const Float32Point *pt, void *cb)
{
    reinterpret_cast<SkPath*>(cb)->moveTo(F32PtToSkPoint(*pt));
    return noErr;
}

OSStatus SkScalerContext_Mac::Line(const Float32Point *pt, void *cb)
{
    reinterpret_cast<SkPath*>(cb)->lineTo(F32PtToSkPoint(*pt));
    return noErr;
}

OSStatus SkScalerContext_Mac::Curve(const Float32Point *pt1,
                                    const Float32Point *pt2,
                                    const Float32Point *pt3, void *cb)
{
    reinterpret_cast<SkPath*>(cb)->cubicTo(F32PtToSkPoint(*pt1),
                                           F32PtToSkPoint(*pt2),
                                           F32PtToSkPoint(*pt3));
    return noErr;
}

OSStatus SkScalerContext_Mac::Close(void *cb)
{
    reinterpret_cast<SkPath*>(cb)->close();
    return noErr;
}

#pragma mark -

void SkFontHost::Serialize(const SkTypeface* face, SkWStream* stream) {
    SkASSERT(!"SkFontHost::Serialize unimplemented");
}

SkTypeface* SkFontHost::Deserialize(SkStream* stream) {
    SkASSERT(!"SkFontHost::Deserialize unimplemented");
    return NULL;
}

SkTypeface* SkFontHost::CreateTypefaceFromStream(SkStream* stream) {
    return NULL;
}

SkTypeface* SkFontHost::CreateTypefaceFromFile(const char path[]) {
    return NULL;
}

// static
SkAdvancedTypefaceMetrics* SkFontHost::GetAdvancedTypefaceMetrics(
        uint32_t fontID,
        SkAdvancedTypefaceMetrics::PerGlyphInfo perGlyphInfo,
        const uint32_t* glyphIDs,
        uint32_t glyphIDsCount) {
    SkASSERT(!"SkFontHost::GetAdvancedTypefaceMetrics unimplemented");
    return NULL;
}

SkScalerContext* SkFontHost::CreateScalerContext(const SkDescriptor* desc) {
    return new SkScalerContext_Mac(desc);
}

SkFontID SkFontHost::NextLogicalFont(SkFontID currFontID, SkFontID origFontID) {
    uint32_t newFontID = find_default_fontID();
    if (newFontID == currFontID) {
        newFontID = 0;
    }
    return newFontID;
}

SkTypeface* SkFontHost::CreateTypeface(const SkTypeface* familyFace,
                            const char familyName[],
                            const void* data, size_t bytelength,
                            SkTypeface::Style style) {
    // todo: we don't know how to respect style bits
    if (NULL == familyName && NULL != familyFace) {
        familyFace->ref();
        return const_cast<SkTypeface*>(familyFace);
    } else {
        return CreateTypeface_(familyName, style);
    }
}

size_t SkFontHost::ShouldPurgeFontCache(size_t sizeAllocatedSoFar) {
    if (sizeAllocatedSoFar > FONT_CACHE_MEMORY_BUDGET)
        return sizeAllocatedSoFar - FONT_CACHE_MEMORY_BUDGET;
    else
        return 0;   // nothing to do
}

int SkFontHost::ComputeGammaFlag(const SkPaint& paint) {
    return 0;
}

void SkFontHost::GetGammaTables(const uint8_t* tables[2]) {
    tables[0] = NULL;   // black gamma (e.g. exp=1.4)
    tables[1] = NULL;   // white gamma (e.g. exp= 1/1.4)
}

///////////////////////////////////////////////////////////////////////////////

struct SkSFNTHeader {
    uint32_t    fVersion;
    uint16_t    fNumTables;
    uint16_t    fSearchRange;
    uint16_t    fEntrySelector;
    uint16_t    fRangeShift;
};

struct SkSFNTDirEntry {
    uint32_t    fTag;
    uint32_t    fChecksum;
    uint32_t    fOffset;
    uint32_t    fLength;
};

struct SfntHeader {
    SfntHeader(SkFontID fontID, bool needDir) : fCount(0), fData(NULL) {
        ByteCount size;
        if (ATSFontGetTableDirectory(fontID, 0, NULL, &size)) {
            return;
        }

        SkAutoMalloc storage(size);
        SkSFNTHeader* header = reinterpret_cast<SkSFNTHeader*>(storage.get());
        if (ATSFontGetTableDirectory(fontID, size, header, &size)) {
            return;
        }

        fCount = SkEndian_SwapBE16(header->fNumTables);
        fData = header;
        storage.detach();
    }

    ~SfntHeader() {
        sk_free(fData);
    }

    int count() const { return fCount; }
    const SkSFNTDirEntry* entries() const {
        return reinterpret_cast<const SkSFNTDirEntry*>
            (reinterpret_cast<char*>(fData) + sizeof(SkSFNTHeader));
    }

private:
    int     fCount;
    void*   fData;
};

int SkFontHost::CountTables(SkFontID fontID) {
    SfntHeader header(fontID, false);
    return header.count();
}

int SkFontHost::GetTableTags(SkFontID fontID, SkFontTableTag tags[]) {
    SfntHeader header(fontID, true);
    int count = header.count();
    const SkSFNTDirEntry* entry = header.entries();
    for (int i = 0; i < count; i++) {
        tags[i] = SkEndian_SwapBE32(entry[i].fTag);
    }
    return count;
}

size_t SkFontHost::GetTableSize(SkFontID fontID, SkFontTableTag tag) {
    ByteCount size;
    if (ATSFontGetTable(fontID, tag, 0, 0, NULL, &size)) {
        return 0;
    }
    return size;
}

size_t SkFontHost::GetTableData(SkFontID fontID, SkFontTableTag tag,
                                size_t offset, size_t length, void* data) {
    ByteCount size;
    if (ATSFontGetTable(fontID, tag, offset, length, data, &size)) {
        return 0;
    }
    if (offset >= size) {
        return 0;
    }
    if (offset + length > size) {
        length = size - offset;
    }
    return length;
}
