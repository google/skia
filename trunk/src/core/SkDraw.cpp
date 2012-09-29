
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkDraw.h"
#include "SkBlitter.h"
#include "SkBounder.h"
#include "SkCanvas.h"
#include "SkColorPriv.h"
#include "SkDevice.h"
#include "SkFixed.h"
#include "SkMaskFilter.h"
#include "SkPaint.h"
#include "SkPathEffect.h"
#include "SkRasterClip.h"
#include "SkRasterizer.h"
#include "SkScan.h"
#include "SkShader.h"
#include "SkStroke.h"
#include "SkTemplatesPriv.h"
#include "SkTLazy.h"
#include "SkUtils.h"

#include "SkAutoKern.h"
#include "SkBitmapProcShader.h"
#include "SkDrawProcs.h"

//#define TRACE_BITMAP_DRAWS

#define kBlitterStorageLongCount    (sizeof(SkBitmapProcShader) >> 2)

/** Helper for allocating small blitters on the stack.
 */
class SkAutoBlitterChoose : SkNoncopyable {
public:
    SkAutoBlitterChoose() {
        fBlitter = NULL;
    }
    SkAutoBlitterChoose(const SkBitmap& device, const SkMatrix& matrix,
                        const SkPaint& paint) {
        fBlitter = SkBlitter::Choose(device, matrix, paint,
                                     fStorage, sizeof(fStorage));
    }

    ~SkAutoBlitterChoose();

    SkBlitter*  operator->() { return fBlitter; }
    SkBlitter*  get() const { return fBlitter; }

    void choose(const SkBitmap& device, const SkMatrix& matrix,
                const SkPaint& paint) {
        SkASSERT(!fBlitter);
        fBlitter = SkBlitter::Choose(device, matrix, paint,
                                     fStorage, sizeof(fStorage));
    }

private:
    SkBlitter*  fBlitter;
    uint32_t    fStorage[kBlitterStorageLongCount];
};

SkAutoBlitterChoose::~SkAutoBlitterChoose() {
    if ((void*)fBlitter == (void*)fStorage) {
        fBlitter->~SkBlitter();
    } else {
        SkDELETE(fBlitter);
    }
}

/**
 *  Since we are providing the storage for the shader (to avoid the perf cost
 *  of calling new) we insist that in our destructor we can account for all
 *  owners of the shader.
 */
class SkAutoBitmapShaderInstall : SkNoncopyable {
public:
    SkAutoBitmapShaderInstall(const SkBitmap& src, const SkPaint& paint)
            : fPaint(paint) /* makes a copy of the paint */ {
        fPaint.setShader(SkShader::CreateBitmapShader(src,
                           SkShader::kClamp_TileMode, SkShader::kClamp_TileMode,
                           fStorage, sizeof(fStorage)));
        // we deliberately left the shader with an owner-count of 2
        SkASSERT(2 == fPaint.getShader()->getRefCnt());
    }

    ~SkAutoBitmapShaderInstall() {
        SkShader* shader = fPaint.getShader();
        // since we manually destroy shader, we insist that owners == 2
        SkASSERT(2 == shader->getRefCnt());

        fPaint.setShader(NULL); // unref the shader by 1

        // now destroy to take care of the 2nd owner-count
        if ((void*)shader == (void*)fStorage) {
            shader->~SkShader();
        } else {
            SkDELETE(shader);
        }
    }

    // return the new paint that has the shader applied
    const SkPaint& paintWithShader() const { return fPaint; }

private:
    SkPaint     fPaint; // copy of caller's paint (which we then modify)
    uint32_t    fStorage[kBlitterStorageLongCount];
};

///////////////////////////////////////////////////////////////////////////////

SkDraw::SkDraw() {
    sk_bzero(this, sizeof(*this));
}

SkDraw::SkDraw(const SkDraw& src) {
    memcpy(this, &src, sizeof(*this));
}

///////////////////////////////////////////////////////////////////////////////

typedef void (*BitmapXferProc)(void* pixels, size_t bytes, uint32_t data);

static void D_Clear_BitmapXferProc(void* pixels, size_t bytes, uint32_t) {
    sk_bzero(pixels, bytes);
}

static void D_Dst_BitmapXferProc(void*, size_t, uint32_t data) {}

static void D32_Src_BitmapXferProc(void* pixels, size_t bytes, uint32_t data) {
    sk_memset32((uint32_t*)pixels, data, bytes >> 2);
}

static void D16_Src_BitmapXferProc(void* pixels, size_t bytes, uint32_t data) {
    sk_memset16((uint16_t*)pixels, data, bytes >> 1);
}

static void DA8_Src_BitmapXferProc(void* pixels, size_t bytes, uint32_t data) {
    memset(pixels, data, bytes);
}

static BitmapXferProc ChooseBitmapXferProc(const SkBitmap& bitmap,
                                           const SkPaint& paint,
                                           uint32_t* data) {
    // todo: we can apply colorfilter up front if no shader, so we wouldn't
    // need to abort this fastpath
    if (paint.getShader() || paint.getColorFilter()) {
        return NULL;
    }

    SkXfermode::Mode mode;
    if (!SkXfermode::AsMode(paint.getXfermode(), &mode)) {
        return NULL;
    }

    SkColor color = paint.getColor();

    // collaps modes based on color...
    if (SkXfermode::kSrcOver_Mode == mode) {
        unsigned alpha = SkColorGetA(color);
        if (0 == alpha) {
            mode = SkXfermode::kDst_Mode;
        } else if (0xFF == alpha) {
            mode = SkXfermode::kSrc_Mode;
        }
    }

    switch (mode) {
        case SkXfermode::kClear_Mode:
//            SkDebugf("--- D_Clear_BitmapXferProc\n");
            return D_Clear_BitmapXferProc;  // ignore data
        case SkXfermode::kDst_Mode:
//            SkDebugf("--- D_Dst_BitmapXferProc\n");
            return D_Dst_BitmapXferProc;    // ignore data
        case SkXfermode::kSrc_Mode: {
            /*
                should I worry about dithering for the lower depths?
            */
            SkPMColor pmc = SkPreMultiplyColor(color);
            switch (bitmap.config()) {
                case SkBitmap::kARGB_8888_Config:
                    if (data) {
                        *data = pmc;
                    }
//                    SkDebugf("--- D32_Src_BitmapXferProc\n");
                    return D32_Src_BitmapXferProc;
                case SkBitmap::kARGB_4444_Config:
                    if (data) {
                        *data = SkPixel32ToPixel4444(pmc);
                    }
//                    SkDebugf("--- D16_Src_BitmapXferProc\n");
                    return D16_Src_BitmapXferProc;
                case SkBitmap::kRGB_565_Config:
                    if (data) {
                        *data = SkPixel32ToPixel16(pmc);
                    }
//                    SkDebugf("--- D16_Src_BitmapXferProc\n");
                    return D16_Src_BitmapXferProc;
                case SkBitmap::kA8_Config:
                    if (data) {
                        *data = SkGetPackedA32(pmc);
                    }
//                    SkDebugf("--- DA8_Src_BitmapXferProc\n");
                    return DA8_Src_BitmapXferProc;
                default:
                    break;
            }
            break;
        }
        default:
            break;
    }
    return NULL;
}

static void CallBitmapXferProc(const SkBitmap& bitmap, const SkIRect& rect,
                               BitmapXferProc proc, uint32_t procData) {
    int shiftPerPixel;
    switch (bitmap.config()) {
        case SkBitmap::kARGB_8888_Config:
            shiftPerPixel = 2;
            break;
        case SkBitmap::kARGB_4444_Config:
        case SkBitmap::kRGB_565_Config:
            shiftPerPixel = 1;
            break;
        case SkBitmap::kA8_Config:
            shiftPerPixel = 0;
            break;
        default:
            SkDEBUGFAIL("Can't use xferproc on this config");
            return;
    }

    uint8_t* pixels = (uint8_t*)bitmap.getPixels();
    SkASSERT(pixels);
    const size_t rowBytes = bitmap.rowBytes();
    const int widthBytes = rect.width() << shiftPerPixel;

    // skip down to the first scanline and X position
    pixels += rect.fTop * rowBytes + (rect.fLeft << shiftPerPixel);
    for (int scans = rect.height() - 1; scans >= 0; --scans) {
        proc(pixels, widthBytes, procData);
        pixels += rowBytes;
    }
}

void SkDraw::drawPaint(const SkPaint& paint) const {
    SkDEBUGCODE(this->validate();)

    if (fRC->isEmpty()) {
        return;
    }

    SkIRect    devRect;
    devRect.set(0, 0, fBitmap->width(), fBitmap->height());
    if (fBounder && !fBounder->doIRect(devRect)) {
        return;
    }

    if (fRC->isBW()) {
        /*  If we don't have a shader (i.e. we're just a solid color) we may
            be faster to operate directly on the device bitmap, rather than invoking
            a blitter. Esp. true for xfermodes, which require a colorshader to be
            present, which is just redundant work. Since we're drawing everywhere
            in the clip, we don't have to worry about antialiasing.
        */
        uint32_t procData = 0;  // to avoid the warning
        BitmapXferProc proc = ChooseBitmapXferProc(*fBitmap, paint, &procData);
        if (proc) {
            if (D_Dst_BitmapXferProc == proc) { // nothing to do
                return;
            }

            SkRegion::Iterator iter(fRC->bwRgn());
            while (!iter.done()) {
                CallBitmapXferProc(*fBitmap, iter.rect(), proc, procData);
                iter.next();
            }
            return;
        }
    }

    // normal case: use a blitter
    SkAutoBlitterChoose blitter(*fBitmap, *fMatrix, paint);
    SkScan::FillIRect(devRect, *fRC, blitter.get());
}

///////////////////////////////////////////////////////////////////////////////

struct PtProcRec {
    SkCanvas::PointMode fMode;
    const SkPaint*  fPaint;
    const SkRegion* fClip;
    const SkRasterClip* fRC;

    // computed values
    SkFixed fRadius;

    typedef void (*Proc)(const PtProcRec&, const SkPoint devPts[], int count,
                         SkBlitter*);

    bool init(SkCanvas::PointMode, const SkPaint&, const SkMatrix* matrix,
              const SkRasterClip*);
    Proc chooseProc(SkBlitter** blitter);

private:
    SkAAClipBlitterWrapper fWrapper;
};

static void bw_pt_rect_hair_proc(const PtProcRec& rec, const SkPoint devPts[],
                                 int count, SkBlitter* blitter) {
    SkASSERT(rec.fClip->isRect());
    const SkIRect& r = rec.fClip->getBounds();

    for (int i = 0; i < count; i++) {
        int x = SkScalarFloor(devPts[i].fX);
        int y = SkScalarFloor(devPts[i].fY);
        if (r.contains(x, y)) {
            blitter->blitH(x, y, 1);
        }
    }
}

