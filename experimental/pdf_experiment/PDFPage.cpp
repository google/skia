// Copyright 2018 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "PDFPage.h"

#include "PDFTypes.h"

#include "SkDraw.h"
#include "SkDrawShadowInfo.h"
#include "SkFloatToDecimal.h"
#include "SkGeometry.h"
#include "SkMaskFilterBase.h"
#include "SkPathOps.h"
#include "SkShadowUtils.h"
#include "SkStrokeRec.h"
#include "SkTextBlobPriv.h"

#include "PDFDocument.h"

static SkPath to_path(SkRect r) {
    SkPath path;
    path.addRect(r);
    path.setIsVolatile(false);
    return path;
}

static void append_scalar(float v, SkDynamicMemoryWStream& content) {
    char scalarBuffer[kMaximumSkFloatToDecimalLength];
    content.write(scalarBuffer, SkFloatToDecimal(v, scalarBuffer));
}

static void append_point(SkPoint p, SkDynamicMemoryWStream& content) {
    char scalarBuffer[kMaximumSkFloatToDecimalLength];
    content.write(scalarBuffer, SkFloatToDecimal(p.x(), scalarBuffer));
    content.writeText(" ");
    content.write(scalarBuffer, SkFloatToDecimal(p.y(), scalarBuffer));
}

static void append_cubic(SkPoint c1, SkPoint c2, SkPoint c3, SkDynamicMemoryWStream& content) {
    append_point(c1, content);
    content.writeText(" ");
    append_point(c2, content);
    content.writeText(" ");
    append_point(c3, content);
    content.writeText(" c\n");
}

static void append_quad(const SkPoint quad[], SkDynamicMemoryWStream& content) {
    SkPoint cubic[4];
    SkConvertQuadToCubic(quad, cubic);
    append_cubic(cubic[1], cubic[2], cubic[3], content);
}

static void append_transform(const SkMatrix& matrix, SkDynamicMemoryWStream& content) {
    char scalarBuffer[kMaximumSkFloatToDecimalLength];
    SkScalar values[6];
    if (matrix.asAffine(values)) {
        for (SkScalar v : values) {
            content.write(scalarBuffer, SkFloatToDecimal(v, scalarBuffer));
            content.writeText(" ");
        }
        content.writeText("cm\n");
    }
}

static void emit_path(const SkPath& path, SkDynamicMemoryWStream& content) {
    SkPoint args[4];
    SkPath::Iter iter(path, false);
    bool doConsumeDegerates = true;
    for (SkPath::Verb verb = iter.next(args, doConsumeDegerates);
         verb != SkPath::kDone_Verb;
         verb = iter.next(args, doConsumeDegerates))
    {
        switch (verb) {
            case SkPath::kMove_Verb:
                append_point(args[0], content);
                content.writeText(" m\n");
                break;
            case SkPath::kLine_Verb:
                append_point(args[1], content);
                content.writeText(" l\n");
                break;
            case SkPath::kQuad_Verb:
                append_quad(args, content);
                break;
            case SkPath::kConic_Verb:
                {
                    SkAutoConicToQuads converter;
                    float tolerance = 0.25f;
                    const SkPoint* quads =
                        converter.computeQuads(args, iter.conicWeight(), tolerance);
                    for (int i = 0; i < converter.countQuads(); ++i) {
                        append_quad(&quads[i * 2], content);
                    }
                }
                break;
            case SkPath::kCubic_Verb:
                append_cubic(args[1], args[2], args[3], content);
                break;
            case SkPath::kClose_Verb:
                content.writeText("h\n");
                break;
            default:
                SkASSERT(false);
                break;
        }
    }
}

static SkPath clip_path(const SkClipStack& clipStack, SkRect bounds) {
    SkPath clipPath;
    (void)clipStack.asPath(&clipPath);
    SkPath bPath = to_path(bounds);
    if (!Op(clipPath, bPath, kIntersect_SkPathOp, &clipPath)) {
        clipPath = bPath;
    }
    return clipPath;
}

