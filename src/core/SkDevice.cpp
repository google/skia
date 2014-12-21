/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDevice.h"
#include "SkDeviceProperties.h"
#include "SkDraw.h"
#include "SkMetaData.h"
#include "SkPatchUtils.h"
#include "SkShader.h"
#include "SkTextBlob.h"

SkBaseDevice::SkBaseDevice()
    : fLeakyProperties(SkNEW_ARGS(SkDeviceProperties, (SkDeviceProperties::kLegacyLCD_InitType)))
#ifdef SK_DEBUG
    , fAttachedToCanvas(false)
#endif
{
    fOrigin.setZero();
    fMetaData = NULL;
}

SkBaseDevice::SkBaseDevice(const SkDeviceProperties& dp)
    : fLeakyProperties(SkNEW_ARGS(SkDeviceProperties, (dp)))
#ifdef SK_DEBUG
    , fAttachedToCanvas(false)
#endif
{
    fOrigin.setZero();
    fMetaData = NULL;
}

SkBaseDevice::~SkBaseDevice() {
    SkDELETE(fLeakyProperties);
    SkDELETE(fMetaData);
}

SkMetaData& SkBaseDevice::getMetaData() {
    // metadata users are rare, so we lazily allocate it. If that changes we
    // can decide to just make it a field in the device (rather than a ptr)
    if (NULL == fMetaData) {
        fMetaData = new SkMetaData;
    }
    return *fMetaData;
}

SkImageInfo SkBaseDevice::imageInfo() const {
    return SkImageInfo::MakeUnknown();
}

const SkBitmap& SkBaseDevice::accessBitmap(bool changePixels) {
    const SkBitmap& bitmap = this->onAccessBitmap();
    if (changePixels) {
        bitmap.notifyPixelsChanged();
    }
    return bitmap;
}

SkPixelGeometry SkBaseDevice::CreateInfo::AdjustGeometry(const SkImageInfo& info,
                                                         Usage usage,
                                                         SkPixelGeometry geo) {
    switch (usage) {
        case kGeneral_Usage:
            break;
        case kSaveLayer_Usage:
            if (info.alphaType() != kOpaque_SkAlphaType) {
                geo = kUnknown_SkPixelGeometry;
            }
            break;
        case kImageFilter_Usage:
            geo = kUnknown_SkPixelGeometry;
            break;
    }
    return geo;
}

void SkBaseDevice::initForRootLayer(SkPixelGeometry geo) {
    // For now we don't expect to change the geometry for the root-layer, but we make the call
    // anyway to document logically what is going on.
    //
    fLeakyProperties->setPixelGeometry(CreateInfo::AdjustGeometry(this->imageInfo(),
                                                                  kGeneral_Usage,
                                                                  geo));
}

SkSurface* SkBaseDevice::newSurface(const SkImageInfo&, const SkSurfaceProps&) { return NULL; }

const void* SkBaseDevice::peekPixels(SkImageInfo*, size_t*) { return NULL; }

void SkBaseDevice::drawDRRect(const SkDraw& draw, const SkRRect& outer,
                              const SkRRect& inner, const SkPaint& paint) {
    SkPath path;
    path.addRRect(outer);
    path.addRRect(inner);
    path.setFillType(SkPath::kEvenOdd_FillType);

    const SkMatrix* preMatrix = NULL;
    const bool pathIsMutable = true;
    this->drawPath(draw, path, paint, preMatrix, pathIsMutable);
}

void SkBaseDevice::drawPatch(const SkDraw& draw, const SkPoint cubics[12], const SkColor colors[4],
                             const SkPoint texCoords[4], SkXfermode* xmode, const SkPaint& paint) {
    SkPatchUtils::VertexData data;
    
    SkISize lod = SkPatchUtils::GetLevelOfDetail(cubics, draw.fMatrix);

    // It automatically adjusts lodX and lodY in case it exceeds the number of indices.
    // If it fails to generate the vertices, then we do not draw. 
    if (SkPatchUtils::getVertexData(&data, cubics, colors, texCoords, lod.width(), lod.height())) {
        this->drawVertices(draw, SkCanvas::kTriangles_VertexMode, data.fVertexCount, data.fPoints,
                           data.fTexCoords, data.fColors, xmode, data.fIndices, data.fIndexCount,
                           paint);
    }
}

