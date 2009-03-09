/*
 ** Copyright 2006, The Android Open Source Project
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

#include <carbon/carbon.h>
#include "SkFontHost.h"
#include "SkDescriptor.h"
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
    virtual unsigned generateGlyphCount() const;
    virtual uint16_t generateCharToGlyph(SkUnichar uni);
    virtual void generateAdvance(SkGlyph* glyph);
    virtual void generateMetrics(SkGlyph* glyph);
    virtual void generateImage(const SkGlyph& glyph);
    virtual void generatePath(const SkGlyph& glyph, SkPath* path);
    virtual void generateLineHeight(SkPoint* ascent, SkPoint* descent);
    virtual void generateFontMetrics(SkPaint::FontMetrics* mX, SkPaint::FontMetrics* mY);
//    virtual SkDeviceContext getDC() { return NULL; } // not implemented on Mac

private:
    ATSUTextLayout  fLayout;
    ATSUStyle       fStyle;
    CGColorSpaceRef fGrayColorSpace;
    
    static OSStatus MoveTo(const Float32Point *pt, void *cb);
    static OSStatus Line(const Float32Point *pt, void *cb);
    static OSStatus Curve(const Float32Point *pt1, const Float32Point *pt2, const Float32Point *pt3, void *cb);
    static OSStatus Close(void *cb);
};

SkScalerContext_Mac::SkScalerContext_Mac(const SkDescriptor* desc)
        : SkScalerContext(desc), fLayout(0), fStyle(0) {
    SkAutoMutexAcquire  ac(gFTMutex);
    OSStatus err;
    
    err = ::ATSUCreateStyle(&fStyle);
    SkASSERT(0 == err);

    Fixed fixedSize = SkScalarToFixed(fRec.fTextSize);
    static const ATSUAttributeTag sizeTag = kATSUSizeTag;
    static const ByteCount sizeTagSize = sizeof(Fixed);
    const ATSUAttributeValuePtr values[] = { &fixedSize };
    err = ::ATSUSetAttributes(fStyle,1,&sizeTag,&sizeTagSize,values);
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

unsigned SkScalerContext_Mac::generateGlyphCount() const {
    return 0xFFFF;
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
    ATSGlyphScreenMetrics metrics;

    OSStatus err = ATSUGlyphGetScreenMetrics(fStyle, 1, &glyphID, 0, true, true,
                                             &metrics);
    if (noErr != err) {
        set_glyph_metrics_on_error(glyph);
    } else {
        glyph->fAdvanceX = SkFloatToFixed(metrics.deviceAdvance.x);
        glyph->fAdvanceY = -SkFloatToFixed(metrics.deviceAdvance.y);
        glyph->fWidth = metrics.width;
        glyph->fHeight = metrics.height;
        glyph->fLeft = sk_float_round2int(metrics.topLeft.x);
        glyph->fTop = -sk_float_round2int(metrics.topLeft.y);
    }
}

void SkScalerContext_Mac::generateFontMetrics(SkPaint::FontMetrics* mx,
                                              SkPaint::FontMetrics* my) {
#if 0
    OSStatus ATSFontGetVerticalMetrics (
                                        ATSFontRef iFont,
                                        ATSOptionFlags iOptions,
                                        ATSFontMetrics *oMetrics
    );
#endif
    //SkASSERT(false);
    if (mx)
        memset(mx, 0, sizeof(SkPaint::FontMetrics));
    if (my)
        memset(my, 0, sizeof(SkPaint::FontMetrics));
    return;
}

void SkScalerContext_Mac::generateImage(const SkGlyph& glyph)
{
    SkAutoMutexAcquire  ac(gFTMutex);
    SkASSERT(fLayout);

    bzero(glyph.fImage, glyph.fHeight * glyph.rowBytes());
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
    
    CGGlyph glyphID = glyph.getGlyphID();
    CGFontRef fontRef = CGFontCreateWithPlatformFont(&fRec.fFontID);
    CGContextSetFont(contextRef, fontRef);
    CGContextSetFontSize(contextRef, SkScalarToFloat(fRec.fTextSize));
    CGContextShowGlyphsAtPoint(contextRef, -glyph.fLeft,
                               glyph.fTop + glyph.fHeight, &glyphID, 1);

    ::CGContextRelease(contextRef);
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

void SkScalerContext_Mac::generateLineHeight(SkPoint* ascent, SkPoint* descent)
{
    ATSUTextMeasurement     textAscent, textDescent;
    ByteCount actual = 0;
    OSStatus err = ::ATSUGetAttribute(fStyle,kATSULineAscentTag,sizeof(ATSUTextMeasurement),&textAscent,&actual);
    ascent->set(0,textAscent);
    err = ::ATSUGetAttribute(fStyle,kATSULineDescentTag,sizeof(ATSUTextMeasurement),&textDescent,&actual);
    descent->set(0,textDescent);
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

SkScalerContext* SkFontHost::CreateScalerContext(const SkDescriptor* desc) {
    return new SkScalerContext_Mac(desc);
}

SkScalerContext* SkFontHost::CreateFallbackScalerContext(const SkScalerContext::Rec& rec)
{
    SkAutoDescriptor    ad(sizeof(rec) + SkDescriptor::ComputeOverhead(1));
    SkDescriptor*       desc = ad.getDesc();
    
    desc->init();
    SkScalerContext::Rec* newRec =
    (SkScalerContext::Rec*)desc->addEntry(kRec_SkDescriptorTag,
                                          sizeof(rec), &rec);
    newRec->fFontID = find_default_fontID();
    desc->computeChecksum();
    
    return SkFontHost::CreateScalerContext(desc);
}

SkTypeface* SkFontHost::CreateTypeface(const SkTypeface* familyFace,
                            const char familyName[], SkTypeface::Style style) {
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