static void emit_clip(const SkPath& path, SkDynamicMemoryWStream& content) {
    emit_path(path, content);
    if (path.getFillType() == SkPath::kEvenOdd_FillType) {
        content.writeText("W* n\n");
    } else {
        content.writeText("W n\n");
    }
}

void PDFPage::saveLayer(const SkClipStack& cs, const SkMatrix& ctm, const SkCanvas::SaveLayerRec&) {}

void PDFPage::restoreLayer() {}  // FIXME

namespace {
class GlyphRunMatrixIterator {
    std::unique_ptr<SkPoint[]> fBuffer;
    const SkScalar* fPos;
    SkPoint fOffset;
    SkDEBUGCODE(uint32_t fGlyphCount;)
    SkTextBlobRunIterator::GlyphPositioning fGlyphPositioning;
public:
    GlyphRunMatrixIterator(const SkTextBlobRunIterator& it) 
        : fPos(it.pos())
        , fOffset(it.offset())
        SkDEBUGCODE(, fGlyphCount(it.glyphCount()))
        , fGlyphPositioning(it.positioning()) {
        if (SkTextBlobRunIterator::kDefault_Positioning == fGlyphPositioning) {
            fBuffer.reset(new SkPoint[it.glyphCount()]);
            it.font().getPos(it.glyphs(), it.glyphCount(), fBuffer.get(), fOffset);
        }
    }
    void set(SkMatrix* dst, uint32_t i) {
        SkASSERT(i < fGlyphCount);
        switch (fGlyphPositioning) {
            case SkTextBlobRunIterator::kDefault_Positioning:
                dst->setTranslate(fBuffer[i].x(), fBuffer[i].y());
                break;
            case SkTextBlobRunIterator::kHorizontal_Positioning:
                SkASSERT(fOffset.x() == 0);
                dst->setTranslate(fPos[i], fOffset.y());
                break;
            case SkTextBlobRunIterator::kFull_Positioning:
                SkASSERT((fOffset == SkPoint{0, 0}));
                dst->setTranslate(((const SkPoint*)fPos)[i].x(),
                                  ((const SkPoint*)fPos)[i].y());
                break;
            case SkTextBlobRunIterator::kRSXform_Positioning:
                SkASSERT((fOffset == SkPoint{0, 0}));
                dst->setRSXform(((const SkRSXform*)fPos)[i]);
                break;
        }
    }
};
}

void PDFPage::drawTextBlob(const SkClipStack& cs, const SkMatrix& ctm,
                           const SkTextBlob* blob, SkPoint origin, const SkPaint& paint) {
    SkPath path;
    for (SkTextBlobRunIterator it(blob); !it.done(); it.next()) {
        uint32_t runSize = it.glyphCount();
        const SkFont& font = it.font();
        GlyphRunMatrixIterator glyphRunMatrixIterator(it);
        for (uint32_t i = 0; i < runSize; ++i) {
            SkPath glyphPath;
            SkMatrix matrix;
            glyphRunMatrixIterator.set(&matrix, i);
            font.getPath(it.glyphs()[i], &glyphPath);
            path.addPath(glyphPath, matrix);
        }
    }
    fLayers.back().fContent.writeText("%% begin draw text\n");
    this->drawPath(cs, ctm, path, paint);
    fLayers.back().fContent.writeText("%% end draw text\n");
}

struct ShaderColorAlpha {
    sk_sp<SkShader> fShader;
    float fR = 0, fG = 0, fB = 0;  // only matter if fShader is nullptr
    float fAlpha = 1;
};

