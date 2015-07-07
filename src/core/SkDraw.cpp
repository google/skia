/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#define __STDC_LIMIT_MACROS

#include "SkDraw.h"
#include "SkBlitter.h"
#include "SkCanvas.h"
#include "SkColorPriv.h"
#include "SkDevice.h"
#include "SkDeviceLooper.h"
#include "SkFixed.h"
#include "SkMaskFilter.h"
#include "SkPaint.h"
#include "SkPathEffect.h"
#include "SkRasterClip.h"
#include "SkRasterizer.h"
#include "SkRRect.h"
#include "SkScan.h"
#include "SkShader.h"
#include "SkSmallAllocator.h"
#include "SkString.h"
#include "SkStroke.h"
#include "SkTextMapStateProc.h"
#include "SkTLazy.h"
#include "SkUtils.h"
#include "SkVertState.h"

#include "SkAutoKern.h"
#include "SkBitmapProcShader.h"
#include "SkDrawProcs.h"
#include "SkMatrixUtils.h"

//#define TRACE_BITMAP_DRAWS


/** Helper for allocating small blitters on the stack.
 */
class SkAutoBlitterChoose : SkNoncopyable {
public:
    SkAutoBlitterChoose() {
        fBlitter = NULL;
    }
    SkAutoBlitterChoose(const SkPixmap& dst, const SkMatrix& matrix,
                        const SkPaint& paint, bool drawCoverage = false) {
        fBlitter = SkBlitter::Choose(dst, matrix, paint, &fAllocator, drawCoverage);
    }
    
    SkBlitter*  operator->() { return fBlitter; }
    SkBlitter*  get() const { return fBlitter; }

    void choose(const SkPixmap& dst, const SkMatrix& matrix,
                const SkPaint& paint, bool drawCoverage = false) {
        SkASSERT(!fBlitter);
        fBlitter = SkBlitter::Choose(dst, matrix, paint, &fAllocator, drawCoverage);
    }

private:
    // Owned by fAllocator, which will handle the delete.
    SkBlitter*          fBlitter;
    SkTBlitterAllocator fAllocator;
};
#define SkAutoBlitterChoose(...) SK_REQUIRE_LOCAL_VAR(SkAutoBlitterChoose)

/**
 *  Since we are providing the storage for the shader (to avoid the perf cost
 *  of calling new) we insist that in our destructor we can account for all
 *  owners of the shader.
 */
class SkAutoBitmapShaderInstall : SkNoncopyable {
public:
    SkAutoBitmapShaderInstall(const SkBitmap& src, const SkPaint& paint,
                              const SkMatrix* localMatrix = NULL)
            : fPaint(paint) /* makes a copy of the paint */ {
        fPaint.setShader(SkCreateBitmapShader(src, SkShader::kClamp_TileMode,
                                              SkShader::kClamp_TileMode,
                                              localMatrix, &fAllocator));
        // we deliberately left the shader with an owner-count of 2
        SkASSERT(2 == fPaint.getShader()->getRefCnt());
    }

    ~SkAutoBitmapShaderInstall() {
        // since fAllocator will destroy shader, we insist that owners == 2
        SkASSERT(2 == fPaint.getShader()->getRefCnt());

        fPaint.setShader(NULL); // unref the shader by 1

    }

    // return the new paint that has the shader applied
    const SkPaint& paintWithShader() const { return fPaint; }

private:
    // copy of caller's paint (which we then modify)
    SkPaint             fPaint;
    // Stores the shader.
    SkTBlitterAllocator fAllocator;
};
#define SkAutoBitmapShaderInstall(...) SK_REQUIRE_LOCAL_VAR(SkAutoBitmapShaderInstall)

///////////////////////////////////////////////////////////////////////////////

SkDraw::SkDraw() {
    sk_bzero(this, sizeof(*this));
}

SkDraw::SkDraw(const SkDraw& src) {
    memcpy(this, &src, sizeof(*this));
}

bool SkDraw::computeConservativeLocalClipBounds(SkRect* localBounds) const {
    if (fRC->isEmpty()) {
        return false;
    }

    SkMatrix inverse;
    if (!fMatrix->invert(&inverse)) {
        return false;
    }

    SkIRect devBounds = fRC->getBounds();
    // outset to have slop for antialasing and hairlines
    devBounds.outset(1, 1);
    inverse.mapRect(localBounds, SkRect::Make(devBounds));
    return true;
}

///////////////////////////////////////////////////////////////////////////////

typedef void (*BitmapXferProc)(void* pixels, size_t bytes, uint32_t data);

static void D_Clear_BitmapXferProc(void* pixels, size_t bytes, uint32_t) {
    sk_bzero(pixels, bytes);
}

static void D_Dst_BitmapXferProc(void*, size_t, uint32_t data) {}

static void D32_Src_BitmapXferProc(void* pixels, size_t bytes, uint32_t data) {
    sk_memset32((uint32_t*)pixels, data, SkToInt(bytes >> 2));
}

static void D16_Src_BitmapXferProc(void* pixels, size_t bytes, uint32_t data) {
    sk_memset16((uint16_t*)pixels, data, SkToInt(bytes >> 1));
}

static void DA8_Src_BitmapXferProc(void* pixels, size_t bytes, uint32_t data) {
    memset(pixels, data, bytes);
}

