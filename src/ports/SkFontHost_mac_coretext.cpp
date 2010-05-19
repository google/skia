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
#include <vector>

#include "SkFontHost.h"
#include "SkDescriptor.h"
#include "SkString.h"
#include "SkPaint.h"





//============================================================================
//      Constants
//----------------------------------------------------------------------------
static const SkFontID kSkInvalidFontID                              = 0;

static const size_t FONT_CACHE_MEMORY_BUDGET                        = 1024 * 1024;
static const char  *FONT_DEFAULT_NAME                               = "Lucida Sans";





//============================================================================
//      Types
//----------------------------------------------------------------------------
// Native font info
typedef struct {
    SkString                name;
    SkTypeface::Style       style;
    SkFontID                fontID;
    CTFontRef               fontRef;
} SkNativeFontInfo;

typedef std::vector<SkNativeFontInfo>                               SkNativeFontInfoList;
typedef SkNativeFontInfoList::iterator                              SkNativeFontInfoListIterator;
typedef SkNativeFontInfoList::const_iterator                        SkNativeFontInfoListConstIterator;





//============================================================================
//      Macros
//----------------------------------------------------------------------------
// Release a CFTypeRef
#ifndef CFSafeRelease
#define CFSafeRelease(_object)                                      \
    do                                                              \
        {                                                           \
        if ((_object) != NULL)                                      \
            {                                                       \
            CFRelease((CFTypeRef) (_object));                       \
            (_object) = NULL;                                       \
            }                                                       \
        }                                                           \
    while (false)
#endif





//============================================================================
//      SkNativeFontCache
//----------------------------------------------------------------------------
#pragma mark -
class SkNativeFontCache {
public:
                                        SkNativeFontCache(void);
    virtual                            ~SkNativeFontCache(void);


    // Is a font ID valid?
    bool                                IsValid(SkFontID fontID);
    
    
    // Get a font
    CTFontRef                           GetFont(SkFontID fontID);
    SkNativeFontInfo                    GetFontInfo(const SkString &theName, SkTypeface::Style theStyle);
    
    
    // Create a font
    SkNativeFontInfo                    CreateFont(const SkString &theName, SkTypeface::Style theStyle);


    // Get the font table
    static SkNativeFontCache           *Get(void);


private:
    CTFontRef                           CreateNativeFont(const SkString &name, SkTypeface::Style style);


private:
    SkNativeFontInfoList                mFonts;
    SkMutex                             mMutex;
};

SkNativeFontCache::SkNativeFontCache(void)
{   SkAutoMutexAcquire      acquireLock(mMutex);
    SkNativeFontInfo        fontInfo;


    // Initialise ourselves
    //
    // SkTypeface uses a uint32_t to identify fonts, however CoreText font references
    // are opaque pointers.
    //
    // To support 64-bit builds, we need a separate index to look up a 64-bit font
    // reference from its 32-bit SkFontID. As an ID of 0 is reserved, we insert a
    // dummy entry into the cache so we can use the array index as the font ID.
    //
    // This could be simplified if SkFontID was changed to a intptr_t, and SkTypeface
    // returned an SkFontID from uniqueID().
    fontInfo.name    = SkString("__SkNativeFontCache__");
    fontInfo.style   = SkTypeface::kNormal;
    fontInfo.fontID  = kSkInvalidFontID;
    fontInfo.fontRef = NULL;
        
    mFonts.push_back(fontInfo);
}

SkNativeFontCache::~SkNativeFontCache(void)
{   SkAutoMutexAcquire                  acquireLock(mMutex);
    SkNativeFontInfoListIterator        theIter;


    // Clean up
    for (theIter = mFonts.begin(); theIter != mFonts.end(); theIter++)
        CFSafeRelease(theIter->fontRef);
}

bool SkNativeFontCache::IsValid(SkFontID fontID)
{   SkAutoMutexAcquire  acquireLock(mMutex);
    bool                isValid;


    // Check the ID
    isValid = (fontID >= 1 && fontID < mFonts.size());
    return(isValid);
}

CTFontRef SkNativeFontCache::GetFont(SkFontID fontID)
{   SkAutoMutexAcquire  acquireLock(mMutex);


    // Validate our parameters
    SkASSERT(fontID >= 1 && fontID < mFonts.size());


    // Get the font
    return(mFonts.at(fontID).fontRef);
}

