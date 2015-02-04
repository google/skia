/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSVGDevice.h"

#include "SkBitmap.h"
#include "SkDraw.h"
#include "SkPaint.h"
#include "SkParsePath.h"
#include "SkPathOps.h"
#include "SkShader.h"
#include "SkStream.h"
#include "SkTypeface.h"
#include "SkUtils.h"
#include "SkXMLWriter.h"

namespace {

static SkString svg_color(SkColor color) {
    return SkStringPrintf("rgb(%u,%u,%u)",
                          SkColorGetR(color),
                          SkColorGetG(color),
                          SkColorGetB(color));
}

static SkScalar svg_opacity(SkColor color) {
    return SkIntToScalar(SkColorGetA(color)) / SK_AlphaOPAQUE;
}

// Keep in sync with SkPaint::Cap
static const char* cap_map[]  = {
    NULL,    // kButt_Cap (default)
    "round", // kRound_Cap
    "square" // kSquare_Cap
};
SK_COMPILE_ASSERT(SK_ARRAY_COUNT(cap_map) == SkPaint::kCapCount, missing_cap_map_entry);

static const char* svg_cap(SkPaint::Cap cap) {
    SkASSERT(cap < SK_ARRAY_COUNT(cap_map));
    return cap_map[cap];
}

// Keep in sync with SkPaint::Join
static const char* join_map[] = {
    NULL,    // kMiter_Join (default)
    "round", // kRound_Join
    "bevel"  // kBevel_Join
};
SK_COMPILE_ASSERT(SK_ARRAY_COUNT(join_map) == SkPaint::kJoinCount, missing_join_map_entry);

static const char* svg_join(SkPaint::Join join) {
    SkASSERT(join < SK_ARRAY_COUNT(join_map));
    return join_map[join];
}

static void append_escaped_unichar(SkUnichar c, SkString* text) {
    switch(c) {
    case '&':
        text->append("&amp;");
        break;
    case '"':
        text->append("&quot;");
        break;
    case '\'':
        text->append("&apos;");
        break;
    case '<':
        text->append("&lt;");
        break;
    case '>':
        text->append("&gt;");
        break;
    default:
        text->appendUnichar(c);
        break;
    }
}

static SkString svg_text(const void* text, size_t byteLen, const SkPaint& paint) {
    SkString svgText;
    int count = paint.countText(text, byteLen);

    switch(paint.getTextEncoding()) {
    case SkPaint::kGlyphID_TextEncoding: {
        SkASSERT(count * sizeof(uint16_t) == byteLen);
        SkAutoSTArray<64, SkUnichar> unichars(count);
        paint.glyphsToUnichars((const uint16_t*)text, count, unichars.get());
        for (int i = 0; i < count; ++i) {
            append_escaped_unichar(unichars[i], &svgText);
        }
    } break;
    case SkPaint::kUTF8_TextEncoding: {
        const char* c8 = reinterpret_cast<const char*>(text);
        for (int i = 0; i < count; ++i) {
            append_escaped_unichar(SkUTF8_NextUnichar(&c8), &svgText);
        }
        SkASSERT(reinterpret_cast<const char*>(text) + byteLen == c8);
    } break;
    case SkPaint::kUTF16_TextEncoding: {
        const uint16_t* c16 = reinterpret_cast<const uint16_t*>(text);
        for (int i = 0; i < count; ++i) {
            append_escaped_unichar(SkUTF16_NextUnichar(&c16), &svgText);
        }
        SkASSERT(SkIsAlign2(byteLen));
        SkASSERT(reinterpret_cast<const uint16_t*>(text) + (byteLen / 2) == c16);
    } break;
    case SkPaint::kUTF32_TextEncoding: {
        SkASSERT(count * sizeof(uint32_t) == byteLen);
        const uint32_t* c32 = reinterpret_cast<const uint32_t*>(text);
        for (int i = 0; i < count; ++i) {
            append_escaped_unichar(c32[i], &svgText);
        }
    } break;
    default:
        SkFAIL("unknown text encoding");
    }

    return svgText;
}

struct Resources {
    Resources(const SkPaint& paint)
        : fPaintServer(svg_color(paint.getColor())) {}

    SkString fPaintServer;
    SkString fClip;
};

}

