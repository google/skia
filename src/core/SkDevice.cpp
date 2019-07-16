/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkDevice.h"

#include "include/core/SkColorFilter.h"
#include "include/core/SkDrawable.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkPathMeasure.h"
#include "include/core/SkRSXform.h"
#include "include/core/SkShader.h"
#include "include/core/SkVertices.h"
#include "include/private/SkTo.h"
#include "src/core/SkDraw.h"
#include "src/core/SkGlyphRun.h"
#include "src/core/SkImageFilterCache.h"
#include "src/core/SkImagePriv.h"
#include "src/core/SkLatticeIter.h"
#include "src/core/SkMakeUnique.h"
#include "src/core/SkMatrixPriv.h"
#include "src/core/SkPathPriv.h"
#include "src/core/SkRasterClip.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkTLazy.h"
#include "src/core/SkTextBlobPriv.h"
#include "src/core/SkUtils.h"
#include "src/image/SkImage_Base.h"
#include "src/shaders/SkLocalMatrixShader.h"
#include "src/utils/SkPatchUtils.h"

SkBaseDevice::SkBaseDevice(const SkImageInfo& info, const SkSurfaceProps& surfaceProps)
    : fInfo(info)
    , fSurfaceProps(surfaceProps)
{
    fOrigin = {0, 0};
    fCTM.reset();
}

void SkBaseDevice::setOrigin(const SkMatrix& globalCTM, int x, int y) {
    fOrigin.set(x, y);
    fCTM = globalCTM;
    fCTM.postTranslate(SkIntToScalar(-x), SkIntToScalar(-y));
}

void SkBaseDevice::setGlobalCTM(const SkMatrix& ctm) {
    fCTM = ctm;
    if (fOrigin.fX | fOrigin.fY) {
        fCTM.postTranslate(-SkIntToScalar(fOrigin.fX), -SkIntToScalar(fOrigin.fY));
    }
}

bool SkBaseDevice::clipIsWideOpen() const {
    if (kRect_ClipType == this->onGetClipType()) {
        SkRegion rgn;
        this->onAsRgnClip(&rgn);
        SkASSERT(rgn.isRect());
        return rgn.getBounds() == SkIRect::MakeWH(this->width(), this->height());
    } else {
        return false;
    }
}

SkPixelGeometry SkBaseDevice::CreateInfo::AdjustGeometry(TileUsage tileUsage, SkPixelGeometry geo) {
    switch (tileUsage) {
        case kPossible_TileUsage:
            // (we think) for compatibility with old clients, we assume this layer can support LCD
            // even though they may not have marked it as opaque... seems like we should update
            // our callers (reed/robertphilips).
            break;
        case kNever_TileUsage:
            geo = kUnknown_SkPixelGeometry;
            break;
    }
    return geo;
}

static inline bool is_int(float x) {
    return x == (float) sk_float_round2int(x);
}

void SkBaseDevice::drawRegion(const SkRegion& region, const SkPaint& paint) {
    const SkMatrix& ctm = this->ctm();
    bool isNonTranslate = ctm.getType() & ~(SkMatrix::kTranslate_Mask);
    bool complexPaint = paint.getStyle() != SkPaint::kFill_Style || paint.getMaskFilter() ||
                        paint.getPathEffect();
    bool antiAlias = paint.isAntiAlias() && (!is_int(ctm.getTranslateX()) ||
                                             !is_int(ctm.getTranslateY()));
    if (isNonTranslate || complexPaint || antiAlias) {
        SkPath path;
        region.getBoundaryPath(&path);
        path.setIsVolatile(true);
        return this->drawPath(path, paint, true);
    }

    SkRegion::Iterator it(region);
    while (!it.done()) {
        this->drawRect(SkRect::Make(it.rect()), paint);
        it.next();
    }
}

void SkBaseDevice::drawArc(const SkRect& oval, SkScalar startAngle,
                           SkScalar sweepAngle, bool useCenter, const SkPaint& paint) {
    SkPath path;
    bool isFillNoPathEffect = SkPaint::kFill_Style == paint.getStyle() && !paint.getPathEffect();
    SkPathPriv::CreateDrawArcPath(&path, oval, startAngle, sweepAngle, useCenter,
                                  isFillNoPathEffect);
    this->drawPath(path, paint);
}

void SkBaseDevice::drawDRRect(const SkRRect& outer,
                              const SkRRect& inner, const SkPaint& paint) {
    SkPath path;
    path.addRRect(outer);
    path.addRRect(inner);
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.setIsVolatile(true);

    this->drawPath(path, paint, true);
}

