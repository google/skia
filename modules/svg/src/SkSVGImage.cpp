/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/svg/include/SkSVGImage.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkString.h"
#include "include/private/base/SkAssert.h"
#include "modules/skresources/include/SkResources.h"
#include "modules/svg/include/SkSVGAttributeParser.h"
#include "modules/svg/include/SkSVGRenderContext.h"
#include "src/utils/SkOSPath.h"

#include <utility>

bool SkSVGImage::parseAndSetAttribute(const char* n, const char* v) {
    return INHERITED::parseAndSetAttribute(n, v) ||
           this->setX(SkSVGAttributeParser::parse<SkSVGLength>("x", n, v)) ||
           this->setY(SkSVGAttributeParser::parse<SkSVGLength>("y", n, v)) ||
           this->setWidth(SkSVGAttributeParser::parse<SkSVGLength>("width", n, v)) ||
           this->setHeight(SkSVGAttributeParser::parse<SkSVGLength>("height", n, v)) ||
           this->setHref(SkSVGAttributeParser::parse<SkSVGIRI>("xlink:href", n, v)) ||
           this->setPreserveAspectRatio(SkSVGAttributeParser::parse<SkSVGPreserveAspectRatio>(
                   "preserveAspectRatio", n, v));
}

bool SkSVGImage::onPrepareToRender(SkSVGRenderContext* ctx) const {
    // Width or height of 0 disables rendering per spec:
    // https://www.w3.org/TR/SVG11/struct.html#ImageElement
    return !fHref.iri().isEmpty() && fWidth.value() > 0 && fHeight.value() > 0 &&
           INHERITED::onPrepareToRender(ctx);
}

static sk_sp<SkImage> LoadImage(const sk_sp<skresources::ResourceProvider>& rp,
                                const SkSVGIRI& href) {
    // TODO: It may be better to use the SVG 'id' attribute as the asset id, to allow
    // clients to perform asset substitution based on element id.
    sk_sp<skresources::ImageAsset> imageAsset;
    switch (href.type()) {
        case SkSVGIRI::Type::kDataURI:
            imageAsset = rp->loadImageAsset("", href.iri().c_str(), "");
            break;
        case SkSVGIRI::Type::kNonlocal: {
            const auto path = SkOSPath::Dirname(href.iri().c_str());
            const auto name = SkOSPath::Basename(href.iri().c_str());
            imageAsset = rp->loadImageAsset(path.c_str(), name.c_str(), /* id */ name.c_str());
            break;
        }
        default:
            SkDEBUGF("error loading image: unhandled iri type %d\n", (int)href.type());
            return nullptr;
    }

    return imageAsset ? imageAsset->getFrameData(0).image : nullptr;
}

SkSVGImage::ImageInfo SkSVGImage::LoadImage(const sk_sp<skresources::ResourceProvider>& rp,
                                            const SkSVGIRI& iri,
                                            const SkRect& viewPort,
                                            SkSVGPreserveAspectRatio par) {
    SkASSERT(rp);

    // TODO: svg sources
    sk_sp<SkImage> image = ::LoadImage(rp, iri);
    if (!image) {
        return {};
    }

    // Per spec: raster content has implicit viewbox of '0 0 width height'.
    const SkRect viewBox = SkRect::Make(image->bounds());

    // Map and place at x, y specified by viewport
    const SkMatrix m = ComputeViewboxMatrix(viewBox, viewPort, par);
    const SkRect dst = m.mapRect(viewBox).makeOffset(viewPort.fLeft, viewPort.fTop);

    return {std::move(image), dst};
}

void SkSVGImage::onRender(const SkSVGRenderContext& ctx) const {
    // Per spec: x, w, width, height attributes establish the new viewport.
    const SkSVGLengthContext& lctx = ctx.lengthContext();
    const SkRect viewPort = lctx.resolveRect(fX, fY, fWidth, fHeight);

    const auto imgInfo = LoadImage(ctx.resourceProvider(), fHref, viewPort, fPreserveAspectRatio);
    if (!imgInfo.fImage) {
        SkDEBUGF("can't render image: load image failed\n");
        return;
    }

    // TODO: image-rendering property
    ctx.canvas()->drawImageRect(
            imgInfo.fImage, imgInfo.fDst, SkSamplingOptions(SkFilterMode::kLinear));
}

SkPath SkSVGImage::onAsPath(const SkSVGRenderContext&) const { return {}; }

SkRect SkSVGImage::onTransformableObjectBoundingBox(const SkSVGRenderContext& ctx) const {
    const SkSVGLengthContext& lctx = ctx.lengthContext();
    return lctx.resolveRect(fX, fY, fWidth, fHeight);
}