SkNativeFontInfo SkNativeFontCache::GetFontInfo(const SkString &theName, SkTypeface::Style theStyle)
{   SkAutoMutexAcquire              acquireLock(mMutex);
    SkNativeFontInfo                fontInfo;
    SkNativeFontInfoListIterator    theIter;


    // Validate our parameters
    SkASSERT(!theName.isEmpty());


    // Get the state we need
    fontInfo.style   = SkTypeface::kNormal;
    fontInfo.fontID  = kSkInvalidFontID;
    fontInfo.fontRef = NULL;


    // Get the font
    for (theIter = mFonts.begin(); theIter != mFonts.end(); theIter++)
        {
        if (theIter->name == theName && theIter->style == theStyle)
            return(*theIter);
        }
    
    return(fontInfo);
}

SkNativeFontInfo SkNativeFontCache::CreateFont(const SkString &theName, SkTypeface::Style theStyle)
{   SkAutoMutexAcquire      acquireLock(mMutex);
    SkNativeFontInfo        fontInfo;


    // Validate our parameters
    SkASSERT(!theName.isEmpty());


    // Create the font
    fontInfo.name    = theName;
    fontInfo.style   = theStyle;
    fontInfo.fontID  = mFonts.size();
    fontInfo.fontRef = CreateNativeFont(theName, theStyle);

    mFonts.push_back(fontInfo);
    return(fontInfo);
}

SkNativeFontCache *SkNativeFontCache::Get(void)
{   static SkNativeFontCache    sInstance;


    // Get the instance
    //
    // We use a local static for well-defined static initialisation order.
    return(&sInstance);
}

///////////////////////////////////////////////////////////////////////////
CTFontRef SkNativeFontCache::CreateNativeFont(const SkString &theName, SkTypeface::Style theStyle)
{   CFMutableDictionaryRef      cfAttributes, cfTraits;
    CFNumberRef                 cfFontTraits;
    CTFontSymbolicTraits        ctFontTraits;
    CTFontDescriptorRef         ctFontDesc;
    CFStringRef                 cfFontName;
    CTFontRef                   ctFont;


    // Get the state we need
    ctFontDesc   = NULL;
    ctFont       = NULL;
    ctFontTraits = 0;

    if (theStyle & SkTypeface::kBold)
        ctFontTraits |= kCTFontBoldTrait;

    if (theStyle & SkTypeface::kItalic)
        ctFontTraits |= kCTFontItalicTrait;


    // Create the font info
    cfFontName   = CFStringCreateWithCString(NULL, theName.c_str(), kCFStringEncodingUTF8);
    cfFontTraits = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &ctFontTraits);
    cfAttributes = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    cfTraits     = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);


    // Create the font
    //
    // Fonts are scaled using the Sk matrix, so we always request a font of size 1.
    if (cfFontName != NULL && cfFontTraits != NULL && cfAttributes != NULL && cfTraits != NULL)
        {
        CFDictionaryAddValue(cfTraits, kCTFontSymbolicTrait, cfFontTraits);

        CFDictionaryAddValue(cfAttributes, kCTFontFamilyNameAttribute, cfFontName);
        CFDictionaryAddValue(cfAttributes, kCTFontTraitsAttribute,     cfTraits);

        ctFontDesc = CTFontDescriptorCreateWithAttributes(cfAttributes);
        if (ctFontDesc != NULL)
            ctFont = CTFontCreateWithFontDescriptor(ctFontDesc, 1.0, NULL);

        }


    // Clean up
    CFSafeRelease(cfFontName);
    CFSafeRelease(cfFontTraits);
    CFSafeRelease(cfAttributes);
    CFSafeRelease(cfTraits);
    CFSafeRelease(ctFontDesc);

    return(ctFont);
}





//============================================================================
//      SkTypeface_Mac
//----------------------------------------------------------------------------
#pragma mark -
class SkTypeface_Mac : public SkTypeface {
public:
                                        SkTypeface_Mac(SkTypeface::Style style, uint32_t fontID);
};


SkTypeface_Mac::SkTypeface_Mac(SkTypeface::Style style, uint32_t fontID)
        : SkTypeface(style, fontID)
{
}