void SkBaseDevice::drawTextBlob(const SkDraw& draw, const SkTextBlob* blob, SkScalar x, SkScalar y,
                                const SkPaint &paint) {

    SkPaint runPaint = paint;

    SkTextBlob::RunIterator it(blob);
    while (!it.done()) {
        size_t textLen = it.glyphCount() * sizeof(uint16_t);
        const SkPoint& offset = it.offset();
        // applyFontToPaint() always overwrites the exact same attributes,
        // so it is safe to not re-seed the paint.
        it.applyFontToPaint(&runPaint);
        runPaint.setFlags(this->filterTextFlags(runPaint));

        switch (it.positioning()) {
        case SkTextBlob::kDefault_Positioning:
            this->drawText(draw, it.glyphs(), textLen, x + offset.x(), y + offset.y(), runPaint);
            break;
        case SkTextBlob::kHorizontal_Positioning:
            this->drawPosText(draw, it.glyphs(), textLen, it.pos(), 1,
                              SkPoint::Make(x, y + offset.y()), runPaint);
            break;
        case SkTextBlob::kFull_Positioning:
            this->drawPosText(draw, it.glyphs(), textLen, it.pos(), 2,
                              SkPoint::Make(x, y), runPaint);
            break;
        default:
            SkFAIL("unhandled positioning mode");
        }

        it.next();
    }
}

bool SkBaseDevice::readPixels(const SkImageInfo& info, void* dstP, size_t rowBytes, int x, int y) {
#ifdef SK_DEBUG
    SkASSERT(info.width() > 0 && info.height() > 0);
    SkASSERT(dstP);
    SkASSERT(rowBytes >= info.minRowBytes());
    SkASSERT(x >= 0 && y >= 0);

    const SkImageInfo& srcInfo = this->imageInfo();
    SkASSERT(x + info.width() <= srcInfo.width());
    SkASSERT(y + info.height() <= srcInfo.height());
#endif
    return this->onReadPixels(info, dstP, rowBytes, x, y);
}

bool SkBaseDevice::writePixels(const SkImageInfo& info, const void* pixels, size_t rowBytes,
                               int x, int y) {
#ifdef SK_DEBUG
    SkASSERT(info.width() > 0 && info.height() > 0);
    SkASSERT(pixels);
    SkASSERT(rowBytes >= info.minRowBytes());
    SkASSERT(x >= 0 && y >= 0);

    const SkImageInfo& dstInfo = this->imageInfo();
    SkASSERT(x + info.width() <= dstInfo.width());
    SkASSERT(y + info.height() <= dstInfo.height());
#endif
    return this->onWritePixels(info, pixels, rowBytes, x, y);
}

bool SkBaseDevice::onWritePixels(const SkImageInfo&, const void*, size_t, int, int) {
    return false;
}

bool SkBaseDevice::onReadPixels(const SkImageInfo&, void*, size_t, int x, int y) {
    return false;
}

void* SkBaseDevice::accessPixels(SkImageInfo* info, size_t* rowBytes) {
    SkImageInfo tmpInfo;
    size_t tmpRowBytes;
    if (NULL == info) {
        info = &tmpInfo;
    }
    if (NULL == rowBytes) {
        rowBytes = &tmpRowBytes;
    }
    return this->onAccessPixels(info, rowBytes);
}

void* SkBaseDevice::onAccessPixels(SkImageInfo* info, size_t* rowBytes) {
    return NULL;
}

bool SkBaseDevice::EXPERIMENTAL_drawPicture(SkCanvas*, const SkPicture*, const SkMatrix*,
                                            const SkPaint*) {
    // The base class doesn't perform any accelerated picture rendering
    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////

uint32_t SkBaseDevice::filterTextFlags(const SkPaint& paint) const {
    uint32_t flags = paint.getFlags();

    if (!paint.isLCDRenderText() || !paint.isAntiAlias()) {
        return flags;
    }

    if (kUnknown_SkPixelGeometry == fLeakyProperties->pixelGeometry()
        || this->onShouldDisableLCD(paint)) {

        flags &= ~SkPaint::kLCDRenderText_Flag;
        flags |= SkPaint::kGenA8FromLCD_Flag;
    }

    return flags;
}