static BitmapXferProc ChooseBitmapXferProc(const SkPixmap& dst, const SkPaint& paint,
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
            switch (dst.colorType()) {
                case kN32_SkColorType:
                    if (data) {
                        *data = pmc;
                    }
//                    SkDebugf("--- D32_Src_BitmapXferProc\n");
                    return D32_Src_BitmapXferProc;
                case kRGB_565_SkColorType:
                    if (data) {
                        *data = SkPixel32ToPixel16(pmc);
                    }
//                    SkDebugf("--- D16_Src_BitmapXferProc\n");
                    return D16_Src_BitmapXferProc;
                case kAlpha_8_SkColorType:
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

static void CallBitmapXferProc(const SkPixmap& dst, const SkIRect& rect, BitmapXferProc proc,
                               uint32_t procData) {
    int shiftPerPixel;
    switch (dst.colorType()) {
        case kN32_SkColorType:
            shiftPerPixel = 2;
            break;
        case kRGB_565_SkColorType:
            shiftPerPixel = 1;
            break;
        case kAlpha_8_SkColorType:
            shiftPerPixel = 0;
            break;
        default:
            SkDEBUGFAIL("Can't use xferproc on this config");
            return;
    }

    uint8_t* pixels = (uint8_t*)dst.writable_addr();
    SkASSERT(pixels);
    const size_t rowBytes = dst.rowBytes();
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
    devRect.set(0, 0, fDst.width(), fDst.height());

    if (fRC->isBW()) {
        /*  If we don't have a shader (i.e. we're just a solid color) we may
            be faster to operate directly on the device bitmap, rather than invoking
            a blitter. Esp. true for xfermodes, which require a colorshader to be
            present, which is just redundant work. Since we're drawing everywhere
            in the clip, we don't have to worry about antialiasing.
        */
        uint32_t procData = 0;  // to avoid the warning
        BitmapXferProc proc = ChooseBitmapXferProc(fDst, paint, &procData);
        if (proc) {
            if (D_Dst_BitmapXferProc == proc) { // nothing to do
                return;
            }

            SkRegion::Iterator iter(fRC->bwRgn());
            while (!iter.done()) {
                CallBitmapXferProc(fDst, iter.rect(), proc, procData);
                iter.next();
            }
            return;
        }
    }

    // normal case: use a blitter
    SkAutoBlitterChoose blitter(fDst, *fMatrix, paint);
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
        int x = SkScalarFloorToInt(devPts[i].fX);
        int y = SkScalarFloorToInt(devPts[i].fY);
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
    const SkPixmap* dst = blitter->justAnOpaqueColor(&value);
    SkASSERT(dst);

    uint16_t* addr = dst->writable_addr16(0, 0);
    size_t    rb = dst->rowBytes();

    for (int i = 0; i < count; i++) {
        int x = SkScalarFloorToInt(devPts[i].fX);
        int y = SkScalarFloorToInt(devPts[i].fY);
        if (r.contains(x, y)) {
            ((uint16_t*)((char*)addr + y * rb))[x] = SkToU16(value);
        }
    }
}

static void bw_pt_rect_32_hair_proc(const PtProcRec& rec,
                                    const SkPoint devPts[], int count,
                                    SkBlitter* blitter) {
    SkASSERT(rec.fRC->isRect());
    const SkIRect& r = rec.fRC->getBounds();
    uint32_t value;
    const SkPixmap* dst = blitter->justAnOpaqueColor(&value);
    SkASSERT(dst);

    SkPMColor* addr = dst->writable_addr32(0, 0);
    size_t     rb = dst->rowBytes();

    for (int i = 0; i < count; i++) {
        int x = SkScalarFloorToInt(devPts[i].fX);
        int y = SkScalarFloorToInt(devPts[i].fY);
        if (r.contains(x, y)) {
            ((SkPMColor*)((char*)addr + y * rb))[x] = value;
        }
    }
}

static void bw_pt_hair_proc(const PtProcRec& rec, const SkPoint devPts[],
                            int count, SkBlitter* blitter) {
    for (int i = 0; i < count; i++) {
        int x = SkScalarFloorToInt(devPts[i].fX);
        int y = SkScalarFloorToInt(devPts[i].fY);
        if (rec.fClip->contains(x, y)) {
            blitter->blitH(x, y, 1);
        }
    }
}

static void bw_line_hair_proc(const PtProcRec& rec, const SkPoint devPts[],
                              int count, SkBlitter* blitter) {
    for (int i = 0; i < count; i += 2) {
        SkScan::HairLine(&devPts[i], 2, *rec.fRC, blitter);
    }
}

static void bw_poly_hair_proc(const PtProcRec& rec, const SkPoint devPts[],
                              int count, SkBlitter* blitter) {
    SkScan::HairLine(devPts, count, *rec.fRC, blitter);
}

// aa versions

static void aa_line_hair_proc(const PtProcRec& rec, const SkPoint devPts[],
                              int count, SkBlitter* blitter) {
    for (int i = 0; i < count; i += 2) {
        SkScan::AntiHairLine(&devPts[i], 2, *rec.fRC, blitter);
    }
}

static void aa_poly_hair_proc(const PtProcRec& rec, const SkPoint devPts[],
                              int count, SkBlitter* blitter) {
    SkScan::AntiHairLine(devPts, count, *rec.fRC, blitter);
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
    if ((unsigned)mode > (unsigned)SkCanvas::kPolygon_PointMode) {
        return false;
    }

    if (paint.getPathEffect()) {
        return false;
    }
    SkScalar width = paint.getStrokeWidth();
    if (0 == width) {
        fMode = mode;
        fPaint = &paint;
        fClip = NULL;
        fRC = rc;
        fRadius = SK_FixedHalf;
        return true;
    }
    if (paint.getStrokeCap() != SkPaint::kRound_Cap &&
        matrix->isScaleTranslate() && SkCanvas::kPoints_PointMode == mode) {
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

    if (fPaint->isAntiAlias()) {
        if (0 == fPaint->getStrokeWidth()) {
            static const Proc gAAProcs[] = {
                aa_square_proc, aa_line_hair_proc, aa_poly_hair_proc
            };
            proc = gAAProcs[fMode];
        } else if (fPaint->getStrokeCap() != SkPaint::kRound_Cap) {
            SkASSERT(SkCanvas::kPoints_PointMode == fMode);
            proc = aa_square_proc;
        }
    } else {    // BW
        if (fRadius <= SK_FixedHalf) {    // small radii and hairline
            if (SkCanvas::kPoints_PointMode == fMode && fClip->isRect()) {
                uint32_t value;
                const SkPixmap* bm = blitter->justAnOpaqueColor(&value);
                if (bm && kRGB_565_SkColorType == bm->colorType()) {
                    proc = bw_pt_rect_16_hair_proc;
                } else if (bm && kN32_SkColorType == bm->colorType()) {
                    proc = bw_pt_rect_32_hair_proc;
                } else {
                    proc = bw_pt_rect_hair_proc;
                }
            } else {
                static Proc gBWProcs[] = {
                    bw_pt_hair_proc, bw_line_hair_proc, bw_poly_hair_proc
                };
                proc = gBWProcs[fMode];
            }
        } else {
            proc = bw_square_proc;
        }
    }
    return proc;
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

    PtProcRec rec;
    if (!forceUseDevice && rec.init(mode, paint, fMatrix, fRC)) {
        SkAutoBlitterChoose blitter(fDst, *fMatrix, paint);

        SkPoint             devPts[MAX_DEV_PTS];
        const SkMatrix*     matrix = fMatrix;
        SkBlitter*          bltr = blitter.get();
        PtProcRec::Proc     proc = rec.chooseProc(&bltr);
        // we have to back up subsequent passes if we're in polygon mode
        const size_t backup = (SkCanvas::kPolygon_PointMode == mode);

        do {
            int n = SkToInt(count);
            if (n > MAX_DEV_PTS) {
                n = MAX_DEV_PTS;
            }
            matrix->mapPoints(devPts, pts, n);
            proc(rec, devPts, n, bltr);
            pts += n - backup;
            SkASSERT(SkToInt(count) >= n);
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
                        path.setIsVolatile((count-1) == i);
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
                if (2 == count && paint.getPathEffect()) {
                    // most likely a dashed line - see if it is one of the ones
                    // we can accelerate
                    SkStrokeRec rec(paint);
                    SkPathEffect::PointData pointData;

                    SkPath path;
                    path.moveTo(pts[0]);
                    path.lineTo(pts[1]);

                    SkRect cullRect = SkRect::Make(fRC->getBounds());

                    if (paint.getPathEffect()->asPoints(&pointData, path, rec,
                                                        *fMatrix, &cullRect)) {
                        // 'asPoints' managed to find some fast path

                        SkPaint newP(paint);
                        newP.setPathEffect(NULL);
                        newP.setStyle(SkPaint::kFill_Style);

                        if (!pointData.fFirst.isEmpty()) {
                            if (fDevice) {
                                fDevice->drawPath(*this, pointData.fFirst, newP);
                            } else {
                                this->drawPath(pointData.fFirst, newP);
                            }
                        }

                        if (!pointData.fLast.isEmpty()) {
                            if (fDevice) {
                                fDevice->drawPath(*this, pointData.fLast, newP);
                            } else {
                                this->drawPath(pointData.fLast, newP);
                            }
                        }

                        if (pointData.fSize.fX == pointData.fSize.fY) {
                            // The rest of the dashed line can just be drawn as points
                            SkASSERT(pointData.fSize.fX == SkScalarHalf(newP.getStrokeWidth()));

                            if (SkPathEffect::PointData::kCircles_PointFlag & pointData.fFlags) {
                                newP.setStrokeCap(SkPaint::kRound_Cap);
                            } else {
                                newP.setStrokeCap(SkPaint::kButt_Cap);
                            }

                            if (fDevice) {
                                fDevice->drawPoints(*this,
                                                    SkCanvas::kPoints_PointMode,
                                                    pointData.fNumPoints,
                                                    pointData.fPoints,
                                                    newP);
                            } else {
                                this->drawPoints(SkCanvas::kPoints_PointMode,
                                                 pointData.fNumPoints,
                                                 pointData.fPoints,
                                                 newP,
                                                 forceUseDevice);
                            }
                            break;
                        } else {
                            // The rest of the dashed line must be drawn as rects
                            SkASSERT(!(SkPathEffect::PointData::kCircles_PointFlag &
                                      pointData.fFlags));

                            SkRect r;

                            for (int i = 0; i < pointData.fNumPoints; ++i) {
                                r.set(pointData.fPoints[i].fX - pointData.fSize.fX,
                                      pointData.fPoints[i].fY - pointData.fSize.fY,
                                      pointData.fPoints[i].fX + pointData.fSize.fX,
                                      pointData.fPoints[i].fY + pointData.fSize.fY);
                                if (fDevice) {
                                    fDevice->drawRect(*this, r, newP);
                                } else {
                                    this->drawRect(r, newP);
                                }
                            }
                        }

                        break;
                    }
                }
                // couldn't take fast path so fall through!
            case SkCanvas::kPolygon_PointMode: {
                count -= 1;
                SkPath path;
                SkPaint p(paint);
                p.setStyle(SkPaint::kStroke_Style);
                size_t inc = (SkCanvas::kLines_PointMode == mode) ? 2 : 1;
                path.setIsVolatile(true);
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

static inline SkPoint compute_stroke_size(const SkPaint& paint, const SkMatrix& matrix) {
    SkASSERT(matrix.rectStaysRect());
    SkASSERT(SkPaint::kFill_Style != paint.getStyle());

    SkVector size;
    SkPoint pt = { paint.getStrokeWidth(), paint.getStrokeWidth() };
    matrix.mapVectors(&size, &pt, 1);
    return SkPoint::Make(SkScalarAbs(size.fX), SkScalarAbs(size.fY));
}

static bool easy_rect_join(const SkPaint& paint, const SkMatrix& matrix,
                           SkPoint* strokeSize) {
    if (SkPaint::kMiter_Join != paint.getStrokeJoin() ||
        paint.getStrokeMiter() < SK_ScalarSqrt2) {
        return false;
    }

    *strokeSize = compute_stroke_size(paint, matrix);
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
    return SkTCast<const SkPoint*>(&r);
}

static SkPoint* rect_points(SkRect& r) {
    return SkTCast<SkPoint*>(&r);
}

void SkDraw::drawRect(const SkRect& prePaintRect, const SkPaint& paint,
                      const SkMatrix* paintMatrix, const SkRect* postPaintRect) const {
    SkDEBUGCODE(this->validate();)

    // nothing to draw
    if (fRC->isEmpty()) {
        return;
    }

    const SkMatrix* matrix;
    SkMatrix combinedMatrixStorage;
    if (paintMatrix) {
        SkASSERT(postPaintRect);
        combinedMatrixStorage.setConcat(*fMatrix, *paintMatrix);
        matrix = &combinedMatrixStorage;
    } else {
        SkASSERT(!postPaintRect);
        matrix = fMatrix;
    }

    SkPoint strokeSize;
    RectType rtype = ComputeRectType(paint, *fMatrix, &strokeSize);

    if (kPath_RectType == rtype) {
        SkDraw draw(*this);
        if (paintMatrix) {
            draw.fMatrix = matrix;
        }
        SkPath  tmp;
        tmp.addRect(prePaintRect);
        tmp.setFillType(SkPath::kWinding_FillType);
        draw.drawPath(tmp, paint, NULL, true);
        return;
    }

    SkRect devRect;
    const SkRect& paintRect = paintMatrix ? *postPaintRect : prePaintRect;
    // skip the paintMatrix when transforming the rect by the CTM
    fMatrix->mapPoints(rect_points(devRect), rect_points(paintRect), 2);
    devRect.sort();

    // look for the quick exit, before we build a blitter
    SkRect bbox = devRect;
    if (paint.getStyle() != SkPaint::kFill_Style) {
        // extra space for hairlines
        if (paint.getStrokeWidth() == 0) {
            bbox.outset(1, 1);
        } else {
            // For kStroke_RectType, strokeSize is already computed.
            const SkPoint& ssize = (kStroke_RectType == rtype)
                ? strokeSize
                : compute_stroke_size(paint, *fMatrix);
            bbox.outset(SkScalarHalf(ssize.x()), SkScalarHalf(ssize.y()));
        }
    }

    SkIRect ir = bbox.roundOut();
    if (fRC->quickReject(ir)) {
        return;
    }

    SkDeviceLooper looper(fDst, *fRC, ir, paint.isAntiAlias());
    while (looper.next()) {
        SkRect localDevRect;
        looper.mapRect(&localDevRect, devRect);
        SkMatrix localMatrix;
        looper.mapMatrix(&localMatrix, *matrix);

        SkAutoBlitterChoose blitterStorage(looper.getPixmap(), localMatrix, paint);
        const SkRasterClip& clip = looper.getRC();
        SkBlitter*          blitter = blitterStorage.get();

        // we want to "fill" if we are kFill or kStrokeAndFill, since in the latter
        // case we are also hairline (if we've gotten to here), which devolves to
        // effectively just kFill
        switch (rtype) {
            case kFill_RectType:
                if (paint.isAntiAlias()) {
                    SkScan::AntiFillRect(localDevRect, clip, blitter);
                } else {
                    SkScan::FillRect(localDevRect, clip, blitter);
                }
                break;
            case kStroke_RectType:
                if (paint.isAntiAlias()) {
                    SkScan::AntiFrameRect(localDevRect, strokeSize, clip, blitter);
                } else {
                    SkScan::FrameRect(localDevRect, strokeSize, clip, blitter);
                }
                break;
            case kHair_RectType:
                if (paint.isAntiAlias()) {
                    SkScan::AntiHairRect(localDevRect, clip, blitter);
                } else {
                    SkScan::HairRect(localDevRect, clip, blitter);
                }
                break;
            default:
                SkDEBUGFAIL("bad rtype");
        }
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

    SkAutoBlitterChoose blitterChooser(fDst, *fMatrix, paint);
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

bool SkDrawTreatAAStrokeAsHairline(SkScalar strokeWidth, const SkMatrix& matrix,
                                   SkScalar* coverage) {
    SkASSERT(strokeWidth > 0);
    // We need to try to fake a thick-stroke with a modulated hairline.

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
        if (coverage) {
            *coverage = SkScalarAve(len0, len1);
        }
        return true;
    }
    return false;
}

void SkDraw::drawRRect(const SkRRect& rrect, const SkPaint& paint) const {
    SkDEBUGCODE(this->validate());

    if (fRC->isEmpty()) {
        return;
    }

    {
        // TODO: Investigate optimizing these options. They are in the same
        // order as SkDraw::drawPath, which handles each case. It may be
        // that there is no way to optimize for these using the SkRRect path.
        SkScalar coverage;
        if (SkDrawTreatAsHairline(paint, *fMatrix, &coverage)) {
            goto DRAW_PATH;
        }

        if (paint.getPathEffect() || paint.getStyle() != SkPaint::kFill_Style) {
            goto DRAW_PATH;
        }

        if (paint.getRasterizer()) {
            goto DRAW_PATH;
        }
    }

    if (paint.getMaskFilter()) {
        // Transform the rrect into device space.
        SkRRect devRRect;
        if (rrect.transform(*fMatrix, &devRRect)) {
            SkAutoBlitterChoose blitter(fDst, *fMatrix, paint);
            if (paint.getMaskFilter()->filterRRect(devRRect, *fMatrix, *fRC, blitter.get(),
                                                   SkPaint::kFill_Style)) {
                return; // filterRRect() called the blitter, so we're done
            }
        }
    }

DRAW_PATH:
    // Now fall back to the default case of using a path.
    SkPath path;
    path.addRRect(rrect);
    this->drawPath(path, paint, NULL, true);
}

static SkScalar compute_res_scale_for_stroking(const SkMatrix& matrix) {
    if (!matrix.hasPerspective()) {
        SkScalar sx = SkPoint::Length(matrix[SkMatrix::kMScaleX], matrix[SkMatrix::kMSkewY]);
        SkScalar sy = SkPoint::Length(matrix[SkMatrix::kMSkewX],  matrix[SkMatrix::kMScaleY]);
        if (SkScalarsAreFinite(sx, sy)) {
            return SkTMax(sx, sy);
        }
    }
    return 1;
}

void SkDraw::drawPath(const SkPath& origSrcPath, const SkPaint& origPaint,
                      const SkMatrix* prePathMatrix, bool pathIsMutable,
                      bool drawCoverage, SkBlitter* customBlitter) const {
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
    tmpPath.setIsVolatile(true);

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
            tmpMatrix.setConcat(*matrix, *prePathMatrix);
            matrix = &tmpMatrix;
        }
    }
    // at this point we're done with prePathMatrix
    SkDEBUGCODE(prePathMatrix = (const SkMatrix*)0x50FF8001;)

    SkTCopyOnFirstWrite<SkPaint> paint(origPaint);

    {
        SkScalar coverage;
        if (SkDrawTreatAsHairline(origPaint, *matrix, &coverage)) {
            if (SK_Scalar1 == coverage) {
                paint.writable()->setStrokeWidth(0);
            } else if (SkXfermode::SupportsCoverageAsAlpha(origPaint.getXfermode())) {
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
                SkPaint* writablePaint = paint.writable();
                writablePaint->setStrokeWidth(0);
                writablePaint->setAlpha(newAlpha);
            }
        }
    }

    if (paint->getPathEffect() || paint->getStyle() != SkPaint::kFill_Style) {
        SkRect cullRect;
        const SkRect* cullRectPtr = NULL;
        if (this->computeConservativeLocalClipBounds(&cullRect)) {
            cullRectPtr = &cullRect;
        }
        doFill = paint->getFillPath(*pathPtr, &tmpPath, cullRectPtr,
                                    compute_res_scale_for_stroking(*fMatrix));
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

    SkBlitter* blitter = NULL;
    SkAutoBlitterChoose blitterStorage;
    if (NULL == customBlitter) {
        blitterStorage.choose(fDst, *fMatrix, *paint, drawCoverage);
        blitter = blitterStorage.get();
    } else {
        blitter = customBlitter;
    }

    if (paint->getMaskFilter()) {
        SkPaint::Style style = doFill ? SkPaint::kFill_Style :
            SkPaint::kStroke_Style;
        if (paint->getMaskFilter()->filterPath(*devPathPtr, *fMatrix, *fRC, blitter, style)) {
            return; // filterPath() called the blitter, so we're done
        }
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
    proc(*devPathPtr, *fRC, blitter);
}

/** For the purposes of drawing bitmaps, if a matrix is "almost" translate
    go ahead and treat it as if it were, so that subsequent code can go fast.
 */
static bool just_translate(const SkMatrix& matrix, const SkBitmap& bitmap) {
    unsigned bits = 0;  // TODO: find a way to allow the caller to tell us to
                        // respect filtering.
    return SkTreatAsSprite(matrix, bitmap.width(), bitmap.height(), bits);
}

void SkDraw::drawBitmapAsMask(const SkBitmap& bitmap,
                              const SkPaint& paint) const {
    SkASSERT(bitmap.colorType() == kAlpha_8_SkColorType);

    if (just_translate(*fMatrix, bitmap)) {
        int ix = SkScalarRoundToInt(fMatrix->getTranslateX());
        int iy = SkScalarRoundToInt(fMatrix->getTranslateY());

        SkAutoPixmapUnlock result;
        if (!bitmap.requestLock(&result)) {
            return;
        }
        const SkPixmap& pmap = result.pixmap();
        SkMask  mask;
        mask.fBounds.set(ix, iy, ix + pmap.width(), iy + pmap.height());
        mask.fFormat = SkMask::kA8_Format;
        mask.fRowBytes = SkToU32(pmap.rowBytes());
        // fImage is typed as writable, but in this case it is used read-only
        mask.fImage = (uint8_t*)pmap.addr8(0, 0);

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
            devBounds.set(0, 0, fDst.width(), fDst.height());
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
            device.installPixels(SkImageInfo::MakeA8(mask.fBounds.width(), mask.fBounds.height()),
                                 mask.fImage, mask.fRowBytes);

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
    m.mapRect(&dstR, srcR);
    return c.quickReject(dstR.roundOut());
}

static bool clipped_out(const SkMatrix& matrix, const SkRasterClip& clip,
                        int width, int height) {
    SkRect  r;
    r.set(0, 0, SkIntToScalar(width), SkIntToScalar(height));
    return clipped_out(matrix, clip, r);
}

static bool clipHandlesSprite(const SkRasterClip& clip, int x, int y, const SkPixmap& pmap) {
    return clip.isBW() || clip.quickContains(x, y, x + pmap.width(), y + pmap.height());
}

void SkDraw::drawBitmap(const SkBitmap& bitmap, const SkMatrix& prematrix,
                        const SkRect* dstBounds, const SkPaint& origPaint) const {
    SkDEBUGCODE(this->validate();)

    // nothing to draw
    if (fRC->isEmpty() ||
            bitmap.width() == 0 || bitmap.height() == 0 ||
            bitmap.colorType() == kUnknown_SkColorType) {
        return;
    }

    SkPaint paint(origPaint);
    paint.setStyle(SkPaint::kFill_Style);

    SkMatrix matrix;
    matrix.setConcat(*fMatrix, prematrix);

    if (clipped_out(matrix, *fRC, bitmap.width(), bitmap.height())) {
        return;
    }

    if (bitmap.colorType() != kAlpha_8_SkColorType && just_translate(matrix, bitmap)) {
        //
        // It is safe to call lock pixels now, since we know the matrix is
        // (more or less) identity.
        //
        SkAutoPixmapUnlock unlocker;
        if (!bitmap.requestLock(&unlocker)) {
            return;
        }
        const SkPixmap& pmap = unlocker.pixmap();
        int ix = SkScalarRoundToInt(matrix.getTranslateX());
        int iy = SkScalarRoundToInt(matrix.getTranslateY());
        if (clipHandlesSprite(*fRC, ix, iy, pmap)) {
            SkTBlitterAllocator allocator;
            // blitter will be owned by the allocator.
            SkBlitter* blitter = SkBlitter::ChooseSprite(fDst, paint, pmap, ix, iy, &allocator);
            if (blitter) {
                SkScan::FillIRect(SkIRect::MakeXYWH(ix, iy, pmap.width(), pmap.height()),
                                  *fRC, blitter);
                return;
            }
            // if !blitter, then we fall-through to the slower case
        }
    }

    // now make a temp draw on the stack, and use it
    //
    SkDraw draw(*this);
    draw.fMatrix = &matrix;

    if (bitmap.colorType() == kAlpha_8_SkColorType) {
        draw.drawBitmapAsMask(bitmap, paint);
    } else {
        SkAutoBitmapShaderInstall install(bitmap, paint);
        const SkPaint& paintWithShader = install.paintWithShader();
        const SkRect srcBounds = SkRect::MakeIWH(bitmap.width(), bitmap.height());
        if (dstBounds) {
            this->drawRect(srcBounds, paintWithShader, &prematrix, dstBounds);
        } else {
            draw.drawRect(srcBounds, paintWithShader);
        }
    }
}

void SkDraw::drawSprite(const SkBitmap& bitmap, int x, int y, const SkPaint& origPaint) const {
    SkDEBUGCODE(this->validate();)

    // nothing to draw
    if (fRC->isEmpty() ||
            bitmap.width() == 0 || bitmap.height() == 0 ||
            bitmap.colorType() == kUnknown_SkColorType) {
        return;
    }

    const SkIRect bounds = SkIRect::MakeXYWH(x, y, bitmap.width(), bitmap.height());

    if (fRC->quickReject(bounds)) {
        return; // nothing to draw
    }

    SkPaint paint(origPaint);
    paint.setStyle(SkPaint::kFill_Style);

    SkAutoPixmapUnlock unlocker;
    if (!bitmap.requestLock(&unlocker)) {
        return;
    }
    const SkPixmap& pmap = unlocker.pixmap();

    if (NULL == paint.getColorFilter() && clipHandlesSprite(*fRC, x, y, pmap)) {
        SkTBlitterAllocator allocator;
        // blitter will be owned by the allocator.
        SkBlitter* blitter = SkBlitter::ChooseSprite(fDst, paint, pmap, x, y, &allocator);
        if (blitter) {
            SkScan::FillIRect(bounds, *fRC, blitter);
            return;
        }
    }

    SkMatrix        matrix;
    SkRect          r;

    // get a scalar version of our rect
    r.set(bounds);

    // create shader with offset
    matrix.setTranslate(r.fLeft, r.fTop);
    SkAutoBitmapShaderInstall install(bitmap, paint, &matrix);
    const SkPaint& shaderPaint = install.paintWithShader();

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

bool SkDraw::ShouldDrawTextAsPaths(const SkPaint& paint, const SkMatrix& ctm) {
    // hairline glyphs are fast enough so we don't need to cache them
    if (SkPaint::kStroke_Style == paint.getStyle() && 0 == paint.getStrokeWidth()) {
        return true;
    }

    // we don't cache perspective
    if (ctm.hasPerspective()) {
        return true;
    }

    SkMatrix textM;
    return SkPaint::TooBigToUseCache(ctm, *paint.setTextMatrix(&textM));
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

static void D1G_RectClip(const SkDraw1Glyph& state, Sk48Dot16 fx, Sk48Dot16 fy, const SkGlyph& glyph) {
    // Prevent glyphs from being drawn outside of or straddling the edge of device space.
    if ((fx >> 16) > INT_MAX - (INT16_MAX + UINT16_MAX) ||
        (fx >> 16) < INT_MIN - (INT16_MIN + 0 /*UINT16_MIN*/) ||
        (fy >> 16) > INT_MAX - (INT16_MAX + UINT16_MAX) ||
        (fy >> 16) < INT_MIN - (INT16_MIN + 0 /*UINT16_MIN*/))
    {
        return;
    }

    int left = Sk48Dot16FloorToInt(fx);
    int top = Sk48Dot16FloorToInt(fy);
    SkASSERT(glyph.fWidth > 0 && glyph.fHeight > 0);
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
    state.blitMask(mask, *bounds);
}

static void D1G_RgnClip(const SkDraw1Glyph& state, Sk48Dot16 fx, Sk48Dot16 fy, const SkGlyph& glyph) {
    int left = Sk48Dot16FloorToInt(fx);
    int top = Sk48Dot16FloorToInt(fy);
    SkASSERT(glyph.fWidth > 0 && glyph.fHeight > 0);
    SkASSERT(!state.fClip->isRect());

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
            state.blitMask(mask, cr);
            clipper.next();
        } while (!clipper.done());
    }
}

static bool hasCustomD1GProc(const SkDraw& draw) {
    return draw.fProcs && draw.fProcs->fD1GProc;
}

static bool needsRasterTextBlit(const SkDraw& draw) {
    return !hasCustomD1GProc(draw);
}

SkDraw1Glyph::Proc SkDraw1Glyph::init(const SkDraw* draw, SkBlitter* blitter, SkGlyphCache* cache,
                                      const SkPaint& pnt) {
    fDraw = draw;
    fBlitter = blitter;
    fCache = cache;
    fPaint = &pnt;

    if (cache->isSubpixel()) {
        fHalfSampleX = fHalfSampleY = SkFixedToScalar(SkGlyph::kSubpixelRound);
    } else {
        fHalfSampleX = fHalfSampleY = SK_ScalarHalf;
    }

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
        if (fClip->isRect()) {
            return D1G_RectClip;
        } else {
            return D1G_RgnClip;
        }
    } else {    // aaclip
        fAAClip = &draw->fRC->aaRgn();
        fClip = NULL;
        fClipBounds = fAAClip->getBounds();
        return D1G_RectClip;
    }
}

void SkDraw1Glyph::blitMaskAsSprite(const SkMask& mask) const {
    SkASSERT(SkMask::kARGB32_Format == mask.fFormat);

    SkBitmap bm;
    bm.installPixels(SkImageInfo::MakeN32Premul(mask.fBounds.width(), mask.fBounds.height()),
                     (SkPMColor*)mask.fImage, mask.fRowBytes);

    fDraw->drawSprite(bm, mask.fBounds.x(), mask.fBounds.y(), *fPaint);
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

    // SkScalarRec doesn't currently have a way of representing hairline stroke and
    // will fill if its frame-width is 0.
    if (ShouldDrawTextAsPaths(paint, *fMatrix)) {
        this->drawText_asPaths(text, byteLength, x, y, paint);
        return;
    }

    SkDrawCacheProc glyphCacheProc = paint.getDrawCacheProc();

    SkAutoGlyphCache    autoCache(paint, &fDevice->surfaceProps(), fMatrix);
    SkGlyphCache*       cache = autoCache.getCache();

    // transform our starting point
    {
        SkPoint loc;
        fMatrix->mapXY(x, y, &loc);
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

    const char* stop = text + byteLength;

    SkAAClipBlitter     aaBlitter;
    SkAutoBlitterChoose blitterChooser;
    SkBlitter*          blitter = NULL;
    if (needsRasterTextBlit(*this)) {
        blitterChooser.choose(fDst, *fMatrix, paint);
        blitter = blitterChooser.get();
        if (fRC->isAA()) {
            aaBlitter.init(blitter, &fRC->aaRgn());
            blitter = &aaBlitter;
        }
    }

    SkAutoKern          autokern;
    SkDraw1Glyph        d1g;
    SkDraw1Glyph::Proc  proc = d1g.init(this, blitter, cache, paint);

    SkFixed fxMask = ~0;
    SkFixed fyMask = ~0;
    if (cache->isSubpixel()) {
        SkAxisAlignment baseline = SkComputeAxisAlignmentForHText(*fMatrix);
        if (kX_SkAxisAlignment == baseline) {
            fyMask = 0;
            d1g.fHalfSampleY = SK_ScalarHalf;
        } else if (kY_SkAxisAlignment == baseline) {
            fxMask = 0;
            d1g.fHalfSampleX = SK_ScalarHalf;
        }
    }

    Sk48Dot16 fx = SkScalarTo48Dot16(x + d1g.fHalfSampleX);
    Sk48Dot16 fy = SkScalarTo48Dot16(y + d1g.fHalfSampleY);

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

//////////////////////////////////////////////////////////////////////////////

void SkDraw::drawPosText_asPaths(const char text[], size_t byteLength,
                                 const SkScalar pos[], int scalarsPerPosition,
                                 const SkPoint& offset, const SkPaint& origPaint) const {
    // setup our std paint, in hopes of getting hits in the cache
    SkPaint paint(origPaint);
    SkScalar matrixScale = paint.setupForAsPaths();

    SkMatrix matrix;
    matrix.setScale(matrixScale, matrixScale);

    // Temporarily jam in kFill, so we only ever ask for the raw outline from the cache.
    paint.setStyle(SkPaint::kFill_Style);
    paint.setPathEffect(NULL);

    SkDrawCacheProc     glyphCacheProc = paint.getDrawCacheProc();
    SkAutoGlyphCache    autoCache(paint, &fDevice->surfaceProps(), NULL);
    SkGlyphCache*       cache = autoCache.getCache();

    const char*        stop = text + byteLength;
    SkTextAlignProc    alignProc(paint.getTextAlign());
    SkTextMapStateProc tmsProc(SkMatrix::I(), offset, scalarsPerPosition);

    // Now restore the original settings, so we "draw" with whatever style/stroking.
    paint.setStyle(origPaint.getStyle());
    paint.setPathEffect(origPaint.getPathEffect());

    while (text < stop) {
        const SkGlyph& glyph = glyphCacheProc(cache, &text, 0, 0);
        if (glyph.fWidth) {
            const SkPath* path = cache->findPath(glyph);
            if (path) {
                SkPoint tmsLoc;
                tmsProc(pos, &tmsLoc);
                SkPoint loc;
                alignProc(tmsLoc, glyph, &loc);

                matrix[SkMatrix::kMTransX] = loc.fX;
                matrix[SkMatrix::kMTransY] = loc.fY;
                if (fDevice) {
                    fDevice->drawPath(*this, *path, paint, &matrix, false);
                } else {
                    this->drawPath(*path, paint, &matrix, false);
                }
            }
        }
        pos += scalarsPerPosition;
    }
}

void SkDraw::drawPosText(const char text[], size_t byteLength,
                         const SkScalar pos[], int scalarsPerPosition,
                         const SkPoint& offset, const SkPaint& paint) const {
    SkASSERT(byteLength == 0 || text != NULL);
    SkASSERT(1 == scalarsPerPosition || 2 == scalarsPerPosition);

    SkDEBUGCODE(this->validate();)

    // nothing to draw
    if (text == NULL || byteLength == 0 || fRC->isEmpty()) {
        return;
    }

    if (ShouldDrawTextAsPaths(paint, *fMatrix)) {
        this->drawPosText_asPaths(text, byteLength, pos, scalarsPerPosition, offset, paint);
        return;
    }

    SkDrawCacheProc     glyphCacheProc = paint.getDrawCacheProc();
    SkAutoGlyphCache    autoCache(paint, &fDevice->surfaceProps(), fMatrix);
    SkGlyphCache*       cache = autoCache.getCache();

    SkAAClipBlitterWrapper wrapper;
    SkAutoBlitterChoose blitterChooser;
    SkBlitter* blitter = NULL;
    if (needsRasterTextBlit(*this)) {
        blitterChooser.choose(fDst, *fMatrix, paint);
        blitter = blitterChooser.get();
        if (fRC->isAA()) {
            wrapper.init(*fRC, blitter);
            blitter = wrapper.getBlitter();
        }
    }

    const char*        stop = text + byteLength;
    SkTextAlignProc    alignProc(paint.getTextAlign());
    SkDraw1Glyph       d1g;
    SkDraw1Glyph::Proc proc = d1g.init(this, blitter, cache, paint);
    SkTextMapStateProc tmsProc(*fMatrix, offset, scalarsPerPosition);

    if (cache->isSubpixel()) {
        // maybe we should skip the rounding if linearText is set
        SkAxisAlignment baseline = SkComputeAxisAlignmentForHText(*fMatrix);

        SkFixed fxMask = ~0;
        SkFixed fyMask = ~0;
        if (kX_SkAxisAlignment == baseline) {
            fyMask = 0;
            d1g.fHalfSampleY = SK_ScalarHalf;
        } else if (kY_SkAxisAlignment == baseline) {
            fxMask = 0;
            d1g.fHalfSampleX = SK_ScalarHalf;
        }

        if (SkPaint::kLeft_Align == paint.getTextAlign()) {
            while (text < stop) {
                SkPoint tmsLoc;
                tmsProc(pos, &tmsLoc);

                Sk48Dot16 fx = SkScalarTo48Dot16(tmsLoc.fX + d1g.fHalfSampleX);
                Sk48Dot16 fy = SkScalarTo48Dot16(tmsLoc.fY + d1g.fHalfSampleY);

                const SkGlyph& glyph = glyphCacheProc(cache, &text, fx & fxMask, fy & fyMask);

                if (glyph.fWidth) {
                    proc(d1g, fx, fy, glyph);
                }
                pos += scalarsPerPosition;
            }
        } else {
            while (text < stop) {
                const char* currentText = text;
                const SkGlyph& metricGlyph = glyphCacheProc(cache, &text, 0, 0);

                if (metricGlyph.fWidth) {
                    SkDEBUGCODE(SkFixed prevAdvX = metricGlyph.fAdvanceX;)
                    SkDEBUGCODE(SkFixed prevAdvY = metricGlyph.fAdvanceY;)
                    SkPoint tmsLoc;
                    tmsProc(pos, &tmsLoc);

                    SkPoint alignLoc;
                    alignProc(tmsLoc, metricGlyph, &alignLoc);

                    Sk48Dot16 fx = SkScalarTo48Dot16(alignLoc.fX + d1g.fHalfSampleX);
                    Sk48Dot16 fy = SkScalarTo48Dot16(alignLoc.fY + d1g.fHalfSampleY);

                    // have to call again, now that we've been "aligned"
                    const SkGlyph& glyph = glyphCacheProc(cache, &currentText,
                                                          fx & fxMask, fy & fyMask);
                    // the assumption is that the metrics haven't changed
                    SkASSERT(prevAdvX == glyph.fAdvanceX);
                    SkASSERT(prevAdvY == glyph.fAdvanceY);
                    SkASSERT(glyph.fWidth);

                    proc(d1g, fx, fy, glyph);
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
                    SkPoint tmsLoc;
                    tmsProc(pos, &tmsLoc);

                    proc(d1g,
                         SkScalarTo48Dot16(tmsLoc.fX + SK_ScalarHalf), //d1g.fHalfSampleX,
                         SkScalarTo48Dot16(tmsLoc.fY + SK_ScalarHalf), //d1g.fHalfSampleY,
                         glyph);
                }
                pos += scalarsPerPosition;
            }
        } else {
            while (text < stop) {
                // the last 2 parameters are ignored
                const SkGlyph& glyph = glyphCacheProc(cache, &text, 0, 0);

                if (glyph.fWidth) {
                    SkPoint tmsLoc;
                    tmsProc(pos, &tmsLoc);

                    SkPoint alignLoc;
                    alignProc(tmsLoc, glyph, &alignLoc);

                    proc(d1g,
                         SkScalarTo48Dot16(alignLoc.fX + SK_ScalarHalf), //d1g.fHalfSampleX,
                         SkScalarTo48Dot16(alignLoc.fY + SK_ScalarHalf), //d1g.fHalfSampleY,
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

static SkScan::HairRCProc ChooseHairProc(bool doAntiAlias) {
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

    size_t contextSize() const override;

    class TriColorShaderContext : public SkShader::Context {
    public:
        TriColorShaderContext(const SkTriColorShader& shader, const ContextRec&);
        virtual ~TriColorShaderContext();

        bool setup(const SkPoint pts[], const SkColor colors[], int, int, int);

        void shadeSpan(int x, int y, SkPMColor dstC[], int count) override;

    private:
        SkMatrix    fDstToUnit;
        SkPMColor   fColors[3];

        typedef SkShader::Context INHERITED;
    };

    SK_TO_STRING_OVERRIDE()

    // For serialization.  This will never be called.
    Factory getFactory() const override { sk_throw(); return NULL; }

protected:
    Context* onCreateContext(const ContextRec& rec, void* storage) const override {
        return SkNEW_PLACEMENT_ARGS(storage, TriColorShaderContext, (*this, rec));
    }

private:
    typedef SkShader INHERITED;
};

bool SkTriColorShader::TriColorShaderContext::setup(const SkPoint pts[], const SkColor colors[],
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
    // We can't call getTotalInverse(), because we explicitly don't want to look at the localmatrix
    // as our interators are intrinsically tied to the vertices, and nothing else.
    SkMatrix ctmInv;
    if (!this->getCTM().invert(&ctmInv)) {
        return false;
    }
    fDstToUnit.setConcat(im, ctmInv);
    return true;
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


SkTriColorShader::TriColorShaderContext::TriColorShaderContext(const SkTriColorShader& shader,
                                                               const ContextRec& rec)
    : INHERITED(shader, rec) {}

SkTriColorShader::TriColorShaderContext::~TriColorShaderContext() {}

size_t SkTriColorShader::contextSize() const {
    return sizeof(TriColorShaderContext);
}
void SkTriColorShader::TriColorShaderContext::shadeSpan(int x, int y, SkPMColor dstC[], int count) {
    const int alphaScale = Sk255To256(this->getPaintAlpha());

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

        if (256 != alphaScale) {
            scale0 = SkAlphaMul(scale0, alphaScale);
            scale1 = SkAlphaMul(scale1, alphaScale);
            scale2 = SkAlphaMul(scale2, alphaScale);
        }

        dstC[i] = SkAlphaMulQ(fColors[0], scale0) +
                  SkAlphaMulQ(fColors[1], scale1) +
                  SkAlphaMulQ(fColors[2], scale2);
    }
}

#ifndef SK_IGNORE_TO_STRING
void SkTriColorShader::toString(SkString* str) const {
    str->append("SkTriColorShader: (");

    this->INHERITED::toString(str);

    str->append(")");
}
#endif

void SkDraw::drawVertices(SkCanvas::VertexMode vmode, int count,
                          const SkPoint vertices[], const SkPoint textures[],
                          const SkColor colors[], SkXfermode* xmode,
                          const uint16_t indices[], int indexCount,
                          const SkPaint& paint) const {
    SkASSERT(0 == count || vertices);

    // abort early if there is nothing to draw
    if (count < 3 || (indices && indexCount < 3) || fRC->isEmpty()) {
        return;
    }

    // transform out vertices into device coordinates
    SkAutoSTMalloc<16, SkPoint> storage(count);
    SkPoint* devVerts = storage.get();
    fMatrix->mapPoints(devVerts, vertices, count);

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
    SkAutoTUnref<SkComposeShader> composeShader;
    if (colors) {
        if (NULL == textures) {
            // just colors (no texture)
            shader = p.setShader(&triShader);
        } else {
            // colors * texture
            SkASSERT(shader);
            bool releaseMode = false;
            if (NULL == xmode) {
                xmode = SkXfermode::Create(SkXfermode::kModulate_Mode);
                releaseMode = true;
            }
            composeShader.reset(SkNEW_ARGS(SkComposeShader, (&triShader, shader, xmode)));
            p.setShader(composeShader);
            if (releaseMode) {
                xmode->unref();
            }
        }
    }

    SkAutoBlitterChoose blitter(fDst, *fMatrix, p);
    // Abort early if we failed to create a shader context.
    if (blitter->isNullBlitter()) {
        return;
    }

    // setup our state and function pointer for iterating triangles
    VertState       state(count, indices, indexCount);
    VertState::Proc vertProc = state.chooseProc(vmode);

    if (textures || colors) {
        while (vertProc(&state)) {
            if (textures) {
                SkMatrix tempM;
                if (texture_to_matrix(state, vertices, textures, &tempM)) {
                    SkShader::ContextRec rec(p, *fMatrix, &tempM);
                    if (!blitter->resetShaderContext(rec)) {
                        continue;
                    }
                }
            }
            if (colors) {
                // Find the context for triShader.
                SkTriColorShader::TriColorShaderContext* triColorShaderContext;

                SkShader::Context* shaderContext = blitter->getShaderContext();
                SkASSERT(shaderContext);
                if (p.getShader() == &triShader) {
                    triColorShaderContext =
                            static_cast<SkTriColorShader::TriColorShaderContext*>(shaderContext);
                } else {
                    // The shader is a compose shader and triShader is its first shader.
                    SkASSERT(p.getShader() == composeShader);
                    SkASSERT(composeShader->getShaderA() == &triShader);
                    SkComposeShader::ComposeShaderContext* composeShaderContext =
                            static_cast<SkComposeShader::ComposeShaderContext*>(shaderContext);
                    SkShader::Context* shaderContextA = composeShaderContext->getShaderContextA();
                    triColorShaderContext =
                            static_cast<SkTriColorShader::TriColorShaderContext*>(shaderContextA);
                }

                if (!triColorShaderContext->setup(vertices, colors,
                                                  state.f0, state.f1, state.f2)) {
                    continue;
                }
            }

            SkPoint tmp[] = {
                devVerts[state.f0], devVerts[state.f1], devVerts[state.f2]
            };
            SkScan::FillTriangle(tmp, *fRC, blitter.get());
        }
    } else {
        // no colors[] and no texture, stroke hairlines with paint's color.
        SkScan::HairRCProc hairProc = ChooseHairProc(paint.isAntiAlias());
        const SkRasterClip& clip = *fRC;
        while (vertProc(&state)) {
            SkPoint array[] = {
                devVerts[state.f0], devVerts[state.f1], devVerts[state.f2], devVerts[state.f0]
            };
            hairProc(array, 4, clip, blitter.get());
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG

void SkDraw::validate() const {
    SkASSERT(fMatrix != NULL);
    SkASSERT(fClip != NULL);
    SkASSERT(fRC != NULL);

    const SkIRect&  cr = fRC->getBounds();
    SkIRect         br;

    br.set(0, 0, fDst.width(), fDst.height());
    SkASSERT(cr.isEmpty() || br.contains(cr));
}

#endif

////////////////////////////////////////////////////////////////////////////////////////////////

#include "SkPath.h"
#include "SkDraw.h"
#include "SkRegion.h"
#include "SkBlitter.h"

static bool compute_bounds(const SkPath& devPath, const SkIRect* clipBounds,
                           const SkMaskFilter* filter, const SkMatrix* filterMatrix,
                           SkIRect* bounds) {
    if (devPath.isEmpty()) {
        return false;
    }

    //  init our bounds from the path
    *bounds = devPath.getBounds().makeOutset(SK_ScalarHalf, SK_ScalarHalf).roundOut();

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
        // Ugh. Guard against gigantic margins from wacky filters. Without this
        // check we can request arbitrary amounts of slop beyond our visible
        // clip, and bring down the renderer (at least on finite RAM machines
        // like handsets, etc.). Need to balance this invented value between
        // quality of large filters like blurs, and the corresponding memory
        // requests.
        static const int MAX_MARGIN = 128;
        if (!bounds->intersect(clipBounds->makeOutset(SkMin32(margin.fX, MAX_MARGIN),
                                                      SkMin32(margin.fY, MAX_MARGIN)))) {
            return false;
        }
    }

    return true;
}

static void draw_into_mask(const SkMask& mask, const SkPath& devPath, SkPaint::Style style) {
    SkDraw draw;
    if (!draw.fDst.reset(mask)) {
        return;
    }

    SkRasterClip    clip;
    SkMatrix        matrix;
    SkPaint         paint;

    clip.setRect(SkIRect::MakeWH(mask.fBounds.width(), mask.fBounds.height()));
    matrix.setTranslate(-SkIntToScalar(mask.fBounds.fLeft),
                        -SkIntToScalar(mask.fBounds.fTop));

    draw.fRC        = &clip;
    draw.fClip      = &clip.bwRgn();
    draw.fMatrix    = &matrix;
    paint.setAntiAlias(true);
    paint.setStyle(style);
    draw.drawPath(devPath, paint);
}

bool SkDraw::DrawToMask(const SkPath& devPath, const SkIRect* clipBounds,
                        const SkMaskFilter* filter, const SkMatrix* filterMatrix,
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