// For now all this does is serve unique serial IDs, but it will eventually evolve to track
// and deduplicate resources.
class SkSVGDevice::ResourceBucket : ::SkNoncopyable {
public:
    ResourceBucket() : fGradientCount(0), fClipCount(0) {}

    SkString addLinearGradient() {
        return SkStringPrintf("gradient_%d", fGradientCount++);
    }

    SkString addClip() {
        return SkStringPrintf("clip_%d", fClipCount++);
    }

private:
    uint32_t fGradientCount;
    uint32_t fClipCount;
};

class SkSVGDevice::AutoElement : ::SkNoncopyable {
public:
    AutoElement(const char name[], SkXMLWriter* writer)
        : fWriter(writer)
        , fResourceBucket(NULL) {
        fWriter->startElement(name);
    }

    AutoElement(const char name[], SkXMLWriter* writer, ResourceBucket* bucket,
                const SkDraw& draw, const SkPaint& paint)
        : fWriter(writer)
        , fResourceBucket(bucket) {

        Resources res = this->addResources(draw, paint);

        fWriter->startElement(name);

        this->addPaint(paint, res);
        this->addTransform(*draw.fMatrix);
    }

    ~AutoElement() {
        fWriter->endElement();
    }

    void addAttribute(const char name[], const char val[]) {
        fWriter->addAttribute(name, val);
    }

    void addAttribute(const char name[], const SkString& val) {
        fWriter->addAttribute(name, val.c_str());
    }

    void addAttribute(const char name[], int32_t val) {
        fWriter->addS32Attribute(name, val);
    }

    void addAttribute(const char name[], SkScalar val) {
        fWriter->addScalarAttribute(name, val);
    }

    void addText(const SkString& text) {
        fWriter->addText(text.c_str());
    }

    void addRectAttributes(const SkRect&);
    void addFontAttributes(const SkPaint&);

private:
    Resources addResources(const SkDraw& draw, const SkPaint& paint);
    void addClipResources(const SkDraw& draw, Resources* resources);
    void addShaderResources(const SkPaint& paint, Resources* resources);

    void addPaint(const SkPaint& paint, const Resources& resources);
    void addTransform(const SkMatrix& transform, const char name[] = "transform");

    SkString addLinearGradientDef(const SkShader::GradientInfo& info, const SkShader* shader);

    SkXMLWriter*    fWriter;
    ResourceBucket* fResourceBucket;
};

void SkSVGDevice::AutoElement::addPaint(const SkPaint& paint, const Resources& resources) {
    SkPaint::Style style = paint.getStyle();
    if (style == SkPaint::kFill_Style || style == SkPaint::kStrokeAndFill_Style) {
        this->addAttribute("fill", resources.fPaintServer);

        if (SK_AlphaOPAQUE != SkColorGetA(paint.getColor())) {
            this->addAttribute("fill-opacity", svg_opacity(paint.getColor()));
        }
    } else {
        SkASSERT(style == SkPaint::kStroke_Style);
        this->addAttribute("fill", "none");
    }

    if (style == SkPaint::kStroke_Style || style == SkPaint::kStrokeAndFill_Style) {
        this->addAttribute("stroke", resources.fPaintServer);

        SkScalar strokeWidth = paint.getStrokeWidth();
        if (strokeWidth == 0) {
            // Hairline stroke
            strokeWidth = 1;
            this->addAttribute("vector-effect", "non-scaling-stroke");
        }
        this->addAttribute("stroke-width", strokeWidth);

        if (const char* cap = svg_cap(paint.getStrokeCap())) {
            this->addAttribute("stroke-linecap", cap);
        }

        if (const char* join = svg_join(paint.getStrokeJoin())) {
            this->addAttribute("stroke-linejoin", join);
        }

        if (paint.getStrokeJoin() == SkPaint::kMiter_Join) {
            this->addAttribute("stroke-miterlimit", paint.getStrokeMiter());
        }

        if (SK_AlphaOPAQUE != SkColorGetA(paint.getColor())) {
            this->addAttribute("stroke-opacity", svg_opacity(paint.getColor()));
        }
    } else {
        SkASSERT(style == SkPaint::kFill_Style);
        this->addAttribute("stroke", "none");
    }

    if (!resources.fClip.isEmpty()) {
        this->addAttribute("clip-path", resources.fClip);
    }
}

