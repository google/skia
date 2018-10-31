// Copyright 2018 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "Target.h"

#include "include/core/SkStrokeRec.h"
#include "include/utils/SkShadowUtils.h"
#include "src/core/SkDraw.h"
#include "src/core/SkDrawShadowInfo.h"
#include "src/core/SkMaskFilterBase.h"

void Target::saveLayer(const SkClipStack& cs, const SkMatrix& ctm, const SkCanvas::SaveLayerRec&) {}

void Target::restoreLayer() {}

void Target::drawGlyphs(const SkClipStack& cs, const SkMatrix& ctm,
                        const SkGlyphRunList& glyphRuns) {
//     for (const SkGlyphRun& glyphRun : glyphRuns) {
//        auto offset = glyphRuns.origin();
//        SkPaint paint = glyphRun.paint();
//        paint.setTextEncoding(SkPaint::kGlyphID_TextEncoding);
//        SkPath path;
//        paint.getPosTextPath(glyphRun.glyphsIDs().data(),
//                             glyphRun.glyphsIDs().size() * sizeof(SkGlyphID),
//                             glyphRun.positions().data(),
//                             &path);
//        path.offset(offset.x(), offset.y());
//        this->drawPath(cs, ctm, path, paint);
//     }
}

void Target::drawGlyphsRSXform(const SkClipStack& cs, const SkMatrix& ctm,
                               const SkGlyphRunList& glyphRuns, const SkRSXform* xform) {
    if (!glyphRuns.empty()) {
//        auto glyphRun = glyphRuns[0];
//        SkPaint paint = glyphRun.paint();
//        paint.setTextEncoding(SkPaint::kGlyphID_TextEncoding);
//        SkPath runPath;
//        glyphRun.eachGlyphToGlyphRun([&](SkGlyphRun* run, SkPaint* runPaint) {
//                SkMatrix glyphCTM;
//                glyphCTM.setRSXform(*xform++);
//                SkPath path;
//                paint.getPosTextPath(run->glyphsIDs().data(),
//                                     run->glyphsIDs().size() * sizeof(SkGlyphID),
//                                     run->positions().data(),
//                                     &path);
//                runPath.addPath(path, glyphCTM);
//        });
//        this->drawPath(cs, ctm, runPath, paint);
    }
}

struct ShaderColorAlpha {
    sk_sp<SkShader> fShader;
    float fR = 0, fG = 0, fB = 0;  // only matter if fShader is nullptr
    float fAlpha = 1;
};

static ShaderColorAlpha to_shaderalpha(SkColorFilter* colorFilter,
                                       SkColor4f color, SkShader* shader) {
    ShaderColorAlpha result;
    result.fAlpha = color.fA;
    if (colorFilter) {
        if (shader) {
//            if (color.fA != 1) {
//                SkScalar colorMatrix[20] = {
//                    1, 0, 0, 0, 0,
//                    0, 1, 0, 0, 0,
//                    0, 0, 1, 0, 0,
//                    0, 0, 0, color.fA, 0};
//                auto alphafilter = SkColorFilter::MakeMatrixFilterRowMajor255(colorMatrix);
//                auto tmpFilter = colorFilter->makeComposed(std::move(alphafilter));
//                result.fShader = shader->makeWithColorFilter(std::move(tmpFilter));
//                result.fAlpha = 1;
//            } else {
                result.fShader = shader->makeWithColorFilter(sk_ref_sp(colorFilter));
//            }
        } else {
            color = colorFilter->filterColor4f(color, nullptr);
            result.fR = color.fR;
            result.fG = color.fG;
            result.fB = color.fB;
            result.fAlpha = color.fA;
        }
    } else {
        if (shader) {
            result.fShader = sk_ref_sp(shader);
            result.fAlpha = color.fA;
        } else {
            result.fR = color.fR;
            result.fG = color.fG;
            result.fB = color.fB;
            result.fAlpha = color.fA;
        }
    }
    return result;
}

sk_sp<SkSurface> make_n32_surface(float rasterScale, SkIRect rect) {
    auto s = SkSurface::MakeRasterN32Premul(rasterScale * rect.width(),
                                            rasterScale * rect.height());
    s->getCanvas()->clear(0);
    s->getCanvas()->scale(rasterScale, rasterScale);
    s->getCanvas()->translate(-rect.x(), -rect.y());
    return s;
}

sk_sp<SkImage> draw_path_with_mask(SkIRect bounds,
                                   const SkMatrix& ctm,
                                   const SkPath& pathSrc,
                                   SkMaskFilter* maskFilter,
                                   bool hairline) {
    SkASSERT(maskFilter);
    SkPath path(pathSrc);
    path.transform(ctm, &path);
    SkMask sourceMask;
    SkStrokeRec::InitStyle initStyle = hairline ? SkStrokeRec::kHairline_InitStyle
                                                : SkStrokeRec::kFill_InitStyle;
    if (!SkDraw::DrawToMask(path, &bounds, maskFilter, &SkMatrix::I(),
                &sourceMask, SkMask::kComputeBoundsAndRenderImage_CreateMode,
                initStyle)) {
        return nullptr;
    }
    SkAutoMaskFreeImage srcAutoMaskFreeImage(sourceMask.fImage);
    SkMask mask;
    SkIPoint margin;
    if (!as_MFB(maskFilter)->filterMask(&mask, sourceMask, ctm, &margin)) {
        return nullptr;
    }
    //return mask_to_greyscale_image(&mask);
    SkPixmap pm(SkImageInfo::Make(mask.fBounds.width(), mask.fBounds.height(),
                                  kGray_8_SkColorType, kOpaque_SkAlphaType),
                                  mask.fImage, mask.fRowBytes);
    auto freefn = [](const void* p, void*) { SkMask::FreeImage((void*)p); };
    return SkImage::MakeFromRaster(pm, freefn, nullptr);
}

