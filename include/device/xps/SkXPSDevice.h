/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkXPSDevice_DEFINED
#define SkXPSDevice_DEFINED

#include "SkTypes.h"
#include <ObjBase.h>
#include <XpsObjectModel.h>

#include "SkAutoCoInitialize.h"
#include "SkBitmapDevice.h"
#include "SkBitSet.h"
#include "SkCanvas.h"
#include "SkColor.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkPoint.h"
#include "SkShader.h"
#include "SkSize.h"
#include "SkTArray.h"
#include "SkTScopedComPtr.h"
#include "SkTypeface.h"

/** \class SkXPSDevice

    The drawing context for the XPS backend.
*/
class SkXPSDevice : public SkBitmapDevice {
public:
    SK_API SkXPSDevice();
    SK_API virtual ~SkXPSDevice();

    virtual bool beginPortfolio(SkWStream* outputStream);
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
    virtual bool beginSheet(
        const SkVector& unitsPerMeter,
        const SkVector& pixelsPerMeter,
        const SkSize& trimSize,
        const SkRect* mediaBox = NULL,
        const SkRect* bleedBox = NULL,
        const SkRect* artBox = NULL,
        const SkRect* cropBox = NULL);

    virtual bool endSheet();
    virtual bool endPortfolio();

    virtual uint32_t getDeviceCapabilities() SK_OVERRIDE;

protected:
    virtual void clear(SkColor color) SK_OVERRIDE;

    virtual void drawPaint(const SkDraw&, const SkPaint& paint) SK_OVERRIDE;

    virtual void drawPoints(
        const SkDraw&,
        SkCanvas::PointMode mode,
        size_t count, const SkPoint[],
        const SkPaint& paint) SK_OVERRIDE;

    virtual void drawRect(
        const SkDraw&,
        const SkRect& r,
        const SkPaint& paint) SK_OVERRIDE;

    virtual void drawPath(
        const SkDraw&,
        const SkPath& platonicPath,
        const SkPaint& paint,
        const SkMatrix* prePathMatrix,
        bool pathIsMutable) SK_OVERRIDE;

    virtual void drawBitmap(
        const SkDraw&,
        const SkBitmap& bitmap,
        const SkMatrix& matrix,
        const SkPaint& paint) SK_OVERRIDE;

    virtual void drawSprite(
        const SkDraw&,
        const SkBitmap& bitmap,
        int x, int y,
        const SkPaint& paint) SK_OVERRIDE;

    virtual void drawText(
        const SkDraw&,
        const void* text, size_t len,
        SkScalar x, SkScalar y,
        const SkPaint& paint) SK_OVERRIDE;

    virtual void drawPosText(
        const SkDraw&,
        const void* text, size_t len,
        const SkScalar pos[], SkScalar constY, int scalarsPerPos,
        const SkPaint& paint) SK_OVERRIDE;

    virtual void drawTextOnPath(
        const SkDraw&,
        const void* text, size_t len,
        const SkPath& path,
        const SkMatrix* matrix,
        const SkPaint& paint) SK_OVERRIDE;

    virtual void drawVertices(
        const SkDraw&,
        SkCanvas::VertexMode,
        int vertexCount, const SkPoint verts[],
        const SkPoint texs[], const SkColor colors[],
        SkXfermode* xmode,
        const uint16_t indices[], int indexCount,
        const SkPaint& paint) SK_OVERRIDE;

    virtual void drawDevice(
        const SkDraw&,
        SkBaseDevice* device,
        int x, int y,
        const SkPaint& paint) SK_OVERRIDE;

    virtual bool onReadPixels(const SkBitmap& bitmap,
                              int x,
                              int y,
                              SkCanvas::Config8888) SK_OVERRIDE;

    virtual bool allowImageFilter(SkImageFilter*) SK_OVERRIDE;

private:
    class TypefaceUse : ::SkNoncopyable {
    public:
        SkFontID typefaceId;
        int ttcIndex;
        SkStream* fontData;
        IXpsOMFontResource* xpsFont;
        SkBitSet* glyphsUsed;

        explicit TypefaceUse();
        ~TypefaceUse();
    };
    friend static HRESULT subset_typeface(TypefaceUse* current);

    SkXPSDevice(IXpsOMObjectFactory* xpsFactory);

    SkAutoCoInitialize fAutoCo;
    SkTScopedComPtr<IXpsOMObjectFactory> fXpsFactory;
    SkTScopedComPtr<IStream> fOutputStream;
    SkTScopedComPtr<IXpsOMPackageWriter> fPackageWriter;

    unsigned int fCurrentPage;
    SkTScopedComPtr<IXpsOMCanvas> fCurrentXpsCanvas;
    SkSize fCurrentCanvasSize;
    SkVector fCurrentUnitsPerMeter;
    SkVector fCurrentPixelsPerMeter;

    SkTArray<TypefaceUse, true> fTypefaces;

    HRESULT initXpsDocumentWriter(IXpsOMImageResource* image);

    HRESULT createXpsPage(
        const XPS_SIZE& pageSize,
        IXpsOMPage** page);

    HRESULT createXpsThumbnail(
        IXpsOMPage* page, const unsigned int pageNumber,
        IXpsOMImageResource** image);

    void internalDrawRect(
        const SkDraw&,
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
        const SkDraw& d,
        IXpsOMObjectFactory* xpsFactory,
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
        const SkDraw& d);
    HRESULT clipToPath(
        IXpsOMVisual* xpsVisual,
        const SkPath& clipPath,
        XPS_FILL_RULE fillRule);

    HRESULT drawInverseWindingPath(
        const SkDraw& d,
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
        const SkDraw& d,
        const SkMask& mask,
        const SkVector& ppuScale,
        IXpsOMPath* shadedPath);

    // override from SkBaseDevice
    virtual SkBaseDevice* onCreateCompatibleDevice(SkBitmap::Config config,
                                                   int width, int height,
                                                   bool isOpaque,
                                                   Usage usage) SK_OVERRIDE;

    // Disable the default copy and assign implementation.
    SkXPSDevice(const SkXPSDevice&);
    void operator=(const SkXPSDevice&);

    typedef SkBitmapDevice INHERITED;
};

#endif
