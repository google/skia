/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkXPSDevice_DEFINED
#define SkXPSDevice_DEFINED

#include "SkTypes.h"

#ifdef SK_BUILD_FOR_WIN

#include <ObjBase.h>
#include <XpsObjectModel.h>

#include "SkAutoCoInitialize.h"
#include "SkBitSet.h"
#include "SkBitmapDevice.h"
#include "SkCanvas.h"
#include "SkClipStackDevice.h"
#include "SkColor.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkPoint.h"
#include "SkShader.h"
#include "SkSize.h"
#include "SkTArray.h"
#include "SkTScopedComPtr.h"
#include "SkTypeface.h"

class SkGlyphRunList;

//#define SK_XPS_USE_DETERMINISTIC_IDS

/** \class SkXPSDevice

    The drawing context for the XPS backend.
*/
class SkXPSDevice : public SkClipStackDevice {
public:
    SK_API SkXPSDevice(SkISize);
    SK_API ~SkXPSDevice() override;

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
        const SkRect* mediaBox = NULL,
        const SkRect* bleedBox = NULL,
        const SkRect* artBox = NULL,
        const SkRect* cropBox = NULL);

    bool endSheet();
    bool endPortfolio();

protected:
    void drawPaint(const SkPaint& paint) override;
    void drawPoints(SkCanvas::PointMode mode, size_t count,
                    const SkPoint[], const SkPaint& paint) override;
    void drawRect(const SkRect& r,
                  const SkPaint& paint) override;
    void drawOval(const SkRect& oval,
                  const SkPaint& paint) override;
    void drawRRect(const SkRRect& rr,
                   const SkPaint& paint) override;
    void drawPath(const SkPath& path,
                  const SkPaint& paint,
                  bool pathIsMutable = false) override;
    void drawSprite(const SkBitmap& bitmap,
                    int x, int y, const SkPaint& paint) override;
    void drawBitmapRect(const SkBitmap&,
                        const SkRect* srcOrNull, const SkRect& dst,
                        const SkPaint& paint,
                        SkCanvas::SrcRectConstraint) override;
    void drawGlyphRunList(const SkGlyphRunList& glyphRunList) override {
        SK_ABORT("Needs an implementation");
    }
    void drawVertices(const SkVertices*, const SkVertices::Bone bones[], int boneCount, SkBlendMode,
                      const SkPaint&) override;
    void drawDevice(SkBaseDevice*, int x, int y,
                    const SkPaint&) override;

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
    friend HRESULT subset_typeface(TypefaceUse* current);

    bool createCanvasForLayer();

    SkTScopedComPtr<IXpsOMObjectFactory> fXpsFactory;
    SkTScopedComPtr<IStream> fOutputStream;
    SkTScopedComPtr<IXpsOMPackageWriter> fPackageWriter;

    unsigned int fCurrentPage;
    SkTScopedComPtr<IXpsOMCanvas> fCurrentXpsCanvas;
    SkSize fCurrentCanvasSize;
    SkVector fCurrentUnitsPerMeter;
    SkVector fCurrentPixelsPerMeter;

    SkTArray<TypefaceUse, true> fTypefaces;

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

    SkBaseDevice* onCreateDevice(const CreateInfo&, const SkPaint*) override;

    // Disable the default copy and assign implementation.
    SkXPSDevice(const SkXPSDevice&);
    void operator=(const SkXPSDevice&);

    typedef SkClipStackDevice INHERITED;
};

#endif  // SK_BUILD_FOR_WIN
#endif  // SkXPSDevice_DEFINED
