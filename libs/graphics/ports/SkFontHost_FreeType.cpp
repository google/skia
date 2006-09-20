#include "SkScalerContext.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkDescriptor.h"
#include "SkFDot6.h"
#include "SkFontHost.h"
#include "SkString.h"
#include "SkThread.h"
#include "SkTemplates.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H
#include FT_SIZES_H

//#define ENABLE_GLYPH_SPEW     // for tracing calls to generateMetrics/generateImage

#ifdef SK_DEBUG
    #define SkASSERT_CONTINUE(pred)                                                         \
        do {                                                                                \
            if (!(pred))                                                                    \
                SkDebugf("file %s:%d: assert failed '" #pred "'\n", __FILE__, __LINE__);    \
        } while (false)
#else
    #define SkASSERT_CONTINUE(pred)
#endif

//////////////////////////////////////////////////////////////////////////

struct SkFaceRec;

static SkMutex		gFTMutex;
static int			gFTCount;
static FT_Library	gFTLibrary;
static SkFaceRec*	gFaceRecHead;

/////////////////////////////////////////////////////////////////////////

class SkScalerContext_FreeType : public SkScalerContext {
public:
	SkScalerContext_FreeType(const SkDescriptor* desc);
	virtual ~SkScalerContext_FreeType();

protected:
	virtual void generateMetrics(SkGlyph* glyph);
	virtual void generateImage(const SkGlyph& glyph);
	virtual void generatePath(const SkGlyph& glyph, SkPath* path);
	virtual void generateLineHeight(SkPoint* ascent, SkPoint* descent);

private:
    SkFaceRec*  fFaceRec;
	FT_Face     fFace;              // reference to shared face in gFaceRecHead
    FT_Size     fFTSize;            // our own copy
	SkFixed     fScaleX, fScaleY;
	SkFixed     fMatrix22[2][2];
    uint32_t    fLoadGlyphFlags;

	FT_Error setupSize();
};

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

#include "SkStream.h"

struct SkFaceRec {
	SkFaceRec*      fNext;
	FT_Face         fFace;
	FT_StreamRec    fFTStream;
	SkFILEStream    fSkStream;
	uint32_t        fRefCnt;
	SkString        fFileName;

	SkFaceRec(const char filename[]);    
    ~SkFaceRec()
    {
//        SkDEBUGF(("~SkFaceRec: closing %s\n", fFileName.c_str()));
    }
};

extern "C" {
    static unsigned long sk_stream_read(FT_Stream       stream,
                                        unsigned long   offset,
                                        unsigned char*  buffer,
                                        unsigned long   count )
    {
        SkFaceRec* rec = (SkFaceRec*)stream->descriptor.pointer;
        SkStream* str = &rec->fSkStream;

        if (count)
        {
            if (!str->rewind())
            {
                SkDEBUGF(("sk_stream_read(file:%s offset:%ld count:%ld): rewind failed\n",
				rec->fFileName.c_str(), offset, count));
                return 0;
            }
            else
            {
                unsigned long ret;
                if (offset)
                {
                    ret = str->read(nil, offset);
                    if (ret != offset) {
                        SkDEBUGF(("sk_stream_read(file:%s offset:%d) read returned %d\n",
                                  rec->fFileName.c_str(), offset, ret));
                        return 0;
                    }
                }
                ret = str->read(buffer, count);
                if (ret != count) {
                    SkDEBUGF(("sk_stream_read(file:%s offset:%d count:%d) read returned %d\n",
                             rec->fFileName.c_str(), offset, count, ret));
                    return 0;
                }
                count = ret;
            }
        }
        return count;
    }

    static void sk_stream_close( FT_Stream stream)
    {
    }
}

SkFaceRec::SkFaceRec(const char filename[]) : fSkStream(filename)
{
//    SkDEBUGF(("SkFaceRec: opening %s (%p)\n", filename, fSkStream.getSkFILE()));

    fFileName.set(filename);

    memset(&fFTStream, 0, sizeof(fFTStream));
    fFTStream.size = fSkStream.read(nil, 0);   // find out how big the file is
    fFTStream.descriptor.pointer = this;
    fFTStream.read	= sk_stream_read;
    fFTStream.close = sk_stream_close;
}

static SkFaceRec* ref_ft_face(const char filename[], size_t filenameLen)
{
	SkFaceRec* rec = gFaceRecHead;
	while (rec)
	{
		if (rec->fFileName.equals(filename, filenameLen))
		{
			SkASSERT(rec->fFace);
			rec->fRefCnt += 1;
			return rec;
		}
		rec = rec->fNext;
	}

	rec = SkNEW_ARGS(SkFaceRec, (filename));

    FT_Open_Args	args;
    memset(&args, 0, sizeof(args));
    args.flags = FT_OPEN_STREAM;
    args.stream = &rec->fFTStream;

	FT_Error err = FT_Open_Face(gFTLibrary, &args, 0, &rec->fFace);

	if (err)	// bad filename, try the default font
	{
		fprintf(stderr, "ERROR: unable to open font '%s'\n", filename);
		SkDELETE(rec);
		sk_throw();
		return 0;
	}
	else
	{
		SkASSERT(rec->fFace);
		//fprintf(stderr, "Opened font '%s'\n", filename.c_str());
		rec->fNext = gFaceRecHead;
		gFaceRecHead = rec;
		rec->fRefCnt = 1;
		return rec;
	}
}

static void unref_ft_face(FT_Face face)
{
	SkFaceRec*	rec = gFaceRecHead;
	SkFaceRec*	prev = nil;
	while (rec)
	{
		SkFaceRec* next = rec->fNext;
		if (rec->fFace == face)
		{
			if (--rec->fRefCnt == 0)
			{
				if (prev)
					prev->fNext = next;
				else
					gFaceRecHead = next;

				FT_Done_Face(face);
				SkDELETE(rec);
			}
			return;
		}
		prev = rec;
		rec = next;
	}
	SkASSERT("shouldn't get here, face not in list");
}

///////////////////////////////////////////////////////////////////////////

struct FontFaceRec;
extern void FontFaceRec_getFileName(const FontFaceRec*, SkString*);

SkScalerContext_FreeType::SkScalerContext_FreeType(const SkDescriptor* desc)
	: SkScalerContext(desc), fFTSize(nil)
{
    SkAutoMutexAcquire	ac(gFTMutex);

	FT_Error	err;

    if (gFTCount == 0)
    {
        err = FT_Init_FreeType(&gFTLibrary);
//        SkDEBUGF(("FT_Init_FreeType returned %d\n", err));
        SkASSERT(err == 0);
    }
    ++gFTCount;

    // load the font file
    {
        SkString     fileName;
        const FontFaceRec** face = (const FontFaceRec**)desc->findEntry(kTypeface_SkDescriptorTag, nil);
        FontFaceRec_getFileName(*face, &fileName);
        fFaceRec = ref_ft_face(fileName.c_str(), fileName.size());
        fFace = fFaceRec ? fFaceRec->fFace : nil;
    }

	// compute our factors from the record

	SkMatrix	m;

	fRec.getSingleMatrix(&m);

	//	now compute our scale factors
	SkScalar	sx = m.getScaleX();
	SkScalar	sy = m.getScaleY();

	if (m.getSkewX() || m.getSkewY() || sx < 0 || sy < 0)	// sort of give up on hinting
	{
		sx = SkMaxScalar(SkScalarAbs(sx), SkScalarAbs(m.getSkewX()));
		sy = SkMaxScalar(SkScalarAbs(m.getSkewY()), SkScalarAbs(sy));
		sx = sy = SkScalarAve(sx, sy);

		SkScalar inv = SkScalarInvert(sx);

		// flip the skew elements to go from our Y-down system to FreeType's
		fMatrix22[0][0] = SkScalarToFixed(SkScalarMul(m.getScaleX(), inv));
		fMatrix22[0][1] = -SkScalarToFixed(SkScalarMul(m.getSkewX(), inv));
		fMatrix22[1][0] = -SkScalarToFixed(SkScalarMul(m.getSkewY(), inv));
		fMatrix22[1][1] = SkScalarToFixed(SkScalarMul(m.getScaleY(), inv));
	}
	else
	{
		fMatrix22[0][0] = fMatrix22[1][1] = SK_Fixed1;
		fMatrix22[0][1] = fMatrix22[1][0] = 0;
	}

	fScaleX = SkScalarToFixed(sx);
	fScaleY = SkScalarToFixed(sy);

    // compute the flags we send to Load_Glyph
    {
        uint32_t flags = 0;

        if (!fRec.fUseHints) {
            flags |= FT_LOAD_NO_HINTING;
        }
        if (!fRec.fDoAA) {
            flags |= FT_LOAD_TARGET_MONO;
        }
        fLoadGlyphFlags = flags;
    }
    
    // now create the FT_Size

    {    
        FT_Error	err;

        err = FT_New_Size(fFace, &fFTSize);
        if (err != 0) {
            SkDEBUGF(("SkScalerContext_FreeType::FT_New_Size(%s): FT_Set_Char_Size(0x%x, 0x%x) returned 0x%x\n",
                        fFaceRec->fFileName.c_str(), fScaleX, fScaleY, err));
            fFace = nil;
            return;
        }
        
        err = FT_Activate_Size(fFTSize);
        if (err != 0) {
            SkDEBUGF(("SkScalerContext_FreeType::FT_Activate_Size(%s, 0x%x, 0x%x) returned 0x%x\n",
                        fFaceRec->fFileName.c_str(), fScaleX, fScaleY, err));
            fFTSize = nil;
        }

        err = FT_Set_Char_Size(	fFace,
                                SkFixedToFDot6(fScaleX), SkFixedToFDot6(fScaleY),
                                72, 72);
        if (err != 0) {
            SkDEBUGF(("SkScalerContext_FreeType::FT_Set_Char_Size(%s, 0x%x, 0x%x) returned 0x%x\n",
                        fFaceRec->fFileName.c_str(), fScaleX, fScaleY, err));
            fFace = nil;
            return;
        }

//        SkDEBUGF(("FT_Set_Transform %p %p %p [%x %x %x %x]\n", this, fFace, fFTSize,
//                    fMatrix22[0][0], fMatrix22[0][1], fMatrix22[1][0], fMatrix22[1][1]));
        // not sure if I need this here, given that I redo it in setup()
        FT_Set_Transform( fFace, (FT_Matrix*)&fMatrix22, nil);
    }
}

SkScalerContext_FreeType::~SkScalerContext_FreeType()
{
    if (fFTSize != nil)
        FT_Done_Size(fFTSize);

	SkAutoMutexAcquire	ac(gFTMutex);

    if (fFace != nil)
        unref_ft_face(fFace);

	if (--gFTCount == 0)
	{
//        SkDEBUGF(("FT_Done_FreeType\n"));
		FT_Done_FreeType(gFTLibrary);
		SkDEBUGCODE(gFTLibrary = nil;)
	}
}

/*	We call this before each use of the fFace, since we may be sharing
	this face with other context (at different sizes).
*/
FT_Error SkScalerContext_FreeType::setupSize()
{
	FT_Error	err = FT_Activate_Size(fFTSize);

    if (err != 0) {
        SkDEBUGF(("SkScalerContext_FreeType::FT_Activate_Size(%s, 0x%x, 0x%x) returned 0x%x\n",
                    fFaceRec->fFileName.c_str(), fScaleX, fScaleY, err));
        fFTSize = nil;
    }
    else
    {
        // seems we need to reset this every time (not sure why, but without it
        // I get random italics from some other fFTSize)
        FT_Set_Transform( fFace, (FT_Matrix*)&fMatrix22, nil);
    }
    return err;
}

void SkScalerContext_FreeType::generateMetrics(SkGlyph* glyph)
{
#if 0
    {
        static bool gOnce = true;
        if (gOnce) {
            gOnce = false;
            for (int i = 403; i < fFace->num_glyphs; i++) {
                FT_Error err = FT_Load_Glyph( fFace, i, fLoadGlyphFlags );
                if (err != 0) {
                    printf("FT_Load_Glyph[%d] returned %d\n", i, err);
                }
            }
        }
    }
#endif

    SkAutoMutexAcquire	ac(gFTMutex);

	FT_Error	err;

	if (this->setupSize())
        goto ERROR;

	glyph->fGlyphID = SkToU16(FT_Get_Char_Index( fFace, glyph->fCharCode ));

	err = FT_Load_Glyph( fFace, glyph->fGlyphID, fLoadGlyphFlags );
	if (err != 0)
	{
        SkDEBUGF(("SkScalerContext_FreeType::generateMetrics(%s): FT_Load_Glyph(char:%d glyph:%d flags:%d) returned 0x%x\n",
                    fFaceRec->fFileName.c_str(), glyph->fCharCode, glyph->fGlyphID, fLoadGlyphFlags, err));
    ERROR:
		glyph->fWidth	= 0;
		glyph->fHeight	= 0;
		glyph->fTop		= 0;
		glyph->fLeft	= 0;
		glyph->fAdvanceX = 0;
		glyph->fAdvanceY = 0;
        return;
	}
    
    switch ( fFace->glyph->format )
    {
      case FT_GLYPH_FORMAT_OUTLINE:
        FT_BBox bbox;
        
        FT_Outline_Get_CBox(&fFace->glyph->outline, &bbox);
        bbox.xMin &= ~63;
        bbox.yMin &= ~63;
        bbox.xMax  = (bbox.xMax + 63) & ~63;
        bbox.yMax  = (bbox.yMax + 63) & ~63;

        glyph->fWidth	= SkToU16((bbox.xMax - bbox.xMin) >> 6);
        glyph->fHeight	= SkToU16((bbox.yMax - bbox.yMin) >> 6);
        glyph->fTop		= -SkToS16(bbox.yMax >> 6);
        glyph->fLeft	= SkToS16(bbox.xMin >> 6);
        break;
        
      case FT_GLYPH_FORMAT_BITMAP:
        glyph->fWidth	= SkToU16(fFace->glyph->bitmap.width);
        glyph->fHeight	= SkToU16(fFace->glyph->bitmap.rows);
        glyph->fTop		= -SkToS16(fFace->glyph->bitmap_top);
        glyph->fLeft	= SkToS16(fFace->glyph->bitmap_left);
        break;

      default:
        SkASSERT(!"unknown glyph format");
        goto ERROR;      
    }
    
	if (fRec.fUseHints)
	{
		glyph->fAdvanceX = SkFDot6ToFixed(fFace->glyph->advance.x);
		glyph->fAdvanceY = -SkFDot6ToFixed(fFace->glyph->advance.y);
	}
	else
	{
		glyph->fAdvanceX = SkFixedMul(fMatrix22[0][0], fFace->glyph->linearHoriAdvance);
		glyph->fAdvanceY = -SkFixedMul(fMatrix22[1][0], fFace->glyph->linearHoriAdvance);
	}

#ifdef ENABLE_GLYPH_SPEW
    SkDEBUGF(("FT_Set_Char_Size(this:%p sx:%x sy:%x ", this, fScaleX, fScaleY));
    SkDEBUGF(("Metrics(glyph:%d flags:0x%x) w:%d\n", glyph->fGlyphID, fLoadGlyphFlags, glyph->fWidth));
#endif
}

void SkScalerContext_FreeType::generateImage(const SkGlyph& glyph)
{
    SkAutoMutexAcquire	ac(gFTMutex);

	FT_Error	err;

	if (this->setupSize())
        goto ERROR;

	err = FT_Load_Glyph( fFace, glyph.fGlyphID, fLoadGlyphFlags);
    if (err != 0) {
        SkDEBUGF(("SkScalerContext_FreeType::generateImage: FT_Load_Glyph(char:%d glyph:%d width:%d height:%d rb:%d flags:%d) returned 0x%x\n",
                    glyph.fCharCode, glyph.fGlyphID, glyph.fWidth, glyph.fHeight, glyph.fRowBytes, fLoadGlyphFlags, err));
    ERROR:
        memset(glyph.fImage, 0, glyph.fRowBytes * glyph.fHeight);
        return;
    }

    switch ( fFace->glyph->format ) {
    case FT_GLYPH_FORMAT_OUTLINE:
        {
            FT_Outline* outline = &fFace->glyph->outline;
            FT_BBox     bbox;
            FT_Bitmap   target;
            
            FT_Outline_Get_CBox(outline, &bbox);
            FT_Outline_Translate(outline, -(bbox.xMin & ~63), -(bbox.yMin & ~63));

            target.width = glyph.fWidth;
            target.rows = glyph.fHeight;
            target.pitch = glyph.fRowBytes;
            target.buffer = reinterpret_cast<uint8_t*>(glyph.fImage);
            target.pixel_mode = fRec.fDoAA ? FT_PIXEL_MODE_GRAY : FT_PIXEL_MODE_MONO;
            target.num_grays = 256;
            
            memset(glyph.fImage, 0, glyph.fRowBytes * glyph.fHeight);
            FT_Outline_Get_Bitmap(gFTLibrary, outline, &target);
        }
        break;
    
    case FT_GLYPH_FORMAT_BITMAP:
        SkASSERT_CONTINUE(glyph.fWidth == fFace->glyph->bitmap.width);
        SkASSERT_CONTINUE(glyph.fHeight == fFace->glyph->bitmap.rows);
        SkASSERT_CONTINUE(glyph.fTop == -fFace->glyph->bitmap_top);
        SkASSERT_CONTINUE(glyph.fLeft == fFace->glyph->bitmap_left);

        {
            const U8*	src = (const U8*)fFace->glyph->bitmap.buffer;
            U8*			dst = (U8*)glyph.fImage;
            unsigned	srcRowBytes = fFace->glyph->bitmap.pitch;
            unsigned	dstRowBytes = glyph.fRowBytes;
            unsigned    minRowBytes = SkMin32(srcRowBytes, dstRowBytes);
            unsigned    extraRowBytes = dstRowBytes - minRowBytes;

            for (int y = fFace->glyph->bitmap.rows - 1; y >= 0; --y)
            {
                memcpy(dst, src, minRowBytes);
                memset(dst + minRowBytes, 0, extraRowBytes);
                src += srcRowBytes;
                dst += dstRowBytes;
            }
        }
        break;
    
    default:
        SkASSERT(!"unknown glyph format");
        goto ERROR;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////

#define ft2sk(x)	SkFixedToScalar((x) << 10)

static int move_proc(FT_Vector* pt, void* ctx)
{
	SkPath* path = (SkPath*)ctx;
	path->close();	// to close the previous contour (if any)
	path->moveTo(ft2sk(pt->x), -ft2sk(pt->y));
	return 0;
}

static int line_proc(FT_Vector* pt, void* ctx)
{
	SkPath* path = (SkPath*)ctx;
	path->lineTo(ft2sk(pt->x), -ft2sk(pt->y));
	return 0;
}

static int quad_proc(FT_Vector* pt0, FT_Vector* pt1, void* ctx)
{
	SkPath* path = (SkPath*)ctx;
	path->quadTo(ft2sk(pt0->x), -ft2sk(pt0->y), ft2sk(pt1->x), -ft2sk(pt1->y));
	return 0;
}

static int cubic_proc(FT_Vector* pt0, FT_Vector* pt1, FT_Vector* pt2, void* ctx)
{
	SkPath* path = (SkPath*)ctx;
	path->cubicTo(ft2sk(pt0->x), -ft2sk(pt0->y), ft2sk(pt1->x), -ft2sk(pt1->y), ft2sk(pt2->x), -ft2sk(pt2->y));
	return 0;
}

void SkScalerContext_FreeType::generatePath(const SkGlyph& glyph, SkPath* path)
{
    SkAutoMutexAcquire	ac(gFTMutex);

	SkASSERT(&glyph && path);

	if (this->setupSize()) {
        path->reset();
        return;
    }

	U32 flags = fLoadGlyphFlags;
    flags |= FT_LOAD_NO_BITMAP; // ignore embedded bitmaps so we're sure to get the outline
    flags &= ~FT_LOAD_RENDER;   // don't scan convert (we just want the outline)

	FT_Error err = FT_Load_Glyph( fFace, glyph.fGlyphID, flags);

    if (err != 0) {
        SkDEBUGF(("SkScalerContext_FreeType::generatePath: FT_Load_Glyph(char:%d glyph:%d flags:%d) returned 0x%x\n",
                    glyph.fCharCode, glyph.fGlyphID, flags, err));
        path->reset();
        return;
    }

	FT_Outline_Funcs	funcs;

	funcs.move_to	= move_proc;
	funcs.line_to	= line_proc;
	funcs.conic_to	= quad_proc;
	funcs.cubic_to	= cubic_proc;
	funcs.shift		= 0;
	funcs.delta		= 0;

	err = FT_Outline_Decompose(&fFace->glyph->outline, &funcs, path);

    if (err != 0) {
        SkDEBUGF(("SkScalerContext_FreeType::generatePath: FT_Load_Glyph(char:%d glyph:%d flags:%d) returned 0x%x\n",
                    glyph.fCharCode, glyph.fGlyphID, flags, err));
        path->reset();
        return;
    }

	path->close();
}

static void map_y_to_pt(const FT_Matrix& mat, SkFixed y, SkPoint* pt)
{
	SkFixed x = SkFixedMul(mat.xy, y);
	y = SkFixedMul(mat.yy, y);

	pt->set(SkFixedToScalar(x), SkFixedToScalar(y));
}

void SkScalerContext_FreeType::generateLineHeight(SkPoint* ascent, SkPoint* descent)
{
    SkAutoMutexAcquire	ac(gFTMutex);

	if (this->setupSize()) {
        if (ascent)
            ascent->set(0, 0);
        if (descent)
            descent->set(0, 0);
        return;
    }

	if (ascent)
	{
		SkFixed a = SkMulDiv(fScaleY, -fFace->ascender, fFace->units_per_EM);
		map_y_to_pt(*(FT_Matrix*)fMatrix22, a, ascent);
	}
	if (descent)
	{
		SkFixed d = SkMulDiv(fScaleY, -fFace->descender, fFace->units_per_EM);
		map_y_to_pt(*(FT_Matrix*)fMatrix22, d, descent);
	}
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

SkScalerContext* SkFontHost::CreateScalerContext(const SkDescriptor* desc)
{
	return SkNEW_ARGS(SkScalerContext_FreeType, (desc));
}

