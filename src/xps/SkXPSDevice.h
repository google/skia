/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkXPSDevice_DEFINED
#define SkXPSDevice_DEFINED

#include "include/core/SkTypes.h"

#ifdef SK_BUILD_FOR_WIN

#include <ObjBase.h>
#include <XpsObjectModel.h>

#include "include/core/SkCPURecorder.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkTypeface.h"
#include "include/private/base/SkTArray.h"
#include "src/core/SkBitmapDevice.h"
#include "src/core/SkClipStackDevice.h"
#include "src/utils/SkBitSet.h"
#include "src/utils/win/SkAutoCoInitialize.h"
#include "src/utils/win/SkTScopedComPtr.h"

namespace sktext {
class GlyphRunList;
}
//#define SK_XPS_USE_DETERMINISTIC_IDS

/** \class SkXPSDevice

    The drawing context for the XPS backend.
*/
class SkXPSDevice final : public SkClipStackDevice {
public:
    SK_SPI SkXPSDevice(SkISize);
    SK_SPI ~SkXPSDevice() override;

    bool beginPortfolio(SkWStream* outputStream, IXpsOMObjectFactory*);
    /**
      @param unitsPerMeter converts geometry units into physical units.
      @param pixelsPerMeter resolution to use when geometry must be rasterized.
      @param trimSize final page size in physical units.
                      The top left of the trim is the origin of physical space.
      @param mediaBox The size of the physical media in physical units.
                      The top and left must be less than zero.
                      The bottom and right must be greater than the trimSize.
                      The default is to coincide with the trimSize.
      @param bleedBox The size of the bleed box in physical units.
                      Must be contained within the mediaBox.
                      The default is to coincide with the mediaBox.
      @param artBox The size of the content box in physical units.
                    Must be contained within the trimSize.
                    The default is to coincide with the trimSize.
      @param cropBox The size of the recommended view port in physical units.
                     Must be contained within the mediaBox.
                     The default is to coincide with the mediaBox.
     */
    bool beginSheet(
        const SkVector& unitsPerMeter,
        const SkVector& pixelsPerMeter,
        const SkSize& trimSize,
        const SkRect* mediaBox = nullptr,
        const SkRect* bleedBox = nullptr,
        const SkRect* artBox = nullptr,
        const SkRect* cropBox = nullptr);

    bool endSheet();
    bool endPortfolio();

    void drawPaint(const SkPaint& paint) override;
    void drawPoints(SkCanvas::PointMode, SkSpan<const SkPoint>, const SkPaint&) override;
    void drawRect(const SkRect& r,
                  const SkPaint& paint) override;
    void drawOval(const SkRect& oval,
                  const SkPaint& paint) override;
    void drawRRect(const SkRRect& rr,
                   const SkPaint& paint) override;
    void drawPath(const SkPath& path,
                  const SkPaint& paint,
                  bool pathIsMutable = false) override;
    void drawImageRect(const SkImage*,
                       const SkRect* srcOrNull, const SkRect& dst,
                       const SkSamplingOptions&, const SkPaint& paint,
                       SkCanvas::SrcRectConstraint) override;

    void drawVertices(const SkVertices*, sk_sp<SkBlender>, const SkPaint&, bool) override;
    void drawMesh(const SkMesh&, sk_sp<SkBlender>, const SkPaint&) override;

    void drawDevice(SkDevice*, const SkSamplingOptions&, const SkPaint&) override;

    sk_sp<SkDevice> createDevice(const CreateInfo&, const SkPaint*) override;

    SkRecorder* baseRecorder() const override {
        // TODO(kjlubick) the creation of this should likely involve a CPU context.
        return skcpu::Recorder::TODO();
    }

private:
    class TypefaceUse {
    public:
        TypefaceUse(SkTypefaceID id, int index, std::unique_ptr<SkStream> data,
                    SkTScopedComPtr<IXpsOMFontResource> xps, size_t numGlyphs)
            : typefaceId(id), ttcIndex(index), fontData(std::move(data))
            , xpsFont(std::move(xps)), glyphsUsed(numGlyphs) {}
        const SkTypefaceID typefaceId;
        const int ttcIndex;
        const std::unique_ptr<SkStream> fontData;
        const SkTScopedComPtr<IXpsOMFontResource> xpsFont;
        SkBitSet glyphsUsed;
    };
    friend HRESULT subset_typeface(const TypefaceUse& current);