static void bw_pt_rect_16_hair_proc(const PtProcRec& rec,
                                    const SkPoint devPts[], int count,
                                    SkBlitter* blitter) {
    SkASSERT(rec.fRC->isRect());
    const SkIRect& r = rec.fRC->getBounds();
    uint32_t value;
    const SkBitmap* bitmap = blitter->justAnOpaqueColor(&value);
    SkASSERT(bitmap);

    uint16_t* addr = bitmap->getAddr16(0, 0);
    int rb = bitmap->rowBytes();

    for (int i = 0; i < count; i++) {
        int x = SkScalarFloor(devPts[i].fX);
        int y = SkScalarFloor(devPts[i].fY);
        if (r.contains(x, y)) {
//            *bitmap->getAddr16(x, y) = SkToU16(value);
            ((uint16_t*)((char*)addr + y * rb))[x] = SkToU16(value);
        }
    }
}

static void bw_pt_hair_proc(const PtProcRec& rec, const SkPoint devPts[],
                            int count, SkBlitter* blitter) {
    for (int i = 0; i < count; i++) {
        int x = SkScalarFloor(devPts[i].fX);
        int y = SkScalarFloor(devPts[i].fY);
        if (rec.fClip->contains(x, y)) {
            blitter->blitH(x, y, 1);
        }
    }
}

static void bw_line_hair_proc(const PtProcRec& rec, const SkPoint devPts[],
                              int count, SkBlitter* blitter) {
    for (int i = 0; i < count; i += 2) {
        SkScan::HairLine(devPts[i], devPts[i+1], *rec.fRC, blitter);
    }
}

static void bw_poly_hair_proc(const PtProcRec& rec, const SkPoint devPts[],
                              int count, SkBlitter* blitter) {
    for (int i = 0; i < count - 1; i++) {
        SkScan::HairLine(devPts[i], devPts[i+1], *rec.fRC, blitter);
    }
}

// aa versions

static void aa_line_hair_proc(const PtProcRec& rec, const SkPoint devPts[],
                              int count, SkBlitter* blitter) {
    for (int i = 0; i < count; i += 2) {
        SkScan::AntiHairLine(devPts[i], devPts[i+1], *rec.fRC, blitter);
    }
}

static void aa_poly_hair_proc(const PtProcRec& rec, const SkPoint devPts[],
                              int count, SkBlitter* blitter) {
    for (int i = 0; i < count - 1; i++) {
        SkScan::AntiHairLine(devPts[i], devPts[i+1], *rec.fRC, blitter);
    }
}

// square procs (strokeWidth > 0 but matrix is square-scale (sx == sy)

static void bw_square_proc(const PtProcRec& rec, const SkPoint devPts[],
                           int count, SkBlitter* blitter) {
    const SkFixed radius = rec.fRadius;
    for (int i = 0; i < count; i++) {
        SkFixed x = SkScalarToFixed(devPts[i].fX);
        SkFixed y = SkScalarToFixed(devPts[i].fY);

        SkXRect r;
        r.fLeft = x - radius;
        r.fTop = y - radius;
        r.fRight = x + radius;
        r.fBottom = y + radius;

        SkScan::FillXRect(r, *rec.fRC, blitter);
    }
}

static void aa_square_proc(const PtProcRec& rec, const SkPoint devPts[],
                           int count, SkBlitter* blitter) {
    const SkFixed radius = rec.fRadius;
    for (int i = 0; i < count; i++) {
        SkFixed x = SkScalarToFixed(devPts[i].fX);
        SkFixed y = SkScalarToFixed(devPts[i].fY);

        SkXRect r;
        r.fLeft = x - radius;
        r.fTop = y - radius;
        r.fRight = x + radius;
        r.fBottom = y + radius;

        SkScan::AntiFillXRect(r, *rec.fRC, blitter);
    }
}

// If this guy returns true, then chooseProc() must return a valid proc
bool PtProcRec::init(SkCanvas::PointMode mode, const SkPaint& paint,
                     const SkMatrix* matrix, const SkRasterClip* rc) {
    if (paint.getPathEffect()) {
        return false;
    }
    SkScalar width = paint.getStrokeWidth();
    if (0 == width) {
        fMode = mode;
        fPaint = &paint;
        fClip = NULL;
        fRC = rc;
        fRadius = SK_Fixed1 >> 1;
        return true;
    }
    if (paint.getStrokeCap() != SkPaint::kRound_Cap &&
            matrix->rectStaysRect() && SkCanvas::kPoints_PointMode == mode) {
        SkScalar sx = matrix->get(SkMatrix::kMScaleX);
        SkScalar sy = matrix->get(SkMatrix::kMScaleY);
        if (SkScalarNearlyZero(sx - sy)) {
            if (sx < 0) {
                sx = -sx;
            }

            fMode = mode;
            fPaint = &paint;
            fClip = NULL;
            fRC = rc;
            fRadius = SkScalarToFixed(SkScalarMul(width, sx)) >> 1;
            return true;
        }
    }
    return false;
}

PtProcRec::Proc PtProcRec::chooseProc(SkBlitter** blitterPtr) {
    Proc proc = NULL;

    SkBlitter* blitter = *blitterPtr;
    if (fRC->isBW()) {
        fClip = &fRC->bwRgn();
    } else {
        fWrapper.init(*fRC, blitter);
        fClip = &fWrapper.getRgn();
        blitter = fWrapper.getBlitter();
        *blitterPtr = blitter;
    }

    // for our arrays
    SkASSERT(0 == SkCanvas::kPoints_PointMode);
    SkASSERT(1 == SkCanvas::kLines_PointMode);
    SkASSERT(2 == SkCanvas::kPolygon_PointMode);
    SkASSERT((unsigned)fMode <= (unsigned)SkCanvas::kPolygon_PointMode);

    // first check for hairlines
    if (0 == fPaint->getStrokeWidth()) {
        if (fPaint->isAntiAlias()) {
            static const Proc gAAProcs[] = {
                aa_square_proc, aa_line_hair_proc, aa_poly_hair_proc
            };
            proc = gAAProcs[fMode];
        } else {
            if (SkCanvas::kPoints_PointMode == fMode && fClip->isRect()) {
                uint32_t value;
                const SkBitmap* bm = blitter->justAnOpaqueColor(&value);
                if (bm && bm->config() == SkBitmap::kRGB_565_Config) {
                    proc = bw_pt_rect_16_hair_proc;
                } else {
                    proc = bw_pt_rect_hair_proc;
                }
            } else {
                static Proc gBWProcs[] = {
                    bw_pt_hair_proc, bw_line_hair_proc, bw_poly_hair_proc
                };
                proc = gBWProcs[fMode];
            }
        }
    } else if (fPaint->getStrokeCap() != SkPaint::kRound_Cap) {
        SkASSERT(SkCanvas::kPoints_PointMode == fMode);
        if (fPaint->isAntiAlias()) {
            proc = aa_square_proc;
        } else {
            proc = bw_square_proc;
        }
    }
    return proc;
}

static bool bounder_points(SkBounder* bounder, SkCanvas::PointMode mode,
                           size_t count, const SkPoint pts[],
                           const SkPaint& paint, const SkMatrix& matrix) {
    SkIRect ibounds;
    SkRect bounds;
    SkScalar inset = paint.getStrokeWidth();

    bounds.set(pts, count);
    bounds.inset(-inset, -inset);
    matrix.mapRect(&bounds);

    bounds.roundOut(&ibounds);
    return bounder->doIRect(ibounds);
}

// each of these costs 8-bytes of stack space, so don't make it too large
// must be even for lines/polygon to work
#define MAX_DEV_PTS     32

void SkDraw::drawPoints(SkCanvas::PointMode mode, size_t count,
                        const SkPoint pts[], const SkPaint& paint,
                        bool forceUseDevice) const {
    // if we're in lines mode, force count to be even
    if (SkCanvas::kLines_PointMode == mode) {
        count &= ~(size_t)1;
    }

    if ((long)count <= 0) {
        return;
    }

    SkASSERT(pts != NULL);
    SkDEBUGCODE(this->validate();)

     // nothing to draw
    if (fRC->isEmpty()) {
        return;
    }

    if (fBounder) {
        if (!bounder_points(fBounder, mode, count, pts, paint, *fMatrix)) {
            return;
        }

        // clear the bounder and call this again, so we don't invoke the bounder
        // later if we happen to call ourselves for drawRect, drawPath, etc.
        SkDraw noBounder(*this);
        noBounder.fBounder = NULL;
        noBounder.drawPoints(mode, count, pts, paint, forceUseDevice);
        return;
    }

    PtProcRec rec;
    if (!forceUseDevice && rec.init(mode, paint, fMatrix, fRC)) {
        SkAutoBlitterChoose blitter(*fBitmap, *fMatrix, paint);

        SkPoint             devPts[MAX_DEV_PTS];
        const SkMatrix*     matrix = fMatrix;
        SkBlitter*          bltr = blitter.get();
        PtProcRec::Proc     proc = rec.chooseProc(&bltr);
        // we have to back up subsequent passes if we're in polygon mode
        const size_t backup = (SkCanvas::kPolygon_PointMode == mode);

        do {
            size_t n = count;
            if (n > MAX_DEV_PTS) {
                n = MAX_DEV_PTS;
            }
            matrix->mapPoints(devPts, pts, n);
            proc(rec, devPts, n, bltr);
            pts += n - backup;
            SkASSERT(count >= n);
            count -= n;
            if (count > 0) {
                count += backup;
            }
        } while (count != 0);
    } else {
        switch (mode) {
            case SkCanvas::kPoints_PointMode: {
                // temporarily mark the paint as filling.
                SkPaint newPaint(paint);
                newPaint.setStyle(SkPaint::kFill_Style);

                SkScalar width = newPaint.getStrokeWidth();
                SkScalar radius = SkScalarHalf(width);

                if (newPaint.getStrokeCap() == SkPaint::kRound_Cap) {
                    SkPath      path;
                    SkMatrix    preMatrix;

                    path.addCircle(0, 0, radius);
                    for (size_t i = 0; i < count; i++) {
                        preMatrix.setTranslate(pts[i].fX, pts[i].fY);
                        // pass true for the last point, since we can modify
                        // then path then
                        if (fDevice) {
                            fDevice->drawPath(*this, path, newPaint, &preMatrix,
                                              (count-1) == i);
                        } else {
                            this->drawPath(path, newPaint, &preMatrix,
                                           (count-1) == i);
                        }
                    }
                } else {
                    SkRect  r;

                    for (size_t i = 0; i < count; i++) {
                        r.fLeft = pts[i].fX - radius;
                        r.fTop = pts[i].fY - radius;
                        r.fRight = r.fLeft + width;
                        r.fBottom = r.fTop + width;
                        if (fDevice) {
                            fDevice->drawRect(*this, r, newPaint);
                        } else {
                            this->drawRect(r, newPaint);
                        }
                    }
                }
                break;
            }
            case SkCanvas::kLines_PointMode:
            case SkCanvas::kPolygon_PointMode: {
                count -= 1;
                SkPath path;
                SkPaint p(paint);
                p.setStyle(SkPaint::kStroke_Style);
                size_t inc = (SkCanvas::kLines_PointMode == mode) ? 2 : 1;
                for (size_t i = 0; i < count; i += inc) {
                    path.moveTo(pts[i]);
                    path.lineTo(pts[i+1]);
                    if (fDevice) {
                        fDevice->drawPath(*this, path, p, NULL, true);
                    } else {
                        this->drawPath(path, p, NULL, true);
                    }
                    path.rewind();
                }
                break;
            }
        }
    }
}

static inline SkPoint* as_lefttop(SkRect* r) {
    return (SkPoint*)(void*)r;
}