//============================================================================
//      SkScalerContext_Mac
//----------------------------------------------------------------------------
#pragma mark -
class SkScalerContext_Mac : public SkScalerContext {
public:
                                        SkScalerContext_Mac(const SkDescriptor* desc);
    virtual                            ~SkScalerContext_Mac(void);


protected:
    unsigned                            generateGlyphCount(void) const;
    uint16_t                            generateCharToGlyph(SkUnichar uni);
    void                                generateAdvance(SkGlyph* glyph);
    void                                generateMetrics(SkGlyph* glyph);
    void                                generateImage(const SkGlyph& glyph);
    void                                generatePath( const SkGlyph& glyph, SkPath* path);
    void                                generateFontMetrics(SkPaint::FontMetrics* mX, SkPaint::FontMetrics* mY);


private:
    static void                         CTPathElement(void *info, const CGPathElement *element);


private:
    CGColorSpaceRef                     mColorSpace;
    CGAffineTransform                   mTransform;

    CTFontRef                           mFont;
    uint16_t                            mGlyphCount;
};

SkScalerContext_Mac::SkScalerContext_Mac(const SkDescriptor* desc)
        : SkScalerContext(desc)
{   CFIndex             numGlyphs;
    CTFontRef           ctFont;
    SkMatrix            skMatrix;



    // Get the state we need
    fRec.getSingleMatrix(&skMatrix);

    ctFont    = SkNativeFontCache::Get()->GetFont(fRec.fFontID);
    numGlyphs = CTFontGetGlyphCount(ctFont);
    SkASSERT(numGlyphs >= 1 && numGlyphs <= 0xFFFF);


    // Initialise ourselves
    mColorSpace = CGColorSpaceCreateDeviceGray();
    mTransform  = CGAffineTransformMake(SkScalarToFloat(skMatrix[SkMatrix::kMScaleX]),
                                        SkScalarToFloat(skMatrix[SkMatrix::kMSkewX]),
                                        SkScalarToFloat(skMatrix[SkMatrix::kMSkewY]),
                                        SkScalarToFloat(skMatrix[SkMatrix::kMScaleY]),
                                        SkScalarToFloat(skMatrix[SkMatrix::kMTransX]),
                                        SkScalarToFloat(skMatrix[SkMatrix::kMTransY]));

    mFont       = CTFontCreateCopyWithAttributes(ctFont, 0.0, &mTransform, NULL);
    mGlyphCount = (uint16_t) numGlyphs;
}

SkScalerContext_Mac::~SkScalerContext_Mac(void)
{

    // Clean up
    CFSafeRelease(mColorSpace);
    CFSafeRelease(mFont);
}

unsigned SkScalerContext_Mac::generateGlyphCount(void) const
{
    return(mGlyphCount);
}

uint16_t SkScalerContext_Mac::generateCharToGlyph(SkUnichar uni)
{   CGGlyph     cgGlyph;
    UniChar     theChar;


    // Validate our parameters and state
    SkASSERT(uni             <= 0x0000FFFF);
    SkASSERT(sizeof(CGGlyph) <= sizeof(uint16_t));


    // Get the glyph
    theChar = (UniChar) uni;

    if (!CTFontGetGlyphsForCharacters(mFont, &theChar, &cgGlyph, 1))
        cgGlyph = 0;

    return(cgGlyph);
}

void SkScalerContext_Mac::generateAdvance(SkGlyph* glyph)
{
    this->generateMetrics(glyph);
}

void SkScalerContext_Mac::generateMetrics(SkGlyph* glyph)
{   CGSize      theAdvance;
    CGRect      theBounds;
    CGGlyph     cgGlyph;



    // Get the state we need
    cgGlyph = (CGGlyph) glyph->getGlyphID(fBaseGlyphCount);

    CTFontGetBoundingRectsForGlyphs(mFont, kCTFontDefaultOrientation, &cgGlyph, &theBounds,  1);
    CTFontGetAdvancesForGlyphs(     mFont, kCTFontDefaultOrientation, &cgGlyph, &theAdvance, 1);



    // Adjust the bounds
    //
    // CTFontGetBoundingRectsForGlyphs ignores the font transform, so we need
    // to transform the bounding box ourselves.
    //
    // The bounds are also expanded by 1 pixel, to give CG room for anti-aliasing.
    theBounds = CGRectApplyAffineTransform(theBounds, mTransform);
    theBounds = CGRectInset(theBounds, -1, -1);



    // Get the metrics
    glyph->zeroMetrics();
    glyph->fAdvanceX =  SkFloatToFixed(theAdvance.width);
    glyph->fAdvanceY = -SkFloatToFixed(theAdvance.height);
    glyph->fWidth    =  sk_float_round2int(theBounds.size.width);
    glyph->fHeight   =  sk_float_round2int(theBounds.size.height);
    glyph->fTop      = -sk_float_round2int(CGRectGetMaxY(theBounds));
    glyph->fLeft     =  sk_float_round2int(CGRectGetMinX(theBounds));
}