void SkBaseDevice::drawPatch(const SkPoint cubics[12], const SkColor colors[4],
                             const SkPoint texCoords[4], SkBlendMode bmode, const SkPaint& paint) {
    SkISize lod = SkPatchUtils::GetLevelOfDetail(cubics, &this->ctm());
    auto vertices = SkPatchUtils::MakeVertices(cubics, colors, texCoords, lod.width(), lod.height(),
                                               this->imageInfo().colorSpace());
    if (vertices) {
        this->drawVertices(vertices.get(), nullptr, 0, bmode, paint);
    }
}

void SkBaseDevice::drawImageRect(const SkImage* image, const SkRect* src,
                                 const SkRect& dst, const SkPaint& paint,
                                 SkCanvas::SrcRectConstraint constraint) {
    SkBitmap bm;
    if (as_IB(image)->getROPixels(&bm)) {
        this->drawBitmapRect(bm, src, dst, paint, constraint);
    }
}

void SkBaseDevice::drawImageNine(const SkImage* image, const SkIRect& center,
                                 const SkRect& dst, const SkPaint& paint) {
    SkLatticeIter iter(image->width(), image->height(), center, dst);

    SkRect srcR, dstR;
    while (iter.next(&srcR, &dstR)) {
        this->drawImageRect(image, &srcR, dstR, paint, SkCanvas::kStrict_SrcRectConstraint);
    }
}

void SkBaseDevice::drawBitmapNine(const SkBitmap& bitmap, const SkIRect& center,
                                  const SkRect& dst, const SkPaint& paint) {
    SkLatticeIter iter(bitmap.width(), bitmap.height(), center, dst);

    SkRect srcR, dstR;
    while (iter.next(&srcR, &dstR)) {
        this->drawBitmapRect(bitmap, &srcR, dstR, paint, SkCanvas::kStrict_SrcRectConstraint);
    }
}

void SkBaseDevice::drawImageLattice(const SkImage* image,
                                    const SkCanvas::Lattice& lattice, const SkRect& dst,
                                    const SkPaint& paint) {
    SkLatticeIter iter(lattice, dst);

    SkRect srcR, dstR;
    SkColor c;
    bool isFixedColor = false;
    const SkImageInfo info = SkImageInfo::Make(1, 1, kBGRA_8888_SkColorType, kUnpremul_SkAlphaType);

    while (iter.next(&srcR, &dstR, &isFixedColor, &c)) {
          if (isFixedColor || (srcR.width() <= 1.0f && srcR.height() <= 1.0f &&
                               image->readPixels(info, &c, 4, srcR.fLeft, srcR.fTop))) {
              // Fast draw with drawRect, if this is a patch containing a single color
              // or if this is a patch containing a single pixel.
              if (0 != c || !paint.isSrcOver()) {
                   SkPaint paintCopy(paint);
                   int alpha = SkAlphaMul(SkColorGetA(c), SkAlpha255To256(paint.getAlpha()));
                   paintCopy.setColor(SkColorSetA(c, alpha));
                   this->drawRect(dstR, paintCopy);
              }
        } else {
            this->drawImageRect(image, &srcR, dstR, paint, SkCanvas::kStrict_SrcRectConstraint);
        }
    }
}

void SkBaseDevice::drawBitmapLattice(const SkBitmap& bitmap,
                                     const SkCanvas::Lattice& lattice, const SkRect& dst,
                                     const SkPaint& paint) {
    SkLatticeIter iter(lattice, dst);

    SkRect srcR, dstR;
    while (iter.next(&srcR, &dstR)) {
        this->drawBitmapRect(bitmap, &srcR, dstR, paint, SkCanvas::kStrict_SrcRectConstraint);
    }
}

static SkPoint* quad_to_tris(SkPoint tris[6], const SkPoint quad[4]) {
    tris[0] = quad[0];
    tris[1] = quad[1];
    tris[2] = quad[2];

    tris[3] = quad[0];
    tris[4] = quad[2];
    tris[5] = quad[3];

    return tris + 6;
}

void SkBaseDevice::drawAtlas(const SkImage* atlas, const SkRSXform xform[],
                             const SkRect tex[], const SkColor colors[], int quadCount,
                             SkBlendMode mode, const SkPaint& paint) {
    const int triCount = quadCount << 1;
    const int vertexCount = triCount * 3;
    uint32_t flags = SkVertices::kHasTexCoords_BuilderFlag;
    if (colors) {
        flags |= SkVertices::kHasColors_BuilderFlag;
    }
    SkVertices::Builder builder(SkVertices::kTriangles_VertexMode, vertexCount, 0, flags);

    SkPoint* vPos = builder.positions();
    SkPoint* vTex = builder.texCoords();
    SkColor* vCol = builder.colors();
    for (int i = 0; i < quadCount; ++i) {
        SkPoint tmp[4];
        xform[i].toQuad(tex[i].width(), tex[i].height(), tmp);
        vPos = quad_to_tris(vPos, tmp);

        tex[i].toQuad(tmp);
        vTex = quad_to_tris(vTex, tmp);

        if (colors) {
            sk_memset32(vCol, colors[i], 6);
            vCol += 6;
        }
    }
    SkPaint p(paint);
    p.setShader(atlas->makeShader());
    this->drawVertices(builder.detach().get(), nullptr, 0, mode, p);
}