static inline SkPoint* as_rightbottom(SkRect* r) {
    return ((SkPoint*)(void*)r) + 1;
}

static bool easy_rect_join(const SkPaint& paint, const SkMatrix& matrix,
                           SkPoint* strokeSize) {
    if (SkPaint::kMiter_Join != paint.getStrokeJoin() ||
        paint.getStrokeMiter() < SK_ScalarSqrt2) {
        return false;
    }

    SkASSERT(matrix.rectStaysRect());
    SkPoint pt = { paint.getStrokeWidth(), paint.getStrokeWidth() };
    matrix.mapVectors(strokeSize, &pt, 1);
    strokeSize->fX = SkScalarAbs(strokeSize->fX);
    strokeSize->fY = SkScalarAbs(strokeSize->fY);
    return true;
}

SkDraw::RectType SkDraw::ComputeRectType(const SkPaint& paint,
                                         const SkMatrix& matrix,
                                         SkPoint* strokeSize) {
    RectType rtype;
    const SkScalar width = paint.getStrokeWidth();
    const bool zeroWidth = (0 == width);
    SkPaint::Style style = paint.getStyle();

    if ((SkPaint::kStrokeAndFill_Style == style) && zeroWidth) {
        style = SkPaint::kFill_Style;
    }

    if (paint.getPathEffect() || paint.getMaskFilter() ||
        paint.getRasterizer() || !matrix.rectStaysRect() ||
        SkPaint::kStrokeAndFill_Style == style) {
        rtype = kPath_RectType;
    } else if (SkPaint::kFill_Style == style) {
        rtype = kFill_RectType;
    } else if (zeroWidth) {
        rtype = kHair_RectType;
    } else if (easy_rect_join(paint, matrix, strokeSize)) {
        rtype = kStroke_RectType;
    } else {
        rtype = kPath_RectType;
    }
    return rtype;
}

static const SkPoint* rect_points(const SkRect& r) {
    return (const SkPoint*)(void*)&r;
}

static SkPoint* rect_points(SkRect& r) {
    return (SkPoint*)(void*)&r;
}

void SkDraw::drawRect(const SkRect& rect, const SkPaint& paint) const {
    SkDEBUGCODE(this->validate();)

    // nothing to draw
    if (fRC->isEmpty()) {
        return;
    }

    SkPoint strokeSize;
    RectType rtype = ComputeRectType(paint, *fMatrix, &strokeSize);

#ifdef SK_DISABLE_FAST_AA_STROKE_RECT
    if (kStroke_RectType == rtype && paint.isAntiAlias()) {
        rtype = kPath_RectType;
    }
#endif

    if (kPath_RectType == rtype) {
        SkPath  tmp;
        tmp.addRect(rect);
        tmp.setFillType(SkPath::kWinding_FillType);
        this->drawPath(tmp, paint, NULL, true);
        return;
    }

    const SkMatrix& matrix = *fMatrix;
    SkRect          devRect;

    // transform rect into devRect
    matrix.mapPoints(rect_points(devRect), rect_points(rect), 2);
    devRect.sort();

    if (fBounder && !fBounder->doRect(devRect, paint)) {
        return;
    }

    // look for the quick exit, before we build a blitter
    if (true) {
        SkIRect ir;
        devRect.roundOut(&ir);
        if (paint.getStyle() != SkPaint::kFill_Style) {
            // extra space for hairlines
            ir.inset(-1, -1);
        }
        if (fRC->quickReject(ir))
            return;
    }

    SkAutoBlitterChoose blitterStorage(*fBitmap, matrix, paint);
    const SkRasterClip& clip = *fRC;
    SkBlitter*          blitter = blitterStorage.get();

    // we want to "fill" if we are kFill or kStrokeAndFill, since in the latter
    // case we are also hairline (if we've gotten to here), which devolves to
    // effectively just kFill
    switch (rtype) {
        case kFill_RectType:
            if (paint.isAntiAlias()) {
                SkScan::AntiFillRect(devRect, clip, blitter);
            } else {
                SkScan::FillRect(devRect, clip, blitter);
            }
            break;
        case kStroke_RectType:
            if (paint.isAntiAlias()) {
                SkScan::AntiFrameRect(devRect, strokeSize, clip, blitter);
            } else {
                SkScan::FrameRect(devRect, strokeSize, clip, blitter);
            }
            break;
        case kHair_RectType:
            if (paint.isAntiAlias()) {
                SkScan::AntiHairRect(devRect, clip, blitter);
            } else {
                SkScan::HairRect(devRect, clip, blitter);
            }
            break;
        default:
            SkDEBUGFAIL("bad rtype");
    }
}

void SkDraw::drawDevMask(const SkMask& srcM, const SkPaint& paint) const {
    if (srcM.fBounds.isEmpty()) {
        return;
    }

    const SkMask* mask = &srcM;

    SkMask dstM;
    if (paint.getMaskFilter() &&
            paint.getMaskFilter()->filterMask(&dstM, srcM, *fMatrix, NULL)) {
        mask = &dstM;
    } else {
        dstM.fImage = NULL;
    }
    SkAutoMaskFreeImage ami(dstM.fImage);

    if (fBounder && !fBounder->doIRect(mask->fBounds)) {
        return;
    }

    SkAutoBlitterChoose blitterChooser(*fBitmap, *fMatrix, paint);
    SkBlitter* blitter = blitterChooser.get();

    SkAAClipBlitterWrapper wrapper;
    const SkRegion* clipRgn;

    if (fRC->isBW()) {
        clipRgn = &fRC->bwRgn();
    } else {
        wrapper.init(*fRC, blitter);
        clipRgn = &wrapper.getRgn();
        blitter = wrapper.getBlitter();
    }
    blitter->blitMaskRegion(*mask, *clipRgn);
}

static SkScalar fast_len(const SkVector& vec) {
    SkScalar x = SkScalarAbs(vec.fX);
    SkScalar y = SkScalarAbs(vec.fY);
    if (x < y) {
        SkTSwap(x, y);
    }
    return x + SkScalarHalf(y);
}

static bool xfermodeSupportsCoverageAsAlpha(SkXfermode* xfer) {
    SkXfermode::Coeff dc;
    if (!SkXfermode::AsCoeff(xfer, NULL, &dc)) {
        return false;
    }

    switch (dc) {
        case SkXfermode::kOne_Coeff:
        case SkXfermode::kISA_Coeff:
        case SkXfermode::kISC_Coeff:
            return true;
        default:
            return false;
    }
}

bool SkDrawTreatAsHairline(const SkPaint& paint, const SkMatrix& matrix,
                           SkScalar* coverage) {
    SkASSERT(coverage);
    if (SkPaint::kStroke_Style != paint.getStyle()) {
        return false;
    }
    SkScalar strokeWidth = paint.getStrokeWidth();
    if (0 == strokeWidth) {
        *coverage = SK_Scalar1;
        return true;
    }

    // if we get here, we need to try to fake a thick-stroke with a modulated
    // hairline

    if (!paint.isAntiAlias()) {
        return false;
    }
    if (matrix.hasPerspective()) {
        return false;
    }

    SkVector src[2], dst[2];
    src[0].set(strokeWidth, 0);
    src[1].set(0, strokeWidth);
    matrix.mapVectors(dst, src, 2);
    SkScalar len0 = fast_len(dst[0]);
    SkScalar len1 = fast_len(dst[1]);
    if (len0 <= SK_Scalar1 && len1 <= SK_Scalar1) {
        *coverage = SkScalarAve(len0, len1);
        return true;
    }
    return false;
}

void SkDraw::drawPath(const SkPath& origSrcPath, const SkPaint& origPaint,
                      const SkMatrix* prePathMatrix, bool pathIsMutable) const {
    SkDEBUGCODE(this->validate();)

    // nothing to draw
    if (fRC->isEmpty()) {
        return;
    }

    SkPath*         pathPtr = (SkPath*)&origSrcPath;
    bool            doFill = true;
    SkPath          tmpPath;
    SkMatrix        tmpMatrix;
    const SkMatrix* matrix = fMatrix;

    if (prePathMatrix) {
        if (origPaint.getPathEffect() || origPaint.getStyle() != SkPaint::kFill_Style ||
                origPaint.getRasterizer()) {
            SkPath* result = pathPtr;

            if (!pathIsMutable) {
                result = &tmpPath;
                pathIsMutable = true;
            }
            pathPtr->transform(*prePathMatrix, result);
            pathPtr = result;
        } else {
            if (!tmpMatrix.setConcat(*matrix, *prePathMatrix)) {
                // overflow
                return;
            }
            matrix = &tmpMatrix;
        }
    }
    // at this point we're done with prePathMatrix
    SkDEBUGCODE(prePathMatrix = (const SkMatrix*)0x50FF8001;)

    const SkPaint* paint = &origPaint;
    SkTLazy<SkPaint> lazyPaint;

    {
        SkScalar coverage;
        if (SkDrawTreatAsHairline(origPaint, *matrix, &coverage)) {
            if (SK_Scalar1 == coverage) {
                lazyPaint.set(origPaint);
                lazyPaint.get()->setStrokeWidth(0);
                paint = lazyPaint.get();
            } else if (xfermodeSupportsCoverageAsAlpha(origPaint.getXfermode())) {
                U8CPU newAlpha;
#if 0
                newAlpha = SkToU8(SkScalarRoundToInt(coverage *
                                                     origPaint.getAlpha()));
#else
                // this is the old technique, which we preserve for now so
                // we don't change previous results (testing)
                // the new way seems fine, its just (a tiny bit) different
                int scale = (int)SkScalarMul(coverage, 256);
                newAlpha = origPaint.getAlpha() * scale >> 8;
#endif
                lazyPaint.set(origPaint);
                lazyPaint.get()->setStrokeWidth(0);
                lazyPaint.get()->setAlpha(newAlpha);
                paint = lazyPaint.get();
            }
        }
    }

    if (paint->getPathEffect() || paint->getStyle() != SkPaint::kFill_Style) {
        doFill = paint->getFillPath(*pathPtr, &tmpPath);
        pathPtr = &tmpPath;
    }

    if (paint->getRasterizer()) {
        SkMask  mask;
        if (paint->getRasterizer()->rasterize(*pathPtr, *matrix,
                            &fRC->getBounds(), paint->getMaskFilter(), &mask,
                            SkMask::kComputeBoundsAndRenderImage_CreateMode)) {
            this->drawDevMask(mask, *paint);
            SkMask::FreeImage(mask.fImage);
        }
        return;
    }

    // avoid possibly allocating a new path in transform if we can
    SkPath* devPathPtr = pathIsMutable ? pathPtr : &tmpPath;

    // transform the path into device space
    pathPtr->transform(*matrix, devPathPtr);

    SkAutoBlitterChoose blitter(*fBitmap, *fMatrix, *paint);

    if (paint->getMaskFilter()) {
        SkPaint::Style style = doFill ? SkPaint::kFill_Style :
            SkPaint::kStroke_Style;
        if (paint->getMaskFilter()->filterPath(*devPathPtr, *fMatrix, *fRC,
                                               fBounder, blitter.get(),
                                               style)) {
            return; // filterPath() called the blitter, so we're done
        }
    }

    if (fBounder && !fBounder->doPath(*devPathPtr, *paint, doFill)) {
        return;
    }

    void (*proc)(const SkPath&, const SkRasterClip&, SkBlitter*);
    if (doFill) {
        if (paint->isAntiAlias()) {
            proc = SkScan::AntiFillPath;
        } else {
            proc = SkScan::FillPath;
        }
    } else {    // hairline
        if (paint->isAntiAlias()) {
            proc = SkScan::AntiHairPath;
        } else {
            proc = SkScan::HairPath;
        }
    }
    proc(*devPathPtr, *fRC, blitter.get());
}