void Target::drawPath(const SkClipStack& cs, const SkMatrix& ctm,
                      const SkPath& pathSrc, const SkPaint& paint) {
    if (SkImageFilter* imageFilter = paint.getImageFilter()) {
        constexpr float kRasterScale = 2.0f;
        SkIRect rect = cs.bounds(SkIRect::MakeSize(fSize)).roundOut();
        auto s = make_n32_surface(kRasterScale, rect);
        s->getCanvas()->concat(ctm);
        SkPaint p(paint);
        p.setBlendMode(SkBlendMode::kSrcOver);
        s->getCanvas()->drawPath(pathSrc, p);
        p.reset();
        p.setBlendMode(paint.getBlendMode());
        this->drawImageRect(cs, SkMatrix::I(), s->makeImageSnapshot().get(), nullptr,
                            SkRect::Make(rect), &p, SkCanvas::kStrict_SrcRectConstraint);
        return;
    }
    ShaderColorAlpha shaderAlpha = to_shaderalpha(paint.getColorFilter(), paint.getColor4f(),
                                                   paint.getShader());
    SkPath path(pathSrc);
    bool hairline = !paint.getFillPath(path, &path);
    SkBlendMode blendMode = paint.getBlendMode();

    if (SkMaskFilter* maskFilter = paint.getMaskFilter()) {
        SkIRect rect = cs.bounds(SkIRect::MakeSize(fSize)).roundOut();
        sk_sp<SkImage> mask = draw_path_with_mask(rect, ctm, pathSrc, maskFilter, hairline);
        if (mask) {
            // add smask graphic state
            // draw bounds of mask with clip, ctm, blendmode, shaderAlpha
        }
        return;
    }
    (void)blendMode;
    //FIXME
    //this->setClip(cs);
    //this->setCTM(ctm);
    //this->setShaderAlphaBlendMode(shaderAlpha, blendMode);
    //this->drawPath(path, hairline);
}

void Target::drawPoints(const SkClipStack& cs, const SkMatrix& ctm,
                        SkCanvas::PointMode mode, size_t count, const SkPoint pts[],
                        const SkPaint& paint) {}

void Target::drawVertices(const SkClipStack& cs, const SkMatrix& ctm,
                          const SkVertices* vertices, const SkVertices::Bone bones[],
                          int boneCount, SkBlendMode mode, const SkPaint& paint) {}

static sk_sp<SkShader> to_image_shader(const SkImage* image, const SkRect* src, const SkRect& dst) {
    SkRect srcRect = src ? *src : SkRect::Make(image->bounds());
    SkMatrix local = SkMatrix::MakeRectToRect(srcRect, dst, SkMatrix::kFill_ScaleToFit);
    return image->makeShader(&local);
}

static SkPath to_path(SkRect r) {
    SkPath path;
    path.addRect(r);
    path.setIsVolatile(false);
    return path;
}

static SkPaint shader_paint(sk_sp<SkShader> shader, const SkPaint* src) {
    SkPaint paint = src ? *src : SkPaint();
    paint.setShader(std::move(shader));
    return paint;
}

void Target::drawImageRect(const SkClipStack& cs, const SkMatrix& ctm,
                           const SkImage* image, const SkRect* src, const SkRect& dst,
                           const SkPaint* paint, SkCanvas::SrcRectConstraint) {
    if (!image) { return; }
    if (image->isAlphaOnly()) {
        // make soft mask from shader_paint(to_image_shader(image, src, dst)
        this->drawPath(cs, ctm, to_path(dst), paint ? *paint : SkPaint());
    }
    this->drawPath(cs, ctm, to_path(dst), shader_paint(to_image_shader(image, src, dst), paint));
}

void Target::drawAnnotation(const SkClipStack& cs, const SkMatrix& ctm,
                            const SkRect& rect, const char key[], SkData* value) {}

void Target::drawShadowRec(const SkClipStack& cs, const SkMatrix& ctm,
                           const SkPath& path, const SkDrawShadowRec& rec) {
    SkIRect rect = cs.bounds(SkIRect::MakeSize(fSize)).roundOut();
    auto s = make_n32_surface(1, rect);
    SkCanvas* canvas = s->getCanvas();
    canvas->concat(ctm);
    SkPoint pt = ctm.mapXY(rec.fLightPos.x(), rec.fLightPos.y());
    canvas->concat(ctm);
    SkShadowUtils::DrawShadow(canvas, path, rec.fZPlaneParams,
                              SkPoint3{pt.x(), pt.y(), rec.fLightPos.z()},
                              rec.fLightRadius, rec.fAmbientColor, rec.fSpotColor, rec.fFlags);
    this->drawImageRect(cs, SkMatrix::I(), s->makeImageSnapshot().get(), nullptr,
                        SkRect::Make(rect), nullptr, SkCanvas::kStrict_SrcRectConstraint);
}