void SkScalerContext_Mac::generateImage(const SkGlyph& glyph)
{   CGContextRef        cgContext;
    CGGlyph             cgGlyph;
    CGFontRef           cgFont;


    // Get the state we need
    sk_bzero(glyph.fImage, glyph.fHeight * glyph.rowBytes());

    cgGlyph   = (CGGlyph) glyph.getGlyphID(fBaseGlyphCount);
    cgFont    = CTFontCopyGraphicsFont(mFont, NULL);
    cgContext = CGBitmapContextCreate(  glyph.fImage, glyph.fWidth, glyph.fHeight, 8,
                                        glyph.rowBytes(), mColorSpace, kCGImageAlphaNone);


    // Draw the glyph
    if (cgFont != NULL && cgContext != NULL)
        {
        CGContextSetGrayFillColor(  cgContext, 1.0, 1.0);
        CGContextSetTextDrawingMode(cgContext, kCGTextFill);
        CGContextSetFont(           cgContext, cgFont);
        CGContextSetFontSize(       cgContext, 1.0);
        CGContextSetTextMatrix(     cgContext, mTransform);
        CGContextShowGlyphsAtPoint( cgContext, -glyph.fLeft, glyph.fTop + glyph.fHeight, &cgGlyph, 1);
        }


    // Clean up
    CFSafeRelease(cgFont);
    CFSafeRelease(cgContext);
}

void SkScalerContext_Mac::generatePath(const SkGlyph& glyph, SkPath* path)
{   CGGlyph     cgGlyph;
    CGPathRef   cgPath;


    // Get the state we need
    cgGlyph = (CGGlyph) glyph.getGlyphID(fBaseGlyphCount);
    cgPath  = CTFontCreatePathForGlyph(mFont, cgGlyph, NULL);


    // Get the path
    path->reset();

    if (cgPath != NULL)
        CGPathApply(cgPath, path, SkScalerContext_Mac::CTPathElement);

    CFSafeRelease(cgPath);
}

void SkScalerContext_Mac::generateFontMetrics(SkPaint::FontMetrics* mx, SkPaint::FontMetrics* my)
{   SkPaint::FontMetrics        theMetrics;
    CGRect                      theBounds;


    // Get the state we need
    theBounds = CTFontGetBoundingBox(mFont);


    // Get the metrics
    theMetrics.fTop          = -CGRectGetMaxY(theBounds);
    theMetrics.fAscent       = -CTFontGetAscent(mFont);
    theMetrics.fDescent      =  CTFontGetDescent(mFont);
    theMetrics.fBottom       = -CGRectGetMinY(theBounds);
    theMetrics.fLeading      =  CTFontGetLeading(mFont);
    theMetrics.fAvgCharWidth =  CGRectGetWidth(theBounds);
    theMetrics.fXMin         =  CGRectGetMinX(theBounds);
    theMetrics.fXMax         =  CGRectGetMaxX(theBounds);
    theMetrics.fXHeight      =  CTFontGetXHeight(mFont);


    // Return the metrics
    SkASSERT(theMetrics.fTop          <= 0.0);
    SkASSERT(theMetrics.fAscent       <= 0.0);
    SkASSERT(theMetrics.fDescent      >= 0.0);
    SkASSERT(theMetrics.fBottom       >= 0.0);
    SkASSERT(theMetrics.fLeading      >= 0.0);
    SkASSERT(theMetrics.fAvgCharWidth >= 0.0);
    SkASSERT(theMetrics.fXMin         <= 0.0);
    SkASSERT(theMetrics.fXMax         >  0.0);
    SkASSERT(theMetrics.fXHeight      >= 0.0);

    if (mx != NULL)
        *mx = theMetrics;
    
    if (my != NULL)
        *my = theMetrics;
}

void SkScalerContext_Mac::CTPathElement(void *info, const CGPathElement *element)
{   SkPath      *skPath = (SkPath *) info;


    // Process the path element
    switch (element->type) {
        case kCGPathElementMoveToPoint:
            skPath->moveTo( element->points[0].x, -element->points[0].y);
            break;

        case kCGPathElementAddLineToPoint:
            skPath->lineTo( element->points[0].x, -element->points[0].y);
            break;

        case kCGPathElementAddQuadCurveToPoint:
            skPath->quadTo( element->points[0].x, -element->points[0].y,
                            element->points[1].x, -element->points[1].y);
            break;

        case kCGPathElementAddCurveToPoint:
            skPath->cubicTo(element->points[0].x, -element->points[0].y,
                            element->points[1].x, -element->points[1].y,
                            element->points[2].x, -element->points[2].y);
            break;

        case kCGPathElementCloseSubpath:
            skPath->close();
            break;
        
        default:
            SkASSERT("Unknown path element!");
            break;
        }
}