void SkSVGDevice::AutoElement::addTransform(const SkMatrix& t, const char name[]) {
    if (t.isIdentity()) {
        return;
    }

    SkString tstr;
    switch (t.getType()) {
    case SkMatrix::kPerspective_Mask:
        SkDebugf("Can't handle perspective matrices.");
        break;
    case SkMatrix::kTranslate_Mask:
        tstr.printf("translate(%g %g)",
                     SkScalarToFloat(t.getTranslateX()),
                     SkScalarToFloat(t.getTranslateY()));
        break;
    case SkMatrix::kScale_Mask:
        tstr.printf("scale(%g %g)",
                     SkScalarToFloat(t.getScaleX()),
                     SkScalarToFloat(t.getScaleY()));
        break;
    default:
        tstr.printf("matrix(%g %g %g %g %g %g)",
                     SkScalarToFloat(t.getScaleX()), SkScalarToFloat(t.getSkewY()),
                     SkScalarToFloat(t.getSkewX()), SkScalarToFloat(t.getScaleY()),
                     SkScalarToFloat(t.getTranslateX()), SkScalarToFloat(t.getTranslateY()));
        break;
    }

    fWriter->addAttribute(name, tstr.c_str());
}

Resources SkSVGDevice::AutoElement::addResources(const SkDraw& draw, const SkPaint& paint) {
    Resources resources(paint);

    // FIXME: this is a weak heuristic and we end up with LOTS of redundant clips.
    bool hasClip   = !draw.fClipStack->isWideOpen();
    bool hasShader = SkToBool(paint.getShader());

    if (hasClip || hasShader) {
        AutoElement defs("defs", fWriter);

        if (hasClip) {
            this->addClipResources(draw, &resources);
        }

        if (hasShader) {
            this->addShaderResources(paint, &resources);
        }
    }

    return resources;
}

void SkSVGDevice::AutoElement::addShaderResources(const SkPaint& paint, Resources* resources) {
    const SkShader* shader = paint.getShader();
    SkASSERT(SkToBool(shader));

    SkShader::GradientInfo grInfo;
    grInfo.fColorCount = 0;
    if (SkShader::kLinear_GradientType != shader->asAGradient(&grInfo)) {
        // TODO: non-linear gradient support
        SkDebugf("unsupported shader type\n");
        return;
    }

    SkAutoSTArray<16, SkColor>  grColors(grInfo.fColorCount);
    SkAutoSTArray<16, SkScalar> grOffsets(grInfo.fColorCount);
    grInfo.fColors = grColors.get();
    grInfo.fColorOffsets = grOffsets.get();

    // One more call to get the actual colors/offsets.
    shader->asAGradient(&grInfo);
    SkASSERT(grInfo.fColorCount <= grColors.count());
    SkASSERT(grInfo.fColorCount <= grOffsets.count());

    resources->fPaintServer.printf("url(#%s)", addLinearGradientDef(grInfo, shader).c_str());
}

void SkSVGDevice::AutoElement::addClipResources(const SkDraw& draw, Resources* resources) {
    SkASSERT(!draw.fClipStack->isWideOpen());

    SkPath clipPath;
    (void) draw.fClipStack->asPath(&clipPath);

    SkString clipID = fResourceBucket->addClip();
    const char* clipRule = clipPath.getFillType() == SkPath::kEvenOdd_FillType ?
                           "evenodd" : "nonzero";
    {
        // clipPath is in device space, but since we're only pushing transform attributes
        // to the leaf nodes, so are all our elements => SVG userSpaceOnUse == device space.
        AutoElement clipPathElement("clipPath", fWriter);
        clipPathElement.addAttribute("id", clipID);

        SkRect clipRect = SkRect::MakeEmpty();
        if (clipPath.isEmpty() || clipPath.isRect(&clipRect)) {
            AutoElement rectElement("rect", fWriter);
            rectElement.addRectAttributes(clipRect);
            rectElement.addAttribute("clip-rule", clipRule);
        } else {
            AutoElement pathElement("path", fWriter);
            SkString pathStr;
            SkParsePath::ToSVGString(clipPath, &pathStr);
            pathElement.addAttribute("d", pathStr.c_str());
            pathElement.addAttribute("clip-rule", clipRule);
        }
    }

    resources->fClip.printf("url(#%s)", clipID.c_str());
}

