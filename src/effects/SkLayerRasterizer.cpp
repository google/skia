
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkLayerRasterizer.h"
#include "SkBuffer.h"
#include "SkDraw.h"
#include "SkMask.h"
#include "SkMaskFilter.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "../core/SkRasterClip.h"
#include "SkXfermode.h"
#include <new>

struct SkLayerRasterizer_Rec {
    SkPaint     fPaint;
    SkVector    fOffset;
};

SkLayerRasterizer::SkLayerRasterizer() : fLayers(sizeof(SkLayerRasterizer_Rec))
{
}

SkLayerRasterizer::~SkLayerRasterizer() {
    SkDeque::F2BIter        iter(fLayers);
    SkLayerRasterizer_Rec*  rec;

    while ((rec = (SkLayerRasterizer_Rec*)iter.next()) != NULL)
        rec->fPaint.~SkPaint();
}

void SkLayerRasterizer::addLayer(const SkPaint& paint, SkScalar dx,
                                 SkScalar dy) {
    SkLayerRasterizer_Rec* rec = (SkLayerRasterizer_Rec*)fLayers.push_back();

    new (&rec->fPaint) SkPaint(paint);
    rec->fOffset.set(dx, dy);
}

static bool compute_bounds(const SkDeque& layers, const SkPath& path,
                           const SkMatrix& matrix,
                           const SkIRect* clipBounds, SkIRect* bounds) {
    SkDeque::F2BIter        iter(layers);
    SkLayerRasterizer_Rec*  rec;

    bounds->set(SK_MaxS32, SK_MaxS32, SK_MinS32, SK_MinS32);

    while ((rec = (SkLayerRasterizer_Rec*)iter.next()) != NULL) {
        const SkPaint&  paint = rec->fPaint;
        SkPath          fillPath, devPath;
        const SkPath*   p = &path;

        if (paint.getPathEffect() || paint.getStyle() != SkPaint::kFill_Style) {
            paint.getFillPath(path, &fillPath);
            p = &fillPath;
        }
        if (p->isEmpty()) {
            continue;
        }

        // apply the matrix and offset
        {
            SkMatrix m = matrix;
            m.preTranslate(rec->fOffset.fX, rec->fOffset.fY);
            p->transform(m, &devPath);
        }

        SkMask  mask;
        if (!SkDraw::DrawToMask(devPath, clipBounds, paint.getMaskFilter(),
                                &matrix, &mask,
                                SkMask::kJustComputeBounds_CreateMode)) {
            return false;
        }

        bounds->join(mask.fBounds);
    }
    return true;
}

bool SkLayerRasterizer::onRasterize(const SkPath& path, const SkMatrix& matrix,
                                    const SkIRect* clipBounds,
                                    SkMask* mask, SkMask::CreateMode mode) {
    if (fLayers.empty()) {
        return false;
    }

    if (SkMask::kJustRenderImage_CreateMode != mode) {
        if (!compute_bounds(fLayers, path, matrix, clipBounds, &mask->fBounds))
            return false;
    }

    if (SkMask::kComputeBoundsAndRenderImage_CreateMode == mode) {
        mask->fFormat   = SkMask::kA8_Format;
        mask->fRowBytes = mask->fBounds.width();
        size_t size = mask->computeImageSize();
        if (0 == size) {
            return false;   // too big to allocate, abort
        }
        mask->fImage = SkMask::AllocImage(size);
        memset(mask->fImage, 0, size);
    }

    if (SkMask::kJustComputeBounds_CreateMode != mode) {
        SkBitmap        device;
        SkRasterClip    rectClip;
        SkDraw          draw;
        SkMatrix        translatedMatrix;  // this translates us to our local pixels
        SkMatrix        drawMatrix;        // this translates the path by each layer's offset

        rectClip.setRect(SkIRect::MakeWH(mask->fBounds.width(), mask->fBounds.height()));

        translatedMatrix = matrix;
        translatedMatrix.postTranslate(-SkIntToScalar(mask->fBounds.fLeft),
                                       -SkIntToScalar(mask->fBounds.fTop));

        device.setConfig(SkBitmap::kA8_Config, mask->fBounds.width(), mask->fBounds.height(), mask->fRowBytes);
        device.setPixels(mask->fImage);

        draw.fBitmap    = &device;
        draw.fMatrix    = &drawMatrix;
        draw.fRC        = &rectClip;
        draw.fClip      = &rectClip.bwRgn();
        // we set the matrixproc in the loop, as the matrix changes each time (potentially)
        draw.fBounder   = NULL;

        SkDeque::F2BIter        iter(fLayers);
        SkLayerRasterizer_Rec*  rec;

        while ((rec = (SkLayerRasterizer_Rec*)iter.next()) != NULL) {
            drawMatrix = translatedMatrix;
            drawMatrix.preTranslate(rec->fOffset.fX, rec->fOffset.fY);
            draw.drawPath(path, rec->fPaint);
        }
    }
    return true;
}