///////////////////////////////////////////////////////////////////////////
#pragma mark -

SkTypeface* SkFontHost::CreateTypeface(const SkTypeface* familyFace,
                            const char familyName[],
                            const void* data, size_t bytelength,
                            SkTypeface::Style style)
{   SkTypeface              *theTypeface;
    SkNativeFontCache       *fontTable;
    SkNativeFontInfo        fontInfo;
    SkString                fontName;


    // Get the state we need
    fontName  = SkString(familyName);
    fontTable = SkNativeFontCache::Get();


    // Clone an existing typeface
    if (familyName == NULL && familyFace != NULL)
        {
        familyFace->ref();
        return(const_cast<SkTypeface*>(familyFace));
        }


    // Get the native font
    fontInfo = fontTable->GetFontInfo(fontName, style);
    if (fontInfo.fontID == kSkInvalidFontID)
        fontInfo = fontTable->CreateFont(fontName, style);


    // Create the typeface
    theTypeface = new SkTypeface_Mac(fontInfo.style, fontInfo.fontID);
    return(theTypeface);
}

SkTypeface* SkFontHost::CreateTypefaceFromStream(SkStream* stream)
{
    SkASSERT(!"SkFontHost::CreateTypefaceFromStream unimplemented");
    return(NULL);
}

SkTypeface* SkFontHost::CreateTypefaceFromFile(const char path[])
{
    SkASSERT(!"SkFontHost::CreateTypefaceFromFile unimplemented");
    return(NULL);
}

///////////////////////////////////////////////////////////////////////////

bool SkFontHost::ValidFontID(SkFontID uniqueID)
{

    // Check the font ID
    return(SkNativeFontCache::Get()->IsValid(uniqueID));
}

SkStream* SkFontHost::OpenStream(SkFontID uniqueID)
{
    SkASSERT(!"SkFontHost::OpenStream unimplemented");
    return(NULL);
}

size_t SkFontHost::GetFileName(SkFontID fontID, char path[], size_t length, int32_t* index)
{
    SkASSERT(!"SkFontHost::GetFileName unimplemented");
    return(0);
}

///////////////////////////////////////////////////////////////////////////

void SkFontHost::Serialize(const SkTypeface* face, SkWStream* stream)
{
    SkASSERT(!"SkFontHost::Serialize unimplemented");
}

SkTypeface* SkFontHost::Deserialize(SkStream* stream)
{
    SkASSERT(!"SkFontHost::Deserialize unimplemented");
    return(NULL);
}

///////////////////////////////////////////////////////////////////////////

SkScalerContext* SkFontHost::CreateScalerContext(const SkDescriptor* desc)
{
    return new SkScalerContext_Mac(desc);
}

uint32_t SkFontHost::NextLogicalFont(uint32_t fontID)
{   SkTypeface      *typeFace;
    uint32_t        newFontID;


    // Get the state we need
    newFontID = kSkInvalidFontID;
    typeFace  = CreateTypeface(NULL, FONT_DEFAULT_NAME, NULL, 0, SkTypeface::kNormal);

    if (typeFace == NULL)
        return(0);


    // Get the next font
    //
    // When we're passed in the default font, we've reached the end.
    newFontID = typeFace->uniqueID();
    if (newFontID == fontID)
        newFontID = 0;


    // Clean up
    typeFace->unref();

    return(newFontID);
}

void SkFontHost::FilterRec(SkScalerContext::Rec* rec)
{
    // we only support 2 levels of hinting
    SkPaint::Hinting h = rec->getHinting();
    if (SkPaint::kSlight_Hinting == h) {
        h = SkPaint::kNo_Hinting;
    } else if (SkPaint::kFull_Hinting == h) {
        h = SkPaint::kNormal_Hinting;
    }
    rec->setHinting(h);

    // we don't support LCD text
    if (SkMask::FormatIsLCD((SkMask::Format)rec->fMaskFormat)) {
        rec->fMaskFormat = SkMask::kA8_Format;
    }
}

