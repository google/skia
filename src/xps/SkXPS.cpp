/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkXPS.h"

#ifdef SK_BUILD_FOR_WIN

#ifndef UNICODE
  #define UNICODE
#endif
#ifndef _UNICODE
  #define _UNICODE
#endif

#include <FontSub.h>
#include <ObjBase.h>
#include <T2EmbApi.h>

#include "SkAutoCoInitialize.h"
#include "SkBitSet.h"
#include "SkColor.h"
#include "SkData.h"
#include "SkDraw.h"
#include "SkEndian.h"
#include "SkFindAndPlaceGlyph.h"
#include "SkGeometry.h"
#include "SkGlyphCache.h"
#include "SkHRESULT.h"
#include "SkIStream.h"
#include "SkImage.h"
#include "SkImageEncoder.h"
#include "SkImageFilter.h"
#include "SkMaskFilter.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkPathEffect.h"
#include "SkPathOps.h"
#include "SkPoint.h"
#include "SkRasterClip.h"
#include "SkRasterizer.h"
#include "SkSFNTHeader.h"
#include "SkShader.h"
#include "SkSize.h"
#include "SkStream.h"
#include "SkTArray.h"
#include "SkTDArray.h"
#include "SkTLazy.h"
#include "SkTTCFHeader.h"
#include "SkTypeface.h"
#include "SkTypefacePriv.h"
#include "SkUtils.h"

////////////////////////////////////////////////////////////////////////////////////////////////////


struct SkXPS::TypefaceUse {
    SkFontID typefaceId = 0xFFFFFFFF;
    int ttcIndex = 0;
    SkStream* fontData = nullptr;
    SkTScopedComPtr<IXpsOMFontResource> xpsFont;
    SkBitSet glyphsUsed;

    explicit TypefaceUse(int numberOfGlyphs) : glyphsUsed(numberOfGlyphs) {}
    ~TypefaceUse() {}

    TypefaceUse(SkXPS::TypefaceUse&&) = delete;
    TypefaceUse(const SkXPS::TypefaceUse&) = delete;
    TypefaceUse& operator=(SkXPS::TypefaceUse&&) = delete;
    TypefaceUse& operator=(const SkXPS::TypefaceUse&) = delete;
};

struct SkXPS::Layer {
    ////////////////////////////////////////
    // SkPaint fLayerPaint;
    // sk_sp<const SkImageFilter> fLayerBackDrop;

    IXpsOMObjectFactory* fXpsFactory;  // non-owning
    SkXPS* fXps;

    unsigned int fCurrentPage;
    SkTScopedComPtr<IXpsOMCanvas> fCurrentXpsCanvas;
    SkSize fCurrentCanvasSize;
    SkVector fCurrentUnitsPerMeter;
    SkVector fCurrentPixelsPerMeter;


    ////////////////////////////////////////
    Layer(IXpsOMObjectFactory*, SkXPS*);

    ~Layer();

    void drawPaint(const SkMatrix&, const SkPath&, const SkPaint& paint);

    void drawPath(const SkMatrix&, const SkPath&,
                  const SkPath& platonicPath,
                  const SkPaint& paint,
                  const SkMatrix* prePathMatrix,
                  bool pathIsMutable);

    void drawBitmap(const SkMatrix&, const SkPath&,
                    const SkBitmap& bitmap,
                    const SkMatrix& matrix,
                    const SkPaint& paint);

    void drawText(
        const SkMatrix&, const SkPath&,
        const void* text, size_t len,
        SkScalar x, SkScalar y,
        const SkPaint& paint);

    void drawPosText(
        const SkMatrix&, const SkPath&,
        const void* text, size_t len,
        const SkScalar pos[], int scalarsPerPos,
        const SkPoint& offset, const SkPaint& paint);

    /** Creates a GUID based id and places it into buffer.
        buffer should have space for at least GUID_ID_LEN wide characters.
        The string will always be wchar null terminated.
        XXXXXXXXsXXXXsXXXXsXXXXsXXXXXXXXXXXX0
        The string may begin with a digit,
        and so may not be suitable as a bare resource key.
     */

    HRESULT initXpsDocumentWriter(IXpsOMImageResource* image);

    void internalDrawRect(
        const SkMatrix&, const SkPath&,
        const SkRect& r,
        bool transformRect,
        const SkPaint& paint);

    HRESULT createXpsBrush(
        const SkPaint& skPaint,
        IXpsOMBrush** xpsBrush,
        const SkMatrix* parentTransform = NULL);

    HRESULT createXpsSolidColorBrush(
        const SkColor skColor, const SkAlpha alpha,
        IXpsOMBrush** xpsBrush);

    HRESULT createXpsImageBrush(
        const SkBitmap& bitmap,
        const SkMatrix& localMatrix,
        const SkShader::TileMode (&xy)[2],
        const SkAlpha alpha,
        IXpsOMTileBrush** xpsBrush);

    HRESULT createXpsLinearGradient(
        SkShader::GradientInfo info,
        const SkAlpha alpha,
        const SkMatrix& localMatrix,
        IXpsOMMatrixTransform* xpsMatrixToUse,
        IXpsOMBrush** xpsBrush);

    HRESULT createXpsRadialGradient(
        SkShader::GradientInfo info,
        const SkAlpha alpha,
        const SkMatrix& localMatrix,
        IXpsOMMatrixTransform* xpsMatrixToUse,
        IXpsOMBrush** xpsBrush);

    HRESULT createXpsGradientStop(
        const SkColor skColor,
        const SkScalar offset,
        IXpsOMGradientStop** xpsGradStop);

    HRESULT createXpsTransform(
        const SkMatrix& matrix,
        IXpsOMMatrixTransform ** xpsTransform);

    HRESULT createXpsRect(
        const SkRect& rect,
        BOOL stroke, BOOL fill,
        IXpsOMGeometryFigure** xpsRect);

    HRESULT createXpsQuad(
        const SkPoint (&points)[4],
        BOOL stroke, BOOL fill,
        IXpsOMGeometryFigure** xpsQuad);

    HRESULT CreateTypefaceUse(
        const SkPaint& paint,
        TypefaceUse** fontResource);

    HRESULT AddGlyphs(
        const SkMatrix& ctm, const SkPath& clipPath,
        IXpsOMCanvas* canvas,
        TypefaceUse* font,
        LPCWSTR text,
        XPS_GLYPH_INDEX* xpsGlyphs,
        UINT32 xpsGlyphsLen,
        XPS_POINT *origin,
        FLOAT fontSize,
        XPS_STYLE_SIMULATION sims,
        const SkMatrix& transform,
        const SkPaint& paint);

    HRESULT addXpsPathGeometry(
        IXpsOMGeometryFigureCollection* figures,
        BOOL stroke, BOOL fill, const SkPath& path);

    HRESULT createPath(
        IXpsOMGeometryFigure* figure,
        IXpsOMVisualCollection* visuals,
        IXpsOMPath** path);

    HRESULT sideOfClamp(
        const SkRect& leftPoints, const XPS_RECT& left,
        IXpsOMImageResource* imageResource,
        IXpsOMVisualCollection* visuals);

    HRESULT cornerOfClamp(
        const SkRect& tlPoints,
        const SkColor color,
        IXpsOMVisualCollection* visuals);

    HRESULT clip(
        IXpsOMVisual* xpsVisual,
        const SkMatrix& ctm, const SkPath& clipPath);
    HRESULT clipToPath(
        IXpsOMVisual* xpsVisual,
        const SkPath& clipPath,
        XPS_FILL_RULE fillRule);

    HRESULT drawInverseWindingPath(
        const SkMatrix& ctm, const SkPath& clipPath,
        const SkPath& devicePath,
        IXpsOMPath* xpsPath);

    HRESULT shadePath(
        IXpsOMPath* shadedPath,
        const SkPaint& shaderPaint,
        const SkMatrix& matrix,
        BOOL* fill, BOOL* stroke);

    void convertToPpm(
        const SkMaskFilter* filter,
        SkMatrix* matrix,
        SkVector* ppuScale,
        const SkIRect& clip, SkIRect* clipIRect);

    HRESULT applyMask(
        const SkMatrix& ctm, const SkPath& clipPath,
        const SkMask& mask,
        const SkVector& ppuScale,
        IXpsOMPath* shadedPath);