/////////// Routines for flattening /////////////////

static void paint_read(SkPaint* paint, SkFlattenableReadBuffer& buffer) {
    paint->setAntiAlias(buffer.readBool());
    paint->setStyle((SkPaint::Style)buffer.readU8());
    paint->setAlpha(buffer.readU8());

    if (paint->getStyle() != SkPaint::kFill_Style) {
        paint->setStrokeWidth(buffer.readScalar());
        paint->setStrokeMiter(buffer.readScalar());
        paint->setStrokeCap((SkPaint::Cap)buffer.readU8());
        paint->setStrokeJoin((SkPaint::Join)buffer.readU8());
    }

    SkSafeUnref(paint->setMaskFilter((SkMaskFilter*)buffer.readFlattenable()));
    SkSafeUnref(paint->setPathEffect((SkPathEffect*)buffer.readFlattenable()));
    SkSafeUnref(paint->setRasterizer((SkRasterizer*)buffer.readFlattenable()));
    SkSafeUnref(paint->setXfermode((SkXfermode*)buffer.readFlattenable()));
}

static void paint_write(const SkPaint& paint, SkFlattenableWriteBuffer& buffer) {
    buffer.writeBool(paint.isAntiAlias());
    buffer.write8(paint.getStyle());
    buffer.write8(paint.getAlpha());

    if (paint.getStyle() != SkPaint::kFill_Style) {
        buffer.writeScalar(paint.getStrokeWidth());
        buffer.writeScalar(paint.getStrokeMiter());
        buffer.write8(paint.getStrokeCap());
        buffer.write8(paint.getStrokeJoin());
    }

    buffer.writeFlattenable(paint.getMaskFilter());
    buffer.writeFlattenable(paint.getPathEffect());
    buffer.writeFlattenable(paint.getRasterizer());
    buffer.writeFlattenable(paint.getXfermode());
}

SkLayerRasterizer::SkLayerRasterizer(SkFlattenableReadBuffer& buffer)
    : SkRasterizer(buffer), fLayers(sizeof(SkLayerRasterizer_Rec)) {
    int count = buffer.readS32();

    for (int i = 0; i < count; i++) {
        SkLayerRasterizer_Rec* rec = (SkLayerRasterizer_Rec*)fLayers.push_back();

#if 0
        new (&rec->fPaint) SkPaint(buffer);
#else
        new (&rec->fPaint) SkPaint;
        paint_read(&rec->fPaint, buffer);
#endif
        rec->fOffset.fX = buffer.readScalar();
        rec->fOffset.fY = buffer.readScalar();
    }
}

void SkLayerRasterizer::flatten(SkFlattenableWriteBuffer& buffer) {
    this->INHERITED::flatten(buffer);

    buffer.write32(fLayers.count());

    SkDeque::F2BIter                iter(fLayers);
    const SkLayerRasterizer_Rec*    rec;

    while ((rec = (const SkLayerRasterizer_Rec*)iter.next()) != NULL) {
#if 0
        rec->fPaint.flatten(buffer);
#else
        paint_write(rec->fPaint, buffer);
#endif
        buffer.writeScalar(rec->fOffset.fX);
        buffer.writeScalar(rec->fOffset.fY);
    }
}

SkFlattenable* SkLayerRasterizer::CreateProc(SkFlattenableReadBuffer& buffer) {
    return SkNEW_ARGS(SkLayerRasterizer, (buffer));
}

SkFlattenable::Factory SkLayerRasterizer::getFactory() {
    return CreateProc;
}

static SkFlattenable::Registrar gReg("SkLayerRasterizer",
                                     SkLayerRasterizer::CreateProc);