static ShaderColorAlpha to_shaderalpha(SkColorFilter* colorFilter,
                                       SkColor4f color, SkShader* shader) {
    ShaderColorAlpha result;
    result.fR = 1;
    result.fAlpha = color.fA;
    if (colorFilter) {
        if (shader) {
            if (color.fA != 1) {
                SkScalar colorMatrix[20] = {
                    1, 0, 0, 0, 0,
                    0, 1, 0, 0, 0,
                    0, 0, 1, 0, 0,
                    0, 0, 0, color.fA, 0};
                auto alphafilter = SkColorFilter::MakeMatrixFilterRowMajor255(colorMatrix);
                auto tmpFilter = colorFilter->makeComposed(std::move(alphafilter));
                result.fShader = shader->makeWithColorFilter(std::move(tmpFilter));
                result.fAlpha = 1;
            } else {
                result.fShader = shader->makeWithColorFilter(sk_ref_sp(colorFilter));
            }
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

PDFPage::PDFPage(SkISize s, PDFDocument* doc) : fSize(s), fDoc(doc) {
    fLayers.emplace_back();
    SkMatrix initialTransform;
    initialTransform.setScaleTranslate(1, -1, 0, fSize.height());
    append_transform(initialTransform, fLayers.back().fContent);
}

void PDFPage::drawPath(const SkClipStack& cs, const SkMatrix& ctm,
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
    
    SkDynamicMemoryWStream& content = fLayers.back().fContent;
    content.writeText("q\n");

    SkRect bounds;
    cs.bounds(SkIRect::MakeSize(fSize)).roundOut(&bounds);
    if (!cs.isWideOpen()) {
        emit_clip(clip_path(cs, bounds), content);
    }
    if (!ctm.isIdentity()) { 
        append_transform(ctm, content);
    }
    append_scalar(shaderAlpha.fR, content);
    content.writeText(" ");
    append_scalar(shaderAlpha.fG, content);
    content.writeText(" ");
    append_scalar(shaderAlpha.fB, content);
    content.writeText(" RG\n");
    append_scalar(shaderAlpha.fR, content);
    content.writeText(" ");
    append_scalar(shaderAlpha.fG, content);
    content.writeText(" ");
    append_scalar(shaderAlpha.fB, content);
    content.writeText(" rg\n");
    //this->setShaderAlphaBlendMode(shaderAlpha, blendMode);

    SkPath boundedPath;
    boundedPath.addRect(bounds);
    (void)Op(path, boundedPath, kIntersect_SkPathOp, &boundedPath);
    emit_path(boundedPath, content);
    content.writeText("f\n");
    //this->drawPath(path, hairline);

    content.writeText("Q\n");
}

void PDFPage::drawPoints(const SkClipStack& cs, const SkMatrix& ctm,
                        SkCanvas::PointMode mode, size_t count, const SkPoint pts[],
                        const SkPaint& paint) {}

void PDFPage::drawVertices(const SkClipStack& cs, const SkMatrix& ctm,
                          const SkVertices* vertices, const SkVertices::Bone bones[],
                          int boneCount, SkBlendMode mode, const SkPaint& paint) {}

static sk_sp<SkShader> to_image_shader(const SkImage* image, const SkRect* src, const SkRect& dst) {
    SkRect srcRect = src ? *src : SkRect::Make(image->bounds());
    SkMatrix local = SkMatrix::MakeRectToRect(srcRect, dst, SkMatrix::kFill_ScaleToFit);
    return image->makeShader(&local);
}

static SkPaint clone(const SkPaint* src) { return src ? *src : SkPaint(); }

static SkPaint shader_paint(sk_sp<SkShader> shader, SkPaint paint) {
    paint.setShader(std::move(shader));
    //paint.setColor4f(SkColor4f{1, 0, 0, paint.getColor4f().fA}, nullptr);
    paint.setStyle(SkPaint::kFill_Style);
    return paint;
}

void PDFPage::drawImageRect(const SkClipStack& cs, const SkMatrix& ctm,
                           const SkImage* image, const SkRect* src, const SkRect& dst,
                           const SkPaint* paint, SkCanvas::SrcRectConstraint) {
    if (!image) { return; }
    if (image->isAlphaOnly()) {
        // TODO: make soft mask from shader_paint(to_image_shader(image, src, dst)
        this->drawPath(cs, ctm, to_path(dst), clone(paint));
    }
    this->drawPath(cs, ctm, to_path(dst),
                   shader_paint(to_image_shader(image, src, dst), clone(paint)));
}

void PDFPage::drawAnnotation(const SkClipStack& cs, const SkMatrix& ctm,
                            const SkRect& rect, const char key[], SkData* value) {}

void PDFPage::drawShadowRec(const SkClipStack& cs, const SkMatrix& ctm,
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

PDFPage::~PDFPage() {}