/** For the purposes of drawing bitmaps, if a matrix is "almost" translate
    go ahead and treat it as if it were, so that subsequent code can go fast.
 */
static bool just_translate(const SkMatrix& matrix, const SkBitmap& bitmap) {
    SkMatrix::TypeMask mask = matrix.getType();

    if (mask & (SkMatrix::kAffine_Mask | SkMatrix::kPerspective_Mask)) {
        return false;
    }
    if (mask & SkMatrix::kScale_Mask) {
        SkScalar sx = matrix[SkMatrix::kMScaleX];
        SkScalar sy = matrix[SkMatrix::kMScaleY];
        int w = bitmap.width();
        int h = bitmap.height();
        int sw = SkScalarRound(SkScalarMul(sx, SkIntToScalar(w)));
        int sh = SkScalarRound(SkScalarMul(sy, SkIntToScalar(h)));
        return sw == w && sh == h;
    }
    // if we got here, we're either kTranslate_Mask or identity
    return true;
}

void SkDraw::drawBitmapAsMask(const SkBitmap& bitmap,
                              const SkPaint& paint) const {
    SkASSERT(bitmap.getConfig() == SkBitmap::kA8_Config);

    if (just_translate(*fMatrix, bitmap)) {
        int ix = SkScalarRound(fMatrix->getTranslateX());
        int iy = SkScalarRound(fMatrix->getTranslateY());

        SkMask  mask;
        mask.fBounds.set(ix, iy, ix + bitmap.width(), iy + bitmap.height());
        mask.fFormat = SkMask::kA8_Format;
        mask.fRowBytes = bitmap.rowBytes();
        mask.fImage = bitmap.getAddr8(0, 0);

        this->drawDevMask(mask, paint);
    } else {    // need to xform the bitmap first
        SkRect  r;
        SkMask  mask;

        r.set(0, 0,
              SkIntToScalar(bitmap.width()), SkIntToScalar(bitmap.height()));
        fMatrix->mapRect(&r);
        r.round(&mask.fBounds);

        // set the mask's bounds to the transformed bitmap-bounds,
        // clipped to the actual device
        {
            SkIRect    devBounds;
            devBounds.set(0, 0, fBitmap->width(), fBitmap->height());
            // need intersect(l, t, r, b) on irect
            if (!mask.fBounds.intersect(devBounds)) {
                return;
            }
        }

        mask.fFormat = SkMask::kA8_Format;
        mask.fRowBytes = SkAlign4(mask.fBounds.width());
        size_t size = mask.computeImageSize();
        if (0 == size) {
            // the mask is too big to allocated, draw nothing
            return;
        }

        // allocate (and clear) our temp buffer to hold the transformed bitmap
        SkAutoMalloc    storage(size);
        mask.fImage = (uint8_t*)storage.get();
        memset(mask.fImage, 0, size);

        // now draw our bitmap(src) into mask(dst), transformed by the matrix
        {
            SkBitmap    device;
            device.setConfig(SkBitmap::kA8_Config, mask.fBounds.width(),
                             mask.fBounds.height(), mask.fRowBytes);
            device.setPixels(mask.fImage);

            SkCanvas c(device);
            // need the unclipped top/left for the translate
            c.translate(-SkIntToScalar(mask.fBounds.fLeft),
                        -SkIntToScalar(mask.fBounds.fTop));
            c.concat(*fMatrix);

            // We can't call drawBitmap, or we'll infinitely recurse. Instead
            // we manually build a shader and draw that into our new mask
            SkPaint tmpPaint;
            tmpPaint.setFlags(paint.getFlags());
            SkAutoBitmapShaderInstall install(bitmap, tmpPaint);
            SkRect rr;
            rr.set(0, 0, SkIntToScalar(bitmap.width()),
                   SkIntToScalar(bitmap.height()));
            c.drawRect(rr, install.paintWithShader());
        }
        this->drawDevMask(mask, paint);
    }
}

static bool clipped_out(const SkMatrix& m, const SkRasterClip& c,
                        const SkRect& srcR) {
    SkRect  dstR;
    SkIRect devIR;

    m.mapRect(&dstR, srcR);
    dstR.roundOut(&devIR);
    return c.quickReject(devIR);
}

static bool clipped_out(const SkMatrix& matrix, const SkRasterClip& clip,
                        int width, int height) {
    SkRect  r;
    r.set(0, 0, SkIntToScalar(width), SkIntToScalar(height));
    return clipped_out(matrix, clip, r);
}

static bool clipHandlesSprite(const SkRasterClip& clip, int x, int y,
                              const SkBitmap& bitmap) {
    return clip.isBW() ||
           clip.quickContains(x, y, x + bitmap.width(), y + bitmap.height());
}

void SkDraw::drawBitmap(const SkBitmap& bitmap, const SkMatrix& prematrix,
                        const SkPaint& origPaint) const {
    SkDEBUGCODE(this->validate();)

    // nothing to draw
    if (fRC->isEmpty() ||
            bitmap.width() == 0 || bitmap.height() == 0 ||
            bitmap.getConfig() == SkBitmap::kNo_Config) {
        return;
    }

#ifndef SK_ALLOW_OVER_32K_BITMAPS
    // run away on too-big bitmaps for now (exceed 16.16)
    if (bitmap.width() > 32767 || bitmap.height() > 32767) {
        return;
    }
#endif

    SkPaint paint(origPaint);
    paint.setStyle(SkPaint::kFill_Style);

    SkMatrix matrix;
    if (!matrix.setConcat(*fMatrix, prematrix)) {
        return;
    }

    if (clipped_out(matrix, *fRC, bitmap.width(), bitmap.height())) {
        return;
    }

    if (fBounder && just_translate(matrix, bitmap)) {
        SkIRect ir;
        int32_t ix = SkScalarRound(matrix.getTranslateX());
        int32_t iy = SkScalarRound(matrix.getTranslateY());
        ir.set(ix, iy, ix + bitmap.width(), iy + bitmap.height());
        if (!fBounder->doIRect(ir)) {
            return;
        }
    }

    // only lock the pixels if we passed the clip and bounder tests
    SkAutoLockPixels alp(bitmap);
    // after the lock, check if we are valid
    if (!bitmap.readyToDraw()) {
        return;
    }

    if (bitmap.getConfig() != SkBitmap::kA8_Config &&
            just_translate(matrix, bitmap)) {
        int ix = SkScalarRound(matrix.getTranslateX());
        int iy = SkScalarRound(matrix.getTranslateY());
        if (clipHandlesSprite(*fRC, ix, iy, bitmap)) {
            uint32_t    storage[kBlitterStorageLongCount];
            SkBlitter*  blitter = SkBlitter::ChooseSprite(*fBitmap, paint, bitmap,
                                                ix, iy, storage, sizeof(storage));
            if (blitter) {
                SkAutoTPlacementDelete<SkBlitter>   ad(blitter, storage);

                SkIRect    ir;
                ir.set(ix, iy, ix + bitmap.width(), iy + bitmap.height());

                SkScan::FillIRect(ir, *fRC, blitter);
                return;
            }
        }
    }

    // now make a temp draw on the stack, and use it
    //
    SkDraw draw(*this);
    draw.fMatrix = &matrix;

    if (bitmap.getConfig() == SkBitmap::kA8_Config) {
        draw.drawBitmapAsMask(bitmap, paint);
    } else {
        SkAutoBitmapShaderInstall install(bitmap, paint);

        SkRect  r;
        r.set(0, 0, SkIntToScalar(bitmap.width()),
              SkIntToScalar(bitmap.height()));
        // is this ok if paint has a rasterizer?
        draw.drawRect(r, install.paintWithShader());
    }
}

void SkDraw::drawSprite(const SkBitmap& bitmap, int x, int y,
                        const SkPaint& origPaint) const {
    SkDEBUGCODE(this->validate();)

    // nothing to draw
    if (fRC->isEmpty() ||
            bitmap.width() == 0 || bitmap.height() == 0 ||
            bitmap.getConfig() == SkBitmap::kNo_Config) {
        return;
    }

    SkIRect    bounds;
    bounds.set(x, y, x + bitmap.width(), y + bitmap.height());

    if (fRC->quickReject(bounds)) {
        return; // nothing to draw
    }

    SkPaint paint(origPaint);
    paint.setStyle(SkPaint::kFill_Style);

    if (NULL == paint.getColorFilter() && clipHandlesSprite(*fRC, x, y, bitmap)) {
        uint32_t    storage[kBlitterStorageLongCount];
        SkBlitter*  blitter = SkBlitter::ChooseSprite(*fBitmap, paint, bitmap,
                                                x, y, storage, sizeof(storage));

        if (blitter) {
            SkAutoTPlacementDelete<SkBlitter> ad(blitter, storage);

            if (fBounder && !fBounder->doIRect(bounds)) {
                return;
            }

            SkScan::FillIRect(bounds, *fRC, blitter);
            return;
        }
    }

    SkAutoBitmapShaderInstall install(bitmap, paint);
    const SkPaint& shaderPaint = install.paintWithShader();

    SkMatrix        matrix;
    SkRect          r;

    // get a scalar version of our rect
    r.set(bounds);

    // tell the shader our offset
    matrix.setTranslate(r.fLeft, r.fTop);
    shaderPaint.getShader()->setLocalMatrix(matrix);

    SkDraw draw(*this);
    matrix.reset();
    draw.fMatrix = &matrix;
    // call ourself with a rect
    // is this OK if paint has a rasterizer?
    draw.drawRect(r, shaderPaint);
}

///////////////////////////////////////////////////////////////////////////////

#include "SkScalerContext.h"
#include "SkGlyphCache.h"
#include "SkTextToPathIter.h"
#include "SkUtils.h"

static void measure_text(SkGlyphCache* cache, SkDrawCacheProc glyphCacheProc,
                const char text[], size_t byteLength, SkVector* stopVector) {
    SkFixed     x = 0, y = 0;
    const char* stop = text + byteLength;

    SkAutoKern  autokern;

    while (text < stop) {
        // don't need x, y here, since all subpixel variants will have the
        // same advance
        const SkGlyph& glyph = glyphCacheProc(cache, &text, 0, 0);

        x += autokern.adjust(glyph) + glyph.fAdvanceX;
        y += glyph.fAdvanceY;
    }
    stopVector->set(SkFixedToScalar(x), SkFixedToScalar(y));

    SkASSERT(text == stop);
}

