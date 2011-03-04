#include "SkScalerContext.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkDescriptor.h"
#include "SkFDot6.h"
#include "SkFontHost.h"
#include "SkMask.h"
#include "SkStream.h"
#include "SkString.h"
#include "SkThread.h"
#include "SkTemplates.h"

#include <acaapi.h>

//////////////////////////////////////////////////////////////////////////

#include "SkMMapStream.h"

class SkScalerContext_Ascender : public SkScalerContext {
public:
    SkScalerContext_Ascender(const SkDescriptor* desc);
    virtual ~SkScalerContext_Ascender();

protected:
    virtual unsigned generateGlyphCount();
    virtual uint16_t generateCharToGlyph(SkUnichar uni);
    virtual void generateMetrics(SkGlyph* glyph);
    virtual void generateImage(const SkGlyph& glyph);
    virtual void generatePath(const SkGlyph& glyph, SkPath* path);
    virtual void generateFontMetrics(SkPaint::FontMetrics* mx, SkPaint::FontMetrics* my);

private:
    aca_FontHandle  fHandle;
    void*   fWorkspace;
    void*   fGlyphWorkspace;
    SkStream*   fFontStream;
    SkStream*   fHintStream;
};

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

SkScalerContext_Ascender::SkScalerContext_Ascender(const SkDescriptor* desc)
    : SkScalerContext(desc)
{
    int size = aca_Get_FontHandleRec_Size();
    fHandle = (aca_FontHandle)sk_malloc_throw(size);
    
    // get the pointer to the font
    
    fFontStream = new SkMMAPStream("/UcsGB2312-Hei-H.FDL");
    fHintStream = new SkMMAPStream("/genv6-23.bin");
    
    void* hints = sk_malloc_throw(fHintStream->getLength());
    memcpy(hints, fHintStream->getMemoryBase(), fHintStream->getLength());
    
    aca_Create_Font_Handle(fHandle,
                           (void*)fFontStream->getMemoryBase(), fFontStream->getLength(),
                           "fred",
                           hints, fHintStream->getLength());
    
    // compute our factors from the record

    SkMatrix    m;

    fRec.getSingleMatrix(&m);

    //  now compute our scale factors
    SkScalar    sx = m.getScaleX();
    SkScalar    sy = m.getScaleY();
    
    int ppemX = SkScalarRound(sx);
    int ppemY = SkScalarRound(sy);
    
    size = aca_Find_Font_Memory_Required(fHandle, ppemX, ppemY);
    size *= 8;  // Jeff suggests this :)
    fWorkspace = sk_malloc_throw(size);
    aca_Set_Font_Memory(fHandle, (uint8_t*)fWorkspace, size);

    aca_GlyphAttribsRec rec;
    
    memset(&rec, 0, sizeof(rec));
    rec.xSize = ppemX;
    rec.ySize = ppemY;
    rec.doAdjust = true;
    rec.doExceptions = true;
    rec.doGlyphHints = true;
    rec.doInterpolate = true;
    rec.grayMode = 2;
    aca_Set_Font_Attributes(fHandle, &rec, &size);
    
    fGlyphWorkspace = sk_malloc_throw(size);
    aca_Set_Glyph_Memory(fHandle, fGlyphWorkspace);
}

SkScalerContext_Ascender::~SkScalerContext_Ascender()
{
    delete fHintStream;
    delete fFontStream;
    sk_free(fGlyphWorkspace);
    sk_free(fWorkspace);
    sk_free(fHandle);
}

unsigned SkScalerContext_Ascender::generateGlyphCount()
{
    return 1000;
}

uint16_t SkScalerContext_Ascender::generateCharToGlyph(SkUnichar uni)
{
    return (uint16_t)(uni & 0xFFFF);
}

void SkScalerContext_Ascender::generateMetrics(SkGlyph* glyph)
{
    glyph->fRsbDelta = 0;
    glyph->fLsbDelta = 0;
    
    aca_GlyphImageRec   rec;
    aca_Vector          topLeft;
    
    int adv = aca_Get_Adv_Width(fHandle, glyph->getGlyphID());
    if (aca_GLYPH_NOT_PRESENT == adv)
        goto ERROR;

    aca_Rasterize(glyph->getGlyphID(), fHandle, &rec, &topLeft);

    if (false)  // error
    {
ERROR:
        glyph->fWidth   = 0;
        glyph->fHeight  = 0;
        glyph->fTop     = 0;
        glyph->fLeft    = 0;
        glyph->fAdvanceX = 0;
        glyph->fAdvanceY = 0;
        return;
    }
    
    glyph->fWidth = rec.width;
    glyph->fHeight = rec.rows;
    glyph->fRowBytes = rec.width;
    glyph->fTop = -topLeft.y;
    glyph->fLeft = topLeft.x;
    glyph->fAdvanceX = SkIntToFixed(adv);
    glyph->fAdvanceY = SkIntToFixed(0);
}

void SkScalerContext_Ascender::generateImage(const SkGlyph& glyph)
{
    aca_GlyphImageRec   rec;
    aca_Vector          topLeft;
    
    aca_Rasterize(glyph.getGlyphID(), fHandle, &rec, &topLeft);
    
    const uint8_t* src = (const uint8_t*)rec.buffer;
    uint8_t* dst = (uint8_t*)glyph.fImage;
    int height = glyph.fHeight;
    
    src += rec.y0 * rec.pitch + rec.x0;
    while (--height >= 0)
    {
        memcpy(dst, src, glyph.fWidth);
        src += rec.pitch;
        dst += glyph.fRowBytes;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////

void SkScalerContext_Ascender::generatePath(const SkGlyph& glyph, SkPath* path)
{
    SkRect r;
    
    r.set(0, 0, SkIntToScalar(4), SkIntToScalar(4));
    path->reset();
    path->addRect(r);
}

void SkScalerContext_Ascender::generateFontMetrics(SkPaint::FontMetrics* mx, SkPaint::FontMetrics* my)
{
    if (NULL == mx && NULL == my)
        return;

    if (mx)
    {
        mx->fTop = SkIntToScalar(-16);
        mx->fAscent = SkIntToScalar(-16);
        mx->fDescent = SkIntToScalar(4);
        mx->fBottom = SkIntToScalar(4);
        mx->fLeading = 0;

        // FIXME:
        mx->fAvgCharWidth = 0;
        mx->fXMin = 0;
        mx->fXMax = 0;
        mx->fXHeight = 0;
    }
    if (my)
    {
        my->fTop = SkIntToScalar(-16);
        my->fAscent = SkIntToScalar(-16);
        my->fDescent = SkIntToScalar(4);
        my->fBottom = SkIntToScalar(4);
        my->fLeading = 0;

        // FIXME:
        my->fAvgCharWidth = 0;
        my->fXMin = 0;
        my->fXMax = 0;
        my->fXHeight = 0;
    }
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

SkScalerContext* SkFontHost::CreateScalerContext(const SkDescriptor* desc)
{
    return SkNEW_ARGS(SkScalerContext_Ascender, (desc));
}