///////////////////////////////////////////////////////////////////////////

size_t SkFontHost::ShouldPurgeFontCache(size_t sizeAllocatedSoFar)
{
    if (sizeAllocatedSoFar > FONT_CACHE_MEMORY_BUDGET)
        return sizeAllocatedSoFar - FONT_CACHE_MEMORY_BUDGET;
    else
        return 0;   // nothing to do
}

int SkFontHost::ComputeGammaFlag(const SkPaint& paint)
{
    return 0;
}

void SkFontHost::GetGammaTables(const uint8_t* tables[2])
{
    tables[0] = NULL;   // black gamma (e.g. exp=1.4)
    tables[1] = NULL;   // white gamma (e.g. exp= 1/1.4)
}

///////////////////////////////////////////////////////////////////////////

void SkFontHost::SetSubpixelOrientation(SkFontHost::LCDOrientation orientation)
{
    SkASSERT(!"SkFontHost::SetSubpixelOrientation unimplemented");
}

SkFontHost::LCDOrientation SkFontHost::GetSubpixelOrientation(void)
{
    SkASSERT(!"SkFontHost::GetSubpixelOrientation unimplemented");
    return kHorizontal_LCDOrientation;
}

void SkFontHost::SetSubpixelOrder(SkFontHost::LCDOrder order)
{
    SkASSERT(!"SkFontHost::SetSubpixelOrder unimplemented");
}

SkFontHost::LCDOrder SkFontHost::GetSubpixelOrder(void)
{
    SkASSERT(!"SkFontHost::GetSubpixelOrder unimplemented");
    return kRGB_LCDOrder;
}

///////////////////////////////////////////////////////////////////////////

int SkFontHost::CountTables(SkFontID fontID)
{   int             numTables;
    CFArrayRef      cfArray;
    CTFontRef       ctFont;


    // Get the state we need
    ctFont    = SkNativeFontCache::Get()->GetFont(fontID);
    cfArray   = CTFontCopyAvailableTables(ctFont, kCTFontTableOptionNoOptions);
    numTables = 0;


    // Get the table count
    if (cfArray != NULL)
        {
        numTables = CFArrayGetCount(cfArray);
        CFSafeRelease(cfArray);
        }

    return(numTables);
}

int SkFontHost::GetTableTags(SkFontID fontID, SkFontTableTag tags[])
{   int             n, numTables;
    CFArrayRef      cfArray;
    CTFontRef       ctFont;


    // Get the state we need
    ctFont    = SkNativeFontCache::Get()->GetFont(fontID);
    cfArray   = CTFontCopyAvailableTables(ctFont, kCTFontTableOptionNoOptions);
    numTables = 0;


    // Get the table tags
    if (cfArray != NULL)
        {
        numTables = CFArrayGetCount(cfArray);
        for (n = 0; n < numTables; n++)
            tags[n] = (SkFontTableTag) ((uintptr_t) CFArrayGetValueAtIndex(cfArray, n));

        CFSafeRelease(cfArray);
        }

    return(numTables);
}

size_t SkFontHost::GetTableSize(SkFontID fontID, SkFontTableTag tag)
{   size_t      theSize;
    CTFontRef   ctFont;
    CFDataRef   cfData;


    // Get the state we need
    ctFont  = SkNativeFontCache::Get()->GetFont(fontID);
    cfData  = CTFontCopyTable(ctFont, (CTFontTableTag) tag, kCTFontTableOptionNoOptions);
    theSize = 0;


    // Get the data size
    if (cfData != NULL)
        {
        theSize = CFDataGetLength(cfData);
        CFSafeRelease(cfData);
        }
    
    return(theSize);
}

size_t SkFontHost::GetTableData(SkFontID fontID, SkFontTableTag tag,
                                size_t offset, size_t length, void* data)
{   size_t          theSize;
    CTFontRef       ctFont;
    CFDataRef       cfData;


    // Get the state we need
    ctFont  = SkNativeFontCache::Get()->GetFont(fontID);
    cfData  = CTFontCopyTable(ctFont, (CTFontTableTag) tag, kCTFontTableOptionNoOptions);
    theSize = 0;


    // Get the data
    if (cfData != NULL)
        theSize = CFDataGetLength(cfData);

    if (offset >= theSize)
        return 0;

    if ((offset + length) > theSize)
        length = theSize - offset;

    memcpy(data, CFDataGetBytePtr(cfData) + offset, length);
    return(length);
}