    void onDrawGlyphRunList(SkCanvas*, const sktext::GlyphRunList&, const SkPaint&) override;

    bool createCanvasForLayer();

    SkTScopedComPtr<IXpsOMObjectFactory> fXpsFactory;
    SkTScopedComPtr<IStream> fOutputStream;
    SkTScopedComPtr<IXpsOMPackageWriter> fPackageWriter;

    unsigned int fCurrentPage;
    SkTScopedComPtr<IXpsOMCanvas> fCurrentXpsCanvas;
    SkSize fCurrentCanvasSize;
    SkVector fCurrentUnitsPerMeter;
    SkVector fCurrentPixelsPerMeter;

    skia_private::TArray<TypefaceUse, true> fTypefaces;
    skia_private::TArray<TypefaceUse, true>* fTopTypefaces;

    /** Creates a GUID based id and places it into buffer.
        buffer should have space for at least GUID_ID_LEN wide characters.
        The string will always be wchar null terminated.
        XXXXXXXXsXXXXsXXXXsXXXXsXXXXXXXXXXXX0
        The string may begin with a digit,
        and so may not be suitable as a bare resource key.
     */
    HRESULT createId(wchar_t* buffer, size_t bufferSize, wchar_t sep = '-');
#ifdef SK_XPS_USE_DETERMINISTIC_IDS
    decltype(GUID::Data1) fNextId = 0;
#endif

    HRESULT initXpsDocumentWriter(IXpsOMImageResource* image);

    HRESULT createXpsPage(
        const XPS_SIZE& pageSize,
        IXpsOMPage** page);

    HRESULT createXpsThumbnail(
        IXpsOMPage* page, const unsigned int pageNumber,
        IXpsOMImageResource** image);

    void internalDrawRect(
        const SkRect& r,
        bool transformRect,
        const SkPaint& paint);

    HRESULT createXpsBrush(
        const SkPaint& skPaint,
        IXpsOMBrush** xpsBrush,
        const SkMatrix* parentTransform = nullptr);

    HRESULT createXpsSolidColorBrush(
        const SkColor4f skColor, const SkAlpha alpha,
        IXpsOMBrush** xpsBrush);

    HRESULT createXpsImageBrush(
        const SkPixmap& bitmap,
        const SkMatrix& localMatrix,
        const SkTileMode (&xy)[2],
        const SkAlpha alpha,
        IXpsOMTileBrush** xpsBrush);

    HRESULT createXpsLinearGradient(
        SkShaderBase::GradientInfo info,
        const SkAlpha alpha,
        const SkMatrix& localMatrix,
        IXpsOMMatrixTransform* xpsMatrixToUse,
        IXpsOMBrush** xpsBrush);

    HRESULT createXpsRadialGradient(
        SkShaderBase::GradientInfo info,
        const SkAlpha alpha,
        const SkMatrix& localMatrix,
        IXpsOMMatrixTransform* xpsMatrixToUse,
        IXpsOMBrush** xpsBrush);

    HRESULT createXpsGradientStop(
        const SkColor4f skColor,
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
        const SkFont& font,
        TypefaceUse** fontResource);

    HRESULT AddGlyphs(
        IXpsOMObjectFactory* xpsFactory,
        IXpsOMCanvas* canvas,
        const TypefaceUse* font,
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
        const SkColor4f color,
        IXpsOMVisualCollection* visuals);

    HRESULT clip(IXpsOMVisual* xpsVisual);

    HRESULT clipToPath(
        IXpsOMVisual* xpsVisual,
        const SkPath& clipPath,
        XPS_FILL_RULE fillRule);

    HRESULT drawInverseWindingPath(
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
        const SkMask& mask,
        const SkVector& ppuScale,
        IXpsOMPath* shadedPath);

    // Disable the default copy and assign implementation.
    SkXPSDevice(const SkXPSDevice&);
    void operator=(const SkXPSDevice&);
};

#endif  // SK_BUILD_FOR_WIN
#endif  // SkXPSDevice_DEFINED