    SkXPS::Layer(const SkXPS::Layer&) = delete;
    SkXPS::Layer(SkXPS::Layer&&) = delete;
    SkXPS::Layer& operator=(const SkXPS::Layer&) = delete;
    SkXPS::Layer& operator=(SkXPS::Layer&&) = delete;

};

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG
    #define HRM_DCHECK(ex, msg)       \
        do {                         \
            HRESULT hr = ex;         \
            if (FAILED(hr)) {        \
                SK_TRACEHR(hr, msg); \
                SK_ABORT(#ex);       \
            }                        \
        } while (false)
#else
    #define HRM_DCHECK(ex, msg) do { (void)ex; } while (false)
#endif
#define HR_DCHECK(ex) HRM_DCHECK(ex, nullptr)

//Windows defines a FLOAT type,
//make it clear when converting a scalar that this is what is wanted.
#define SkScalarToFLOAT(n) SkScalarToFloat(n)

//Dummy representation of a GUID from createId.
#define L_GUID_ID L"XXXXXXXXsXXXXsXXXXsXXXXsXXXXXXXXXXXX"
//Length of GUID representation from createId, including nullptr terminator.
#define GUID_ID_LEN SK_ARRAY_COUNT(L_GUID_ID)

SkXPS::SkXPS(SkWStream* wStream,
             IXpsOMObjectFactory* xpsFactory,
             SkScalar dpi)
    : fXpsFactory(SkRefComPtr(xpsFactory))
    , fDpi(dpi) {
    HR_DCHECK(SkWIStream::CreateFromSkWStream(wStream, &fOutputStream));
}

SkXPS::~SkXPS() {}

template <typename T> static constexpr size_t sk_digits_in() {
    return static_cast<size_t>(std::numeric_limits<T>::digits10 + 1);
}

static HRESULT create_xps_page(IXpsOMObjectFactory* xpsFactory,
                               const XPS_SIZE& pageSize,
                               unsigned currentPage,
                               IXpsOMPage** page) {
    constexpr size_t size = SK_ARRAY_COUNT(L"/Documents/1/Pages/.fpage")
                          + sk_digits_in<decltype(currentPage)>();
    wchar_t buffer[size];
    swprintf_s(buffer, size, L"/Documents/1/Pages/%u.fpage", currentPage);
    SkTScopedComPtr<IOpcPartUri> partUri;
    HR(xpsFactory->CreatePartUri(buffer, &partUri));

    //If the language is unknown, use "und" (XPS Spec 2.3.5.1).
    HR(xpsFactory->CreatePage(&pageSize, L"und", partUri.get(), page));
    return S_OK;
}

constexpr size_t kMaxUriSize = SkTMax(
        SK_ARRAY_COUNT(L"/Documents/1/Metadata/.png") + sk_digits_in<unsigned>(),
        SK_ARRAY_COUNT(L"/Metadata/" L_GUID_ID L".png"));


static HRESULT create_xps_thumbnail(IXpsOMObjectFactory* xpsFactory,
                                    IXpsOMPage* page,
                                    wchar_t* uri,
                                    IXpsOMImageResource** image) {
    SkTScopedComPtr<IXpsOMThumbnailGenerator> thumbnailGenerator;
    HR(CoCreateInstance(
            CLSID_XpsOMThumbnailGenerator,
            nullptr,
            CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(&thumbnailGenerator)));
    SkTScopedComPtr<IOpcPartUri> partUri;
    HR(xpsFactory->CreatePartUri(uri, &partUri));
    HR(thumbnailGenerator->GenerateThumbnail(page,
                                             XPS_IMAGE_TYPE_PNG,
                                             XPS_THUMBNAIL_SIZE_LARGE,
                                             partUri.get(),
                                             image));
    return S_OK;
}
void subset_typeface(SkXPS::TypefaceUse* typeface) {
    //CreateFontPackage wants unsigned short.
    //Microsoft, Y U NO stdint.h?
    SkTDArray<unsigned short> keepList;
    typeface->glyphsUsed.exportTo(&keepList);

    int ttcCount = (typeface->ttcIndex + 1);

    //The following are declared with the types required by CreateFontPackage.
    unsigned char *fontPackageBufferRaw = nullptr;
    unsigned long fontPackageBufferSize;
    unsigned long bytesWritten;
    unsigned long result = CreateFontPackage(
        (unsigned char *) typeface->fontData->getMemoryBase(),
        (unsigned long) typeface->fontData->getLength(),
        &fontPackageBufferRaw,
        &fontPackageBufferSize,
        &bytesWritten,
        TTFCFP_FLAGS_SUBSET | TTFCFP_FLAGS_GLYPHLIST | (ttcCount > 0 ? TTFCFP_FLAGS_TTC : 0),
        typeface->ttcIndex,
        TTFCFP_SUBSET,
        0,
        0,
        0,
        keepList.begin(),
        keepList.count(),
        sk_malloc_throw,
        sk_realloc_throw,
        sk_free,
        nullptr);
    SkAutoTMalloc<unsigned char> fontPackageBuffer(fontPackageBufferRaw);
    if (result != NO_ERROR) {
        SkDEBUGF(("CreateFontPackage Error %lu", result));
        return;
    }

    // If it was originally a ttc, keep it a ttc.
    // CreateFontPackage over-allocates, realloc usually decreases the size substantially.
    size_t extra;
    if (ttcCount > 0) {
        // Create space for a ttc header.
        extra = sizeof(SkTTCFHeader) + (ttcCount * sizeof(SK_OT_ULONG));
        fontPackageBuffer.realloc(bytesWritten + extra);
        //overlap is certain, use memmove
        memmove(fontPackageBuffer.get() + extra, fontPackageBuffer.get(), bytesWritten);

        // Write the ttc header.
        SkTTCFHeader* ttcfHeader = reinterpret_cast<SkTTCFHeader*>(fontPackageBuffer.get());
        ttcfHeader->ttcTag = SkTTCFHeader::TAG;
        ttcfHeader->version = SkTTCFHeader::version_1;
        ttcfHeader->numOffsets = SkEndian_SwapBE32(ttcCount);
        SK_OT_ULONG* offsetPtr = SkTAfter<SK_OT_ULONG>(ttcfHeader);
        for (int i = 0; i < ttcCount; ++i, ++offsetPtr) {
            *offsetPtr = SkEndian_SwapBE32(SkToU32(extra));
        }

        // Fix up offsets in sfnt table entries.
        SkSFNTHeader* sfntHeader = SkTAddOffset<SkSFNTHeader>(fontPackageBuffer.get(), extra);
        int numTables = SkEndian_SwapBE16(sfntHeader->numTables);
        SkSFNTHeader::TableDirectoryEntry* tableDirectory =
            SkTAfter<SkSFNTHeader::TableDirectoryEntry>(sfntHeader);
        for (int i = 0; i < numTables; ++i, ++tableDirectory) {
            tableDirectory->offset = SkEndian_SwapBE32(
                SkToU32(SkEndian_SwapBE32(SkToU32(tableDirectory->offset)) + extra));
        }
    } else {
        extra = 0;
        fontPackageBuffer.realloc(bytesWritten);
    }

    std::unique_ptr<SkMemoryStream> newStream(new SkMemoryStream());
    newStream->setMemoryOwned(fontPackageBuffer.release(), bytesWritten + extra);

    SkTScopedComPtr<IStream> newIStream;
    SkIStream::CreateFromSkStream(newStream.release(), true, &newIStream);

    XPS_FONT_EMBEDDING embedding;
    HRV(typeface->xpsFont->GetEmbeddingOption(&embedding));

    SkTScopedComPtr<IOpcPartUri> partUri;
    HRV(typeface->xpsFont->GetPartName(&partUri));

    HRV(typeface->xpsFont->SetContent(
            newIStream.get(),
            embedding,
            partUri.get()));
}

/*
    wchar_t buffer[size];
    if (pageNum > 0) {
        swprintf_s(buffer, size, L"/Documents/1/Metadata/%u.png", pageNum);
    } else {
        wchar_t id[GUID_ID_LEN];
        xps->createId(id, GUID_ID_LEN);
        swprintf_s(buffer, size, L"/Metadata/%s.png", id);
    }
*/
void SkXPS::endPage() {
    if (!fLayers.count()) {
        return;
    }
    while (fLayers.count() > 1) {
        this->restoreLayer();
    }
    XPS_SIZE pageSize = { SkScalarToFLOAT(fCurrentPageSize.width()),
                          SkScalarToFLOAT(fCurrentPageSize.height()) };
    SkTScopedComPtr<IXpsOMPage> page;
    HRV(create_xps_page(fXpsFactory.get(), pageSize, fCurrentPage, &page));
    SkTScopedComPtr<IXpsOMVisualCollection> pageVisuals;
    HRV(page->GetVisuals(&pageVisuals));
    SkXPS::Layer& topLayer = fLayers.back();
    HRV(pageVisuals->Append(topLayer.fCurrentXpsCanvas.get()));
    if (!fPackageWriter.get()) {
        SkTScopedComPtr<IXpsOMImageResource> image;
        wchar_t buffer[kMaxUriSize];
        wchar_t id[GUID_ID_LEN];
        this->createId(id, GUID_ID_LEN);
        swprintf_s(buffer, kMaxUriSize, L"/Metadata/%s.png", id);
        (void)create_xps_thumbnail(fXpsFactory.get(), page.get(), buffer, &image);
        SkTScopedComPtr<IOpcPartUri> partUri;
        HRV(fXpsFactory->CreatePartUri(L"/FixedDocumentSequence.fdseq", &partUri));
        HRV(fXpsFactory->CreatePackageWriterOnStream(
                fOutputStream.get(), TRUE, XPS_INTERLEAVING_OFF,
                partUri.get(), nullptr, image.get(), nullptr, nullptr, &fPackageWriter));
        partUri.reset(nullptr);
        HRV(fXpsFactory->CreatePartUri( L"/Documents/1/FixedDocument.fdoc", &partUri));
        HRV(fPackageWriter->StartNewDocument(partUri.get(), nullptr, nullptr, nullptr, nullptr));
    }
    HRV(fPackageWriter->AddPage(page.get(), &pageSize, nullptr,  nullptr,  nullptr,  nullptr));
}

void SkXPS::newPage(SkSize pageSize) {
    this->endPage();
    this->saveLayer(SkCanvas::SaveLayerRec());
    fCurrentPageSize = pageSize;
}

void SkXPS::endPortfolio() {
    this->endPage();
    if (!fTypefaces.empty()) {
        for (TypefaceUse& typefaceUse : fTypefaces) {
            subset_typeface(&typefaceUse);
        }
    }
    if (fPackageWriter) {
        HRV(fPackageWriter->Close());
    }
}

void SkXPS::saveLayer(const SkCanvas::SaveLayerRec& rec) {
    // SkXPS::Layer& topLayer =
    fLayers.emplace_back(fXpsFactory.get(), this);

    // TODO: implement effects on paint
    // if (rec.fPaint) {
    //    topLayer.fLayerPaint = *rec.fPaint;
    // }
    // TODO: implement SkImageFilter.
    // topLayer.fLayerBackDrop.reset(SkSafeRef(rec.fBackdrop));
}

void SkXPS::restoreLayer() {
    SkASSERT(fLayers.count() > 0);
    if (fLayers.count() <= 1) {
        return;
    }
    SkXPS::Layer& topLayer = fLayers.back();
    SkXPS::Layer& nextLayer = fLayers[fLayers.count() - 2];

    SkTScopedComPtr<IXpsOMVisualCollection> currentVisuals;
    HRVM(nextLayer.fCurrentXpsCanvas->GetVisuals(&currentVisuals), "GetVisuals failed");
    HRVM(currentVisuals->Append(topLayer.fCurrentXpsCanvas.get()), "Append failed");

    fLayers.pop_back();
}

SkXPS::Layer::Layer(IXpsOMObjectFactory* xpsFactory, SkXPS* xps)
    : fCurrentPage(0), fXpsFactory(xpsFactory), fXps(xps) {
    HR_DCHECK(fXpsFactory->CreateCanvas(&fCurrentXpsCanvas));
}

SkXPS::Layer::~Layer() {}

static XPS_COLOR xps_color(const SkColor skColor) {
    //XPS uses non-pre-multiplied alpha (XPS Spec 11.4).
    XPS_COLOR xpsColor;
    xpsColor.colorType = XPS_COLOR_TYPE_SRGB;
    xpsColor.value.sRGB.alpha = SkColorGetA(skColor);
    xpsColor.value.sRGB.red = SkColorGetR(skColor);
    xpsColor.value.sRGB.green = SkColorGetG(skColor);
    xpsColor.value.sRGB.blue = SkColorGetB(skColor);
    return xpsColor;
}

static XPS_POINT xps_point(const SkPoint& point) {
    XPS_POINT xpsPoint = {
        SkScalarToFLOAT(point.fX),
        SkScalarToFLOAT(point.fY),
    };
    return xpsPoint;
}

static XPS_POINT xps_point(const SkPoint& point, const SkMatrix& matrix) {
    SkPoint skTransformedPoint;
    matrix.mapXY(point.fX, point.fY, &skTransformedPoint);
    return xps_point(skTransformedPoint);
}

static XPS_SPREAD_METHOD xps_spread_method(SkShader::TileMode tileMode) {
    switch (tileMode) {
    case SkShader::kClamp_TileMode:
        return XPS_SPREAD_METHOD_PAD;
    case SkShader::kRepeat_TileMode:
        return XPS_SPREAD_METHOD_REPEAT;
    case SkShader::kMirror_TileMode:
        return XPS_SPREAD_METHOD_REFLECT;
    default:
        SkDEBUGFAIL("Unknown tile mode.");
    }
    return XPS_SPREAD_METHOD_PAD;
}

static void transform_offsets(SkScalar* stopOffsets, const int numOffsets,
                              const SkPoint& start, const SkPoint& end,
                              const SkMatrix& transform) {
    SkPoint startTransformed;
    transform.mapXY(start.fX, start.fY, &startTransformed);
    SkPoint endTransformed;
    transform.mapXY(end.fX, end.fY, &endTransformed);

    //Manhattan distance between transformed start and end.
    SkScalar startToEnd = (endTransformed.fX - startTransformed.fX)
                        + (endTransformed.fY - startTransformed.fY);
    if (SkScalarNearlyZero(startToEnd)) {
        for (int i = 0; i < numOffsets; ++i) {
            stopOffsets[i] = 0;
        }
        return;
    }

    for (int i = 0; i < numOffsets; ++i) {
        SkPoint stop;
        stop.fX = SkScalarMul(end.fX - start.fX, stopOffsets[i]);
        stop.fY = SkScalarMul(end.fY - start.fY, stopOffsets[i]);

        SkPoint stopTransformed;
        transform.mapXY(stop.fX, stop.fY, &stopTransformed);

        //Manhattan distance between transformed start and stop.
        SkScalar startToStop = (stopTransformed.fX - startTransformed.fX)
                             + (stopTransformed.fY - startTransformed.fY);
        //Percentage along transformed line.
        stopOffsets[i] = startToStop / startToEnd;
    }
}

HRESULT SkXPS::Layer::createXpsTransform(const SkMatrix& matrix,
                                        IXpsOMMatrixTransform** xpsTransform) {
    SkScalar affine[6];
    if (!matrix.asAffine(affine)) {
        *xpsTransform = nullptr;
        return S_FALSE;
    }
    XPS_MATRIX rawXpsMatrix = {
        SkScalarToFLOAT(affine[SkMatrix::kAScaleX]),
        SkScalarToFLOAT(affine[SkMatrix::kASkewY]),
        SkScalarToFLOAT(affine[SkMatrix::kASkewX]),
        SkScalarToFLOAT(affine[SkMatrix::kAScaleY]),
        SkScalarToFLOAT(affine[SkMatrix::kATransX]),
        SkScalarToFLOAT(affine[SkMatrix::kATransY]),
    };
    HRM(fXpsFactory->CreateMatrixTransform(&rawXpsMatrix, xpsTransform),
        "Could not create transform.");

    return S_OK;
}

HRESULT SkXPS::Layer::createPath(IXpsOMGeometryFigure* figure,
                                IXpsOMVisualCollection* visuals,
                                IXpsOMPath** path) {
    SkTScopedComPtr<IXpsOMGeometry> geometry;
    HRM(fXpsFactory->CreateGeometry(&geometry),
        "Could not create geometry.");

    SkTScopedComPtr<IXpsOMGeometryFigureCollection> figureCollection;
    HRM(geometry->GetFigures(&figureCollection), "Could not get figures.");
    HRM(figureCollection->Append(figure), "Could not add figure.");

    HRM(fXpsFactory->CreatePath(path), "Could not create path.");
    HRM((*path)->SetGeometryLocal(geometry.get()), "Could not set geometry");

    HRM(visuals->Append(*path), "Could not add path to visuals.");
    return S_OK;
}

HRESULT SkXPS::Layer::createXpsSolidColorBrush(const SkColor skColor,
                                              const SkAlpha alpha,
                                              IXpsOMBrush** xpsBrush) {
    XPS_COLOR xpsColor = xps_color(skColor);
    SkTScopedComPtr<IXpsOMSolidColorBrush> solidBrush;
    HRM(fXpsFactory->CreateSolidColorBrush(&xpsColor, nullptr, &solidBrush),
        "Could not create solid color brush.");
    HRM(solidBrush->SetOpacity(alpha / 255.0f), "Could not set opacity.");
    HRM(solidBrush->QueryInterface<IXpsOMBrush>(xpsBrush), "QI Fail.");
    return S_OK;
}

HRESULT SkXPS::Layer::sideOfClamp(const SkRect& areaToFill,
                                 const XPS_RECT& imageViewBox,
                                 IXpsOMImageResource* image,
                                 IXpsOMVisualCollection* visuals) {
    SkTScopedComPtr<IXpsOMGeometryFigure> areaToFillFigure;
    HR(this->createXpsRect(areaToFill, FALSE, TRUE, &areaToFillFigure));

    SkTScopedComPtr<IXpsOMPath> areaToFillPath;
    HR(this->createPath(areaToFillFigure.get(), visuals, &areaToFillPath));

    SkTScopedComPtr<IXpsOMImageBrush> areaToFillBrush;
    HRM(fXpsFactory->CreateImageBrush(image,
                                      &imageViewBox,
                                      &imageViewBox,
                                      &areaToFillBrush),
        "Could not create brush for side of clamp.");
    HRM(areaToFillBrush->SetTileMode(XPS_TILE_MODE_FLIPXY),
        "Could not set tile mode for side of clamp.");
    HRM(areaToFillPath->SetFillBrushLocal(areaToFillBrush.get()),
        "Could not set brush for side of clamp");

    return S_OK;
}

HRESULT SkXPS::Layer::cornerOfClamp(const SkRect& areaToFill,
                                   const SkColor color,
                                   IXpsOMVisualCollection* visuals) {
    SkTScopedComPtr<IXpsOMGeometryFigure> areaToFillFigure;
    HR(this->createXpsRect(areaToFill, FALSE, TRUE, &areaToFillFigure));

    SkTScopedComPtr<IXpsOMPath> areaToFillPath;
    HR(this->createPath(areaToFillFigure.get(), visuals, &areaToFillPath));

    SkTScopedComPtr<IXpsOMBrush> areaToFillBrush;
    HR(this->createXpsSolidColorBrush(color, 0xFF, &areaToFillBrush));
    HRM(areaToFillPath->SetFillBrushLocal(areaToFillBrush.get()),
        "Could not set brush for corner of clamp.");

    return S_OK;
}

static const XPS_TILE_MODE XTM_N  = XPS_TILE_MODE_NONE;
static const XPS_TILE_MODE XTM_T  = XPS_TILE_MODE_TILE;
static const XPS_TILE_MODE XTM_X  = XPS_TILE_MODE_FLIPX;
static const XPS_TILE_MODE XTM_Y  = XPS_TILE_MODE_FLIPY;
static const XPS_TILE_MODE XTM_XY = XPS_TILE_MODE_FLIPXY;

//TODO(bungeman): In the future, should skia add None,
//handle None+Mirror and None+Repeat correctly.
//None is currently an internal hack so masks don't repeat (None+None only).
static XPS_TILE_MODE SkToXpsTileMode[SkShader::kTileModeCount+1]
                                    [SkShader::kTileModeCount+1] = {
               //Clamp  //Repeat //Mirror //None
    /*Clamp */ {XTM_N,  XTM_T,   XTM_Y,   XTM_N},
    /*Repeat*/ {XTM_T,  XTM_T,   XTM_Y,   XTM_N},
    /*Mirror*/ {XTM_X,  XTM_X,   XTM_XY,  XTM_X},
    /*None  */ {XTM_N,  XTM_N,   XTM_Y,   XTM_N},
};

HRESULT SkXPS::Layer::createXpsImageBrush(
        const SkBitmap& bitmap,
        const SkMatrix& localMatrix,
        const SkShader::TileMode (&xy)[2],
        const SkAlpha alpha,
        IXpsOMTileBrush** xpsBrush) {
    SkDynamicMemoryWStream write;
    if (!SkEncodeImage(&write, bitmap, SkEncodedImageFormat::kPNG, 100)) {
        return E_FAIL;
    }
    SkMemoryStream* read = new SkMemoryStream;
    read->setData(write.detachAsData());
    SkTScopedComPtr<IStream> readWrapper;
    HRM(SkIStream::CreateFromSkStream(read, true, &readWrapper),
        "Could not create stream from png data.");

    const size_t size =
        SK_ARRAY_COUNT(L"/Documents/1/Resources/Images/" L_GUID_ID L".png");
    wchar_t buffer[size];
    wchar_t id[GUID_ID_LEN];
    fXps->createId(id, GUID_ID_LEN);
    swprintf_s(buffer, size, L"/Documents/1/Resources/Images/%s.png", id);

    SkTScopedComPtr<IOpcPartUri> imagePartUri;
    HRM(fXpsFactory->CreatePartUri(buffer, &imagePartUri),
        "Could not create image part uri.");

    SkTScopedComPtr<IXpsOMImageResource> imageResource;
    HRM(fXpsFactory->CreateImageResource(
            readWrapper.get(),
            XPS_IMAGE_TYPE_PNG,
            imagePartUri.get(),
            &imageResource),
        "Could not create image resource.");

    XPS_RECT bitmapRect = {
        0.0, 0.0,
        static_cast<FLOAT>(bitmap.width()), static_cast<FLOAT>(bitmap.height())
    };
    SkTScopedComPtr<IXpsOMImageBrush> xpsImageBrush;
    HRM(fXpsFactory->CreateImageBrush(imageResource.get(),
                                            &bitmapRect, &bitmapRect,
                                            &xpsImageBrush),
        "Could not create image brush.");

    if (SkShader::kClamp_TileMode != xy[0] &&
        SkShader::kClamp_TileMode != xy[1]) {

        HRM(xpsImageBrush->SetTileMode(SkToXpsTileMode[xy[0]][xy[1]]),
            "Could not set image tile mode");
        HRM(xpsImageBrush->SetOpacity(alpha / 255.0f),
            "Could not set image opacity.");
        HRM(xpsImageBrush->QueryInterface(xpsBrush), "QI failed.");
    } else {
        //TODO(bungeman): compute how big this really needs to be.
        const SkScalar BIG = SkIntToScalar(1000); //SK_ScalarMax;
        const FLOAT BIG_F = SkScalarToFLOAT(BIG);
        const SkScalar bWidth = SkIntToScalar(bitmap.width());
        const SkScalar bHeight = SkIntToScalar(bitmap.height());

        //create brush canvas
        SkTScopedComPtr<IXpsOMCanvas> brushCanvas;
        HRM(fXpsFactory->CreateCanvas(&brushCanvas),
            "Could not create image brush canvas.");
        SkTScopedComPtr<IXpsOMVisualCollection> brushVisuals;
        HRM(brushCanvas->GetVisuals(&brushVisuals),
            "Could not get image brush canvas visuals collection.");

        //create central figure
        const SkRect bitmapPoints = SkRect::MakeLTRB(0, 0, bWidth, bHeight);
        SkTScopedComPtr<IXpsOMGeometryFigure> centralFigure;
        HR(this->createXpsRect(bitmapPoints, FALSE, TRUE, &centralFigure));

        SkTScopedComPtr<IXpsOMPath> centralPath;
        HR(this->createPath(centralFigure.get(),
                            brushVisuals.get(),
                            &centralPath));
        HRM(xpsImageBrush->SetTileMode(XPS_TILE_MODE_FLIPXY),
            "Could not set tile mode for image brush central path.");
        HRM(centralPath->SetFillBrushLocal(xpsImageBrush.get()),
            "Could not set fill brush for image brush central path.");

        //add left/right
        if (SkShader::kClamp_TileMode == xy[0]) {
            SkRect leftArea = SkRect::MakeLTRB(-BIG, 0, 0, bHeight);
            XPS_RECT leftImageViewBox = {
                0.0, 0.0,
                1.0, static_cast<FLOAT>(bitmap.height()),
            };
            HR(this->sideOfClamp(leftArea, leftImageViewBox,
                                 imageResource.get(),
                                 brushVisuals.get()));

            SkRect rightArea = SkRect::MakeLTRB(bWidth, 0, BIG, bHeight);
            XPS_RECT rightImageViewBox = {
                bitmap.width() - 1.0f, 0.0f,
                1.0f, static_cast<FLOAT>(bitmap.height()),
            };
            HR(this->sideOfClamp(rightArea, rightImageViewBox,
                                 imageResource.get(),
                                 brushVisuals.get()));
        }

        //add top/bottom
        if (SkShader::kClamp_TileMode == xy[1]) {
            SkRect topArea = SkRect::MakeLTRB(0, -BIG, bWidth, 0);
            XPS_RECT topImageViewBox = {
                0.0, 0.0,
                static_cast<FLOAT>(bitmap.width()), 1.0,
            };
            HR(this->sideOfClamp(topArea, topImageViewBox,
                                 imageResource.get(),
                                 brushVisuals.get()));

            SkRect bottomArea = SkRect::MakeLTRB(0, bHeight, bWidth, BIG);
            XPS_RECT bottomImageViewBox = {
                0.0f, bitmap.height() - 1.0f,
                static_cast<FLOAT>(bitmap.width()), 1.0f,
            };
            HR(this->sideOfClamp(bottomArea, bottomImageViewBox,
                                 imageResource.get(),
                                 brushVisuals.get()));
        }

        //add tl, tr, bl, br
        if (SkShader::kClamp_TileMode == xy[0] &&
            SkShader::kClamp_TileMode == xy[1]) {

            SkAutoLockPixels alp(bitmap);

            const SkColor tlColor = bitmap.getColor(0,0);
            const SkRect tlArea = SkRect::MakeLTRB(-BIG, -BIG, 0, 0);
            HR(this->cornerOfClamp(tlArea, tlColor, brushVisuals.get()));

            const SkColor trColor = bitmap.getColor(bitmap.width()-1,0);
            const SkRect trArea = SkRect::MakeLTRB(bWidth, -BIG, BIG, 0);
            HR(this->cornerOfClamp(trArea, trColor, brushVisuals.get()));

            const SkColor brColor = bitmap.getColor(bitmap.width()-1,
                                                    bitmap.height()-1);
            const SkRect brArea = SkRect::MakeLTRB(bWidth, bHeight, BIG, BIG);
            HR(this->cornerOfClamp(brArea, brColor, brushVisuals.get()));

            const SkColor blColor = bitmap.getColor(0,bitmap.height()-1);
            const SkRect blArea = SkRect::MakeLTRB(-BIG, bHeight, 0, BIG);
            HR(this->cornerOfClamp(blArea, blColor, brushVisuals.get()));
        }

        //create visual brush from canvas
        XPS_RECT bound = {};
        if (SkShader::kClamp_TileMode == xy[0] &&
            SkShader::kClamp_TileMode == xy[1]) {

            bound.x = BIG_F / -2;
            bound.y = BIG_F / -2;
            bound.width = BIG_F;
            bound.height = BIG_F;
        } else if (SkShader::kClamp_TileMode == xy[0]) {
            bound.x = BIG_F / -2;
            bound.y = 0.0f;
            bound.width = BIG_F;
            bound.height = static_cast<FLOAT>(bitmap.height());
        } else if (SkShader::kClamp_TileMode == xy[1]) {
            bound.x = 0;
            bound.y = BIG_F / -2;
            bound.width = static_cast<FLOAT>(bitmap.width());
            bound.height = BIG_F;
        }
        SkTScopedComPtr<IXpsOMVisualBrush> clampBrush;
        HRM(fXpsFactory->CreateVisualBrush(&bound, &bound, &clampBrush),
            "Could not create visual brush for image brush.");
        HRM(clampBrush->SetVisualLocal(brushCanvas.get()),
            "Could not set canvas on visual brush for image brush.");
        HRM(clampBrush->SetTileMode(SkToXpsTileMode[xy[0]][xy[1]]),
            "Could not set tile mode on visual brush for image brush.");
        HRM(clampBrush->SetOpacity(alpha / 255.0f),
            "Could not set opacity on visual brush for image brush.");

        HRM(clampBrush->QueryInterface(xpsBrush), "QI failed.");
    }

    SkTScopedComPtr<IXpsOMMatrixTransform> xpsMatrixToUse;
    HR(this->createXpsTransform(localMatrix, &xpsMatrixToUse));
    if (xpsMatrixToUse.get()) {
        HRM((*xpsBrush)->SetTransformLocal(xpsMatrixToUse.get()),
            "Could not set transform for image brush.");
    } else {
        //TODO(bungeman): perspective bitmaps in general.
    }

    return S_OK;
}

HRESULT SkXPS::Layer::createXpsGradientStop(const SkColor skColor,
                                           const SkScalar offset,
                                           IXpsOMGradientStop** xpsGradStop) {
    XPS_COLOR gradStopXpsColor = xps_color(skColor);
    HRM(fXpsFactory->CreateGradientStop(&gradStopXpsColor,
                                              nullptr,
                                              SkScalarToFLOAT(offset),
                                              xpsGradStop),
        "Could not create gradient stop.");
    return S_OK;
}

HRESULT SkXPS::Layer::createXpsLinearGradient(SkShader::GradientInfo info,
                                             const SkAlpha alpha,
                                             const SkMatrix& localMatrix,
                                             IXpsOMMatrixTransform* xpsMatrix,
                                             IXpsOMBrush** xpsBrush) {
    XPS_POINT startPoint;
    XPS_POINT endPoint;
    if (xpsMatrix) {
        startPoint = xps_point(info.fPoint[0]);
        endPoint = xps_point(info.fPoint[1]);
    } else {
        transform_offsets(info.fColorOffsets, info.fColorCount,
                          info.fPoint[0], info.fPoint[1],
                          localMatrix);
        startPoint = xps_point(info.fPoint[0], localMatrix);
        endPoint = xps_point(info.fPoint[1], localMatrix);
    }

    SkTScopedComPtr<IXpsOMGradientStop> gradStop0;
    HR(createXpsGradientStop(info.fColors[0],
                             info.fColorOffsets[0],
                             &gradStop0));

    SkTScopedComPtr<IXpsOMGradientStop> gradStop1;
    HR(createXpsGradientStop(info.fColors[1],
                             info.fColorOffsets[1],
                             &gradStop1));

    SkTScopedComPtr<IXpsOMLinearGradientBrush> gradientBrush;
    HRM(fXpsFactory->CreateLinearGradientBrush(gradStop0.get(),
                                                     gradStop1.get(),
                                                     &startPoint,
                                                     &endPoint,
                                                     &gradientBrush),
        "Could not create linear gradient brush.");
    if (xpsMatrix) {
        HRM(gradientBrush->SetTransformLocal(xpsMatrix),
            "Could not set transform on linear gradient brush.");
    }

    SkTScopedComPtr<IXpsOMGradientStopCollection> gradStopCollection;
    HRM(gradientBrush->GetGradientStops(&gradStopCollection),
        "Could not get linear gradient stop collection.");
    for (int i = 2; i < info.fColorCount; ++i) {
        SkTScopedComPtr<IXpsOMGradientStop> gradStop;
        HR(createXpsGradientStop(info.fColors[i],
                                 info.fColorOffsets[i],
                                 &gradStop));
        HRM(gradStopCollection->Append(gradStop.get()),
            "Could not add linear gradient stop.");
    }

    HRM(gradientBrush->SetSpreadMethod(xps_spread_method(info.fTileMode)),
        "Could not set spread method of linear gradient.");

    HRM(gradientBrush->SetOpacity(alpha / 255.0f),
        "Could not set opacity of linear gradient brush.");
    HRM(gradientBrush->QueryInterface<IXpsOMBrush>(xpsBrush), "QI failed");

    return S_OK;
}

HRESULT SkXPS::Layer::createXpsRadialGradient(SkShader::GradientInfo info,
                                             const SkAlpha alpha,
                                             const SkMatrix& localMatrix,
                                             IXpsOMMatrixTransform* xpsMatrix,
                                             IXpsOMBrush** xpsBrush) {
    SkTScopedComPtr<IXpsOMGradientStop> gradStop0;
    HR(createXpsGradientStop(info.fColors[0],
                             info.fColorOffsets[0],
                             &gradStop0));

    SkTScopedComPtr<IXpsOMGradientStop> gradStop1;
    HR(createXpsGradientStop(info.fColors[1],
                             info.fColorOffsets[1],
                             &gradStop1));

    //TODO: figure out how to fake better if not affine
    XPS_POINT centerPoint;
    XPS_POINT gradientOrigin;
    XPS_SIZE radiiSizes;
    if (xpsMatrix) {
        centerPoint = xps_point(info.fPoint[0]);
        gradientOrigin = xps_point(info.fPoint[0]);
        radiiSizes.width = SkScalarToFLOAT(info.fRadius[0]);
        radiiSizes.height = SkScalarToFLOAT(info.fRadius[0]);
    } else {
        centerPoint = xps_point(info.fPoint[0], localMatrix);
        gradientOrigin = xps_point(info.fPoint[0], localMatrix);

        SkScalar radius = info.fRadius[0];
        SkVector vec[2];

        vec[0].set(radius, 0);
        vec[1].set(0, radius);
        localMatrix.mapVectors(vec, 2);

        SkScalar d0 = vec[0].length();
        SkScalar d1 = vec[1].length();

        radiiSizes.width = SkScalarToFLOAT(d0);
        radiiSizes.height = SkScalarToFLOAT(d1);
    }

    SkTScopedComPtr<IXpsOMRadialGradientBrush> gradientBrush;
    HRM(fXpsFactory->CreateRadialGradientBrush(gradStop0.get(),
                                                     gradStop1.get(),
                                                     &centerPoint,
                                                     &gradientOrigin,
                                                     &radiiSizes,
                                                     &gradientBrush),
        "Could not create radial gradient brush.");
    if (xpsMatrix) {
        HRM(gradientBrush->SetTransformLocal(xpsMatrix),
            "Could not set transform on radial gradient brush.");
    }

    SkTScopedComPtr<IXpsOMGradientStopCollection> gradStopCollection;
    HRM(gradientBrush->GetGradientStops(&gradStopCollection),
        "Could not get radial gradient stop collection.");
    for (int i = 2; i < info.fColorCount; ++i) {
        SkTScopedComPtr<IXpsOMGradientStop> gradStop;
        HR(createXpsGradientStop(info.fColors[i],
                                 info.fColorOffsets[i],
                                 &gradStop));
        HRM(gradStopCollection->Append(gradStop.get()),
            "Could not add radial gradient stop.");
    }

    HRM(gradientBrush->SetSpreadMethod(xps_spread_method(info.fTileMode)),
        "Could not set spread method of radial gradient.");

    HRM(gradientBrush->SetOpacity(alpha / 255.0f),
        "Could not set opacity of radial gradient brush.");
    HRM(gradientBrush->QueryInterface<IXpsOMBrush>(xpsBrush), "QI failed.");

    return S_OK;
}

HRESULT SkXPS::Layer::createXpsBrush(const SkPaint& skPaint,
                                    IXpsOMBrush** brush,
                                    const SkMatrix* parentTransform) {
    const SkShader *shader = skPaint.getShader();
    if (nullptr == shader) {
        HR(this->createXpsSolidColorBrush(skPaint.getColor(), 0xFF, brush));
        return S_OK;
    }

    //Gradient shaders.
    SkShader::GradientInfo info;
    info.fColorCount = 0;
    info.fColors = nullptr;
    info.fColorOffsets = nullptr;
    SkShader::GradientType gradientType = shader->asAGradient(&info);

    if (SkShader::kNone_GradientType == gradientType) {
        //Nothing to see, move along.

    } else if (SkShader::kColor_GradientType == gradientType) {
        SkASSERT(1 == info.fColorCount);
        SkColor color;
        info.fColors = &color;
        shader->asAGradient(&info);
        SkAlpha alpha = skPaint.getAlpha();
        HR(this->createXpsSolidColorBrush(color, alpha, brush));
        return S_OK;

    } else {
        if (info.fColorCount == 0) {
            const SkColor color = skPaint.getColor();
            HR(this->createXpsSolidColorBrush(color, 0xFF, brush));
            return S_OK;
        }

        SkAutoTArray<SkColor> colors(info.fColorCount);
        SkAutoTArray<SkScalar> colorOffsets(info.fColorCount);
        info.fColors = colors.get();
        info.fColorOffsets = colorOffsets.get();
        shader->asAGradient(&info);

        if (1 == info.fColorCount) {
            SkColor color = info.fColors[0];
            SkAlpha alpha = skPaint.getAlpha();
            HR(this->createXpsSolidColorBrush(color, alpha, brush));
            return S_OK;
        }

        SkMatrix localMatrix = shader->getLocalMatrix();
        if (parentTransform) {
            localMatrix.preConcat(*parentTransform);
        }
        SkTScopedComPtr<IXpsOMMatrixTransform> xpsMatrixToUse;
        HR(this->createXpsTransform(localMatrix, &xpsMatrixToUse));

        if (SkShader::kLinear_GradientType == gradientType) {
            HR(this->createXpsLinearGradient(info,
                                             skPaint.getAlpha(),
                                             localMatrix,
                                             xpsMatrixToUse.get(),
                                             brush));
            return S_OK;
        }

        if (SkShader::kRadial_GradientType == gradientType) {
            HR(this->createXpsRadialGradient(info,
                                             skPaint.getAlpha(),
                                             localMatrix,
                                             xpsMatrixToUse.get(),
                                             brush));
            return S_OK;
        }

        if (SkShader::kConical_GradientType == gradientType) {
            //simple if affine and one is 0, otherwise will have to fake
        }

        if (SkShader::kSweep_GradientType == gradientType) {
            //have to fake
        }
    }

    SkBitmap outTexture;
    SkMatrix outMatrix;
    SkShader::TileMode xy[2];
    SkImage* image = shader->isAImage(&outMatrix, xy);
    if (image && image->asLegacyBitmap(&outTexture, SkImage::kRO_LegacyBitmapMode)) {
        //TODO: outMatrix??
        SkMatrix localMatrix = shader->getLocalMatrix();
        if (parentTransform) {
            localMatrix.preConcat(*parentTransform);
        }

        SkTScopedComPtr<IXpsOMTileBrush> tileBrush;
        HR(this->createXpsImageBrush(outTexture,
                                     localMatrix,
                                     xy,
                                     skPaint.getAlpha(),
                                     &tileBrush));

        HRM(tileBrush->QueryInterface<IXpsOMBrush>(brush), "QI failed.");
    } else {
        HR(this->createXpsSolidColorBrush(skPaint.getColor(), 0xFF, brush));
    }
    return S_OK;
}

static bool rect_must_be_pathed(const SkPaint& paint, const SkMatrix& matrix) {
    const bool zeroWidth = (0 == paint.getStrokeWidth());
    const bool stroke = (SkPaint::kFill_Style != paint.getStyle());

    return paint.getPathEffect() ||
           paint.getMaskFilter() ||
           paint.getRasterizer() ||
           (stroke && (
               (matrix.hasPerspective() && !zeroWidth) ||
               SkPaint::kMiter_Join != paint.getStrokeJoin() ||
               (SkPaint::kMiter_Join == paint.getStrokeJoin() &&
                paint.getStrokeMiter() < SK_ScalarSqrt2)
           ))
    ;
}

HRESULT SkXPS::Layer::createXpsRect(const SkRect& rect, BOOL stroke, BOOL fill,
                                   IXpsOMGeometryFigure** xpsRect) {
    const SkPoint points[4] = {
        { rect.fLeft, rect.fTop },
        { rect.fRight, rect.fTop },
        { rect.fRight, rect.fBottom },
        { rect.fLeft, rect.fBottom },
    };
    return this->createXpsQuad(points, stroke, fill, xpsRect);
}
HRESULT SkXPS::Layer::createXpsQuad(const SkPoint (&points)[4],
                                   BOOL stroke, BOOL fill,
                                   IXpsOMGeometryFigure** xpsQuad) {
    // Define the start point.
    XPS_POINT startPoint = xps_point(points[0]);

    // Create the figure.
    HRM(fXpsFactory->CreateGeometryFigure(&startPoint, xpsQuad),
        "Could not create quad geometry figure.");

    // Define the type of each segment.
    XPS_SEGMENT_TYPE segmentTypes[3] = {
        XPS_SEGMENT_TYPE_LINE,
        XPS_SEGMENT_TYPE_LINE,
        XPS_SEGMENT_TYPE_LINE,
    };

    // Define the x and y coordinates of each corner of the figure.
    FLOAT segmentData[6] = {
        SkScalarToFLOAT(points[1].fX), SkScalarToFLOAT(points[1].fY),
        SkScalarToFLOAT(points[2].fX), SkScalarToFLOAT(points[2].fY),
        SkScalarToFLOAT(points[3].fX), SkScalarToFLOAT(points[3].fY),
    };

    // Describe if the segments are stroked.
    BOOL segmentStrokes[3] = {
        stroke, stroke, stroke,
    };

    // Add the segment data to the figure.
    HRM((*xpsQuad)->SetSegments(
            3, 6,
            segmentTypes , segmentData, segmentStrokes),
        "Could not add segment data to quad.");

    // Set the closed and filled properties of the figure.
    HRM((*xpsQuad)->SetIsClosed(stroke), "Could not set quad close.");
    HRM((*xpsQuad)->SetIsFilled(fill), "Could not set quad fill.");

    return S_OK;
}

void SkXPS::Layer::internalDrawRect(const SkMatrix& ctm, const SkPath& clipPath,
                                   const SkRect& r,
                                   bool transformRect,
                                   const SkPaint& paint) {
    //Exit early if there is nothing to draw.
    if (clipPath.isEmpty() ||
        (paint.getAlpha() == 0 && paint.isSrcOver())) {
        return;
    }

    //Path the rect if we can't optimize it.
    if (rect_must_be_pathed(paint, ctm)) {
        SkPath tmp;
        tmp.addRect(r);
        tmp.setFillType(SkPath::kWinding_FillType);
        this->drawPath(ctm, clipPath, tmp, paint, nullptr, true);
        return;
    }

    //Create the shaded path.
    SkTScopedComPtr<IXpsOMPath> shadedPath;
    HRVM(fXpsFactory->CreatePath(&shadedPath),
         "Could not create shaded path for rect.");

    //Create the shaded geometry.
    SkTScopedComPtr<IXpsOMGeometry> shadedGeometry;
    HRVM(fXpsFactory->CreateGeometry(&shadedGeometry),
         "Could not create shaded geometry for rect.");

    //Add the geometry to the shaded path.
    HRVM(shadedPath->SetGeometryLocal(shadedGeometry.get()),
         "Could not set shaded geometry for rect.");

    //Set the brushes.
    BOOL fill = FALSE;
    BOOL stroke = FALSE;
    HRV(this->shadePath(shadedPath.get(), paint, ctm, &fill, &stroke));

    bool xpsTransformsPath = true;
    //Transform the geometry.
    if (transformRect && xpsTransformsPath) {
        SkTScopedComPtr<IXpsOMMatrixTransform> xpsTransform;
        HRV(this->createXpsTransform(ctm, &xpsTransform));
        if (xpsTransform.get()) {
            HRVM(shadedGeometry->SetTransformLocal(xpsTransform.get()),
                 "Could not set transform for rect.");
        } else {
            xpsTransformsPath = false;
        }
    }

    //Create the figure.
    SkTScopedComPtr<IXpsOMGeometryFigure> rectFigure;
    {
        SkPoint points[4] = {
            { r.fLeft, r.fTop },
            { r.fLeft, r.fBottom },
            { r.fRight, r.fBottom },
            { r.fRight, r.fTop },
        };
        if (!xpsTransformsPath && transformRect) {
            ctm.mapPoints(points, SK_ARRAY_COUNT(points));
        }
        HRV(this->createXpsQuad(points, stroke, fill, &rectFigure));
    }

    //Get the figures of the shaded geometry.
    SkTScopedComPtr<IXpsOMGeometryFigureCollection> shadedFigures;
    HRVM(shadedGeometry->GetFigures(&shadedFigures),
         "Could not get shaded figures for rect.");

    //Add the figure to the shaded geometry figures.
    HRVM(shadedFigures->Append(rectFigure.get()),
         "Could not add shaded figure for rect.");

    HRV(this->clip(shadedPath.get(), ctm, clipPath));

    //Add the shaded path to the current visuals.
    SkTScopedComPtr<IXpsOMVisualCollection> currentVisuals;
    HRVM(fCurrentXpsCanvas->GetVisuals(&currentVisuals),
         "Could not get current visuals for rect.");
    HRVM(currentVisuals->Append(shadedPath.get()),
         "Could not add rect to current visuals.");
}

static HRESULT close_figure(const SkTDArray<XPS_SEGMENT_TYPE>& segmentTypes,
                            const SkTDArray<BOOL>& segmentStrokes,
                            const SkTDArray<FLOAT>& segmentData,
                            BOOL stroke, BOOL fill,
                            IXpsOMGeometryFigure* figure,
                            IXpsOMGeometryFigureCollection* figures) {
    // Add the segment data to the figure.
    HRM(figure->SetSegments(segmentTypes.count(), segmentData.count(),
                            segmentTypes.begin() , segmentData.begin(),
                            segmentStrokes.begin()),
        "Could not set path segments.");

    // Set the closed and filled properties of the figure.
    HRM(figure->SetIsClosed(stroke), "Could not set path closed.");
    HRM(figure->SetIsFilled(fill), "Could not set path fill.");

    // Add the figure created above to this geometry.
    HRM(figures->Append(figure), "Could not add path to geometry.");
    return S_OK;
}

HRESULT SkXPS::Layer::addXpsPathGeometry(
        IXpsOMGeometryFigureCollection* xpsFigures,
        BOOL stroke, BOOL fill, const SkPath& path) {
    SkTDArray<XPS_SEGMENT_TYPE> segmentTypes;
    SkTDArray<BOOL> segmentStrokes;
    SkTDArray<FLOAT> segmentData;

    SkTScopedComPtr<IXpsOMGeometryFigure> xpsFigure;
    SkPath::Iter iter(path, true);
    SkPoint points[4];
    SkPath::Verb verb;
    while ((verb = iter.next(points)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kMove_Verb: {
                if (xpsFigure.get()) {
                    HR(close_figure(segmentTypes, segmentStrokes, segmentData,
                                    stroke, fill,
                                    xpsFigure.get() , xpsFigures));
                    xpsFigure.reset();
                    segmentTypes.rewind();
                    segmentStrokes.rewind();
                    segmentData.rewind();
                }
                // Define the start point.
                XPS_POINT startPoint = xps_point(points[0]);
                // Create the figure.
                HRM(fXpsFactory->CreateGeometryFigure(&startPoint,
                                                            &xpsFigure),
                    "Could not create path geometry figure.");
                break;
            }
            case SkPath::kLine_Verb:
                if (iter.isCloseLine()) break; //ignore the line, auto-closed
                segmentTypes.push(XPS_SEGMENT_TYPE_LINE);
                segmentStrokes.push(stroke);
                segmentData.push(SkScalarToFLOAT(points[1].fX));
                segmentData.push(SkScalarToFLOAT(points[1].fY));
                break;
            case SkPath::kQuad_Verb:
                segmentTypes.push(XPS_SEGMENT_TYPE_QUADRATIC_BEZIER);
                segmentStrokes.push(stroke);
                segmentData.push(SkScalarToFLOAT(points[1].fX));
                segmentData.push(SkScalarToFLOAT(points[1].fY));
                segmentData.push(SkScalarToFLOAT(points[2].fX));
                segmentData.push(SkScalarToFLOAT(points[2].fY));
                break;
            case SkPath::kCubic_Verb:
                segmentTypes.push(XPS_SEGMENT_TYPE_BEZIER);
                segmentStrokes.push(stroke);
                segmentData.push(SkScalarToFLOAT(points[1].fX));
                segmentData.push(SkScalarToFLOAT(points[1].fY));
                segmentData.push(SkScalarToFLOAT(points[2].fX));
                segmentData.push(SkScalarToFLOAT(points[2].fY));
                segmentData.push(SkScalarToFLOAT(points[3].fX));
                segmentData.push(SkScalarToFLOAT(points[3].fY));
                break;
            case SkPath::kConic_Verb: {
                const SkScalar tol = SK_Scalar1 / 4;
                SkAutoConicToQuads converter;
                const SkPoint* quads =
                    converter.computeQuads(points, iter.conicWeight(), tol);
                for (int i = 0; i < converter.countQuads(); ++i) {
                    segmentTypes.push(XPS_SEGMENT_TYPE_QUADRATIC_BEZIER);
                    segmentStrokes.push(stroke);
                    segmentData.push(SkScalarToFLOAT(quads[2 * i + 1].fX));
                    segmentData.push(SkScalarToFLOAT(quads[2 * i + 1].fY));
                    segmentData.push(SkScalarToFLOAT(quads[2 * i + 2].fX));
                    segmentData.push(SkScalarToFLOAT(quads[2 * i + 2].fY));
                }
                break;
            }
            case SkPath::kClose_Verb:
                // we ignore these, and just get the whole segment from
                // the corresponding line/quad/cubic verbs
                break;
            default:
                SkDEBUGFAIL("unexpected verb");
                break;
        }
    }
    if (xpsFigure.get()) {
        HR(close_figure(segmentTypes, segmentStrokes, segmentData,
                        stroke, fill,
                        xpsFigure.get(), xpsFigures));
    }
    return S_OK;
}

void SkXPS::Layer::convertToPpm(const SkMaskFilter* filter,
                               SkMatrix* matrix,
                               SkVector* ppuScale,
                               const SkIRect& clip, SkIRect* clipIRect) {
    //This action is in unit space, but the ppm is specified in physical space.
    ppuScale->set(fCurrentPixelsPerMeter.fX / fCurrentUnitsPerMeter.fX,
                  fCurrentPixelsPerMeter.fY / fCurrentUnitsPerMeter.fY);

    matrix->postScale(ppuScale->fX, ppuScale->fY);

    const SkIRect& irect = clip;
    SkRect clipRect = SkRect::MakeLTRB(
        SkScalarMul(SkIntToScalar(irect.fLeft), ppuScale->fX),
        SkScalarMul(SkIntToScalar(irect.fTop), ppuScale->fY),
        SkScalarMul(SkIntToScalar(irect.fRight), ppuScale->fX),
        SkScalarMul(SkIntToScalar(irect.fBottom), ppuScale->fY));
    clipRect.roundOut(clipIRect);
}

HRESULT SkXPS::Layer::applyMask(const SkMatrix& ctm, const SkPath& clipPath,
                               const SkMask& mask,
                               const SkVector& ppuScale,
                               IXpsOMPath* shadedPath) {
    //Get the geometry object.
    SkTScopedComPtr<IXpsOMGeometry> shadedGeometry;
    HRM(shadedPath->GetGeometry(&shadedGeometry),
        "Could not get mask shaded geometry.");

    //Get the figures from the geometry.
    SkTScopedComPtr<IXpsOMGeometryFigureCollection> shadedFigures;
    HRM(shadedGeometry->GetFigures(&shadedFigures),
        "Could not get mask shaded figures.");

    SkMatrix m;
    m.reset();
    m.setTranslate(SkIntToScalar(mask.fBounds.fLeft),
                   SkIntToScalar(mask.fBounds.fTop));
    m.postScale(SkScalarInvert(ppuScale.fX), SkScalarInvert(ppuScale.fY));

    SkShader::TileMode xy[2];
    xy[0] = (SkShader::TileMode)3;
    xy[1] = (SkShader::TileMode)3;

    SkBitmap bm;
    bm.installMaskPixels(mask);

    SkTScopedComPtr<IXpsOMTileBrush> maskBrush;
    HR(this->createXpsImageBrush(bm, m, xy, 0xFF, &maskBrush));
    HRM(shadedPath->SetOpacityMaskBrushLocal(maskBrush.get()),
        "Could not set mask.");

    const SkRect universeRect = SkRect::MakeLTRB(0, 0,
        fCurrentCanvasSize.fWidth, fCurrentCanvasSize.fHeight);
    SkTScopedComPtr<IXpsOMGeometryFigure> shadedFigure;
    HRM(this->createXpsRect(universeRect, FALSE, TRUE, &shadedFigure),
        "Could not create mask shaded figure.");
    HRM(shadedFigures->Append(shadedFigure.get()),
        "Could not add mask shaded figure.");

    HR(this->clip(shadedPath, ctm, clipPath));

    //Add the path to the active visual collection.
    SkTScopedComPtr<IXpsOMVisualCollection> currentVisuals;
    HRM(fCurrentXpsCanvas->GetVisuals(&currentVisuals),
        "Could not get mask current visuals.");
    HRM(currentVisuals->Append(shadedPath),
        "Could not add masked shaded path to current visuals.");

    return S_OK;
}

HRESULT SkXPS::Layer::shadePath(IXpsOMPath* shadedPath,
                               const SkPaint& shaderPaint,
                               const SkMatrix& matrix,
                               BOOL* fill, BOOL* stroke) {
    *fill = FALSE;
    *stroke = FALSE;

    const SkPaint::Style style = shaderPaint.getStyle();
    const bool hasFill = SkPaint::kFill_Style == style
                      || SkPaint::kStrokeAndFill_Style == style;
    const bool hasStroke = SkPaint::kStroke_Style == style
                        || SkPaint::kStrokeAndFill_Style == style;

    //TODO(bungeman): use dictionaries and lookups.
    if (hasFill) {
        *fill = TRUE;
        SkTScopedComPtr<IXpsOMBrush> fillBrush;
        HR(this->createXpsBrush(shaderPaint, &fillBrush, &matrix));
        HRM(shadedPath->SetFillBrushLocal(fillBrush.get()),
            "Could not set fill for shaded path.");
    }

    if (hasStroke) {
        *stroke = TRUE;
        SkTScopedComPtr<IXpsOMBrush> strokeBrush;
        HR(this->createXpsBrush(shaderPaint, &strokeBrush, &matrix));
        HRM(shadedPath->SetStrokeBrushLocal(strokeBrush.get()),
            "Could not set stroke brush for shaded path.");
        HRM(shadedPath->SetStrokeThickness(
                SkScalarToFLOAT(shaderPaint.getStrokeWidth())),
            "Could not set shaded path stroke thickness.");

        if (0 == shaderPaint.getStrokeWidth()) {
            //XPS hair width is a hack. (XPS Spec 11.6.12).
            SkTScopedComPtr<IXpsOMDashCollection> dashes;
            HRM(shadedPath->GetStrokeDashes(&dashes),
                "Could not set dashes for shaded path.");
            XPS_DASH dash;
            dash.length = 1.0;
            dash.gap = 0.0;
            HRM(dashes->Append(&dash), "Could not add dashes to shaded path.");
            HRM(shadedPath->SetStrokeDashOffset(-2.0),
                "Could not set dash offset for shaded path.");
        }
    }
    return S_OK;
}

void SkXPS::Layer::drawPath(const SkMatrix& ctm, const SkPath& clipPath,
                           const SkPath& platonicPath,
                           const SkPaint& origPaint,
                           const SkMatrix* prePathMatrix,
                           bool pathIsMutable) {
    SkTCopyOnFirstWrite<SkPaint> paint(origPaint);

    // nothing to draw
    if (clipPath.isEmpty() ||
        (paint->getAlpha() == 0 && paint->isSrcOver())) {
        return;
    }

    SkPath modifiedPath;
    const bool paintHasPathEffect = paint->getPathEffect()
                                 || paint->getStyle() != SkPaint::kFill_Style;

    //Apply pre-path matrix [Platonic-path -> Skeletal-path].
    SkMatrix matrix = ctm;
    SkPath* skeletalPath = const_cast<SkPath*>(&platonicPath);
    if (prePathMatrix) {
        if (paintHasPathEffect || paint->getRasterizer()) {
            if (!pathIsMutable) {
                skeletalPath = &modifiedPath;
                pathIsMutable = true;
            }
            platonicPath.transform(*prePathMatrix, skeletalPath);
        } else {
            matrix.preConcat(*prePathMatrix);
        }
    }

    //Apply path effect [Skeletal-path -> Fillable-path].
    SkPath* fillablePath = skeletalPath;
    if (paintHasPathEffect) {
        if (!pathIsMutable) {
            fillablePath = &modifiedPath;
            pathIsMutable = true;
        }
        bool fill = paint->getFillPath(*skeletalPath, fillablePath);

        SkPaint* writablePaint = paint.writable();
        writablePaint->setPathEffect(nullptr);
        if (fill) {
            writablePaint->setStyle(SkPaint::kFill_Style);
        } else {
            writablePaint->setStyle(SkPaint::kStroke_Style);
            writablePaint->setStrokeWidth(0);
        }
    }

    //Create the shaded path. This will be the path which is painted.
    SkTScopedComPtr<IXpsOMPath> shadedPath;
    HRVM(fXpsFactory->CreatePath(&shadedPath),
         "Could not create shaded path for path.");

    //Create the geometry for the shaded path.
    SkTScopedComPtr<IXpsOMGeometry> shadedGeometry;
    HRVM(fXpsFactory->CreateGeometry(&shadedGeometry),
         "Could not create shaded geometry for path.");

    //Add the geometry to the shaded path.
    HRVM(shadedPath->SetGeometryLocal(shadedGeometry.get()),
         "Could not add the shaded geometry to shaded path.");

    SkRasterizer* rasterizer = paint->getRasterizer();
    SkMaskFilter* filter = paint->getMaskFilter();

    //Determine if we will draw or shade and mask.
    if (rasterizer || filter) {
        if (paint->getStyle() != SkPaint::kFill_Style) {
            paint.writable()->setStyle(SkPaint::kFill_Style);
        }
    }

    //Set the brushes.
    BOOL fill;
    BOOL stroke;
    HRV(this->shadePath(shadedPath.get(),
                        *paint,
                        ctm,
                        &fill,
                        &stroke));

    //Rasterizer
    if (rasterizer) {
        SkIRect clipIRect;
        SkVector ppuScale;
        this->convertToPpm(filter,
                           &matrix,
                           &ppuScale,
                           clipPath.getBounds().roundOut(),
                           &clipIRect);

        SkMask* mask = nullptr;

        //[Fillable-path -> Mask]
        SkMask rasteredMask;
        if (rasterizer->rasterize(
                *fillablePath,
                matrix,
                &clipIRect,
                filter,  //just to compute how much to draw.
                &rasteredMask,
                SkMask::kComputeBoundsAndRenderImage_CreateMode)) {

            SkAutoMaskFreeImage rasteredAmi(rasteredMask.fImage);
            mask = &rasteredMask;

            //[Mask -> Mask]
            SkMask filteredMask;
            if (filter && filter->filterMask(&filteredMask, *mask, ctm, nullptr)) {
                mask = &filteredMask;
            }
            SkAutoMaskFreeImage filteredAmi(filteredMask.fImage);

            //Draw mask.
            HRV(this->applyMask(ctm, clipPath, *mask, ppuScale, shadedPath.get()));
        }
        return;
    }

    //Mask filter
    if (filter) {
        SkIRect clipIRect;
        SkVector ppuScale;
        this->convertToPpm(filter,
                           &matrix,
                           &ppuScale,
                           clipPath.getBounds().roundOut(),
                           &clipIRect);

        //[Fillable-path -> Pixel-path]
        SkPath* pixelPath = pathIsMutable ? fillablePath : &modifiedPath;
        fillablePath->transform(matrix, pixelPath);

        SkMask* mask = nullptr;

        SkASSERT(SkPaint::kFill_Style == paint->getStyle() ||
                 (SkPaint::kStroke_Style == paint->getStyle() && 0 == paint->getStrokeWidth()));
        SkStrokeRec::InitStyle style = (SkPaint::kFill_Style == paint->getStyle())
                                            ? SkStrokeRec::kFill_InitStyle
                                            : SkStrokeRec::kHairline_InitStyle;
        //[Pixel-path -> Mask]
        SkMask rasteredMask;
        if (SkDraw::DrawToMask(
                        *pixelPath,
                        &clipIRect,
                        filter,  //just to compute how much to draw.
                        &matrix,
                        &rasteredMask,
                        SkMask::kComputeBoundsAndRenderImage_CreateMode,
                        style)) {

            SkAutoMaskFreeImage rasteredAmi(rasteredMask.fImage);
            mask = &rasteredMask;

            //[Mask -> Mask]
            SkMask filteredMask;
            if (filter->filterMask(&filteredMask, rasteredMask, matrix, nullptr)) {
                mask = &filteredMask;
            }
            SkAutoMaskFreeImage filteredAmi(filteredMask.fImage);

            //Draw mask.
            HRV(this->applyMask(ctm, clipPath, *mask, ppuScale, shadedPath.get()));
        }
        return;
    }

    //Get the figures from the shaded geometry.
    SkTScopedComPtr<IXpsOMGeometryFigureCollection> shadedFigures;
    HRVM(shadedGeometry->GetFigures(&shadedFigures),
         "Could not get shaded figures for shaded path.");

    bool xpsTransformsPath = true;

    //Set the fill rule.
    SkPath* xpsCompatiblePath = fillablePath;
    XPS_FILL_RULE xpsFillRule;
    switch (fillablePath->getFillType()) {
        case SkPath::kWinding_FillType:
            xpsFillRule = XPS_FILL_RULE_NONZERO;
            break;
        case SkPath::kEvenOdd_FillType:
            xpsFillRule = XPS_FILL_RULE_EVENODD;
            break;
        case SkPath::kInverseWinding_FillType: {
            //[Fillable-path (inverse winding) -> XPS-path (inverse even odd)]
            if (!pathIsMutable) {
                xpsCompatiblePath = &modifiedPath;
                pathIsMutable = true;
            }
            if (!Simplify(*fillablePath, xpsCompatiblePath)) {
                SkDEBUGF(("Could not simplify inverse winding path."));
                return;
            }
        }
        // The xpsCompatiblePath is noW inverse even odd, so fall through.
        case SkPath::kInverseEvenOdd_FillType: {
            const SkRect universe = SkRect::MakeLTRB(
                0, 0,
                fCurrentCanvasSize.fWidth,
                fCurrentCanvasSize.fHeight);
            SkTScopedComPtr<IXpsOMGeometryFigure> addOneFigure;
            HRV(this->createXpsRect(universe, FALSE, TRUE, &addOneFigure));
            HRVM(shadedFigures->Append(addOneFigure.get()),
                 "Could not add even-odd flip figure to shaded path.");
            xpsTransformsPath = false;
            xpsFillRule = XPS_FILL_RULE_EVENODD;
            break;
        }
        default:
            SkDEBUGFAIL("Unknown SkPath::FillType.");
    }
    HRVM(shadedGeometry->SetFillRule(xpsFillRule),
         "Could not set fill rule for shaded path.");

    //Create the XPS transform, if possible.
    if (xpsTransformsPath) {
        SkTScopedComPtr<IXpsOMMatrixTransform> xpsTransform;
        HRV(this->createXpsTransform(matrix, &xpsTransform));

        if (xpsTransform.get()) {
            HRVM(shadedGeometry->SetTransformLocal(xpsTransform.get()),
                 "Could not set transform on shaded path.");
        } else {
            xpsTransformsPath = false;
        }
    }

    SkPath* devicePath = xpsCompatiblePath;
    if (!xpsTransformsPath) {
        //[Fillable-path -> Layer-path]
        devicePath = pathIsMutable ? xpsCompatiblePath : &modifiedPath;
        xpsCompatiblePath->transform(matrix, devicePath);
    }
    HRV(this->addXpsPathGeometry(shadedFigures.get(),
                                 stroke, fill, *devicePath));

    HRV(this->clip(shadedPath.get(), ctm, clipPath));

    //Add the path to the active visual collection.
    SkTScopedComPtr<IXpsOMVisualCollection> currentVisuals;
    HRVM(fCurrentXpsCanvas->GetVisuals(&currentVisuals),
         "Could not get current visuals for shaded path.");
    HRVM(currentVisuals->Append(shadedPath.get()),
         "Could not add shaded path to current visuals.");
}

HRESULT SkXPS::Layer::clip(IXpsOMVisual* xpsVisual, const SkMatrix& ctm, const SkPath& clipPath) {
    SkPath path(clipPath);
    path.transform(ctm);
    return this->clipToPath(xpsVisual, path, XPS_FILL_RULE_EVENODD);
}

HRESULT SkXPS::Layer::clipToPath(IXpsOMVisual* xpsVisual,
                                const SkPath& clipPath,
                                XPS_FILL_RULE fillRule) {
    //Create the geometry.
    SkTScopedComPtr<IXpsOMGeometry> clipGeometry;
    HRM(fXpsFactory->CreateGeometry(&clipGeometry),
        "Could not create clip geometry.");

    //Get the figure collection of the geometry.
    SkTScopedComPtr<IXpsOMGeometryFigureCollection> clipFigures;
    HRM(clipGeometry->GetFigures(&clipFigures),
        "Could not get the clip figures.");

    //Create the figures into the geometry.
    HR(this->addXpsPathGeometry(
        clipFigures.get(),
        FALSE, TRUE, clipPath));

    HRM(clipGeometry->SetFillRule(fillRule),
        "Could not set fill rule.");
    HRM(xpsVisual->SetClipGeometryLocal(clipGeometry.get()),
        "Could not set clip geometry.");

    return S_OK;
}

void SkXPS::Layer::drawBitmap(const SkMatrix& ctm, const SkPath& clipPath, const SkBitmap& bitmap,
                             const SkMatrix& matrix, const SkPaint& paint) {
    if (clipPath.isEmpty()) {
        return;
    }

    SkIRect srcRect;
    srcRect.set(0, 0, bitmap.width(), bitmap.height());

    //Create the new shaded path.
    SkTScopedComPtr<IXpsOMPath> shadedPath;
    HRVM(fXpsFactory->CreatePath(&shadedPath),
         "Could not create path for bitmap.");

    //Create the shaded geometry.
    SkTScopedComPtr<IXpsOMGeometry> shadedGeometry;
    HRVM(fXpsFactory->CreateGeometry(&shadedGeometry),
         "Could not create geometry for bitmap.");

    //Add the shaded geometry to the shaded path.
    HRVM(shadedPath->SetGeometryLocal(shadedGeometry.get()),
         "Could not set the geometry for bitmap.");

    //Get the shaded figures from the shaded geometry.
    SkTScopedComPtr<IXpsOMGeometryFigureCollection> shadedFigures;
    HRVM(shadedGeometry->GetFigures(&shadedFigures),
         "Could not get the figures for bitmap.");

    SkMatrix transform = matrix;
    transform.postConcat(ctm);

    SkTScopedComPtr<IXpsOMMatrixTransform> xpsTransform;
    HRV(this->createXpsTransform(transform, &xpsTransform));
    if (xpsTransform.get()) {
        HRVM(shadedGeometry->SetTransformLocal(xpsTransform.get()),
             "Could not set transform for bitmap.");
    } else {
        //TODO: perspective that bitmap!
    }

    SkTScopedComPtr<IXpsOMGeometryFigure> rectFigure;
    if (xpsTransform.get()) {
        const SkShader::TileMode xy[2] = {
            SkShader::kClamp_TileMode,
            SkShader::kClamp_TileMode,
        };
        SkTScopedComPtr<IXpsOMTileBrush> xpsImageBrush;
        HRV(this->createXpsImageBrush(bitmap,
                                      transform,
                                      xy,
                                      paint.getAlpha(),
                                      &xpsImageBrush));
        HRVM(shadedPath->SetFillBrushLocal(xpsImageBrush.get()),
             "Could not set bitmap brush.");

        const SkRect bitmapRect = SkRect::MakeLTRB(0, 0,
            SkIntToScalar(srcRect.width()), SkIntToScalar(srcRect.height()));
        HRV(this->createXpsRect(bitmapRect, FALSE, TRUE, &rectFigure));
    }
    HRVM(shadedFigures->Append(rectFigure.get()),
         "Could not add bitmap figure.");

    //Get the current visual collection and add the shaded path to it.
    SkTScopedComPtr<IXpsOMVisualCollection> currentVisuals;
    HRVM(fCurrentXpsCanvas->GetVisuals(&currentVisuals),
         "Could not get current visuals for bitmap");
    HRVM(currentVisuals->Append(shadedPath.get()),
         "Could not add bitmap to current visuals.");

    HRV(this->clip(shadedPath.get(), ctm, clipPath));
}


HRESULT SkXPS::Layer::CreateTypefaceUse(const SkPaint& paint,
                                       TypefaceUse** typefaceUse) {
    SkAutoResolveDefaultTypeface typeface(paint.getTypeface());

    //Check cache.
    const SkFontID typefaceID = typeface->uniqueID();
    SkTArray<SkXPS::TypefaceUse, true>& typefaces = fXps->fTypefaces;
    if (!typefaces.empty()) {
        for (SkXPS::TypefaceUse& tfUse : typefaces) {
            if (tfUse.typefaceId == typefaceID) {
                 *typefaceUse = &tfUse;
                 return S_OK;
            }
        }
    }


    //TODO: create glyph only fonts
    //and let the host deal with what kind of font we're looking at.
    XPS_FONT_EMBEDDING embedding = XPS_FONT_EMBEDDING_RESTRICTED;

    SkTScopedComPtr<IStream> fontStream;
    int ttcIndex;
    SkStream* fontData = typeface->openStream(&ttcIndex);
    //TODO: cannot handle FON fonts.
    HRM(SkIStream::CreateFromSkStream(fontData, true, &fontStream),
        "Could not create font stream.");

    const size_t size =
        SK_ARRAY_COUNT(L"/Resources/Fonts/" L_GUID_ID L".odttf");
    wchar_t buffer[size];
    wchar_t id[GUID_ID_LEN];
    fXps->createId(id, GUID_ID_LEN);
    swprintf_s(buffer, size, L"/Resources/Fonts/%s.odttf", id);

    SkTScopedComPtr<IOpcPartUri> partUri;
    HRM(fXpsFactory->CreatePartUri(buffer, &partUri),
        "Could not create font resource part uri.");

    SkTScopedComPtr<IXpsOMFontResource> xpsFontResource;
    HRM(fXpsFactory->CreateFontResource(fontStream.get(),
                                              embedding,
                                              partUri.get(),
                                              FALSE,
                                              &xpsFontResource),
        "Could not create font resource.");

    //TODO: change openStream to return -1 for non-ttc, get rid of this.
    uint8_t* data = (uint8_t*)fontData->getMemoryBase();
    bool isTTC = (data &&
                  fontData->getLength() >= sizeof(SkTTCFHeader) &&
                  ((SkTTCFHeader*)data)->ttcTag == SkTTCFHeader::TAG);

    SkAutoGlyphCache agc(paint, nullptr, &SkMatrix::I());
    SkGlyphCache* glyphCache = agc.getCache();
    unsigned int glyphCount = glyphCache->getGlyphCount();

    TypefaceUse& newTypefaceUse = typefaces.emplace_back(glyphCache->getGlyphCount());
    newTypefaceUse.typefaceId = typefaceID;
    newTypefaceUse.ttcIndex = isTTC ? ttcIndex : -1;
    newTypefaceUse.fontData = fontData;
    newTypefaceUse.xpsFont.reset(xpsFontResource.release());

    *typefaceUse = &newTypefaceUse;
    return S_OK;
}

HRESULT SkXPS::Layer::AddGlyphs(const SkMatrix& ctm, const SkPath& clipPath,
                               IXpsOMCanvas* canvas,
                               TypefaceUse* font,
                               LPCWSTR text,
                               XPS_GLYPH_INDEX* xpsGlyphs,
                               UINT32 xpsGlyphsLen,
                               XPS_POINT *origin,
                               FLOAT fontSize,
                               XPS_STYLE_SIMULATION sims,
                               const SkMatrix& transform,
                               const SkPaint& paint) {
    SkTScopedComPtr<IXpsOMGlyphs> glyphs;
    HRM(fXpsFactory->CreateGlyphs(font->xpsFont.get(), &glyphs), "Could not create glyphs.");
    HRM(glyphs->SetFontFaceIndex(font->ttcIndex), "Could not set glyph font face index.");

    //XPS uses affine transformations for everything...
    //...except positioning text.
    bool useCanvasForClip;
    if ((transform.getType() & ~SkMatrix::kTranslate_Mask) == 0) {
        origin->x += SkScalarToFLOAT(transform.getTranslateX());
        origin->y += SkScalarToFLOAT(transform.getTranslateY());
        useCanvasForClip = false;
    } else {
        SkTScopedComPtr<IXpsOMMatrixTransform> xpsMatrixToUse;
        HR(this->createXpsTransform(transform, &xpsMatrixToUse));
        if (xpsMatrixToUse.get()) {
            HRM(glyphs->SetTransformLocal(xpsMatrixToUse.get()),
                "Could not set transform matrix.");
            useCanvasForClip = true;
        } else {
            SkDEBUGFAIL("Attempt to add glyphs in perspective.");
            useCanvasForClip = false;
        }
    }

    SkTScopedComPtr<IXpsOMGlyphsEditor> glyphsEditor;
    HRM(glyphs->GetGlyphsEditor(&glyphsEditor), "Could not get glyph editor.");

    if (text) {
        HRM(glyphsEditor->SetUnicodeString(text),
            "Could not set unicode string.");
    }

    if (xpsGlyphs) {
        HRM(glyphsEditor->SetGlyphIndices(xpsGlyphsLen, xpsGlyphs),
            "Could not set glyphs.");
    }

    HRM(glyphsEditor->ApplyEdits(), "Could not apply glyph edits.");

    SkTScopedComPtr<IXpsOMBrush> xpsFillBrush;
    HR(this->createXpsBrush(
            paint,
            &xpsFillBrush,
            useCanvasForClip ? nullptr : &transform));

    HRM(glyphs->SetFillBrushLocal(xpsFillBrush.get()),
        "Could not set fill brush.");

    HRM(glyphs->SetOrigin(origin), "Could not set glyph origin.");

    HRM(glyphs->SetFontRenderingEmSize(fontSize),
        "Could not set font size.");

    HRM(glyphs->SetStyleSimulations(sims),
        "Could not set style simulations.");

    SkTScopedComPtr<IXpsOMVisualCollection> visuals;
    HRM(canvas->GetVisuals(&visuals), "Could not get glyph canvas visuals.");

    if (!useCanvasForClip) {
        HR(this->clip(glyphs.get(), ctm, clipPath));
        HRM(visuals->Append(glyphs.get()), "Could not add glyphs to canvas.");
    } else {
        SkTScopedComPtr<IXpsOMCanvas> glyphCanvas;
        HRM(fXpsFactory->CreateCanvas(&glyphCanvas),
            "Could not create glyph canvas.");

        SkTScopedComPtr<IXpsOMVisualCollection> glyphCanvasVisuals;
        HRM(glyphCanvas->GetVisuals(&glyphCanvasVisuals),
            "Could not get glyph visuals collection.");

        HRM(glyphCanvasVisuals->Append(glyphs.get()),
            "Could not add glyphs to page.");
        HR(this->clip(glyphCanvas.get(), ctm, clipPath));

        HRM(visuals->Append(glyphCanvas.get()),
            "Could not add glyph canvas to page.");
    }

    return S_OK;
}

static int num_glyph_guess(SkPaint::TextEncoding encoding, const void* text, size_t byteLength) {
    switch (encoding) {
    case SkPaint::kUTF8_TextEncoding:
        return SkUTF8_CountUnichars(static_cast<const char *>(text), byteLength);
    case SkPaint::kUTF16_TextEncoding:
        return SkUTF16_CountUnichars(static_cast<const uint16_t *>(text), SkToInt(byteLength));
    case SkPaint::kGlyphID_TextEncoding:
        return SkToInt(byteLength / 2);
    default:
        SK_ABORT("Invalid Text Encoding");
    }
    return 0;
}

static bool text_must_be_pathed(const SkPaint& paint, const SkMatrix& matrix) {
    const SkPaint::Style style = paint.getStyle();
    return matrix.hasPerspective()
        || SkPaint::kStroke_Style == style
        || SkPaint::kStrokeAndFill_Style == style
        || paint.getMaskFilter()
        || paint.getRasterizer()
    ;
}

typedef SkTDArray<XPS_GLYPH_INDEX> GlyphRun;

class ProcessOneGlyph {
public:
    ProcessOneGlyph(FLOAT centemPerUnit, SkBitSet* glyphUse, GlyphRun* xpsGlyphs)
        : fCentemPerUnit(centemPerUnit)
        , fGlyphUse(glyphUse)
        , fXpsGlyphs(xpsGlyphs) { }

    void operator()(const SkGlyph& glyph, SkPoint position, SkPoint) {
        SkASSERT(glyph.fWidth > 0 && glyph.fHeight > 0);

        SkScalar x = position.fX;
        SkScalar y = position.fY;

        XPS_GLYPH_INDEX* xpsGlyph = fXpsGlyphs->append();
        uint16_t glyphID = glyph.getGlyphID();
        fGlyphUse->set(glyphID);
        xpsGlyph->index = glyphID;
        if (1 == fXpsGlyphs->count()) {
            xpsGlyph->advanceWidth = 0.0f;
            xpsGlyph->horizontalOffset = SkScalarToFloat(x) * fCentemPerUnit;
            xpsGlyph->verticalOffset = SkScalarToFloat(y) * -fCentemPerUnit;
        }
        else {
            const XPS_GLYPH_INDEX& first = (*fXpsGlyphs)[0];
            xpsGlyph->advanceWidth = 0.0f;
            xpsGlyph->horizontalOffset = (SkScalarToFloat(x) * fCentemPerUnit)
                - first.horizontalOffset;
            xpsGlyph->verticalOffset = (SkScalarToFloat(y) * -fCentemPerUnit)
                - first.verticalOffset;
        }
    }

private:
    /** [in] Advance width and offsets for glyphs measured in
    hundredths of the font em size (XPS Spec 5.1.3). */
    const FLOAT fCentemPerUnit;
    /** [in,out] The accumulated glyphs used in the current typeface. */
    SkBitSet* const fGlyphUse;
    /** [out] The glyphs to draw. */
    GlyphRun* const fXpsGlyphs;
};

void SkXPS::Layer::drawText(const SkMatrix& ctm, const SkPath& clipPath,
                           const void* text, size_t byteLen,
                           SkScalar x, SkScalar y,
                           const SkPaint& paint) {
    if (byteLen < 1) return;

    if (text_must_be_pathed(paint, ctm)) {
        SkPath path;
        paint.getTextPath(text, byteLen, x, y, &path);
        this->drawPath(ctm, clipPath, path, paint, nullptr, true);
        //TODO: add automation "text"
        return;
    }

    TypefaceUse* typeface;
    HRV(CreateTypefaceUse(paint, &typeface));

    const SkMatrix& matrix = SkMatrix::I();

    SkAutoGlyphCache    autoCache(paint, nullptr, &matrix);
    SkGlyphCache*       cache = autoCache.getCache();

    // Advance width and offsets for glyphs measured in hundredths of the font em size
    // (XPS Spec 5.1.3).
    FLOAT centemPerUnit = 100.0f / SkScalarToFLOAT(paint.getTextSize());
    GlyphRun xpsGlyphs;
    xpsGlyphs.setReserve(num_glyph_guess(paint.getTextEncoding(),
        static_cast<const char*>(text), byteLen));

    ProcessOneGlyph processOneGlyph(centemPerUnit, &typeface->glyphsUsed, &xpsGlyphs);

    SkFindAndPlaceGlyph::ProcessText(
        paint.getTextEncoding(), static_cast<const char*>(text), byteLen,
        SkPoint{ x, y }, matrix, paint.getTextAlign(), cache, processOneGlyph);

    if (xpsGlyphs.count() == 0) {
        return;
    }

    XPS_POINT origin = {
        xpsGlyphs[0].horizontalOffset / centemPerUnit,
        xpsGlyphs[0].verticalOffset / -centemPerUnit,
    };
    xpsGlyphs[0].horizontalOffset = 0.0f;
    xpsGlyphs[0].verticalOffset = 0.0f;

    HRV(AddGlyphs(ctm, clipPath,
                  fCurrentXpsCanvas.get(),
                  typeface,
                  nullptr,
                  xpsGlyphs.begin(), xpsGlyphs.count(),
                  &origin,
                  SkScalarToFLOAT(paint.getTextSize()),
                  XPS_STYLE_SIMULATION_NONE,
                  ctm,
                  paint));
}

void SkXPS::Layer::drawPosText(const SkMatrix& ctm, const SkPath& clipPath,
                              const void* text, size_t byteLen,
                              const SkScalar pos[], int scalarsPerPos,
                              const SkPoint& offset, const SkPaint& paint) {
    if (byteLen < 1) return;

    if (text_must_be_pathed(paint, ctm)) {
        SkPath path;
        //TODO: make this work, Draw currently does not handle as well.
        //paint.getTextPath(text, byteLength, x, y, &path);
        //this->drawPath(ctm, clipPath, path, paint, nullptr, true);
        //TODO: add automation "text"
        return;
    }

    TypefaceUse* typeface;
    HRV(CreateTypefaceUse(paint, &typeface));

    const SkMatrix& matrix = SkMatrix::I();

    SkAutoGlyphCache    autoCache(paint, nullptr, &matrix);
    SkGlyphCache*       cache = autoCache.getCache();

    // Advance width and offsets for glyphs measured in hundredths of the font em size
    // (XPS Spec 5.1.3).
    FLOAT centemPerUnit = 100.0f / SkScalarToFLOAT(paint.getTextSize());
    GlyphRun xpsGlyphs;
    xpsGlyphs.setReserve(num_glyph_guess(paint.getTextEncoding(),
        static_cast<const char*>(text), byteLen));

    ProcessOneGlyph processOneGlyph(centemPerUnit, &typeface->glyphsUsed, &xpsGlyphs);

    SkFindAndPlaceGlyph::ProcessPosText(
        paint.getTextEncoding(), static_cast<const char*>(text), byteLen,
        offset, matrix, pos, scalarsPerPos, paint.getTextAlign(), cache, processOneGlyph);

    if (xpsGlyphs.count() == 0) {
        return;
    }

    XPS_POINT origin = {
        xpsGlyphs[0].horizontalOffset / centemPerUnit,
        xpsGlyphs[0].verticalOffset / -centemPerUnit,
    };
    xpsGlyphs[0].horizontalOffset = 0.0f;
    xpsGlyphs[0].verticalOffset = 0.0f;

    HRV(AddGlyphs(ctm, clipPath,
                  fCurrentXpsCanvas.get(),
                  typeface,
                  nullptr,
                  xpsGlyphs.begin(), xpsGlyphs.count(),
                  &origin,
                  SkScalarToFLOAT(paint.getTextSize()),
                  XPS_STYLE_SIMULATION_NONE,
                  ctm,
                  paint));
}

void SkXPS::drawText(const SkMatrix& ctm, const SkPath& clip,
                     const void* text, size_t textBytes,
                     SkTextBlob::GlyphPositioning positioning,
                     SkPoint origin, const SkScalar* pos, const SkPaint& paint) {
    int scalarsPerPos = 2;
    switch (positioning) {
        case SkTextBlob::kDefault_Positioning:
            fLayers.back().drawText(ctm, clip, text, textBytes, origin.x(), origin.y(), paint);
            return;
        case SkTextBlob::kHorizontal_Positioning:
            scalarsPerPos = 1;
            // fallthrough
        case SkTextBlob::kFull_Positioning:
            fLayers.back().drawPosText(ctm, clip, text, textBytes, pos,
                                       scalarsPerPos, origin, paint);
    }
}

void SkXPS::drawPath(const SkMatrix& ctm, const SkPath& clip,
                     const SkPath& path, const SkPaint& paint) {
    fLayers.back().drawPath(ctm, clip, path, paint, nullptr, false);
}

void SkXPS::drawBitmap(const SkMatrix& ctm, const SkPath& clip,
                       const SkBitmap& bm, SkPoint pos, const SkPaint* paint) {
    SkPaint p = paint ? *paint : SkPaint();
    fLayers.back().drawBitmap(ctm, clip, bm, SkMatrix::MakeTrans(pos.x(), pos.y()), p);
}

/**
   Formats a GUID and places it into buffer.
   buffer should have space for at least GUID_ID_LEN wide characters.
   The string will always be wchar null terminated.
   XXXXXXXXsXXXXsXXXXsXXXXsXXXXXXXXXXXX0
   @return -1 if there was an error, > 0 if success.
 */
void SkXPS::createId(wchar_t* buffer, size_t bufferSize, wchar_t sep) {
    SkASSERT(bufferSize >= GUID_ID_LEN);
    GUID guid = {};
    #ifdef SK_XPS_USE_DETERMINISTIC_IDS
        guid.Data1 = (decltype(GUID::Data1))fNextId++;
        // The following make this a valid Type4 UUID.
        guid.Data3 = 0x4000;
        guid.Data4[0] = 0x80;
    #else
        HR_DCHECK(CoCreateGuid(&guid));
    #endif
    SkAssertResult(-1 != swprintf_s(buffer,
                                    bufferSize,
                                    L"%08lX%c%04X%c%04X%c%02X%02X%c%02X%02X%02X%02X%02X%02X",
                                    guid.Data1,
                                    sep,
                                    guid.Data2,
                                    sep,
                                    guid.Data3,
                                    sep,
                                    guid.Data4[0],
                                    guid.Data4[1],
                                    sep,
                                    guid.Data4[2],
                                    guid.Data4[3],
                                    guid.Data4[4],
                                    guid.Data4[5],
                                    guid.Data4[6],
                                    guid.Data4[7]));
}

#endif  // SK_BUILD_FOR_WIN