void SkBaseDevice::drawEdgeAAQuad(const SkRect& r, const SkPoint clip[4],
                                  SkCanvas::QuadAAFlags aa, SkColor color, SkBlendMode mode) {
    SkPaint paint;
    paint.setColor(color);
    paint.setBlendMode(mode);
    paint.setAntiAlias(aa == SkCanvas::kAll_QuadAAFlags);

    if (clip) {
        // Draw the clip directly as a quad since it's a filled color with no local coords
        SkPath clipPath;
        clipPath.addPoly(clip, 4, true);
        this->drawPath(clipPath, paint);
    } else {
        this->drawRect(r, paint);
    }
}

void SkBaseDevice::drawEdgeAAImageSet(const SkCanvas::ImageSetEntry images[], int count,
                                      const SkPoint dstClips[], const SkMatrix preViewMatrices[],
                                      const SkPaint& paint,
                                      SkCanvas::SrcRectConstraint constraint) {
    SkASSERT(paint.getStyle() == SkPaint::kFill_Style);
    SkASSERT(!paint.getPathEffect());

    SkPaint entryPaint = paint;
    const SkMatrix baseCTM = this->ctm();
    int clipIndex = 0;
    for (int i = 0; i < count; ++i) {
        // TODO: Handle per-edge AA. Right now this mirrors the SkiaRenderer component of Chrome
        // which turns off antialiasing unless all four edges should be antialiased. This avoids
        // seaming in tiled composited layers.
        entryPaint.setAntiAlias(images[i].fAAFlags == SkCanvas::kAll_QuadAAFlags);
        entryPaint.setAlphaf(paint.getAlphaf() * images[i].fAlpha);

        bool needsRestore = false;
        SkASSERT(images[i].fMatrixIndex < 0 || preViewMatrices);
        if (images[i].fMatrixIndex >= 0) {
            this->save();
            this->setGlobalCTM(SkMatrix::Concat(
                    baseCTM, preViewMatrices[images[i].fMatrixIndex]));
            needsRestore = true;
        }

        SkASSERT(!images[i].fHasClip || dstClips);
        if (images[i].fHasClip) {
            // Since drawImageRect requires a srcRect, the dst clip is implemented as a true clip
            if (!needsRestore) {
                this->save();
                needsRestore = true;
            }
            SkPath clipPath;
            clipPath.addPoly(dstClips + clipIndex, 4, true);
            this->clipPath(clipPath, SkClipOp::kIntersect, entryPaint.isAntiAlias());
            clipIndex += 4;
        }
        this->drawImageRect(images[i].fImage.get(), &images[i].fSrcRect, images[i].fDstRect,
                            entryPaint, constraint);
        if (needsRestore) {
            this->restore(baseCTM);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void SkBaseDevice::drawDrawable(SkDrawable* drawable, const SkMatrix* matrix, SkCanvas* canvas) {
    drawable->draw(canvas, matrix);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void SkBaseDevice::drawSpecial(SkSpecialImage*, int x, int y, const SkPaint&,
                               SkImage*, const SkMatrix&) {}
sk_sp<SkSpecialImage> SkBaseDevice::makeSpecial(const SkBitmap&) { return nullptr; }
sk_sp<SkSpecialImage> SkBaseDevice::makeSpecial(const SkImage*) { return nullptr; }
sk_sp<SkSpecialImage> SkBaseDevice::snapSpecial() { return nullptr; }

///////////////////////////////////////////////////////////////////////////////////////////////////

bool SkBaseDevice::readPixels(const SkPixmap& pm, int x, int y) {
    return this->onReadPixels(pm, x, y);
}

bool SkBaseDevice::writePixels(const SkPixmap& pm, int x, int y) {
    return this->onWritePixels(pm, x, y);
}

bool SkBaseDevice::onWritePixels(const SkPixmap&, int, int) {
    return false;
}

bool SkBaseDevice::onReadPixels(const SkPixmap&, int x, int y) {
    return false;
}

bool SkBaseDevice::accessPixels(SkPixmap* pmap) {
    SkPixmap tempStorage;
    if (nullptr == pmap) {
        pmap = &tempStorage;
    }
    return this->onAccessPixels(pmap);
}

bool SkBaseDevice::peekPixels(SkPixmap* pmap) {
    SkPixmap tempStorage;
    if (nullptr == pmap) {
        pmap = &tempStorage;
    }
    return this->onPeekPixels(pmap);
}

//////////////////////////////////////////////////////////////////////////////////////////

#include "src/core/SkUtils.h"

void SkBaseDevice::drawGlyphRunRSXform(const SkFont& font, const SkGlyphID glyphs[],
                                       const SkRSXform xform[], int count, SkPoint origin,
                                       const SkPaint& paint) {
    const SkMatrix originalCTM = this->ctm();
    if (!originalCTM.isFinite() || !SkScalarIsFinite(font.getSize()) ||
        !SkScalarIsFinite(font.getScaleX()) ||
        !SkScalarIsFinite(font.getSkewX())) {
        return;
    }

    SkPoint sharedPos{0, 0};    // we're at the origin
    SkGlyphID glyphID;
    SkGlyphRun glyphRun{
        font,
        SkSpan<const SkPoint>{&sharedPos, 1},
        SkSpan<const SkGlyphID>{&glyphID, 1},
        SkSpan<const char>{},
        SkSpan<const uint32_t>{}
    };

    for (int i = 0; i < count; i++) {
        glyphID = glyphs[i];
        // now "glyphRun" is pointing at the current glyphID

        SkMatrix ctm;
        ctm.setRSXform(xform[i]).postTranslate(origin.fX, origin.fY);

        // We want to rotate each glyph by the rsxform, but we don't want to rotate "space"
        // (i.e. the shader that cares about the ctm) so we have to undo our little ctm trick
        // with a localmatrixshader so that the shader draws as if there was no change to the ctm.
        SkPaint transformingPaint{paint};
        auto shader = transformingPaint.getShader();
        if (shader) {
            SkMatrix inverse;
            if (ctm.invert(&inverse)) {
                transformingPaint.setShader(shader->makeWithLocalMatrix(inverse));
            } else {
                transformingPaint.setShader(nullptr);  // can't handle this xform
            }
        }

        ctm.setConcat(originalCTM, ctm);
        this->setCTM(ctm);

        this->drawGlyphRunList(SkGlyphRunList{glyphRun, transformingPaint});
    }
    this->setCTM(originalCTM);
}

//////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkSurface> SkBaseDevice::makeSurface(SkImageInfo const&, SkSurfaceProps const&) {
    return nullptr;
}

sk_sp<SkSpecialImage> SkBaseDevice::snapBackImage(const SkIRect&) {
    return nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////

void SkBaseDevice::LogDrawScaleFactor(const SkMatrix& view, const SkMatrix& srcToDst,
                                      SkFilterQuality filterQuality) {
#if SK_HISTOGRAMS_ENABLED
    SkMatrix matrix = SkMatrix::Concat(view, srcToDst);
    enum ScaleFactor {
        kUpscale_ScaleFactor,
        kNoScale_ScaleFactor,
        kDownscale_ScaleFactor,
        kLargeDownscale_ScaleFactor,

        kLast_ScaleFactor = kLargeDownscale_ScaleFactor
    };

    float rawScaleFactor = matrix.getMinScale();

    ScaleFactor scaleFactor;
    if (rawScaleFactor < 0.5f) {
        scaleFactor = kLargeDownscale_ScaleFactor;
    } else if (rawScaleFactor < 1.0f) {
        scaleFactor = kDownscale_ScaleFactor;
    } else if (rawScaleFactor > 1.0f) {
        scaleFactor = kUpscale_ScaleFactor;
    } else {
        scaleFactor = kNoScale_ScaleFactor;
    }

    switch (filterQuality) {
        case kNone_SkFilterQuality:
            SK_HISTOGRAM_ENUMERATION("DrawScaleFactor.NoneFilterQuality", scaleFactor,
                                     kLast_ScaleFactor + 1);
            break;
        case kLow_SkFilterQuality:
            SK_HISTOGRAM_ENUMERATION("DrawScaleFactor.LowFilterQuality", scaleFactor,
                                     kLast_ScaleFactor + 1);
            break;
        case kMedium_SkFilterQuality:
            SK_HISTOGRAM_ENUMERATION("DrawScaleFactor.MediumFilterQuality", scaleFactor,
                                     kLast_ScaleFactor + 1);
            break;
        case kHigh_SkFilterQuality:
            SK_HISTOGRAM_ENUMERATION("DrawScaleFactor.HighFilterQuality", scaleFactor,
                                     kLast_ScaleFactor + 1);
            break;
    }

    // Also log filter quality independent scale factor.
    SK_HISTOGRAM_ENUMERATION("DrawScaleFactor.AnyFilterQuality", scaleFactor,
                             kLast_ScaleFactor + 1);

    // Also log an overall histogram of filter quality.
    SK_HISTOGRAM_ENUMERATION("FilterQuality", filterQuality, kLast_SkFilterQuality + 1);
#endif
}

