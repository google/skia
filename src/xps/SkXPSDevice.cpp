/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#if defined(SK_BUILD_FOR_WIN)

#include "src/base/SkLeanWindows.h"

#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#include <ObjBase.h>
#include <XpsObjectModel.h>
#include <T2EmbApi.h>
#include <FontSub.h>
#include <limits>

#include "include/core/SkColor.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPathEffect.h"
#include "include/core/SkPathUtils.h"
#include "include/core/SkPoint.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkStream.h"
#include "include/core/SkVertices.h"
#include "include/encode/SkPngEncoder.h"
#include "include/pathops/SkPathOps.h"
#include "include/private/base/SkTDArray.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkEndian.h"
#include "src/base/SkTLazy.h"
#include "src/base/SkUtils.h"
#include "src/core/SkDraw.h"
#include "src/core/SkFontPriv.h"
#include "src/core/SkGeometry.h"
#include "src/core/SkImagePriv.h"
#include "src/core/SkMaskFilterBase.h"
#include "src/core/SkRasterClip.h"
#include "src/core/SkStrikeCache.h"
#include "src/image/SkImage_Base.h"
#include "src/sfnt/SkSFNTHeader.h"
#include "src/sfnt/SkTTCFHeader.h"
#include "src/shaders/SkColorShader.h"
#include "src/shaders/SkShaderBase.h"
#include "src/text/GlyphRun.h"
#include "src/utils/SkClipStackUtils.h"
#include "src/utils/win/SkHRESULT.h"
#include "src/utils/win/SkIStream.h"
#include "src/utils/win/SkTScopedComPtr.h"
#include "src/xps/SkXPSDevice.h"

using namespace skia_private;

//Windows defines a FLOAT type,
//make it clear when converting a scalar that this is what is wanted.
#define SkScalarToFLOAT(n) SkScalarToFloat(n)

//Placeholder representation of a GUID from createId.
#define L_GUID_ID L"XXXXXXXXsXXXXsXXXXsXXXXsXXXXXXXXXXXX"
//Length of GUID representation from createId, including nullptr terminator.
#define GUID_ID_LEN std::size(L_GUID_ID)

/**
   Formats a GUID and places it into buffer.
   buffer should have space for at least GUID_ID_LEN wide characters.
   The string will always be wchar null terminated.
   XXXXXXXXsXXXXsXXXXsXXXXsXXXXXXXXXXXX0
   @return -1 if there was an error, > 0 if success.
 */
static int format_guid(const GUID& guid,
                       wchar_t* buffer, size_t bufferSize,
                       wchar_t sep = '-') {
    SkASSERT(bufferSize >= GUID_ID_LEN);
    return swprintf_s(buffer,
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
                      guid.Data4[7]);
}

HRESULT SkXPSDevice::createId(wchar_t* buffer, size_t bufferSize, wchar_t sep) {
    GUID guid = {};
#ifdef SK_XPS_USE_DETERMINISTIC_IDS
    guid.Data1 = fNextId++;
    // The following make this a valid Type4 UUID.
    guid.Data3 = 0x4000;
    guid.Data4[0] = 0x80;
#else
    HRM(CoCreateGuid(&guid), "Could not create GUID for id.");
#endif

    if (format_guid(guid, buffer, bufferSize, sep) == -1) {
        HRM(E_UNEXPECTED, "Could not format GUID into id.");
    }

    return S_OK;
}

SkXPSDevice::SkXPSDevice(SkISize s)
    : SkClipStackDevice(SkImageInfo::MakeUnknown(s.width(), s.height()),
                        SkSurfaceProps(0, kUnknown_SkPixelGeometry))
    , fCurrentPage(0), fTopTypefaces(&fTypefaces) {}

SkXPSDevice::~SkXPSDevice() {}

bool SkXPSDevice::beginPortfolio(SkWStream* outputStream, IXpsOMObjectFactory* factory) {
    SkASSERT(factory);
    fXpsFactory.reset(SkRefComPtr(factory));
    HRB(SkWIStream::CreateFromSkWStream(outputStream, &this->fOutputStream));
    return true;
}

bool SkXPSDevice::beginSheet(
        const SkVector& unitsPerMeter,
        const SkVector& pixelsPerMeter,
        const SkSize& trimSize,
        const SkRect* mediaBox,
        const SkRect* bleedBox,
        const SkRect* artBox,
        const SkRect* cropBox) {
    ++this->fCurrentPage;

    //For simplicity, just write everything out in geometry units,
    //then have a base canvas do the scale to physical units.
    this->fCurrentCanvasSize = trimSize;
    this->fCurrentUnitsPerMeter = unitsPerMeter;
    this->fCurrentPixelsPerMeter = pixelsPerMeter;
    return this->createCanvasForLayer();
}

bool SkXPSDevice::createCanvasForLayer() {
    SkASSERT(fXpsFactory);
    fCurrentXpsCanvas.reset();
    HRB(fXpsFactory->CreateCanvas(&fCurrentXpsCanvas));
    return true;
}

template <typename T> static constexpr size_t sk_digits_in() {
    return static_cast<size_t>(std::numeric_limits<T>::digits10 + 1);
}

HRESULT SkXPSDevice::createXpsThumbnail(IXpsOMPage* page,
                                        const unsigned int pageNum,
                                        IXpsOMImageResource** image) {
    SkTScopedComPtr<IXpsOMThumbnailGenerator> thumbnailGenerator;
    HRM(CoCreateInstance(
            CLSID_XpsOMThumbnailGenerator,
            nullptr,
            CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(&thumbnailGenerator)),
        "Could not create thumbnail generator.");

    SkTScopedComPtr<IOpcPartUri> partUri;
    constexpr size_t size = std::max(
            std::size(L"/Documents/1/Metadata/.png") + sk_digits_in<decltype(pageNum)>(),
            std::size(L"/Metadata/" L_GUID_ID L".png"));
    wchar_t buffer[size];
    if (pageNum > 0) {
        swprintf_s(buffer, size, L"/Documents/1/Metadata/%u.png", pageNum);
    } else {
        wchar_t id[GUID_ID_LEN];
        HR(this->createId(id, GUID_ID_LEN));
        swprintf_s(buffer, size, L"/Metadata/%s.png", id);
    }
    HRM(this->fXpsFactory->CreatePartUri(buffer, &partUri),
        "Could not create thumbnail part uri.");

    HRM(thumbnailGenerator->GenerateThumbnail(page,
                                              XPS_IMAGE_TYPE_PNG,
                                              XPS_THUMBNAIL_SIZE_LARGE,
                                              partUri.get(),
                                              image),
        "Could not generate thumbnail.");

    return S_OK;
}

HRESULT SkXPSDevice::createXpsPage(const XPS_SIZE& pageSize,
                                   IXpsOMPage** page) {
    constexpr size_t size =
        std::size(L"/Documents/1/Pages/.fpage")
        + sk_digits_in<decltype(fCurrentPage)>();
    wchar_t buffer[size];
    swprintf_s(buffer, size, L"/Documents/1/Pages/%u.fpage",
                             this->fCurrentPage);
    SkTScopedComPtr<IOpcPartUri> partUri;
    HRM(this->fXpsFactory->CreatePartUri(buffer, &partUri),
        "Could not create page part uri.");

    //If the language is unknown, use "und" (XPS Spec 2.3.5.1).
    HRM(this->fXpsFactory->CreatePage(&pageSize,
                                      L"und",
                                      partUri.get(),
                                      page),
        "Could not create page.");

    return S_OK;
}