void SkDraw::drawText_asPaths(const char text[], size_t byteLength,
                              SkScalar x, SkScalar y,
                              const SkPaint& paint) const {
    SkDEBUGCODE(this->validate();)

    SkTextToPathIter iter(text, byteLength, paint, true);

    SkMatrix    matrix;
    matrix.setScale(iter.getPathScale(), iter.getPathScale());
    matrix.postTranslate(x, y);

    const SkPath* iterPath;
    SkScalar xpos, prevXPos = 0;

    while (iter.next(&iterPath, &xpos)) {
        matrix.postTranslate(xpos - prevXPos, 0);
        if (iterPath) {
            const SkPaint& pnt = iter.getPaint();
            if (fDevice) {
                fDevice->drawPath(*this, *iterPath, pnt, &matrix, false);
            } else {
                this->drawPath(*iterPath, pnt, &matrix, false);
            }
        }
        prevXPos = xpos;
    }
}

// disable warning : local variable used without having been initialized
#if defined _WIN32 && _MSC_VER >= 1300
#pragma warning ( push )
#pragma warning ( disable : 4701 )
#endif

//////////////////////////////////////////////////////////////////////////////

static void D1G_NoBounder_RectClip(const SkDraw1Glyph& state,
                                   SkFixed fx, SkFixed fy,
                                   const SkGlyph& glyph) {
    int left = SkFixedFloor(fx);
    int top = SkFixedFloor(fy);
    SkASSERT(glyph.fWidth > 0 && glyph.fHeight > 0);
    SkASSERT(NULL == state.fBounder);
    SkASSERT((NULL == state.fClip && state.fAAClip) ||
             (state.fClip && NULL == state.fAAClip && state.fClip->isRect()));

    left += glyph.fLeft;
    top  += glyph.fTop;

    int right   = left + glyph.fWidth;
    int bottom  = top + glyph.fHeight;

    SkMask        mask;
    SkIRect        storage;
    SkIRect*    bounds = &mask.fBounds;

    mask.fBounds.set(left, top, right, bottom);

    // this extra test is worth it, assuming that most of the time it succeeds
    // since we can avoid writing to storage
    if (!state.fClipBounds.containsNoEmptyCheck(left, top, right, bottom)) {
        if (!storage.intersectNoEmptyCheck(mask.fBounds, state.fClipBounds))
            return;
        bounds = &storage;
    }

    uint8_t* aa = (uint8_t*)glyph.fImage;
    if (NULL == aa) {
        aa = (uint8_t*)state.fCache->findImage(glyph);
        if (NULL == aa) {
            return; // can't rasterize glyph
        }
    }

    mask.fRowBytes = glyph.rowBytes();
    mask.fFormat = static_cast<SkMask::Format>(glyph.fMaskFormat);
    mask.fImage = aa;
    state.fBlitter->blitMask(mask, *bounds);
}

static void D1G_NoBounder_RgnClip(const SkDraw1Glyph& state,
                                  SkFixed fx, SkFixed fy,
                                  const SkGlyph& glyph) {
    int left = SkFixedFloor(fx);
    int top = SkFixedFloor(fy);
    SkASSERT(glyph.fWidth > 0 && glyph.fHeight > 0);
    SkASSERT(!state.fClip->isRect());
    SkASSERT(NULL == state.fBounder);

    SkMask  mask;

    left += glyph.fLeft;
    top  += glyph.fTop;

    mask.fBounds.set(left, top, left + glyph.fWidth, top + glyph.fHeight);
    SkRegion::Cliperator clipper(*state.fClip, mask.fBounds);

    if (!clipper.done()) {
        const SkIRect&  cr = clipper.rect();
        const uint8_t*  aa = (const uint8_t*)glyph.fImage;
        if (NULL == aa) {
            aa = (uint8_t*)state.fCache->findImage(glyph);
            if (NULL == aa) {
                return;
            }
        }

        mask.fRowBytes = glyph.rowBytes();
        mask.fFormat = static_cast<SkMask::Format>(glyph.fMaskFormat);
        mask.fImage = (uint8_t*)aa;
        do {
            state.fBlitter->blitMask(mask, cr);
            clipper.next();
        } while (!clipper.done());
    }
}

static void D1G_Bounder(const SkDraw1Glyph& state,
                        SkFixed fx, SkFixed fy,
                        const SkGlyph& glyph) {
    int left = SkFixedFloor(fx);
    int top = SkFixedFloor(fy);
    SkASSERT(glyph.fWidth > 0 && glyph.fHeight > 0);

    SkMask  mask;

    left += glyph.fLeft;
    top  += glyph.fTop;

    mask.fBounds.set(left, top, left + glyph.fWidth, top + glyph.fHeight);
    SkRegion::Cliperator clipper(*state.fClip, mask.fBounds);

    if (!clipper.done()) {
        const SkIRect&  cr = clipper.rect();
        const uint8_t*  aa = (const uint8_t*)glyph.fImage;
        if (NULL == aa) {
            aa = (uint8_t*)state.fCache->findImage(glyph);
            if (NULL == aa) {
                return;
            }
        }

        // we need to pass the origin, which we approximate with our
        // (unadjusted) left,top coordinates (the caller called fixedfloor)
        if (state.fBounder->doIRectGlyph(cr,
                                         left - glyph.fLeft,
                                         top - glyph.fTop, glyph)) {
            mask.fRowBytes = glyph.rowBytes();
            mask.fFormat = static_cast<SkMask::Format>(glyph.fMaskFormat);
            mask.fImage = (uint8_t*)aa;
            do {
                state.fBlitter->blitMask(mask, cr);
                clipper.next();
            } while (!clipper.done());
        }
    }
}

static void D1G_Bounder_AAClip(const SkDraw1Glyph& state,
                               SkFixed fx, SkFixed fy,
                               const SkGlyph& glyph) {
    int left = SkFixedFloor(fx);
    int top = SkFixedFloor(fy);
    SkIRect bounds;
    bounds.set(left, top, left + glyph.fWidth, top + glyph.fHeight);

    if (state.fBounder->doIRectGlyph(bounds, left, top, glyph)) {
        D1G_NoBounder_RectClip(state, fx, fy, glyph);
    }
}

static bool hasCustomD1GProc(const SkDraw& draw) {
    return draw.fProcs && draw.fProcs->fD1GProc;
}

static bool needsRasterTextBlit(const SkDraw& draw) {
    return !hasCustomD1GProc(draw);
}

