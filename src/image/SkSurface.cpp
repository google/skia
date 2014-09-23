/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSurface_Base.h"
#include "SkImagePriv.h"
#include "SkCanvas.h"

#include "SkFontLCDConfig.h"
static SkPixelGeometry compute_default_geometry() {
    SkFontLCDConfig::LCDOrder order = SkFontLCDConfig::GetSubpixelOrder();
    if (SkFontLCDConfig::kNONE_LCDOrder == order) {
        return kUnknown_SkPixelGeometry;
    } else {
        // Bit0 is RGB(0), BGR(1)
        // Bit1 is H(0), V(1)
        const SkPixelGeometry gGeo[] = {
            kRGB_H_SkPixelGeometry,
            kBGR_H_SkPixelGeometry,
            kRGB_V_SkPixelGeometry,
            kBGR_V_SkPixelGeometry,
        };
        int index = 0;
        if (SkFontLCDConfig::kBGR_LCDOrder == order) {
            index |= 1;
        }
        if (SkFontLCDConfig::kVertical_LCDOrientation == SkFontLCDConfig::GetSubpixelOrientation()){
            index |= 2;
        }
        return gGeo[index];
    }
}

SkSurfaceProps::SkSurfaceProps() : fFlags(0), fPixelGeometry(kUnknown_SkPixelGeometry) {}

SkSurfaceProps::SkSurfaceProps(InitType) : fFlags(0), fPixelGeometry(compute_default_geometry()) {}

SkSurfaceProps::SkSurfaceProps(uint32_t flags, InitType)
    : fFlags(flags)
    , fPixelGeometry(compute_default_geometry())
{}

SkSurfaceProps::SkSurfaceProps(uint32_t flags, SkPixelGeometry pg)
    : fFlags(flags), fPixelGeometry(pg)
{}

///////////////////////////////////////////////////////////////////////////////

SkSurface_Base::SkSurface_Base(int width, int height, const SkSurfaceProps* props)
    : INHERITED(width, height, props)
{
    fCachedCanvas = NULL;
    fCachedImage = NULL;
}

SkSurface_Base::SkSurface_Base(const SkImageInfo& info, const SkSurfaceProps* props)
    : INHERITED(info, props)
{
    fCachedCanvas = NULL;
    fCachedImage = NULL;
}

SkSurface_Base::~SkSurface_Base() {
    // in case the canvas outsurvives us, we null the callback
    if (fCachedCanvas) {
        fCachedCanvas->setSurfaceBase(NULL);
    }

    SkSafeUnref(fCachedImage);
    SkSafeUnref(fCachedCanvas);
}

void SkSurface_Base::onDraw(SkCanvas* canvas, SkScalar x, SkScalar y, const SkPaint* paint) {
    SkImage* image = this->newImageSnapshot();
    if (image) {
        canvas->drawImage(image, x, y, paint);
        image->unref();
    }
}

void SkSurface_Base::aboutToDraw(ContentChangeMode mode) {
    this->dirtyGenerationID();

    SkASSERT(!fCachedCanvas || fCachedCanvas->getSurfaceBase() == this);

    if (fCachedImage) {
        // the surface may need to fork its backend, if its sharing it with
        // the cached image. Note: we only call if there is an outstanding owner
        // on the image (besides us).
        if (!fCachedImage->unique()) {
            this->onCopyOnWrite(mode);
        }

        // regardless of copy-on-write, we must drop our cached image now, so
        // that the next request will get our new contents.
        fCachedImage->unref();
        fCachedImage = NULL;
    } else if (kDiscard_ContentChangeMode == mode) {
        this->onDiscard();
    }
}

uint32_t SkSurface_Base::newGenerationID() {
    SkASSERT(!fCachedCanvas || fCachedCanvas->getSurfaceBase() == this);
    static int32_t gID;
    return sk_atomic_inc(&gID) + 1;
}

static SkSurface_Base* asSB(SkSurface* surface) {
    return static_cast<SkSurface_Base*>(surface);
}

///////////////////////////////////////////////////////////////////////////////

SkSurface::SkSurface(int width, int height, const SkSurfaceProps* props)
    : fProps(SkSurfacePropsCopyOrDefault(props)), fWidth(width), fHeight(height)
{
    SkASSERT(fWidth >= 0);
    SkASSERT(fHeight >= 0);
    fGenerationID = 0;
}

SkSurface::SkSurface(const SkImageInfo& info, const SkSurfaceProps* props)
    : fProps(SkSurfacePropsCopyOrDefault(props)), fWidth(info.width()), fHeight(info.height())
{
    SkASSERT(fWidth >= 0);
    SkASSERT(fHeight >= 0);
    fGenerationID = 0;
}

uint32_t SkSurface::generationID() {
    if (0 == fGenerationID) {
        fGenerationID = asSB(this)->newGenerationID();
    }
    return fGenerationID;
}

void SkSurface::notifyContentWillChange(ContentChangeMode mode) {
    asSB(this)->aboutToDraw(mode);
}

SkCanvas* SkSurface::getCanvas() {
    return asSB(this)->getCachedCanvas();
}

SkImage* SkSurface::newImageSnapshot() {
    SkImage* image = asSB(this)->getCachedImage();
    SkSafeRef(image);   // the caller will call unref() to balance this
    return image;
}

SkSurface* SkSurface::newSurface(const SkImageInfo& info) {
    return asSB(this)->onNewSurface(info);
}

void SkSurface::draw(SkCanvas* canvas, SkScalar x, SkScalar y,
                     const SkPaint* paint) {
    return asSB(this)->onDraw(canvas, x, y, paint);
}

const void* SkSurface::peekPixels(SkImageInfo* info, size_t* rowBytes) {
    return this->getCanvas()->peekPixels(info, rowBytes);
}

//////////////////////////////////////////////////////////////////////////////////////
#ifdef SK_SUPPORT_LEGACY_TEXTRENDERMODE

static SkSurfaceProps make_props(SkSurface::TextRenderMode trm) {
    uint32_t propsFlags = 0;
    if (SkSurface::kDistanceField_TextRenderMode == trm) {
        propsFlags |= SkSurfaceProps::kUseDistanceFieldFonts_Flag;
    }
    return SkSurfaceProps(propsFlags, SkSurfaceProps::kLegacyFontHost_InitType);
}

SkSurface* SkSurface::NewRenderTargetDirect(GrRenderTarget* target, TextRenderMode trm) {
    SkSurfaceProps props = make_props(trm);
    return NewRenderTargetDirect(target, &props);
}

SkSurface* SkSurface::NewRenderTarget(GrContext* gr, const SkImageInfo& info, int sampleCount,
                                      TextRenderMode trm) {
    SkSurfaceProps props = make_props(trm);
    return NewRenderTarget(gr, info, sampleCount, &props);
}

SkSurface* SkSurface::NewScratchRenderTarget(GrContext* gr, const SkImageInfo& info, int sampleCount,
                                             TextRenderMode trm) {
    SkSurfaceProps props = make_props(trm);
    return NewScratchRenderTarget(gr, info, sampleCount, &props);
}

#endif

#if !SK_SUPPORT_GPU

SkSurface* SkSurface::NewRenderTargetDirect(GrRenderTarget*, const SkSurfaceProps*) {
    return NULL;
}

SkSurface* SkSurface::NewRenderTarget(GrContext*, const SkImageInfo&, int, const SkSurfaceProps*) {
    return NULL;
}

SkSurface* SkSurface::NewScratchRenderTarget(GrContext*, const SkImageInfo&, int sampleCount,
                                             const SkSurfaceProps*) {
    return NULL;
}

#endif