HRESULT SkXPSDevice::initXpsDocumentWriter(IXpsOMImageResource* image) {
    //Create package writer.
    {
        SkTScopedComPtr<IOpcPartUri> partUri;
        HRM(this->fXpsFactory->CreatePartUri(L"/FixedDocumentSequence.fdseq",
                                             &partUri),
            "Could not create document sequence part uri.");
        HRM(this->fXpsFactory->CreatePackageWriterOnStream(
                this->fOutputStream.get(),
                TRUE,
                XPS_INTERLEAVING_OFF, //XPS_INTERLEAVING_ON,
                partUri.get(),
                nullptr,
                image,
                nullptr,
                nullptr,
                &this->fPackageWriter),
            "Could not create package writer.");
    }

    //Begin the lone document.
    {
        SkTScopedComPtr<IOpcPartUri> partUri;
        HRM(this->fXpsFactory->CreatePartUri(
                L"/Documents/1/FixedDocument.fdoc",
                &partUri),
            "Could not create fixed document part uri.");
        HRM(this->fPackageWriter->StartNewDocument(partUri.get(),
                                                   nullptr,
                                                   nullptr,
                                                   nullptr,
                                                   nullptr),
            "Could not start document.");
    }

    return S_OK;
}

bool SkXPSDevice::endSheet() {
    //XPS is fixed at 96dpi (XPS Spec 11.1).
    static const float xpsDPI = 96.0f;
    static const float inchesPerMeter = 10000.0f / 254.0f;
    static const float targetUnitsPerMeter = xpsDPI * inchesPerMeter;
    const float scaleX = targetUnitsPerMeter
                       / SkScalarToFLOAT(this->fCurrentUnitsPerMeter.fX);
    const float scaleY = targetUnitsPerMeter
                       / SkScalarToFLOAT(this->fCurrentUnitsPerMeter.fY);

    //Create the scale canvas.
    SkTScopedComPtr<IXpsOMCanvas> scaleCanvas;
    HRBM(this->fXpsFactory->CreateCanvas(&scaleCanvas),
         "Could not create scale canvas.");
    SkTScopedComPtr<IXpsOMVisualCollection> scaleCanvasVisuals;
    HRBM(scaleCanvas->GetVisuals(&scaleCanvasVisuals),
         "Could not get scale canvas visuals.");

    SkTScopedComPtr<IXpsOMMatrixTransform> geomToPhys;
    XPS_MATRIX rawGeomToPhys = { scaleX, 0, 0, scaleY, 0, 0, };
    HRBM(this->fXpsFactory->CreateMatrixTransform(&rawGeomToPhys, &geomToPhys),
         "Could not create geometry to physical transform.");
    HRBM(scaleCanvas->SetTransformLocal(geomToPhys.get()),
         "Could not set transform on scale canvas.");

    //Add the content canvas to the scale canvas.
    HRBM(scaleCanvasVisuals->Append(this->fCurrentXpsCanvas.get()),
         "Could not add base canvas to scale canvas.");

    //Create the page.
    XPS_SIZE pageSize = {
        SkScalarToFLOAT(this->fCurrentCanvasSize.width()) * scaleX,
        SkScalarToFLOAT(this->fCurrentCanvasSize.height()) * scaleY,
    };
    SkTScopedComPtr<IXpsOMPage> page;
    HRB(this->createXpsPage(pageSize, &page));

    SkTScopedComPtr<IXpsOMVisualCollection> pageVisuals;
    HRBM(page->GetVisuals(&pageVisuals), "Could not get page visuals.");

    //Add the scale canvas to the page.
    HRBM(pageVisuals->Append(scaleCanvas.get()),
         "Could not add scale canvas to page.");

    //Create the package writer if it hasn't been created yet.
    if (nullptr == this->fPackageWriter.get()) {
        SkTScopedComPtr<IXpsOMImageResource> image;
        //Ignore return, thumbnail is completely optional.
        this->createXpsThumbnail(page.get(), 0, &image);

        HRB(this->initXpsDocumentWriter(image.get()));
    }

    HRBM(this->fPackageWriter->AddPage(page.get(),
                                       &pageSize,
                                       nullptr,
                                       nullptr,
                                       nullptr,
                                       nullptr),
         "Could not write the page.");
    this->fCurrentXpsCanvas.reset();

    return true;
}