SkDraw1Glyph::Proc SkDraw1Glyph::init(const SkDraw* draw, SkBlitter* blitter,
                                      SkGlyphCache* cache) {
    fDraw = draw;
    fBounder = draw->fBounder;
    fBlitter = blitter;
    fCache = cache;

    if (hasCustomD1GProc(*draw)) {
        // todo: fix this assumption about clips w/ custom
        fClip = draw->fClip;
        fClipBounds = fClip->getBounds();
        return draw->fProcs->fD1GProc;
    }

    if (draw->fRC->isBW()) {
        fAAClip = NULL;
        fClip = &draw->fRC->bwRgn();
        fClipBounds = fClip->getBounds();
        if (NULL == fBounder) {
            if (fClip->isRect()) {
                return D1G_NoBounder_RectClip;
            } else {
                return D1G_NoBounder_RgnClip;
            }
        } else {
            return D1G_Bounder;
        }
    } else {    // aaclip
        fAAClip = &draw->fRC->aaRgn();
        fClip = NULL;
        fClipBounds = fAAClip->getBounds();
        if (NULL == fBounder) {
            return D1G_NoBounder_RectClip;
        } else {
            return D1G_Bounder_AAClip;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void SkDraw::drawText(const char text[], size_t byteLength,
                      SkScalar x, SkScalar y, const SkPaint& paint) const {
    SkASSERT(byteLength == 0 || text != NULL);

    SkDEBUGCODE(this->validate();)

    // nothing to draw
    if (text == NULL || byteLength == 0 || fRC->isEmpty()) {
        return;
    }

    if (/*paint.isLinearText() ||*/
        (fMatrix->hasPerspective())) {
        this->drawText_asPaths(text, byteLength, x, y, paint);
        return;
    }

    SkDrawCacheProc glyphCacheProc = paint.getDrawCacheProc();

    const SkMatrix* matrix = fMatrix;
    if (hasCustomD1GProc(*this)) {
        // only support the fMVMatrix (for now) for the GPU case, which also
        // sets the fD1GProc
        if (fMVMatrix) {
            matrix = fMVMatrix;
        }
    }

    SkAutoGlyphCache    autoCache(paint, matrix);
    SkGlyphCache*       cache = autoCache.getCache();

    // transform our starting point
    {
        SkPoint loc;
        matrix->mapXY(x, y, &loc);
        x = loc.fX;
        y = loc.fY;
    }

    // need to measure first
    if (paint.getTextAlign() != SkPaint::kLeft_Align) {
        SkVector    stop;

        measure_text(cache, glyphCacheProc, text, byteLength, &stop);

        SkScalar    stopX = stop.fX;
        SkScalar    stopY = stop.fY;

        if (paint.getTextAlign() == SkPaint::kCenter_Align) {
            stopX = SkScalarHalf(stopX);
            stopY = SkScalarHalf(stopY);
        }
        x -= stopX;
        y -= stopY;
    }

    SkFixed fx = SkScalarToFixed(x);
    SkFixed fy = SkScalarToFixed(y);
    const char* stop = text + byteLength;

    SkFixed fxMask = ~0;
    SkFixed fyMask = ~0;
    if (cache->isSubpixel()) {
        SkAxisAlignment baseline = SkComputeAxisAlignmentForHText(*matrix);
        if (kX_SkAxisAlignment == baseline) {
            fyMask = 0;
        } else if (kY_SkAxisAlignment == baseline) {
            fxMask = 0;
        }

    // apply bias here to avoid adding 1/2 the sampling frequency in the loop
        fx += SK_FixedHalf >> SkGlyph::kSubBits;
        fy += SK_FixedHalf >> SkGlyph::kSubBits;
    } else {
        fx += SK_FixedHalf;
        fy += SK_FixedHalf;
    }

    SkAAClipBlitter     aaBlitter;
    SkAutoBlitterChoose blitterChooser;
    SkBlitter*          blitter = NULL;
    if (needsRasterTextBlit(*this)) {
        blitterChooser.choose(*fBitmap, *matrix, paint);
        blitter = blitterChooser.get();
        if (fRC->isAA()) {
            aaBlitter.init(blitter, &fRC->aaRgn());
            blitter = &aaBlitter;
        }
    }

    SkAutoKern          autokern;
    SkDraw1Glyph        d1g;
    SkDraw1Glyph::Proc  proc = d1g.init(this, blitter, cache);

    while (text < stop) {
        const SkGlyph& glyph = glyphCacheProc(cache, &text, fx & fxMask, fy & fyMask);

        fx += autokern.adjust(glyph);

        if (glyph.fWidth) {
            proc(d1g, fx, fy, glyph);
        }
        fx += glyph.fAdvanceX;
        fy += glyph.fAdvanceY;
    }
}

// last parameter is interpreted as SkFixed [x, y]
// return the fixed position, which may be rounded or not by the caller
//   e.g. subpixel doesn't round
typedef void (*AlignProc)(const SkPoint&, const SkGlyph&, SkIPoint*);

static void leftAlignProc(const SkPoint& loc, const SkGlyph& glyph,
                          SkIPoint* dst) {
    dst->set(SkScalarToFixed(loc.fX), SkScalarToFixed(loc.fY));
}

static void centerAlignProc(const SkPoint& loc, const SkGlyph& glyph,
                            SkIPoint* dst) {
    dst->set(SkScalarToFixed(loc.fX) - (glyph.fAdvanceX >> 1),
             SkScalarToFixed(loc.fY) - (glyph.fAdvanceY >> 1));
}

static void rightAlignProc(const SkPoint& loc, const SkGlyph& glyph,
                           SkIPoint* dst) {
    dst->set(SkScalarToFixed(loc.fX) - glyph.fAdvanceX,
             SkScalarToFixed(loc.fY) - glyph.fAdvanceY);
}

static AlignProc pick_align_proc(SkPaint::Align align) {
    static const AlignProc gProcs[] = {
        leftAlignProc, centerAlignProc, rightAlignProc
    };

    SkASSERT((unsigned)align < SK_ARRAY_COUNT(gProcs));

    return gProcs[align];
}

class TextMapState {
public:
    mutable SkPoint fLoc;

    TextMapState(const SkMatrix& matrix, SkScalar y)
        : fMatrix(matrix), fProc(matrix.getMapXYProc()), fY(y) {}

    typedef void (*Proc)(const TextMapState&, const SkScalar pos[]);

    Proc pickProc(int scalarsPerPosition);

private:
    const SkMatrix&     fMatrix;
    SkMatrix::MapXYProc fProc;
    SkScalar            fY; // ignored by MapXYProc
    // these are only used by Only... procs
    SkScalar            fScaleX, fTransX, fTransformedY;

    static void MapXProc(const TextMapState& state, const SkScalar pos[]) {
        state.fProc(state.fMatrix, *pos, state.fY, &state.fLoc);
    }

    static void MapXYProc(const TextMapState& state, const SkScalar pos[]) {
        state.fProc(state.fMatrix, pos[0], pos[1], &state.fLoc);
    }

    static void MapOnlyScaleXProc(const TextMapState& state,
                                  const SkScalar pos[]) {
        state.fLoc.set(SkScalarMul(state.fScaleX, *pos) + state.fTransX,
                       state.fTransformedY);
    }

    static void MapOnlyTransXProc(const TextMapState& state,
                                  const SkScalar pos[]) {
        state.fLoc.set(*pos + state.fTransX, state.fTransformedY);
    }
};

TextMapState::Proc TextMapState::pickProc(int scalarsPerPosition) {
    SkASSERT(1 == scalarsPerPosition || 2 == scalarsPerPosition);

    if (1 == scalarsPerPosition) {
        unsigned mtype = fMatrix.getType();
        if (mtype & (SkMatrix::kAffine_Mask | SkMatrix::kPerspective_Mask)) {
            return MapXProc;
        } else {
            fScaleX = fMatrix.getScaleX();
            fTransX = fMatrix.getTranslateX();
            fTransformedY = SkScalarMul(fY, fMatrix.getScaleY()) +
                            fMatrix.getTranslateY();
            return (mtype & SkMatrix::kScale_Mask) ?
                        MapOnlyScaleXProc : MapOnlyTransXProc;
        }
    } else {
        return MapXYProc;
    }
}

//////////////////////////////////////////////////////////////////////////////

void SkDraw::drawPosText(const char text[], size_t byteLength,
                         const SkScalar pos[], SkScalar constY,
                         int scalarsPerPosition, const SkPaint& paint) const {
    SkASSERT(byteLength == 0 || text != NULL);
    SkASSERT(1 == scalarsPerPosition || 2 == scalarsPerPosition);

    SkDEBUGCODE(this->validate();)

    // nothing to draw
    if (text == NULL || byteLength == 0 || fRC->isEmpty()) {
        return;
    }

    if (/*paint.isLinearText() ||*/
        (fMatrix->hasPerspective())) {
        // TODO !!!!
//      this->drawText_asPaths(text, byteLength, x, y, paint);
        return;
    }

    const SkMatrix* matrix = fMatrix;
    if (hasCustomD1GProc(*this)) {
        // only support the fMVMatrix (for now) for the GPU case, which also
        // sets the fD1GProc
        if (fMVMatrix) {
            matrix = fMVMatrix;
        }
    }

    SkDrawCacheProc     glyphCacheProc = paint.getDrawCacheProc();
    SkAutoGlyphCache    autoCache(paint, matrix);
    SkGlyphCache*       cache = autoCache.getCache();

    SkAAClipBlitterWrapper wrapper;
    SkAutoBlitterChoose blitterChooser;
    SkBlitter* blitter = NULL;
    if (needsRasterTextBlit(*this)) {
        blitterChooser.choose(*fBitmap, *matrix, paint);
        blitter = blitterChooser.get();
        if (fRC->isAA()) {
            wrapper.init(*fRC, blitter);
            blitter = wrapper.getBlitter();
        }
    }

    const char*        stop = text + byteLength;
    AlignProc          alignProc = pick_align_proc(paint.getTextAlign());
    SkDraw1Glyph       d1g;
    SkDraw1Glyph::Proc proc = d1g.init(this, blitter, cache);
    TextMapState       tms(*matrix, constY);
    TextMapState::Proc tmsProc = tms.pickProc(scalarsPerPosition);

    if (cache->isSubpixel()) {
        // maybe we should skip the rounding if linearText is set
        SkAxisAlignment roundBaseline = SkComputeAxisAlignmentForHText(*matrix);

        if (SkPaint::kLeft_Align == paint.getTextAlign()) {
            while (text < stop) {

                tmsProc(tms, pos);

#ifdef SK_DRAW_POS_TEXT_IGNORE_SUBPIXEL_LEFT_ALIGN_FIX
                SkFixed fx = SkScalarToFixed(tms.fLoc.fX);
                SkFixed fy = SkScalarToFixed(tms.fLoc.fY);
#else
                SkFixed fx = SkScalarToFixed(tms.fLoc.fX) + (SK_FixedHalf >> SkGlyph::kSubBits);
                SkFixed fy = SkScalarToFixed(tms.fLoc.fY) + (SK_FixedHalf >> SkGlyph::kSubBits);
#endif
                SkFixed fxMask = ~0;
                SkFixed fyMask = ~0;

                if (kX_SkAxisAlignment == roundBaseline) {
                    fyMask = 0;
                } else if (kY_SkAxisAlignment == roundBaseline) {
                    fxMask = 0;
                }

                const SkGlyph& glyph = glyphCacheProc(cache, &text,
                                                      fx & fxMask, fy & fyMask);

                if (glyph.fWidth) {
                    proc(d1g, fx, fy, glyph);
                }
                pos += scalarsPerPosition;
            }
        } else {
            while (text < stop) {
                const char* currentText = text;
                const SkGlyph* glyph = &glyphCacheProc(cache, &text, 0, 0);

                if (glyph->fWidth) {
                    SkDEBUGCODE(SkFixed prevAdvX = glyph->fAdvanceX;)
                    SkDEBUGCODE(SkFixed prevAdvY = glyph->fAdvanceY;)

                    SkFixed fx, fy;
                    SkFixed fxMask = ~0;
                    SkFixed fyMask = ~0;
                    tmsProc(tms, pos);

                    {
                        SkIPoint fixedLoc;
                        alignProc(tms.fLoc, *glyph, &fixedLoc);
                        fx = fixedLoc.fX + (SK_FixedHalf >> SkGlyph::kSubBits);
                        fy = fixedLoc.fY + (SK_FixedHalf >> SkGlyph::kSubBits);

                        if (kX_SkAxisAlignment == roundBaseline) {
                            fyMask = 0;
                        } else if (kY_SkAxisAlignment == roundBaseline) {
                            fxMask = 0;
                        }
                    }

                    // have to call again, now that we've been "aligned"
                    glyph = &glyphCacheProc(cache, &currentText,
                                            fx & fxMask, fy & fyMask);
                    // the assumption is that the advance hasn't changed
                    SkASSERT(prevAdvX == glyph->fAdvanceX);
                    SkASSERT(prevAdvY == glyph->fAdvanceY);

                    proc(d1g, fx, fy, *glyph);
                }
                pos += scalarsPerPosition;
            }
        }
    } else {    // not subpixel
        if (SkPaint::kLeft_Align == paint.getTextAlign()) {
            while (text < stop) {
                // the last 2 parameters are ignored
                const SkGlyph& glyph = glyphCacheProc(cache, &text, 0, 0);

                if (glyph.fWidth) {
                    tmsProc(tms, pos);

                    proc(d1g,
                         SkScalarToFixed(tms.fLoc.fX) + SK_FixedHalf,
                         SkScalarToFixed(tms.fLoc.fY) + SK_FixedHalf,
                         glyph);
                }
                pos += scalarsPerPosition;
            }
        } else {
            while (text < stop) {
                // the last 2 parameters are ignored
                const SkGlyph& glyph = glyphCacheProc(cache, &text, 0, 0);

                if (glyph.fWidth) {
                    tmsProc(tms, pos);

                    SkIPoint fixedLoc;
                    alignProc(tms.fLoc, glyph, &fixedLoc);

                    proc(d1g,
                         fixedLoc.fX + SK_FixedHalf,
                         fixedLoc.fY + SK_FixedHalf,
                         glyph);
                }
                pos += scalarsPerPosition;
            }
        }
    }
}

#if defined _WIN32 && _MSC_VER >= 1300
#pragma warning ( pop )
#endif

///////////////////////////////////////////////////////////////////////////////

#include "SkPathMeasure.h"

static void morphpoints(SkPoint dst[], const SkPoint src[], int count,
                        SkPathMeasure& meas, const SkMatrix& matrix) {
    SkMatrix::MapXYProc proc = matrix.getMapXYProc();

    for (int i = 0; i < count; i++) {
        SkPoint pos;
        SkVector tangent;

        proc(matrix, src[i].fX, src[i].fY, &pos);
        SkScalar sx = pos.fX;
        SkScalar sy = pos.fY;

        if (!meas.getPosTan(sx, &pos, &tangent)) {
            // set to 0 if the measure failed, so that we just set dst == pos
            tangent.set(0, 0);
        }

        /*  This is the old way (that explains our approach but is way too slow
            SkMatrix    matrix;
            SkPoint     pt;

            pt.set(sx, sy);
            matrix.setSinCos(tangent.fY, tangent.fX);
            matrix.preTranslate(-sx, 0);
            matrix.postTranslate(pos.fX, pos.fY);
            matrix.mapPoints(&dst[i], &pt, 1);
        */
        dst[i].set(pos.fX - SkScalarMul(tangent.fY, sy),
                   pos.fY + SkScalarMul(tangent.fX, sy));
    }
}

/*  TODO

    Need differentially more subdivisions when the follow-path is curvy. Not sure how to
    determine that, but we need it. I guess a cheap answer is let the caller tell us,
    but that seems like a cop-out. Another answer is to get Rob Johnson to figure it out.
*/
static void morphpath(SkPath* dst, const SkPath& src, SkPathMeasure& meas,
                      const SkMatrix& matrix) {
    SkPath::Iter    iter(src, false);
    SkPoint         srcP[4], dstP[3];
    SkPath::Verb    verb;

    while ((verb = iter.next(srcP)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kMove_Verb:
                morphpoints(dstP, srcP, 1, meas, matrix);
                dst->moveTo(dstP[0]);
                break;
            case SkPath::kLine_Verb:
                // turn lines into quads to look bendy
                srcP[0].fX = SkScalarAve(srcP[0].fX, srcP[1].fX);
                srcP[0].fY = SkScalarAve(srcP[0].fY, srcP[1].fY);
                morphpoints(dstP, srcP, 2, meas, matrix);
                dst->quadTo(dstP[0], dstP[1]);
                break;
            case SkPath::kQuad_Verb:
                morphpoints(dstP, &srcP[1], 2, meas, matrix);
                dst->quadTo(dstP[0], dstP[1]);
                break;
            case SkPath::kCubic_Verb:
                morphpoints(dstP, &srcP[1], 3, meas, matrix);
                dst->cubicTo(dstP[0], dstP[1], dstP[2]);
                break;
            case SkPath::kClose_Verb:
                dst->close();
                break;
            default:
                SkDEBUGFAIL("unknown verb");
                break;
        }
    }
}

void SkDraw::drawTextOnPath(const char text[], size_t byteLength,
                            const SkPath& follow, const SkMatrix* matrix,
                            const SkPaint& paint) const {
    SkASSERT(byteLength == 0 || text != NULL);

    // nothing to draw
    if (text == NULL || byteLength == 0 || fRC->isEmpty()) {
        return;
    }

    SkTextToPathIter    iter(text, byteLength, paint, true);
    SkPathMeasure       meas(follow, false);
    SkScalar            hOffset = 0;

    // need to measure first
    if (paint.getTextAlign() != SkPaint::kLeft_Align) {
        SkScalar pathLen = meas.getLength();
        if (paint.getTextAlign() == SkPaint::kCenter_Align) {
            pathLen = SkScalarHalf(pathLen);
        }
        hOffset += pathLen;
    }

    const SkPath*   iterPath;
    SkScalar        xpos;
    SkMatrix        scaledMatrix;
    SkScalar        scale = iter.getPathScale();

    scaledMatrix.setScale(scale, scale);

    while (iter.next(&iterPath, &xpos)) {
        if (iterPath) {
            SkPath      tmp;
            SkMatrix    m(scaledMatrix);

            m.postTranslate(xpos + hOffset, 0);
            if (matrix) {
                m.postConcat(*matrix);
            }
            morphpath(&tmp, *iterPath, meas, m);
            if (fDevice) {
                fDevice->drawPath(*this, tmp, iter.getPaint(), NULL, true);
            } else {
                this->drawPath(tmp, iter.getPaint(), NULL, true);
            }
        }
    }
}

#ifdef SK_BUILD_FOR_ANDROID
void SkDraw::drawPosTextOnPath(const char text[], size_t byteLength,
                               const SkPoint pos[], const SkPaint& paint,
                               const SkPath& path, const SkMatrix* matrix) const {
    // nothing to draw
    if (text == NULL || byteLength == 0 || fRC->isEmpty()) {
        return;
    }

    SkMatrix scaledMatrix;
    SkPathMeasure meas(path, false);

    SkMeasureCacheProc glyphCacheProc = paint.getMeasureCacheProc(
            SkPaint::kForward_TextBufferDirection, true);

    // Copied (modified) from SkTextToPathIter constructor to setup paint
    SkPaint tempPaint(paint);

    tempPaint.setLinearText(true);
    tempPaint.setMaskFilter(NULL); // don't want this affecting our path-cache lookup

    if (tempPaint.getPathEffect() == NULL && !(tempPaint.getStrokeWidth() > 0
            && tempPaint.getStyle() != SkPaint::kFill_Style)) {
        tempPaint.setStyle(SkPaint::kFill_Style);
        tempPaint.setPathEffect(NULL);
    }
    // End copied from SkTextToPathIter constructor

    // detach cache
    SkGlyphCache* cache = tempPaint.detachCache(NULL);

    // Must set scale, even if 1
    SkScalar scale = SK_Scalar1;
    scaledMatrix.setScale(scale, scale);

    // Loop over all glyph ids
    for (const char* stop = text + byteLength; text < stop; pos++) {

        const SkGlyph& glyph = glyphCacheProc(cache, &text);
        SkPath tmp;

        const SkPath* glyphPath = cache->findPath(glyph);
        if (glyphPath == NULL) {
            continue;
        }

        SkMatrix m(scaledMatrix);
        m.postTranslate(pos->fX, 0);

        if (matrix) {
            m.postConcat(*matrix);
        }

        morphpath(&tmp, *glyphPath, meas, m);
        this->drawPath(tmp, tempPaint);

    }

    // re-attach cache
    SkGlyphCache::AttachCache(cache);
}
#endif

///////////////////////////////////////////////////////////////////////////////

struct VertState {
    int f0, f1, f2;

    VertState(int vCount, const uint16_t indices[], int indexCount)
            : fIndices(indices) {
        fCurrIndex = 0;
        if (indices) {
            fCount = indexCount;
        } else {
            fCount = vCount;
        }
    }

    typedef bool (*Proc)(VertState*);
    Proc chooseProc(SkCanvas::VertexMode mode);

private:
    int             fCount;
    int             fCurrIndex;
    const uint16_t* fIndices;

    static bool Triangles(VertState*);
    static bool TrianglesX(VertState*);
    static bool TriangleStrip(VertState*);
    static bool TriangleStripX(VertState*);
    static bool TriangleFan(VertState*);
    static bool TriangleFanX(VertState*);
};

bool VertState::Triangles(VertState* state) {
    int index = state->fCurrIndex;
    if (index + 3 > state->fCount) {
        return false;
    }
    state->f0 = index + 0;
    state->f1 = index + 1;
    state->f2 = index + 2;
    state->fCurrIndex = index + 3;
    return true;
}

bool VertState::TrianglesX(VertState* state) {
    const uint16_t* indices = state->fIndices;
    int index = state->fCurrIndex;
    if (index + 3 > state->fCount) {
        return false;
    }
    state->f0 = indices[index + 0];
    state->f1 = indices[index + 1];
    state->f2 = indices[index + 2];
    state->fCurrIndex = index + 3;
    return true;
}

bool VertState::TriangleStrip(VertState* state) {
    int index = state->fCurrIndex;
    if (index + 3 > state->fCount) {
        return false;
    }
    state->f2 = index + 2;
    if (index & 1) {
        state->f0 = index + 1;
        state->f1 = index + 0;
    } else {
        state->f0 = index + 0;
        state->f1 = index + 1;
    }
    state->fCurrIndex = index + 1;
    return true;
}

bool VertState::TriangleStripX(VertState* state) {
    const uint16_t* indices = state->fIndices;
    int index = state->fCurrIndex;
    if (index + 3 > state->fCount) {
        return false;
    }
    state->f2 = indices[index + 2];
    if (index & 1) {
        state->f0 = indices[index + 1];
        state->f1 = indices[index + 0];
    } else {
        state->f0 = indices[index + 0];
        state->f1 = indices[index + 1];
    }
    state->fCurrIndex = index + 1;
    return true;
}

bool VertState::TriangleFan(VertState* state) {
    int index = state->fCurrIndex;
    if (index + 3 > state->fCount) {
        return false;
    }
    state->f0 = 0;
    state->f1 = index + 1;
    state->f2 = index + 2;
    state->fCurrIndex = index + 1;
    return true;
}

bool VertState::TriangleFanX(VertState* state) {
    const uint16_t* indices = state->fIndices;
    int index = state->fCurrIndex;
    if (index + 3 > state->fCount) {
        return false;
    }
    state->f0 = indices[0];
    state->f1 = indices[index + 1];
    state->f2 = indices[index + 2];
    state->fCurrIndex = index + 1;
    return true;
}

VertState::Proc VertState::chooseProc(SkCanvas::VertexMode mode) {
    switch (mode) {
        case SkCanvas::kTriangles_VertexMode:
            return fIndices ? TrianglesX : Triangles;
        case SkCanvas::kTriangleStrip_VertexMode:
            return fIndices ? TriangleStripX : TriangleStrip;
        case SkCanvas::kTriangleFan_VertexMode:
            return fIndices ? TriangleFanX : TriangleFan;
        default:
            return NULL;
    }
}

typedef void (*HairProc)(const SkPoint&, const SkPoint&, const SkRasterClip&,
                         SkBlitter*);

static HairProc ChooseHairProc(bool doAntiAlias) {
    return doAntiAlias ? SkScan::AntiHairLine : SkScan::HairLine;
}

static bool texture_to_matrix(const VertState& state, const SkPoint verts[],
                              const SkPoint texs[], SkMatrix* matrix) {
    SkPoint src[3], dst[3];

    src[0] = texs[state.f0];
    src[1] = texs[state.f1];
    src[2] = texs[state.f2];
    dst[0] = verts[state.f0];
    dst[1] = verts[state.f1];
    dst[2] = verts[state.f2];
    return matrix->setPolyToPoly(src, dst, 3);
}

class SkTriColorShader : public SkShader {
public:
    SkTriColorShader() {}

    bool setup(const SkPoint pts[], const SkColor colors[], int, int, int);

    virtual void shadeSpan(int x, int y, SkPMColor dstC[], int count);

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkTriColorShader)

protected:
    SkTriColorShader(SkFlattenableReadBuffer& buffer) : SkShader(buffer) {}

private:
    SkMatrix    fDstToUnit;
    SkPMColor   fColors[3];

    typedef SkShader INHERITED;
};

bool SkTriColorShader::setup(const SkPoint pts[], const SkColor colors[],
                             int index0, int index1, int index2) {

    fColors[0] = SkPreMultiplyColor(colors[index0]);
    fColors[1] = SkPreMultiplyColor(colors[index1]);
    fColors[2] = SkPreMultiplyColor(colors[index2]);

    SkMatrix m, im;
    m.reset();
    m.set(0, pts[index1].fX - pts[index0].fX);
    m.set(1, pts[index2].fX - pts[index0].fX);
    m.set(2, pts[index0].fX);
    m.set(3, pts[index1].fY - pts[index0].fY);
    m.set(4, pts[index2].fY - pts[index0].fY);
    m.set(5, pts[index0].fY);
    if (!m.invert(&im)) {
        return false;
    }
    return fDstToUnit.setConcat(im, this->getTotalInverse());
}

#include "SkColorPriv.h"
#include "SkComposeShader.h"

static int ScalarTo256(SkScalar v) {
    int scale = SkScalarToFixed(v) >> 8;
    if (scale < 0) {
        scale = 0;
    }
    if (scale > 255) {
        scale = 255;
    }
    return SkAlpha255To256(scale);
}

void SkTriColorShader::shadeSpan(int x, int y, SkPMColor dstC[], int count) {
    SkPoint src;

    for (int i = 0; i < count; i++) {
        fDstToUnit.mapXY(SkIntToScalar(x), SkIntToScalar(y), &src);
        x += 1;

        int scale1 = ScalarTo256(src.fX);
        int scale2 = ScalarTo256(src.fY);
        int scale0 = 256 - scale1 - scale2;
        if (scale0 < 0) {
            if (scale1 > scale2) {
                scale2 = 256 - scale1;
            } else {
                scale1 = 256 - scale2;
            }
            scale0 = 0;
        }

        dstC[i] = SkAlphaMulQ(fColors[0], scale0) +
        SkAlphaMulQ(fColors[1], scale1) +
        SkAlphaMulQ(fColors[2], scale2);
    }
}

void SkDraw::drawVertices(SkCanvas::VertexMode vmode, int count,
                          const SkPoint vertices[], const SkPoint textures[],
                          const SkColor colors[], SkXfermode* xmode,
                          const uint16_t indices[], int indexCount,
                          const SkPaint& paint) const {
    SkASSERT(0 == count || NULL != vertices);

    // abort early if there is nothing to draw
    if (count < 3 || (indices && indexCount < 3) || fRC->isEmpty()) {
        return;
    }

    // transform out vertices into device coordinates
    SkAutoSTMalloc<16, SkPoint> storage(count);
    SkPoint* devVerts = storage.get();
    fMatrix->mapPoints(devVerts, vertices, count);

    if (fBounder) {
        SkRect bounds;
        bounds.set(devVerts, count);
        if (!fBounder->doRect(bounds, paint)) {
            return;
        }
    }

    /*
        We can draw the vertices in 1 of 4 ways:

        - solid color (no shader/texture[], no colors[])
        - just colors (no shader/texture[], has colors[])
        - just texture (has shader/texture[], no colors[])
        - colors * texture (has shader/texture[], has colors[])

        Thus for texture drawing, we need both texture[] and a shader.
    */

    SkTriColorShader triShader; // must be above declaration of p
    SkPaint p(paint);

    SkShader* shader = p.getShader();
    if (NULL == shader) {
        // if we have no shader, we ignore the texture coordinates
        textures = NULL;
    } else if (NULL == textures) {
        // if we don't have texture coordinates, ignore the shader
        p.setShader(NULL);
        shader = NULL;
    }

    // setup the custom shader (if needed)
    if (NULL != colors) {
        if (NULL == textures) {
            // just colors (no texture)
            p.setShader(&triShader);
        } else {
            // colors * texture
            SkASSERT(shader);
            bool releaseMode = false;
            if (NULL == xmode) {
                xmode = SkXfermode::Create(SkXfermode::kMultiply_Mode);
                releaseMode = true;
            }
            SkShader* compose = SkNEW_ARGS(SkComposeShader,
                                           (&triShader, shader, xmode));
            p.setShader(compose)->unref();
            if (releaseMode) {
                xmode->unref();
            }
        }
    }

    SkAutoBlitterChoose blitter(*fBitmap, *fMatrix, p);
    // setup our state and function pointer for iterating triangles
    VertState       state(count, indices, indexCount);
    VertState::Proc vertProc = state.chooseProc(vmode);

    if (NULL != textures || NULL != colors) {
        SkMatrix  localM, tempM;
        bool      hasLocalM = shader && shader->getLocalMatrix(&localM);

        if (NULL != colors) {
            if (!triShader.setContext(*fBitmap, p, *fMatrix)) {
                colors = NULL;
            }
        }

        while (vertProc(&state)) {
            if (NULL != textures) {
                if (texture_to_matrix(state, vertices, textures, &tempM)) {
                    if (hasLocalM) {
                        tempM.postConcat(localM);
                    }
                    shader->setLocalMatrix(tempM);
                    // need to recal setContext since we changed the local matrix
                    if (!shader->setContext(*fBitmap, p, *fMatrix)) {
                        continue;
                    }
                }
            }
            if (NULL != colors) {
                if (!triShader.setup(vertices, colors,
                                     state.f0, state.f1, state.f2)) {
                    continue;
                }
            }

            SkPoint tmp[] = {
                devVerts[state.f0], devVerts[state.f1], devVerts[state.f2]
            };
            SkScan::FillTriangle(tmp, *fRC, blitter.get());
        }
        // now restore the shader's original local matrix
        if (NULL != shader) {
            if (hasLocalM) {
                shader->setLocalMatrix(localM);
            } else {
                shader->resetLocalMatrix();
            }
        }
    } else {
        // no colors[] and no texture
        HairProc hairProc = ChooseHairProc(paint.isAntiAlias());
        const SkRasterClip& clip = *fRC;
        while (vertProc(&state)) {
            hairProc(devVerts[state.f0], devVerts[state.f1], clip, blitter.get());
            hairProc(devVerts[state.f1], devVerts[state.f2], clip, blitter.get());
            hairProc(devVerts[state.f2], devVerts[state.f0], clip, blitter.get());
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG

void SkDraw::validate() const {
    SkASSERT(fBitmap != NULL);
    SkASSERT(fMatrix != NULL);
    SkASSERT(fClip != NULL);
    SkASSERT(fRC != NULL);

    const SkIRect&  cr = fRC->getBounds();
    SkIRect         br;

    br.set(0, 0, fBitmap->width(), fBitmap->height());
    SkASSERT(cr.isEmpty() || br.contains(cr));

    // assert that both are null, or both are not-null
    SkASSERT(!fMVMatrix == !fExtMatrix);
}

#endif

///////////////////////////////////////////////////////////////////////////////

SkBounder::SkBounder() {
    // initialize up front. This gets reset by SkCanvas before each draw call.
    fClip = &SkRegion::GetEmptyRegion();
}

bool SkBounder::doIRect(const SkIRect& r) {
    SkIRect    rr;
    return rr.intersect(fClip->getBounds(), r) && this->onIRect(rr);
}

// TODO: change the prototype to take fixed, and update the callers
bool SkBounder::doIRectGlyph(const SkIRect& r, int x, int y,
                             const SkGlyph& glyph) {
    SkIRect    rr;
    if (!rr.intersect(fClip->getBounds(), r)) {
        return false;
    }
    GlyphRec rec;
    rec.fLSB.set(SkIntToFixed(x), SkIntToFixed(y));
    rec.fRSB.set(rec.fLSB.fX + glyph.fAdvanceX,
                 rec.fLSB.fY + glyph.fAdvanceY);
    rec.fGlyphID = glyph.getGlyphID();
    rec.fFlags = 0;
    return this->onIRectGlyph(rr, rec);
}

bool SkBounder::doHairline(const SkPoint& pt0, const SkPoint& pt1,
                           const SkPaint& paint) {
    SkIRect     r;
    SkScalar    v0, v1;

    v0 = pt0.fX;
    v1 = pt1.fX;
    if (v0 > v1) {
        SkTSwap<SkScalar>(v0, v1);
    }
    r.fLeft     = SkScalarFloor(v0);
    r.fRight    = SkScalarCeil(v1);

    v0 = pt0.fY;
    v1 = pt1.fY;
    if (v0 > v1) {
        SkTSwap<SkScalar>(v0, v1);
    }
    r.fTop      = SkScalarFloor(v0);
    r.fBottom   = SkScalarCeil(v1);

    if (paint.isAntiAlias()) {
        r.inset(-1, -1);
    }
    return this->doIRect(r);
}

bool SkBounder::doRect(const SkRect& rect, const SkPaint& paint) {
    SkIRect    r;

    if (paint.getStyle() == SkPaint::kFill_Style) {
        rect.round(&r);
    } else {
        int rad = -1;
        rect.roundOut(&r);
        if (paint.isAntiAlias()) {
            rad = -2;
        }
        r.inset(rad, rad);
    }
    return this->doIRect(r);
}

bool SkBounder::doPath(const SkPath& path, const SkPaint& paint, bool doFill) {
    SkIRect       r;
    const SkRect& bounds = path.getBounds();

    if (doFill) {
        bounds.round(&r);
    } else {    // hairline
        bounds.roundOut(&r);
    }

    if (paint.isAntiAlias()) {
        r.inset(-1, -1);
    }
    return this->doIRect(r);
}

void SkBounder::commit() {
    // override in subclass
}

////////////////////////////////////////////////////////////////////////////////////////////////

#include "SkPath.h"
#include "SkDraw.h"
#include "SkRegion.h"
#include "SkBlitter.h"

static bool compute_bounds(const SkPath& devPath, const SkIRect* clipBounds,
                           SkMaskFilter* filter, const SkMatrix* filterMatrix,
                           SkIRect* bounds) {
    if (devPath.isEmpty()) {
        return false;
    }

    //  init our bounds from the path
    {
        SkRect pathBounds = devPath.getBounds();
        pathBounds.inset(-SK_ScalarHalf, -SK_ScalarHalf);
        pathBounds.roundOut(bounds);
    }

    SkIPoint margin = SkIPoint::Make(0, 0);
    if (filter) {
        SkASSERT(filterMatrix);

        SkMask srcM, dstM;

        srcM.fBounds = *bounds;
        srcM.fFormat = SkMask::kA8_Format;
        srcM.fImage = NULL;
        if (!filter->filterMask(&dstM, srcM, *filterMatrix, &margin)) {
            return false;
        }
    }

    // (possibly) trim the bounds to reflect the clip
    // (plus whatever slop the filter needs)
    if (clipBounds) {
        SkIRect tmp = *clipBounds;
        // Ugh. Guard against gigantic margins from wacky filters. Without this
        // check we can request arbitrary amounts of slop beyond our visible
        // clip, and bring down the renderer (at least on finite RAM machines
        // like handsets, etc.). Need to balance this invented value between
        // quality of large filters like blurs, and the corresponding memory
        // requests.
        static const int MAX_MARGIN = 128;
        tmp.inset(-SkMin32(margin.fX, MAX_MARGIN),
                  -SkMin32(margin.fY, MAX_MARGIN));
        if (!bounds->intersect(tmp)) {
            return false;
        }
    }

    return true;
}

static void draw_into_mask(const SkMask& mask, const SkPath& devPath,
                           SkPaint::Style style) {
    SkBitmap        bm;
    SkDraw          draw;
    SkRasterClip    clip;
    SkMatrix        matrix;
    SkPaint         paint;

    bm.setConfig(SkBitmap::kA8_Config, mask.fBounds.width(), mask.fBounds.height(), mask.fRowBytes);
    bm.setPixels(mask.fImage);

    clip.setRect(SkIRect::MakeWH(mask.fBounds.width(), mask.fBounds.height()));
    matrix.setTranslate(-SkIntToScalar(mask.fBounds.fLeft),
                        -SkIntToScalar(mask.fBounds.fTop));

    draw.fBitmap    = &bm;
    draw.fRC        = &clip;
    draw.fClip      = &clip.bwRgn();
    draw.fMatrix    = &matrix;
    draw.fBounder   = NULL;
    paint.setAntiAlias(true);
    paint.setStyle(style);
    draw.drawPath(devPath, paint);
}

bool SkDraw::DrawToMask(const SkPath& devPath, const SkIRect* clipBounds,
                        SkMaskFilter* filter, const SkMatrix* filterMatrix,
                        SkMask* mask, SkMask::CreateMode mode,
                        SkPaint::Style style) {
    if (SkMask::kJustRenderImage_CreateMode != mode) {
        if (!compute_bounds(devPath, clipBounds, filter, filterMatrix, &mask->fBounds))
            return false;
    }

    if (SkMask::kComputeBoundsAndRenderImage_CreateMode == mode) {
        mask->fFormat = SkMask::kA8_Format;
        mask->fRowBytes = mask->fBounds.width();
        size_t size = mask->computeImageSize();
        if (0 == size) {
            // we're too big to allocate the mask, abort
            return false;
        }
        mask->fImage = SkMask::AllocImage(size);
        memset(mask->fImage, 0, mask->computeImageSize());
    }

    if (SkMask::kJustComputeBounds_CreateMode != mode) {
        draw_into_mask(*mask, devPath, style);
    }

    return true;
}