SkString SkSVGDevice::AutoElement::addLinearGradientDef(const SkShader::GradientInfo& info,
                                                        const SkShader* shader) {
    SkASSERT(fResourceBucket);
    SkString id = fResourceBucket->addLinearGradient();

    {
        AutoElement gradient("linearGradient", fWriter);

        gradient.addAttribute("id", id);
        gradient.addAttribute("gradientUnits", "userSpaceOnUse");
        gradient.addAttribute("x1", info.fPoint[0].x());
        gradient.addAttribute("y1", info.fPoint[0].y());
        gradient.addAttribute("x2", info.fPoint[1].x());
        gradient.addAttribute("y2", info.fPoint[1].y());
        gradient.addTransform(shader->getLocalMatrix(), "gradientTransform");

        SkASSERT(info.fColorCount >= 2);
        for (int i = 0; i < info.fColorCount; ++i) {
            SkColor color = info.fColors[i];
            SkString colorStr(svg_color(color));

            {
                AutoElement stop("stop", fWriter);
                stop.addAttribute("offset", info.fColorOffsets[i]);
                stop.addAttribute("stop-color", colorStr.c_str());

                if (SK_AlphaOPAQUE != SkColorGetA(color)) {
                    stop.addAttribute("stop-opacity", svg_opacity(color));
                }
            }
        }
    }

    return id;
}

void SkSVGDevice::AutoElement::addRectAttributes(const SkRect& rect) {
    // x, y default to 0
    if (rect.x() != 0) {
        this->addAttribute("x", rect.x());
    }
    if (rect.y() != 0) {
        this->addAttribute("y", rect.y());
    }

    this->addAttribute("width", rect.width());
    this->addAttribute("height", rect.height());
}

void SkSVGDevice::AutoElement::addFontAttributes(const SkPaint& paint) {
    this->addAttribute("font-size", paint.getTextSize());

    SkTypeface::Style style = paint.getTypeface()->style();
    if (style & SkTypeface::kItalic) {
        this->addAttribute("font-style", "italic");
    }
    if (style & SkTypeface::kBold) {
        this->addAttribute("font-weight", "bold");
    }

    SkAutoTUnref<const SkTypeface> tface(paint.getTypeface() ?
        SkRef(paint.getTypeface()) : SkTypeface::RefDefault(style));
    SkString familyName;
    tface->getFamilyName(&familyName);
    if (!familyName.isEmpty()) {
        this->addAttribute("font-family", familyName);
    }
}

SkBaseDevice* SkSVGDevice::Create(const SkISize& size, SkWStream* wstream) {
    if (!SkToBool(wstream)) {
        return NULL;
    }

    return SkNEW_ARGS(SkSVGDevice, (size, wstream));
}

SkSVGDevice::SkSVGDevice(const SkISize& size, SkWStream* wstream)
    : fWriter(SkNEW_ARGS(SkXMLStreamWriter, (wstream)))
    , fResourceBucket(SkNEW(ResourceBucket)) {

    fLegacyBitmap.setInfo(SkImageInfo::MakeUnknown(size.width(), size.height()));

    fWriter->writeHeader();

    // The root <svg> tag gets closed by the destructor.
    fRootElement.reset(SkNEW_ARGS(AutoElement, ("svg", fWriter)));

    fRootElement->addAttribute("xmlns", "http://www.w3.org/2000/svg");
    fRootElement->addAttribute("xmlns:xlink", "http://www.w3.org/1999/xlink");
    fRootElement->addAttribute("width", size.width());
    fRootElement->addAttribute("height", size.height());
}

SkSVGDevice::~SkSVGDevice() {
}

SkImageInfo SkSVGDevice::imageInfo() const {
    return fLegacyBitmap.info();
}

const SkBitmap& SkSVGDevice::onAccessBitmap() {
    return fLegacyBitmap;
}

void SkSVGDevice::drawPaint(const SkDraw& draw, const SkPaint& paint) {
    AutoElement rect("rect", fWriter, fResourceBucket, draw, paint);
    rect.addRectAttributes(SkRect::MakeWH(SkIntToScalar(this->width()),
                                          SkIntToScalar(this->height())));
}

void SkSVGDevice::drawPoints(const SkDraw&, SkCanvas::PointMode mode, size_t count,
                             const SkPoint[], const SkPaint& paint) {
    // todo
    SkDebugf("unsupported operation: drawPoints()\n");
}