static HRESULT subset_typeface(const SkXPSDevice::TypefaceUse& current) {
    //The CreateFontPackage API is only supported on desktop, not in UWP
    #if defined(SK_WINUWP)
    return E_NOTIMPL;
    #else
    //CreateFontPackage wants unsigned short.
    //Microsoft, Y U NO stdint.h?
    std::vector<unsigned short> keepList;
    current.glyphsUsed.forEachSetIndex([&keepList](size_t v) {
            keepList.push_back((unsigned short)v);
    });

    int ttcCount = (current.ttcIndex + 1);

    //The following are declared with the types required by CreateFontPackage.
    unsigned char *fontPackageBufferRaw = nullptr;
    unsigned long fontPackageBufferSize;
    unsigned long bytesWritten;
    unsigned long result = CreateFontPackage(
        (const unsigned char *) current.fontData->getMemoryBase(),
        (unsigned long) current.fontData->getLength(),
        &fontPackageBufferRaw,
        &fontPackageBufferSize,
        &bytesWritten,
        TTFCFP_FLAGS_SUBSET | TTFCFP_FLAGS_GLYPHLIST | (ttcCount > 0 ? TTFCFP_FLAGS_TTC : 0),
        current.ttcIndex,
        TTFCFP_SUBSET,
        0,
        0,
        0,
        keepList.data(),
        SkTo<unsigned short>(keepList.size()),
        sk_malloc_throw,
        sk_realloc_throw,
        sk_free,
        nullptr);
    AutoTMalloc<unsigned char> fontPackageBuffer(fontPackageBufferRaw);
    if (result != NO_ERROR) {
        SkDEBUGF("CreateFontPackage Error %lu", result);
        return E_UNEXPECTED;
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
    SkIStream::CreateFromSkStream(std::move(newStream), &newIStream);

    XPS_FONT_EMBEDDING embedding;
    HRM(current.xpsFont->GetEmbeddingOption(&embedding),
        "Could not get embedding option from font.");

    SkTScopedComPtr<IOpcPartUri> partUri;
    HRM(current.xpsFont->GetPartName(&partUri),
        "Could not get part uri from font.");

    HRM(current.xpsFont->SetContent(
            newIStream.get(),
            embedding,
            partUri.get()),
        "Could not set new stream for subsetted font.");

    return S_OK;
    #endif //SK_WINUWP
}

bool SkXPSDevice::endPortfolio() {
    //Subset fonts
    for (const TypefaceUse& current : *this->fTopTypefaces) {
        //Ignore return for now, if it didn't subset, let it be.
        subset_typeface(current);
    }

    if (this->fPackageWriter) {
        HRBM(this->fPackageWriter->Close(), "Could not close writer.");
    }

    return true;
}

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

static XPS_SPREAD_METHOD xps_spread_method(SkTileMode tileMode) {
    switch (tileMode) {
    case SkTileMode::kClamp:
        return XPS_SPREAD_METHOD_PAD;
    case SkTileMode::kRepeat:
        return XPS_SPREAD_METHOD_REPEAT;
    case SkTileMode::kMirror:
        return XPS_SPREAD_METHOD_REFLECT;
    case SkTileMode::kDecal:
        // TODO: fake
        return XPS_SPREAD_METHOD_PAD;
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
        stop.fX = (end.fX - start.fX) * stopOffsets[i];
        stop.fY = (end.fY - start.fY) * stopOffsets[i];

        SkPoint stopTransformed;
        transform.mapXY(stop.fX, stop.fY, &stopTransformed);

        //Manhattan distance between transformed start and stop.
        SkScalar startToStop = (stopTransformed.fX - startTransformed.fX)
                             + (stopTransformed.fY - startTransformed.fY);
        //Percentage along transformed line.
        stopOffsets[i] = startToStop / startToEnd;
    }
}

HRESULT SkXPSDevice::createXpsTransform(const SkMatrix& matrix,
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
    HRM(this->fXpsFactory->CreateMatrixTransform(&rawXpsMatrix, xpsTransform),
        "Could not create transform.");

    return S_OK;
}

HRESULT SkXPSDevice::createPath(IXpsOMGeometryFigure* figure,
                                IXpsOMVisualCollection* visuals,
                                IXpsOMPath** path) {
    SkTScopedComPtr<IXpsOMGeometry> geometry;
    HRM(this->fXpsFactory->CreateGeometry(&geometry),
        "Could not create geometry.");

    SkTScopedComPtr<IXpsOMGeometryFigureCollection> figureCollection;
    HRM(geometry->GetFigures(&figureCollection), "Could not get figures.");
    HRM(figureCollection->Append(figure), "Could not add figure.");

    HRM(this->fXpsFactory->CreatePath(path), "Could not create path.");
    HRM((*path)->SetGeometryLocal(geometry.get()), "Could not set geometry");

    HRM(visuals->Append(*path), "Could not add path to visuals.");
    return S_OK;
}

HRESULT SkXPSDevice::createXpsSolidColorBrush(const SkColor skColor,
                                              const SkAlpha alpha,
                                              IXpsOMBrush** xpsBrush) {
    XPS_COLOR xpsColor = xps_color(skColor);
    SkTScopedComPtr<IXpsOMSolidColorBrush> solidBrush;
    HRM(this->fXpsFactory->CreateSolidColorBrush(&xpsColor, nullptr, &solidBrush),
        "Could not create solid color brush.");
    HRM(solidBrush->SetOpacity(alpha / 255.0f), "Could not set opacity.");
    HRM(solidBrush->QueryInterface<IXpsOMBrush>(xpsBrush), "QI Fail.");
    return S_OK;
}

HRESULT SkXPSDevice::sideOfClamp(const SkRect& areaToFill,
                                 const XPS_RECT& imageViewBox,
                                 IXpsOMImageResource* image,
                                 IXpsOMVisualCollection* visuals) {
    SkTScopedComPtr<IXpsOMGeometryFigure> areaToFillFigure;
    HR(this->createXpsRect(areaToFill, FALSE, TRUE, &areaToFillFigure));

    SkTScopedComPtr<IXpsOMPath> areaToFillPath;
    HR(this->createPath(areaToFillFigure.get(), visuals, &areaToFillPath));

    SkTScopedComPtr<IXpsOMImageBrush> areaToFillBrush;
    HRM(this->fXpsFactory->CreateImageBrush(image,
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

HRESULT SkXPSDevice::cornerOfClamp(const SkRect& areaToFill,
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
static XPS_TILE_MODE gSkToXpsTileMode[kSkTileModeCount+1]
                                     [kSkTileModeCount+1] = {
               //Clamp  //Repeat //Mirror //None
    /*Clamp */ {XTM_N,  XTM_T,   XTM_Y,   XTM_N},
    /*Repeat*/ {XTM_T,  XTM_T,   XTM_Y,   XTM_N},
    /*Mirror*/ {XTM_X,  XTM_X,   XTM_XY,  XTM_X},
    /*None  */ {XTM_N,  XTM_N,   XTM_Y,   XTM_N},
};

static XPS_TILE_MODE SkToXpsTileMode(SkTileMode tmx, SkTileMode tmy) {
    return gSkToXpsTileMode[(unsigned)tmx][(unsigned)tmy];
}

HRESULT SkXPSDevice::createXpsImageBrush(
        const SkPixmap& bitmap,
        const SkMatrix& localMatrix,
        const SkTileMode (&xy)[2],
        const SkAlpha alpha,
        IXpsOMTileBrush** xpsBrush) {
    SkDynamicMemoryWStream write;
    if (!SkPngEncoder::Encode(&write, bitmap, {})) {
        HRM(E_FAIL, "Unable to encode bitmap as png.");
    }
    SkTScopedComPtr<IStream> read;
    HRM(SkIStream::CreateFromSkStream(write.detachAsStream(), &read),
        "Could not create stream from png data.");

    const size_t size =
        std::size(L"/Documents/1/Resources/Images/" L_GUID_ID L".png");
    wchar_t buffer[size];
    wchar_t id[GUID_ID_LEN];
    HR(this->createId(id, GUID_ID_LEN));
    swprintf_s(buffer, size, L"/Documents/1/Resources/Images/%s.png", id);

    SkTScopedComPtr<IOpcPartUri> imagePartUri;
    HRM(this->fXpsFactory->CreatePartUri(buffer, &imagePartUri),
        "Could not create image part uri.");

    SkTScopedComPtr<IXpsOMImageResource> imageResource;
    HRM(this->fXpsFactory->CreateImageResource(
            read.get(),
            XPS_IMAGE_TYPE_PNG,
            imagePartUri.get(),
            &imageResource),
        "Could not create image resource.");

    XPS_RECT bitmapRect = {
        0.0, 0.0,
        static_cast<FLOAT>(bitmap.width()), static_cast<FLOAT>(bitmap.height())
    };
    SkTScopedComPtr<IXpsOMImageBrush> xpsImageBrush;
    HRM(this->fXpsFactory->CreateImageBrush(imageResource.get(),
                                            &bitmapRect, &bitmapRect,
                                            &xpsImageBrush),
        "Could not create image brush.");

    if (SkTileMode::kClamp != xy[0] &&
        SkTileMode::kClamp != xy[1]) {

        HRM(xpsImageBrush->SetTileMode(SkToXpsTileMode(xy[0], xy[1])),
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
        HRM(this->fXpsFactory->CreateCanvas(&brushCanvas),
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
        if (SkTileMode::kClamp == xy[0]) {
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
        if (SkTileMode::kClamp == xy[1]) {
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
        if (SkTileMode::kClamp == xy[0] &&
            SkTileMode::kClamp == xy[1]) {

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
        if (SkTileMode::kClamp == xy[0] &&
            SkTileMode::kClamp == xy[1]) {

            bound.x = BIG_F / -2;
            bound.y = BIG_F / -2;
            bound.width = BIG_F;
            bound.height = BIG_F;
        } else if (SkTileMode::kClamp == xy[0]) {
            bound.x = BIG_F / -2;
            bound.y = 0.0f;
            bound.width = BIG_F;
            bound.height = static_cast<FLOAT>(bitmap.height());
        } else if (SkTileMode::kClamp == xy[1]) {
            bound.x = 0;
            bound.y = BIG_F / -2;
            bound.width = static_cast<FLOAT>(bitmap.width());
            bound.height = BIG_F;
        }
        SkTScopedComPtr<IXpsOMVisualBrush> clampBrush;
        HRM(this->fXpsFactory->CreateVisualBrush(&bound, &bound, &clampBrush),
            "Could not create visual brush for image brush.");
        HRM(clampBrush->SetVisualLocal(brushCanvas.get()),
            "Could not set canvas on visual brush for image brush.");
        HRM(clampBrush->SetTileMode(SkToXpsTileMode(xy[0], xy[1])),
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

HRESULT SkXPSDevice::createXpsGradientStop(const SkColor skColor,
                                           const SkScalar offset,
                                           IXpsOMGradientStop** xpsGradStop) {
    XPS_COLOR gradStopXpsColor = xps_color(skColor);
    HRM(this->fXpsFactory->CreateGradientStop(&gradStopXpsColor,
                                              nullptr,
                                              SkScalarToFLOAT(offset),
                                              xpsGradStop),
        "Could not create gradient stop.");
    return S_OK;
}

HRESULT SkXPSDevice::createXpsLinearGradient(SkShaderBase::GradientInfo info,
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
    HRM(this->fXpsFactory->CreateLinearGradientBrush(gradStop0.get(),
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

    HRM(gradientBrush->SetSpreadMethod(xps_spread_method((SkTileMode)info.fTileMode)),
        "Could not set spread method of linear gradient.");

    HRM(gradientBrush->SetOpacity(alpha / 255.0f),
        "Could not set opacity of linear gradient brush.");
    HRM(gradientBrush->QueryInterface<IXpsOMBrush>(xpsBrush), "QI failed");

    return S_OK;
}

HRESULT SkXPSDevice::createXpsRadialGradient(SkShaderBase::GradientInfo info,
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
    HRM(this->fXpsFactory->CreateRadialGradientBrush(gradStop0.get(),
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

    HRM(gradientBrush->SetSpreadMethod(xps_spread_method((SkTileMode)info.fTileMode)),
        "Could not set spread method of radial gradient.");

    HRM(gradientBrush->SetOpacity(alpha / 255.0f),
        "Could not set opacity of radial gradient brush.");
    HRM(gradientBrush->QueryInterface<IXpsOMBrush>(xpsBrush), "QI failed.");

    return S_OK;
}

HRESULT SkXPSDevice::createXpsBrush(const SkPaint& skPaint,
                                    IXpsOMBrush** brush,
                                    const SkMatrix* parentTransform) {
    const SkShader *shader = skPaint.getShader();
    if (nullptr == shader) {
        HR(this->createXpsSolidColorBrush(skPaint.getColor(), 0xFF, brush));
        return S_OK;
    }

    //Gradient shaders.
    auto shaderBase = as_SB(shader);

    if (shaderBase->type() == SkShaderBase::ShaderType::kColor) {
        auto colorShader = static_cast<const SkColorShader*>(shader);
        SkAlpha alpha = skPaint.getAlpha();
        HR(this->createXpsSolidColorBrush(colorShader->color(), alpha, brush));
        return S_OK;
    } else if (shaderBase->type() == SkShaderBase::ShaderType::kGradientBase) {
        SkShaderBase::GradientInfo info;
        SkShaderBase::GradientType gradientType = shaderBase->asGradient(&info);
        if (info.fColorCount == 0) {
            const SkColor color = skPaint.getColor();
            HR(this->createXpsSolidColorBrush(color, 0xFF, brush));
            return S_OK;
        }

        SkMatrix localMatrix;
        AutoTArray<SkColor> colors(info.fColorCount);
        AutoTArray<SkScalar> colorOffsets(info.fColorCount);
        info.fColors = colors.get();
        info.fColorOffsets = colorOffsets.get();
        shaderBase->asGradient(&info, &localMatrix);

        if (1 == info.fColorCount) {
            SkColor color = info.fColors[0];
            SkAlpha alpha = skPaint.getAlpha();
            HR(this->createXpsSolidColorBrush(color, alpha, brush));
            return S_OK;
        }

        if (parentTransform) {
            localMatrix.preConcat(*parentTransform);
        }
        SkTScopedComPtr<IXpsOMMatrixTransform> xpsMatrixToUse;
        HR(this->createXpsTransform(localMatrix, &xpsMatrixToUse));

        if (gradientType == SkShaderBase::GradientType::kLinear) {
            HR(this->createXpsLinearGradient(info,
                                             skPaint.getAlpha(),
                                             localMatrix,
                                             xpsMatrixToUse.get(),
                                             brush));
            return S_OK;
        }

        if (gradientType == SkShaderBase::GradientType::kRadial) {
            HR(this->createXpsRadialGradient(info,
                                             skPaint.getAlpha(),
                                             localMatrix,
                                             xpsMatrixToUse.get(),
                                             brush));
            return S_OK;
        }

        if (gradientType == SkShaderBase::GradientType::kConical) {
            //simple if affine and one is 0, otherwise will have to fake
        }

        if (gradientType == SkShaderBase::GradientType::kSweep) {
            //have to fake
        }
    }

    SkBitmap outTexture;
    SkMatrix outMatrix;
    SkTileMode xy[2];
    SkImage* image = shader->isAImage(&outMatrix, xy);
    if (image->asLegacyBitmap(&outTexture)) {
        if (parentTransform) {
            outMatrix.postConcat(*parentTransform);
        }

        SkTScopedComPtr<IXpsOMTileBrush> tileBrush;
        HR(this->createXpsImageBrush(outTexture.pixmap(), outMatrix, xy, skPaint.getAlpha(),
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
           (stroke && (
               (matrix.hasPerspective() && !zeroWidth) ||
               SkPaint::kMiter_Join != paint.getStrokeJoin() ||
               (SkPaint::kMiter_Join == paint.getStrokeJoin() &&
                paint.getStrokeMiter() < SK_ScalarSqrt2)
           ))
    ;
}

HRESULT SkXPSDevice::createXpsRect(const SkRect& rect, BOOL stroke, BOOL fill,
                                   IXpsOMGeometryFigure** xpsRect) {
    const SkPoint points[4] = {
        { rect.fLeft, rect.fTop },
        { rect.fRight, rect.fTop },
        { rect.fRight, rect.fBottom },
        { rect.fLeft, rect.fBottom },
    };
    return this->createXpsQuad(points, stroke, fill, xpsRect);
}
HRESULT SkXPSDevice::createXpsQuad(const SkPoint (&points)[4],
                                   BOOL stroke, BOOL fill,
                                   IXpsOMGeometryFigure** xpsQuad) {
    // Define the start point.
    XPS_POINT startPoint = xps_point(points[0]);

    // Create the figure.
    HRM(this->fXpsFactory->CreateGeometryFigure(&startPoint, xpsQuad),
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

void SkXPSDevice::drawPoints(SkCanvas::PointMode mode,
                             size_t count, const SkPoint points[],
                             const SkPaint& paint) {
    //TODO
}

void SkXPSDevice::drawVertices(const SkVertices*, sk_sp<SkBlender>, const SkPaint&, bool) {
    //TODO
}

void SkXPSDevice::drawMesh(const SkMesh&, sk_sp<SkBlender>, const SkPaint&) {
    // TODO
}

void SkXPSDevice::drawPaint(const SkPaint& origPaint) {
    const SkRect r = SkRect::MakeSize(this->fCurrentCanvasSize);

    //If trying to paint with a stroke, ignore that and fill.
    SkPaint* fillPaint = const_cast<SkPaint*>(&origPaint);
    SkTCopyOnFirstWrite<SkPaint> paint(origPaint);
    if (paint->getStyle() != SkPaint::kFill_Style) {
        paint.writable()->setStyle(SkPaint::kFill_Style);
    }

    this->internalDrawRect(r, false, *fillPaint);
}

void SkXPSDevice::drawRect(const SkRect& r,
                           const SkPaint& paint) {
    this->internalDrawRect(r, true, paint);
}

void SkXPSDevice::drawRRect(const SkRRect& rr,
                            const SkPaint& paint) {
    SkPath path;
    path.addRRect(rr);
    this->drawPath(path, paint, true);
}

void SkXPSDevice::internalDrawRect(const SkRect& r,
                                   bool transformRect,
                                   const SkPaint& paint) {
    //Exit early if there is nothing to draw.
    if (this->isClipEmpty() || (paint.getAlpha() == 0 && paint.isSrcOver())) {
        return;
    }

    //Path the rect if we can't optimize it.
    if (rect_must_be_pathed(paint, this->localToDevice())) {
        SkPath tmp;
        tmp.addRect(r);
        tmp.setFillType(SkPathFillType::kWinding);
        this->drawPath(tmp, paint, true);
        return;
    }

    //Create the shaded path.
    SkTScopedComPtr<IXpsOMPath> shadedPath;
    HRVM(this->fXpsFactory->CreatePath(&shadedPath),
         "Could not create shaded path for rect.");

    //Create the shaded geometry.
    SkTScopedComPtr<IXpsOMGeometry> shadedGeometry;
    HRVM(this->fXpsFactory->CreateGeometry(&shadedGeometry),
         "Could not create shaded geometry for rect.");

    //Add the geometry to the shaded path.
    HRVM(shadedPath->SetGeometryLocal(shadedGeometry.get()),
         "Could not set shaded geometry for rect.");

    //Set the brushes.
    BOOL fill = FALSE;
    BOOL stroke = FALSE;
    HRV(this->shadePath(shadedPath.get(), paint, this->localToDevice(), &fill, &stroke));

    bool xpsTransformsPath = true;
    //Transform the geometry.
    if (transformRect && xpsTransformsPath) {
        SkTScopedComPtr<IXpsOMMatrixTransform> xpsTransform;
        HRV(this->createXpsTransform(this->localToDevice(), &xpsTransform));
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
            this->localToDevice().mapPoints(points, std::size(points));
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

    HRV(this->clip(shadedPath.get()));

    //Add the shaded path to the current visuals.
    SkTScopedComPtr<IXpsOMVisualCollection> currentVisuals;
    HRVM(this->fCurrentXpsCanvas->GetVisuals(&currentVisuals),
         "Could not get current visuals for rect.");
    HRVM(currentVisuals->Append(shadedPath.get()),
         "Could not add rect to current visuals.");
}

static HRESULT close_figure(const SkTDArray<XPS_SEGMENT_TYPE>& segmentTypes,
                            const SkTDArray<FLOAT>& segmentData,
                            const SkTDArray<BOOL>& segmentStrokes,
                            BOOL stroke, BOOL fill,
                            IXpsOMGeometryFigure* figure,
                            IXpsOMGeometryFigureCollection* figures) {
    // Either all are empty or none are empty.
    SkASSERT(( segmentTypes.empty() &&  segmentData.empty() &&  segmentStrokes.empty()) ||
             (!segmentTypes.empty() && !segmentData.empty() && !segmentStrokes.empty()));

    // SkTDArray::begin() may return nullptr when the segment is empty,
    // but IXpsOMGeometryFigure::SetSegments returns E_POINTER if any of the pointers are nullptr
    // even if the counts are all 0.
    if (!segmentTypes.empty() && !segmentData.empty() && !segmentStrokes.empty()) {
        // Add the segment data to the figure.
        HRM(figure->SetSegments(segmentTypes.size(), segmentData.size(),
                                segmentTypes.begin(), segmentData.begin(), segmentStrokes.begin()),
            "Could not set path segments.");
    }

    // Set the closed and filled properties of the figure.
    HRM(figure->SetIsClosed(stroke), "Could not set path closed.");
    HRM(figure->SetIsFilled(fill), "Could not set path fill.");

    // Add the figure created above to this geometry.
    HRM(figures->Append(figure), "Could not add path to geometry.");
    return S_OK;
}

HRESULT SkXPSDevice::addXpsPathGeometry(
        IXpsOMGeometryFigureCollection* xpsFigures,
        BOOL stroke, BOOL fill, const SkPath& path) {
    SkTDArray<XPS_SEGMENT_TYPE> segmentTypes;
    SkTDArray<FLOAT> segmentData;
    SkTDArray<BOOL> segmentStrokes;

    SkTScopedComPtr<IXpsOMGeometryFigure> xpsFigure;
    SkPath::Iter iter(path, true);
    SkPoint points[4];
    SkPath::Verb verb;
    while ((verb = iter.next(points)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kMove_Verb: {
                if (xpsFigure.get()) {
                    HR(close_figure(segmentTypes, segmentData, segmentStrokes,
                                    stroke, fill,
                                    xpsFigure.get() , xpsFigures));
                    segmentTypes.clear();
                    segmentData.clear();
                    segmentStrokes.clear();
                    xpsFigure.reset();
                }
                // Define the start point.
                XPS_POINT startPoint = xps_point(points[0]);
                // Create the figure.
                HRM(this->fXpsFactory->CreateGeometryFigure(&startPoint,
                                                            &xpsFigure),
                    "Could not create path geometry figure.");
                break;
            }
            case SkPath::kLine_Verb:
                if (iter.isCloseLine()) break; //ignore the line, auto-closed
                segmentTypes.push_back(XPS_SEGMENT_TYPE_LINE);
                segmentData.push_back(SkScalarToFLOAT(points[1].fX));
                segmentData.push_back(SkScalarToFLOAT(points[1].fY));
                segmentStrokes.push_back(stroke);
                break;
            case SkPath::kQuad_Verb:
                segmentTypes.push_back(XPS_SEGMENT_TYPE_QUADRATIC_BEZIER);
                segmentData.push_back(SkScalarToFLOAT(points[1].fX));
                segmentData.push_back(SkScalarToFLOAT(points[1].fY));
                segmentData.push_back(SkScalarToFLOAT(points[2].fX));
                segmentData.push_back(SkScalarToFLOAT(points[2].fY));
                segmentStrokes.push_back(stroke);
                break;
            case SkPath::kCubic_Verb:
                segmentTypes.push_back(XPS_SEGMENT_TYPE_BEZIER);
                segmentData.push_back(SkScalarToFLOAT(points[1].fX));
                segmentData.push_back(SkScalarToFLOAT(points[1].fY));
                segmentData.push_back(SkScalarToFLOAT(points[2].fX));
                segmentData.push_back(SkScalarToFLOAT(points[2].fY));
                segmentData.push_back(SkScalarToFLOAT(points[3].fX));
                segmentData.push_back(SkScalarToFLOAT(points[3].fY));
                segmentStrokes.push_back(stroke);
                break;
            case SkPath::kConic_Verb: {
                const SkScalar tol = SK_Scalar1 / 4;
                SkAutoConicToQuads converter;
                const SkPoint* quads =
                    converter.computeQuads(points, iter.conicWeight(), tol);
                for (int i = 0; i < converter.countQuads(); ++i) {
                    segmentTypes.push_back(XPS_SEGMENT_TYPE_QUADRATIC_BEZIER);
                    segmentData.push_back(SkScalarToFLOAT(quads[2 * i + 1].fX));
                    segmentData.push_back(SkScalarToFLOAT(quads[2 * i + 1].fY));
                    segmentData.push_back(SkScalarToFLOAT(quads[2 * i + 2].fX));
                    segmentData.push_back(SkScalarToFLOAT(quads[2 * i + 2].fY));
                    segmentStrokes.push_back(stroke);
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
        HR(close_figure(segmentTypes, segmentData, segmentStrokes,
                        stroke, fill,
                        xpsFigure.get(), xpsFigures));
    }
    return S_OK;
}

void SkXPSDevice::convertToPpm(const SkMaskFilter* filter,
                               SkMatrix* matrix,
                               SkVector* ppuScale,
                               const SkIRect& clip, SkIRect* clipIRect) {
    //This action is in unit space, but the ppm is specified in physical space.
    ppuScale->set(fCurrentPixelsPerMeter.fX / fCurrentUnitsPerMeter.fX,
                  fCurrentPixelsPerMeter.fY / fCurrentUnitsPerMeter.fY);

    matrix->postScale(ppuScale->fX, ppuScale->fY);

    const SkIRect& irect = clip;
    SkRect clipRect = SkRect::MakeLTRB(SkIntToScalar(irect.fLeft) * ppuScale->fX,
                                       SkIntToScalar(irect.fTop) * ppuScale->fY,
                                       SkIntToScalar(irect.fRight) * ppuScale->fX,
                                       SkIntToScalar(irect.fBottom) * ppuScale->fY);
    clipRect.roundOut(clipIRect);
}

HRESULT SkXPSDevice::applyMask(const SkMask& mask,
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

    SkTileMode xy[2];
    xy[0] = (SkTileMode)3;
    xy[1] = (SkTileMode)3;

    SkASSERT(mask.fFormat == SkMask::kA8_Format);
    SkPixmap pm(SkImageInfo::MakeA8(mask.fBounds.width(), mask.fBounds.height()),
                mask.fImage, mask.fRowBytes);

    SkTScopedComPtr<IXpsOMTileBrush> maskBrush;
    HR(this->createXpsImageBrush(pm, m, xy, 0xFF, &maskBrush));
    HRM(shadedPath->SetOpacityMaskBrushLocal(maskBrush.get()),
        "Could not set mask.");

    const SkRect universeRect = SkRect::MakeLTRB(0, 0,
        this->fCurrentCanvasSize.fWidth, this->fCurrentCanvasSize.fHeight);
    SkTScopedComPtr<IXpsOMGeometryFigure> shadedFigure;
    HRM(this->createXpsRect(universeRect, FALSE, TRUE, &shadedFigure),
        "Could not create mask shaded figure.");
    HRM(shadedFigures->Append(shadedFigure.get()),
        "Could not add mask shaded figure.");

    HR(this->clip(shadedPath));

    //Add the path to the active visual collection.
    SkTScopedComPtr<IXpsOMVisualCollection> currentVisuals;
    HRM(this->fCurrentXpsCanvas->GetVisuals(&currentVisuals),
        "Could not get mask current visuals.");
    HRM(currentVisuals->Append(shadedPath),
        "Could not add masked shaded path to current visuals.");

    return S_OK;
}

HRESULT SkXPSDevice::shadePath(IXpsOMPath* shadedPath,
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

void SkXPSDevice::drawPath(const SkPath& platonicPath,
                           const SkPaint& origPaint,
                           bool pathIsMutable) {
    SkTCopyOnFirstWrite<SkPaint> paint(origPaint);

    // nothing to draw
    if (this->isClipEmpty() || (paint->getAlpha() == 0 && paint->isSrcOver())) {
        return;
    }

    SkPath modifiedPath;
    const bool paintHasPathEffect = paint->getPathEffect()
                                 || paint->getStyle() != SkPaint::kFill_Style;

    //Apply pre-path matrix [Platonic-path -> Skeletal-path].
    SkMatrix matrix = this->localToDevice();
    SkPath* skeletalPath = const_cast<SkPath*>(&platonicPath);

    //Apply path effect [Skeletal-path -> Fillable-path].
    SkPath* fillablePath = skeletalPath;
    if (paintHasPathEffect) {
        if (!pathIsMutable) {
            fillablePath = &modifiedPath;
            pathIsMutable = true;
        }
        bool fill = skpathutils::FillPathWithPaint(*skeletalPath, *paint, fillablePath);

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
    HRVM(this->fXpsFactory->CreatePath(&shadedPath),
         "Could not create shaded path for path.");

    //Create the geometry for the shaded path.
    SkTScopedComPtr<IXpsOMGeometry> shadedGeometry;
    HRVM(this->fXpsFactory->CreateGeometry(&shadedGeometry),
         "Could not create shaded geometry for path.");

    //Add the geometry to the shaded path.
    HRVM(shadedPath->SetGeometryLocal(shadedGeometry.get()),
         "Could not add the shaded geometry to shaded path.");

    SkMaskFilter* filter = paint->getMaskFilter();

    //Determine if we will draw or shade and mask.
    if (filter) {
        if (paint->getStyle() != SkPaint::kFill_Style) {
            paint.writable()->setStyle(SkPaint::kFill_Style);
        }
    }

    //Set the brushes.
    BOOL fill;
    BOOL stroke;
    HRV(this->shadePath(shadedPath.get(),
                        *paint,
                        this->localToDevice(),
                        &fill,
                        &stroke));

    //Mask filter
    if (filter) {
        SkIRect clipIRect;
        SkVector ppuScale;
        this->convertToPpm(filter,
                           &matrix,
                           &ppuScale,
                           this->devClipBounds(),
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
        SkMaskBuilder rasteredMask;
        if (SkDraw::DrawToMask(
                        *pixelPath,
                        clipIRect,
                        filter,  //just to compute how much to draw.
                        &matrix,
                        &rasteredMask,
                        SkMaskBuilder::kComputeBoundsAndRenderImage_CreateMode,
                        style)) {

            SkAutoMaskFreeImage rasteredAmi(rasteredMask.image());
            mask = &rasteredMask;

            //[Mask -> Mask]
            SkMaskBuilder filteredMask;
            if (as_MFB(filter)->filterMask(&filteredMask, rasteredMask, matrix, nullptr)) {
                mask = &filteredMask;
            }
            SkAutoMaskFreeImage filteredAmi(filteredMask.image());

            //Draw mask.
            HRV(this->applyMask(*mask, ppuScale, shadedPath.get()));
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
        case SkPathFillType::kWinding:
            xpsFillRule = XPS_FILL_RULE_NONZERO;
            break;
        case SkPathFillType::kEvenOdd:
            xpsFillRule = XPS_FILL_RULE_EVENODD;
            break;
        case SkPathFillType::kInverseWinding: {
            //[Fillable-path (inverse winding) -> XPS-path (inverse even odd)]
            if (!pathIsMutable) {
                xpsCompatiblePath = &modifiedPath;
                pathIsMutable = true;
            }
            if (!Simplify(*fillablePath, xpsCompatiblePath)) {
                SkDEBUGF("Could not simplify inverse winding path.");
                return;
            }
        }
        [[fallthrough]];  // The xpsCompatiblePath is now inverse even odd, so fall through.
        case SkPathFillType::kInverseEvenOdd: {
            const SkRect universe = SkRect::MakeLTRB(
                0, 0,
                this->fCurrentCanvasSize.fWidth,
                this->fCurrentCanvasSize.fHeight);
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
        //[Fillable-path -> Device-path]
        devicePath = pathIsMutable ? xpsCompatiblePath : &modifiedPath;
        xpsCompatiblePath->transform(matrix, devicePath);
    }
    HRV(this->addXpsPathGeometry(shadedFigures.get(),
                                 stroke, fill, *devicePath));

    HRV(this->clip(shadedPath.get()));

    //Add the path to the active visual collection.
    SkTScopedComPtr<IXpsOMVisualCollection> currentVisuals;
    HRVM(this->fCurrentXpsCanvas->GetVisuals(&currentVisuals),
         "Could not get current visuals for shaded path.");
    HRVM(currentVisuals->Append(shadedPath.get()),
         "Could not add shaded path to current visuals.");
}

HRESULT SkXPSDevice::clip(IXpsOMVisual* xpsVisual) {
    if (this->cs().isWideOpen()) {
        return S_OK;
    }
    SkPath clipPath;
    // clipPath.addRect(this->devClipBounds()));
    SkClipStack_AsPath(this->cs(), &clipPath);
    // TODO: handle all the kinds of paths, like drawPath does
    return this->clipToPath(xpsVisual, clipPath, XPS_FILL_RULE_EVENODD);
}
HRESULT SkXPSDevice::clipToPath(IXpsOMVisual* xpsVisual,
                                const SkPath& clipPath,
                                XPS_FILL_RULE fillRule) {
    //Create the geometry.
    SkTScopedComPtr<IXpsOMGeometry> clipGeometry;
    HRM(this->fXpsFactory->CreateGeometry(&clipGeometry),
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

HRESULT SkXPSDevice::CreateTypefaceUse(const SkFont& font,
                                       TypefaceUse** typefaceUse) {
    SkTypeface* typeface = SkFontPriv::GetTypefaceOrDefault(font);

    //Check cache.
    const SkTypefaceID typefaceID = typeface->uniqueID();
    for (TypefaceUse& current : *this->fTopTypefaces) {
        if (current.typefaceId == typefaceID) {
            *typefaceUse = &current;
            return S_OK;
        }
    }

    //TODO: create glyph only fonts
    //and let the host deal with what kind of font we're looking at.
    XPS_FONT_EMBEDDING embedding = XPS_FONT_EMBEDDING_RESTRICTED;

    SkTScopedComPtr<IStream> fontStream;
    int ttcIndex;
    std::unique_ptr<SkStreamAsset> fontData = typeface->openStream(&ttcIndex);
    if (!fontData) {
        return E_NOTIMPL;
    }
    //TODO: cannot handle FON fonts.
    HRM(SkIStream::CreateFromSkStream(fontData->duplicate(), &fontStream),
        "Could not create font stream.");

    const size_t size =
        std::size(L"/Resources/Fonts/" L_GUID_ID L".odttf");
    wchar_t buffer[size];
    wchar_t id[GUID_ID_LEN];
    HR(this->createId(id, GUID_ID_LEN));
    swprintf_s(buffer, size, L"/Resources/Fonts/%s.odttf", id);

    SkTScopedComPtr<IOpcPartUri> partUri;
    HRM(this->fXpsFactory->CreatePartUri(buffer, &partUri),
        "Could not create font resource part uri.");

    SkTScopedComPtr<IXpsOMFontResource> xpsFontResource;
    HRM(this->fXpsFactory->CreateFontResource(fontStream.get(),
                                              embedding,
                                              partUri.get(),
                                              FALSE,
                                              &xpsFontResource),
        "Could not create font resource.");

    //TODO: change openStream to return -1 for non-ttc, get rid of this.
    const uint8_t* data = (const uint8_t*)fontData->getMemoryBase();
    bool isTTC = (data &&
                  fontData->getLength() >= sizeof(SkTTCFHeader) &&
                  ((const SkTTCFHeader*)data)->ttcTag == SkTTCFHeader::TAG);

    int glyphCount = typeface->countGlyphs();

    TypefaceUse& newTypefaceUse = this->fTopTypefaces->emplace_back(
        typefaceID,
        isTTC ? ttcIndex : -1,
        std::move(fontData),
        std::move(xpsFontResource),
        glyphCount);

    *typefaceUse = &newTypefaceUse;
    return S_OK;
}

HRESULT SkXPSDevice::AddGlyphs(IXpsOMObjectFactory* xpsFactory,
                               IXpsOMCanvas* canvas,
                               const TypefaceUse* font,
                               LPCWSTR text,
                               XPS_GLYPH_INDEX* xpsGlyphs,
                               UINT32 xpsGlyphsLen,
                               XPS_POINT *origin,
                               FLOAT fontSize,
                               XPS_STYLE_SIMULATION sims,
                               const SkMatrix& transform,
                               const SkPaint& paint) {
    SkTScopedComPtr<IXpsOMGlyphs> glyphs;
    HRM(xpsFactory->CreateGlyphs(font->xpsFont.get(), &glyphs), "Could not create glyphs.");
    HRM(glyphs->SetFontFaceIndex(font->ttcIndex), "Could not set glyph font face index.");

    //XPS uses affine transformations for everything...
    //...except positioning text.
    bool useCanvasForClip;
    if (transform.isTranslate()) {
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
        HR(this->clip(glyphs.get()));
        HRM(visuals->Append(glyphs.get()), "Could not add glyphs to canvas.");
    } else {
        SkTScopedComPtr<IXpsOMCanvas> glyphCanvas;
        HRM(this->fXpsFactory->CreateCanvas(&glyphCanvas),
            "Could not create glyph canvas.");

        SkTScopedComPtr<IXpsOMVisualCollection> glyphCanvasVisuals;
        HRM(glyphCanvas->GetVisuals(&glyphCanvasVisuals),
            "Could not get glyph visuals collection.");

        HRM(glyphCanvasVisuals->Append(glyphs.get()),
            "Could not add glyphs to page.");
        HR(this->clip(glyphCanvas.get()));

        HRM(visuals->Append(glyphCanvas.get()),
            "Could not add glyph canvas to page.");
    }

    return S_OK;
}

static bool text_must_be_pathed(const SkPaint& paint, const SkMatrix& matrix) {
    const SkPaint::Style style = paint.getStyle();
    return matrix.hasPerspective()
        || SkPaint::kStroke_Style == style
        || SkPaint::kStrokeAndFill_Style == style
        || paint.getMaskFilter()
    ;
}

void SkXPSDevice::onDrawGlyphRunList(SkCanvas*,
                                     const sktext::GlyphRunList& glyphRunList,
                                     const SkPaint& initailPaint,
                                     const SkPaint& drawingPaint) {
    SkASSERT(!glyphRunList.hasRSXForm());

    for (const auto& run : glyphRunList) {
        const SkGlyphID* glyphIDs = run.glyphsIDs().data();
        size_t glyphCount = run.glyphsIDs().size();
        const SkFont& font = run.font();

        if (!glyphCount || !glyphIDs || font.getSize() <= 0) {
            continue;
        }

        TypefaceUse* typeface;
        if (FAILED(CreateTypefaceUse(font, &typeface)) ||
            text_must_be_pathed(drawingPaint, this->localToDevice())) {
            SkPath path;
            //TODO: make this work, Draw currently does not handle as well.
            //paint.getTextPath(text, byteLength, x, y, &path);
            //this->drawPath(path, paint, nullptr, true);
            //TODO: add automation "text"
            continue;
        }

        //TODO: handle font scale and skew in x (text_scale_skew)

        // Advance width and offsets for glyphs measured in hundredths of the font em size
        // (XPS Spec 5.1.3).
        FLOAT centemPerUnit = 100.0f / SkScalarToFLOAT(font.getSize());
        AutoSTMalloc<32, XPS_GLYPH_INDEX> xpsGlyphs(glyphCount);
        size_t numGlyphs = typeface->glyphsUsed.size();
        size_t actualGlyphCount = 0;

        for (size_t i = 0; i < glyphCount; ++i) {
            if (numGlyphs <= glyphIDs[i]) {
                continue;
            }
            const SkPoint& position = run.positions()[i];
            XPS_GLYPH_INDEX& xpsGlyph = xpsGlyphs[actualGlyphCount++];
            xpsGlyph.index = glyphIDs[i];
            xpsGlyph.advanceWidth = 0.0f;
            xpsGlyph.horizontalOffset = (SkScalarToFloat(position.fX) * centemPerUnit);
            xpsGlyph.verticalOffset = (SkScalarToFloat(position.fY) * -centemPerUnit);
            typeface->glyphsUsed.set(xpsGlyph.index);
        }

        if (actualGlyphCount == 0) {
            return;
        }

        XPS_POINT origin = {
            glyphRunList.origin().x(),
            glyphRunList.origin().y(),
        };

        HRV(AddGlyphs(this->fXpsFactory.get(),
                      this->fCurrentXpsCanvas.get(),
                      typeface,
                      nullptr,
                      xpsGlyphs.get(), actualGlyphCount,
                      &origin,
                      SkScalarToFLOAT(font.getSize()),
                      XPS_STYLE_SIMULATION_NONE,
                      this->localToDevice(),
                      drawingPaint));
    }
}

void SkXPSDevice::drawDevice(SkDevice* dev, const SkSamplingOptions&, const SkPaint&) {
    SkXPSDevice* that = static_cast<SkXPSDevice*>(dev);
    SkASSERT(that->fTopTypefaces == this->fTopTypefaces);

    SkTScopedComPtr<IXpsOMMatrixTransform> xpsTransform;
    HRVM(this->createXpsTransform(dev->getRelativeTransform(*this), &xpsTransform),
         "Could not create layer transform.");
    HRVM(that->fCurrentXpsCanvas->SetTransformLocal(xpsTransform.get()),
         "Could not set layer transform.");

    //Get the current visual collection and add the layer to it.
    SkTScopedComPtr<IXpsOMVisualCollection> currentVisuals;
    HRVM(this->fCurrentXpsCanvas->GetVisuals(&currentVisuals),
         "Could not get current visuals for layer.");
    HRVM(currentVisuals->Append(that->fCurrentXpsCanvas.get()),
         "Could not add layer to current visuals.");
}

sk_sp<SkDevice> SkXPSDevice::createDevice(const CreateInfo& info, const SkPaint*) {
    sk_sp<SkXPSDevice> dev = sk_make_sp<SkXPSDevice>(info.fInfo.dimensions());
    dev->fXpsFactory.reset(SkRefComPtr(fXpsFactory.get()));
    dev->fCurrentCanvasSize = this->fCurrentCanvasSize;
    dev->fCurrentUnitsPerMeter = this->fCurrentUnitsPerMeter;
    dev->fCurrentPixelsPerMeter = this->fCurrentPixelsPerMeter;
    dev->fTopTypefaces = this->fTopTypefaces;
    SkAssertResult(dev->createCanvasForLayer());
    return dev;
}

void SkXPSDevice::drawOval( const SkRect& o, const SkPaint& p) {
    SkPath path;
    path.addOval(o);
    this->drawPath(path, p, true);
}

void SkXPSDevice::drawImageRect(const SkImage* image,
                                const SkRect* src,
                                const SkRect& dst,
                                const SkSamplingOptions& sampling,
                                const SkPaint& paint,
                                SkCanvas::SrcRectConstraint constraint) {
    // TODO: support gpu images
    SkBitmap bitmap;
    if (!as_IB(image)->getROPixels(nullptr, &bitmap)) {
        return;
    }

    SkRect bitmapBounds = SkRect::Make(bitmap.bounds());
    SkRect srcBounds = src ? *src : bitmapBounds;
    SkMatrix matrix = SkMatrix::RectToRect(srcBounds, dst);
    SkRect actualDst;
    if (!src || bitmapBounds.contains(*src)) {
        actualDst = dst;
    } else {
        if (!srcBounds.intersect(bitmapBounds)) {
            return;
        }
        matrix.mapRect(&actualDst, srcBounds);
    }

    auto bitmapShader = SkMakeBitmapShaderForPaint(paint, bitmap,
                                                   SkTileMode::kClamp, SkTileMode::kClamp,
                                                   sampling, &matrix, kNever_SkCopyPixelsMode);
    SkASSERT(bitmapShader);
    if (!bitmapShader) { return; }
    SkPaint paintWithShader(paint);
    paintWithShader.setStyle(SkPaint::kFill_Style);
    paintWithShader.setShader(std::move(bitmapShader));
    this->drawRect(actualDst, paintWithShader);
}
#endif//defined(SK_BUILD_FOR_WIN)
