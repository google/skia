/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkLayerRasterizer.h"
#include "SkDraw.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkMask.h"
#include "SkMaskFilter.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkPathEffect.h"
#include "../core/SkRasterClip.h"
#include "SkXfermode.h"
#include <new>

struct SkLayerRasterizer_Rec {
    SkPaint     fPaint;
    SkVector    fOffset;
};

SkLayerRasterizer::SkLayerRasterizer() : fLayers(new SkDeque(sizeof(SkLayerRasterizer_Rec))) {}

SkLayerRasterizer::SkLayerRasterizer(SkDeque* layers) : fLayers(layers)
{
}

// Helper function to call destructors on SkPaints held by layers and delete layers.
static void clean_up_layers(SkDeque* layers) {
    SkDeque::F2BIter        iter(*layers);
    SkLayerRasterizer_Rec*  rec;

    while ((rec = (SkLayerRasterizer_Rec*)iter.next()) != nullptr)
        rec->fPaint.~SkPaint();

    delete layers;
}

SkLayerRasterizer::~SkLayerRasterizer() {
    SkASSERT(fLayers);
    clean_up_layers(const_cast<SkDeque*>(fLayers));
}

static bool compute_bounds(const SkDeque& layers, const SkPath& path,
                           const SkMatrix& matrix,
                           const SkIRect* clipBounds, SkIRect* bounds) {
    SkDeque::F2BIter        iter(layers);
    SkLayerRasterizer_Rec*  rec;

    bounds->set(SK_MaxS32, SK_MaxS32, SK_MinS32, SK_MinS32);

    while ((rec = (SkLayerRasterizer_Rec*)iter.next()) != nullptr) {
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
                                SkMask::kJustComputeBounds_CreateMode,
                                SkPaint::kFill_Style)) {
            return false;
        }

        bounds->join(mask.fBounds);
    }
    return true;
}

bool SkLayerRasterizer::onRasterize(const SkPath& path, const SkMatrix& matrix,
                                    const SkIRect* clipBounds,
                                    SkMask* mask, SkMask::CreateMode mode) const {
    SkASSERT(fLayers);
    if (fLayers->empty()) {
        return false;
    }

    if (SkMask::kJustRenderImage_CreateMode != mode) {
        if (!compute_bounds(*fLayers, path, matrix, clipBounds, &mask->fBounds))
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
        SkDraw          draw;
        if (!draw.fDst.reset(*mask)) {
            return false;
        }

        SkRasterClip    rectClip;
        SkMatrix        translatedMatrix;  // this translates us to our local pixels
        SkMatrix        drawMatrix;        // this translates the path by each layer's offset

        rectClip.setRect(SkIRect::MakeWH(mask->fBounds.width(), mask->fBounds.height()));

        translatedMatrix = matrix;
        translatedMatrix.postTranslate(-SkIntToScalar(mask->fBounds.fLeft),
                                       -SkIntToScalar(mask->fBounds.fTop));

        draw.fMatrix    = &drawMatrix;
        draw.fRC        = &rectClip;
        draw.fClip      = &rectClip.bwRgn();
        // we set the matrixproc in the loop, as the matrix changes each time (potentially)

        SkDeque::F2BIter        iter(*fLayers);
        SkLayerRasterizer_Rec*  rec;

        while ((rec = (SkLayerRasterizer_Rec*)iter.next()) != nullptr) {
            drawMatrix = translatedMatrix;
            drawMatrix.preTranslate(rec->fOffset.fX, rec->fOffset.fY);
            draw.drawPath(path, rec->fPaint);
        }
    }
    return true;
}

sk_sp<SkFlattenable> SkLayerRasterizer::CreateProc(SkReadBuffer& buffer) {
    return sk_sp<SkFlattenable>(new SkLayerRasterizer(ReadLayers(buffer)));
}

SkDeque* SkLayerRasterizer::ReadLayers(SkReadBuffer& buffer) {
    int count = buffer.readInt();

    SkDeque* layers = new SkDeque(sizeof(SkLayerRasterizer_Rec));
    for (int i = 0; i < count; i++) {
        SkLayerRasterizer_Rec* rec = (SkLayerRasterizer_Rec*)layers->push_back();

        new (&rec->fPaint) SkPaint;
        buffer.readPaint(&rec->fPaint);
        buffer.readPoint(&rec->fOffset);
    }
    return layers;
}

void SkLayerRasterizer::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);

    SkASSERT(fLayers);
    buffer.writeInt(fLayers->count());

    SkDeque::F2BIter                iter(*fLayers);
    const SkLayerRasterizer_Rec*    rec;

    while ((rec = (const SkLayerRasterizer_Rec*)iter.next()) != nullptr) {
        buffer.writePaint(rec->fPaint);
        buffer.writePoint(rec->fOffset);
    }
}

SkLayerRasterizer::Builder::Builder() : fLayers(new SkDeque(sizeof(SkLayerRasterizer_Rec))) {}

SkLayerRasterizer::Builder::~Builder()
{
    if (fLayers != nullptr) {
        clean_up_layers(fLayers);
    }
}

void SkLayerRasterizer::Builder::addLayer(const SkPaint& paint, SkScalar dx,
                                          SkScalar dy) {
    SkASSERT(fLayers);
    SkLayerRasterizer_Rec* rec = (SkLayerRasterizer_Rec*)fLayers->push_back();

    new (&rec->fPaint) SkPaint(paint);
    rec->fOffset.set(dx, dy);
}

sk_sp<SkLayerRasterizer> SkLayerRasterizer::Builder::detach() {
    SkLayerRasterizer* rasterizer;
    if (0 == fLayers->count()) {
        rasterizer = nullptr;
        delete fLayers;
    } else {
        rasterizer = new SkLayerRasterizer(fLayers);
    }
    fLayers = nullptr;
    return sk_sp<SkLayerRasterizer>(rasterizer);
}

sk_sp<SkLayerRasterizer> SkLayerRasterizer::Builder::snapshot() const {
    if (0 == fLayers->count()) {
        return nullptr;
    }
    SkDeque* layers = new SkDeque(sizeof(SkLayerRasterizer_Rec), fLayers->count());
    SkDeque::F2BIter                iter(*fLayers);
    const SkLayerRasterizer_Rec*    recOrig;
    SkDEBUGCODE(int                 count = 0;)
    while ((recOrig = static_cast<SkLayerRasterizer_Rec*>(iter.next())) != nullptr) {
        SkDEBUGCODE(count++);
        SkLayerRasterizer_Rec* recCopy = static_cast<SkLayerRasterizer_Rec*>(layers->push_back());
        new (&recCopy->fPaint) SkPaint(recOrig->fPaint);
        recCopy->fOffset = recOrig->fOffset;
    }
    SkASSERT(fLayers->count() == count);
    SkASSERT(layers->count() == count);
    return sk_sp<SkLayerRasterizer>(new SkLayerRasterizer(layers));
}