void SkSVGDevice::drawRect(const SkDraw& draw, const SkRect& r, const SkPaint& paint) {
    AutoElement rect("rect", fWriter, fResourceBucket, draw, paint);
    rect.addRectAttributes(r);
}

void SkSVGDevice::drawOval(const SkDraw& draw, const SkRect& oval, const SkPaint& paint) {
    AutoElement ellipse("ellipse", fWriter, fResourceBucket, draw, paint);
    ellipse.addAttribute("cx", oval.centerX());
    ellipse.addAttribute("cy", oval.centerY());
    ellipse.addAttribute("rx", oval.width() / 2);
    ellipse.addAttribute("ry", oval.height() / 2);
}

void SkSVGDevice::drawRRect(const SkDraw&, const SkRRect& rr, const SkPaint& paint) {
    // todo
    SkDebugf("unsupported operation: drawRRect()\n");
}

void SkSVGDevice::drawPath(const SkDraw& draw, const SkPath& path, const SkPaint& paint,
                           const SkMatrix* prePathMatrix, bool pathIsMutable) {
    AutoElement elem("path", fWriter, fResourceBucket, draw, paint);

    SkString pathStr;
    SkParsePath::ToSVGString(path, &pathStr);
    elem.addAttribute("d", pathStr.c_str());
}

void SkSVGDevice::drawBitmap(const SkDraw&, const SkBitmap& bitmap,
                             const SkMatrix& matrix, const SkPaint& paint) {
    // todo
    SkDebugf("unsupported operation: drawBitmap()\n");
}

void SkSVGDevice::drawSprite(const SkDraw&, const SkBitmap& bitmap,
                             int x, int y, const SkPaint& paint) {
    // todo
    SkDebugf("unsupported operation: drawSprite()\n");
}

void SkSVGDevice::drawBitmapRect(const SkDraw&, const SkBitmap&, const SkRect* srcOrNull,
                                 const SkRect& dst, const SkPaint& paint,
                                 SkCanvas::DrawBitmapRectFlags flags) {
    // todo
    SkDebugf("unsupported operation: drawBitmapRect()\n");
}

void SkSVGDevice::drawText(const SkDraw& draw, const void* text, size_t len,
                           SkScalar x, SkScalar y, const SkPaint& paint) {
    AutoElement elem("text", fWriter, fResourceBucket, draw, paint);
    elem.addFontAttributes(paint);
    elem.addAttribute("x", x);
    elem.addAttribute("y", y);
    elem.addText(svg_text(text, len, paint));
}

void SkSVGDevice::drawPosText(const SkDraw& draw, const void* text, size_t len,
                              const SkScalar pos[], int scalarsPerPos, const SkPoint& offset,
                              const SkPaint& paint) {
    SkASSERT(scalarsPerPos == 1 || scalarsPerPos == 2);

    AutoElement elem("text", fWriter, fResourceBucket, draw, paint);
    elem.addFontAttributes(paint);

    SkString xStr;
    SkString yStr;
    for (int i = 0; i < paint.countText(text, len); ++i) {
        xStr.appendf("%.8g, ", offset.x() + pos[i * scalarsPerPos]);

        if (scalarsPerPos == 2) {
            yStr.appendf("%.8g, ", offset.y() + pos[i * scalarsPerPos + 1]);
        }
    }

    if (scalarsPerPos != 2) {
        yStr.appendScalar(offset.y());
    }

    elem.addAttribute("x", xStr);
    elem.addAttribute("y", yStr);
    elem.addText(svg_text(text, len, paint));
}

void SkSVGDevice::drawTextOnPath(const SkDraw&, const void* text, size_t len, const SkPath& path,
                                 const SkMatrix* matrix, const SkPaint& paint) {
    // todo
    SkDebugf("unsupported operation: drawTextOnPath()\n");
}

void SkSVGDevice::drawVertices(const SkDraw&, SkCanvas::VertexMode, int vertexCount,
                               const SkPoint verts[], const SkPoint texs[],
                               const SkColor colors[], SkXfermode* xmode,
                               const uint16_t indices[], int indexCount,
                               const SkPaint& paint) {
    // todo
    SkDebugf("unsupported operation: drawVertices()\n");
}

void SkSVGDevice::drawDevice(const SkDraw&, SkBaseDevice*, int x, int y,
                             const SkPaint&) {
    // todo
    SkDebugf("unsupported operation: drawDevice()\n");
}
