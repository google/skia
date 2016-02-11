/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTextContext.h"
#include "GrFontScaler.h"

#include "SkGlyphCache.h"

GrTextContext::GrTextContext(GrContext* context)
    : fContext(context) {
}

bool GrTextContext::ShouldDisableLCD(const SkPaint& paint) {
    if (!SkXfermode::AsMode(paint.getXfermode(), nullptr) ||
        paint.getMaskFilter() ||
        paint.getRasterizer() ||
        paint.getPathEffect() ||
        paint.isFakeBoldText() ||
        paint.getStyle() != SkPaint::kFill_Style)
    {
        return true;
    }
    return false;
}

uint32_t GrTextContext::FilterTextFlags(const SkSurfaceProps& surfaceProps, const SkPaint& paint) {
    uint32_t flags = paint.getFlags();

    if (!paint.isLCDRenderText() || !paint.isAntiAlias()) {
        return flags;
    }

    if (kUnknown_SkPixelGeometry == surfaceProps.pixelGeometry() || ShouldDisableLCD(paint)) {
        flags &= ~SkPaint::kLCDRenderText_Flag;
        flags |= SkPaint::kGenA8FromLCD_Flag;
    }

    return flags;
}

static void GlyphCacheAuxProc(void* data) {
    GrFontScaler* scaler = (GrFontScaler*)data;
    SkSafeUnref(scaler);
}

GrFontScaler* GrTextContext::GetGrFontScaler(SkGlyphCache* cache) {
    void* auxData;
    GrFontScaler* scaler = nullptr;

    if (cache->getAuxProcData(GlyphCacheAuxProc, &auxData)) {
        scaler = (GrFontScaler*)auxData;
    }
    if (nullptr == scaler) {
        scaler = new GrFontScaler(cache);
        cache->setAuxProc(GlyphCacheAuxProc, scaler);
    }

    return scaler;
}
