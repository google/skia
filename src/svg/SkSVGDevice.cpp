/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/svg/SkSVGDevice.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageEncoder.h"
#include "include/core/SkPaint.h"
#include "include/core/SkShader.h"
#include "include/core/SkStream.h"
#include "include/core/SkTypeface.h"
#include "include/private/SkChecksum.h"
#include "include/private/SkTHash.h"
#include "include/private/SkTo.h"
#include "include/svg/SkSVGCanvas.h"
#include "include/utils/SkBase64.h"
#include "include/utils/SkParsePath.h"
#include "src/codec/SkJpegCodec.h"
#include "src/codec/SkPngCodec.h"
#include "src/core/SkAnnotationKeys.h"
#include "src/core/SkClipOpPriv.h"
#include "src/core/SkClipStack.h"
#include "src/core/SkDraw.h"
#include "src/core/SkFontPriv.h"
#include "src/core/SkUtils.h"
#include "src/shaders/SkShaderBase.h"
#include "src/xml/SkXMLWriter.h"

namespace {

static SkString svg_color(SkColor color) {
    // https://www.w3.org/TR/css-color-3/#html4
    auto named_color = [](SkColor c) -> const char* {
        switch (c & 0xffffff) {
        case 0x000000: return "black";
        case 0x000080: return "navy";
        case 0x0000ff: return "blue";
        case 0x008000: return "green";
        case 0x008080: return "teal";
        case 0x00ff00: return "lime";
        case 0x00ffff: return "aqua";
        case 0x800000: return "maroon";
        case 0x800080: return "purple";
        case 0x808000: return "olive";
        case 0x808080: return "gray";
        case 0xc0c0c0: return "silver";
        case 0xff0000: return "red";
        case 0xff00ff: return "fuchsia";
        case 0xffff00: return "yellow";
        case 0xffffff: return "white";
        default: break;
        }

        return nullptr;
    };

    if (const auto* nc = named_color(color)) {
        return SkString(nc);
    }

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
    nullptr,    // kButt_Cap (default)
    "round", // kRound_Cap
    "square" // kSquare_Cap
};
static_assert(SK_ARRAY_COUNT(cap_map) == SkPaint::kCapCount, "missing_cap_map_entry");

static const char* svg_cap(SkPaint::Cap cap) {
    SkASSERT(cap < SK_ARRAY_COUNT(cap_map));
    return cap_map[cap];
}

// Keep in sync with SkPaint::Join
static const char* join_map[] = {
    nullptr,    // kMiter_Join (default)
    "round", // kRound_Join
    "bevel"  // kBevel_Join
};
static_assert(SK_ARRAY_COUNT(join_map) == SkPaint::kJoinCount, "missing_join_map_entry");

static const char* svg_join(SkPaint::Join join) {
    SkASSERT(join < SK_ARRAY_COUNT(join_map));
    return join_map[join];
}

static SkString svg_transform(const SkMatrix& t) {
    SkASSERT(!t.isIdentity());

    SkString tstr;
    switch (t.getType()) {
    case SkMatrix::kPerspective_Mask:
        // TODO: handle perspective matrices?
        break;
    case SkMatrix::kTranslate_Mask:
        tstr.printf("translate(%g %g)", t.getTranslateX(), t.getTranslateY());
        break;
    case SkMatrix::kScale_Mask:
        tstr.printf("scale(%g %g)", t.getScaleX(), t.getScaleY());
        break;
    default:
        // http://www.w3.org/TR/SVG/coords.html#TransformMatrixDefined
        //    | a c e |
        //    | b d f |
        //    | 0 0 1 |
        tstr.printf("matrix(%g %g %g %g %g %g)",
                    t.getScaleX(),     t.getSkewY(),
                    t.getSkewX(),      t.getScaleY(),
                    t.getTranslateX(), t.getTranslateY());
        break;
    }

    return tstr;
}

struct Resources {
    Resources(const SkPaint& paint)
        : fPaintServer(svg_color(paint.getColor())) {}

    SkString fPaintServer;
    SkString fClip;
    SkString fColorFilter;
};

// Determine if the paint requires us to reset the viewport.
// Currently, we do this whenever the paint shader calls
// for a repeating image.
bool RequiresViewportReset(const SkPaint& paint) {
  SkShader* shader = paint.getShader();
  if (!shader)
    return false;

  SkTileMode xy[2];
  SkImage* image = shader->isAImage(nullptr, xy);

  if (!image)
    return false;

  for (int i = 0; i < 2; i++) {
    if (xy[i] == SkTileMode::kRepeat)
      return true;
  }
  return false;
}

SkPath GetPath(const SkGlyphRun& glyphRun, const SkPoint& offset) {
    SkPath path;

    struct Rec {
        SkPath*        fPath;
        const SkPoint  fOffset;
        const SkPoint* fPos;
    } rec = { &path, offset, glyphRun.positions().data() };

    glyphRun.font().getPaths(glyphRun.glyphsIDs().data(), SkToInt(glyphRun.glyphsIDs().size()),
            [](const SkPath* path, const SkMatrix& mx, void* ctx) {
                Rec* rec = reinterpret_cast<Rec*>(ctx);
                if (path) {
                    SkMatrix total = mx;
                    total.postTranslate(rec->fPos->fX + rec->fOffset.fX,
                                        rec->fPos->fY + rec->fOffset.fY);
                    rec->fPath->addPath(*path, total);
                } else {
                    // TODO: this is going to drop color emojis.
                }
                rec->fPos += 1; // move to the next glyph's position
            }, &rec);

    return path;
}

}  // namespace

// For now all this does is serve unique serial IDs, but it will eventually evolve to track
// and deduplicate resources.
class SkSVGDevice::ResourceBucket : ::SkNoncopyable {
public:
    ResourceBucket()
            : fGradientCount(0)
            , fClipCount(0)
            , fPathCount(0)
            , fImageCount(0)
            , fPatternCount(0)
            , fColorFilterCount(0) {}

    SkString addLinearGradient() {
        return SkStringPrintf("gradient_%d", fGradientCount++);
    }

    SkString addClip() {
        return SkStringPrintf("clip_%d", fClipCount++);
    }

    SkString addPath() {
        return SkStringPrintf("path_%d", fPathCount++);
    }

    SkString addImage() {
        return SkStringPrintf("img_%d", fImageCount++);
    }

    SkString addColorFilter() { return SkStringPrintf("cfilter_%d", fColorFilterCount++); }

    SkString addPattern() {
      return SkStringPrintf("pattern_%d", fPatternCount++);
    }

private:
    uint32_t fGradientCount;
    uint32_t fClipCount;
    uint32_t fPathCount;
    uint32_t fImageCount;
    uint32_t fPatternCount;
    uint32_t fColorFilterCount;
};

struct SkSVGDevice::MxCp {
    const SkMatrix* fMatrix;
    const SkClipStack*  fClipStack;

    MxCp(const SkMatrix* mx, const SkClipStack* cs) : fMatrix(mx), fClipStack(cs) {}
    MxCp(SkSVGDevice* device) : fMatrix(&device->ctm()), fClipStack(&device->cs()) {}
};

class SkSVGDevice::AutoElement : ::SkNoncopyable {
public:
    AutoElement(const char name[], SkXMLWriter* writer)
        : fWriter(writer)
        , fResourceBucket(nullptr) {
        fWriter->startElement(name);
    }

    AutoElement(const char name[], const std::unique_ptr<SkXMLWriter>& writer)
        : AutoElement(name, writer.get()) {}

    AutoElement(const char name[], const std::unique_ptr<SkXMLWriter>& writer,
                ResourceBucket* bucket, const MxCp& mc, const SkPaint& paint)
        : fWriter(writer.get())
        , fResourceBucket(bucket) {

        Resources res = this->addResources(mc, paint);

        if (!res.fClip.isEmpty()) {
            // The clip is in device space. Apply it via a <g> wrapper to avoid local transform
            // interference.
            fClipGroup.reset(new AutoElement("g", fWriter));
            fClipGroup->addAttribute("clip-path",res.fClip);
        }

        fWriter->startElement(name);

        this->addPaint(paint, res);

        if (!mc.fMatrix->isIdentity()) {
            this->addAttribute("transform", svg_transform(*mc.fMatrix));
        }
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
        fWriter->addText(text.c_str(), text.size());
    }

    void addRectAttributes(const SkRect&);
    void addPathAttributes(const SkPath&);
    void addTextAttributes(const SkFont&);

private:
    Resources addResources(const MxCp&, const SkPaint& paint);
    void addClipResources(const MxCp&, Resources* resources);
    void addShaderResources(const SkPaint& paint, Resources* resources);
    void addGradientShaderResources(const SkShader* shader, const SkPaint& paint,
                                    Resources* resources);
    void addColorFilterResources(const SkColorFilter& cf, Resources* resources);
    void addImageShaderResources(const SkShader* shader, const SkPaint& paint,
                                 Resources* resources);

    void addPatternDef(const SkBitmap& bm);

    void addPaint(const SkPaint& paint, const Resources& resources);


    SkString addLinearGradientDef(const SkShader::GradientInfo& info, const SkShader* shader);

    SkXMLWriter*               fWriter;
    ResourceBucket*            fResourceBucket;
    std::unique_ptr<AutoElement> fClipGroup;
};

void SkSVGDevice::AutoElement::addPaint(const SkPaint& paint, const Resources& resources) {
    SkPaint::Style style = paint.getStyle();
    if (style == SkPaint::kFill_Style || style == SkPaint::kStrokeAndFill_Style) {
        static constexpr char kDefaultFill[] = "black";
        if (!resources.fPaintServer.equals(kDefaultFill)) {
            this->addAttribute("fill", resources.fPaintServer);

            if (SK_AlphaOPAQUE != SkColorGetA(paint.getColor())) {
                this->addAttribute("fill-opacity", svg_opacity(paint.getColor()));
            }
        }
    } else {
        SkASSERT(style == SkPaint::kStroke_Style);
        this->addAttribute("fill", "none");
    }

    if (!resources.fColorFilter.isEmpty()) {
        this->addAttribute("filter", resources.fColorFilter.c_str());
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
        // SVG default stroke value is "none".
    }
}

Resources SkSVGDevice::AutoElement::addResources(const MxCp& mc, const SkPaint& paint) {
    Resources resources(paint);

    // FIXME: this is a weak heuristic and we end up with LOTS of redundant clips.
    bool hasClip   = !mc.fClipStack->isWideOpen();
    bool hasShader = SkToBool(paint.getShader());

    if (hasClip || hasShader) {
        AutoElement defs("defs", fWriter);

        if (hasClip) {
            this->addClipResources(mc, &resources);
        }

        if (hasShader) {
            this->addShaderResources(paint, &resources);
        }
    }

    if (const SkColorFilter* cf = paint.getColorFilter()) {
        // TODO: Implement skia color filters for blend modes other than SrcIn
        SkBlendMode mode;
        if (cf->asAColorMode(nullptr, &mode) && mode == SkBlendMode::kSrcIn) {
            this->addColorFilterResources(*cf, &resources);
        }
    }
    return resources;
}

void SkSVGDevice::AutoElement::addGradientShaderResources(const SkShader* shader,
                                                          const SkPaint& paint,
                                                          Resources* resources) {
    SkShader::GradientInfo grInfo;
    grInfo.fColorCount = 0;
    if (SkShader::kLinear_GradientType != shader->asAGradient(&grInfo)) {
        // TODO: non-linear gradient support
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

void SkSVGDevice::AutoElement::addColorFilterResources(const SkColorFilter& cf,
                                                       Resources* resources) {
    SkString colorfilterID = fResourceBucket->addColorFilter();
    {
        AutoElement filterElement("filter", fWriter);
        filterElement.addAttribute("id", colorfilterID);
        filterElement.addAttribute("x", "0%");
        filterElement.addAttribute("y", "0%");
        filterElement.addAttribute("width", "100%");
        filterElement.addAttribute("height", "100%");

        SkColor filterColor;
        SkBlendMode mode;
        bool asAColorMode = cf.asAColorMode(&filterColor, &mode);
        SkAssertResult(asAColorMode);
        SkASSERT(mode == SkBlendMode::kSrcIn);

        {
            // first flood with filter color
            AutoElement floodElement("feFlood", fWriter);
            floodElement.addAttribute("flood-color", svg_color(filterColor));
            floodElement.addAttribute("flood-opacity", svg_opacity(filterColor));
            floodElement.addAttribute("result", "flood");
        }

        {
            // apply the transform to filter color
            AutoElement compositeElement("feComposite", fWriter);
            compositeElement.addAttribute("in", "flood");
            compositeElement.addAttribute("operator", "in");
        }
    }
    resources->fColorFilter.printf("url(#%s)", colorfilterID.c_str());
}

// Returns data uri from bytes.
// it will use any cached data if available, otherwise will
// encode as png.
sk_sp<SkData> AsDataUri(SkImage* image) {
    sk_sp<SkData> imageData = image->encodeToData();
    if (!imageData) {
        return nullptr;
    }

    const char* src = (char*)imageData->data();
    const char* selectedPrefix = nullptr;
    size_t selectedPrefixLength = 0;

    const static char pngDataPrefix[] = "data:image/png;base64,";
    const static char jpgDataPrefix[] = "data:image/jpeg;base64,";

    if (SkJpegCodec::IsJpeg(src, imageData->size())) {
        selectedPrefix = jpgDataPrefix;
        selectedPrefixLength = sizeof(jpgDataPrefix);
    } else {
      if (!SkPngCodec::IsPng(src, imageData->size())) {
        imageData = image->encodeToData(SkEncodedImageFormat::kPNG, 100);
      }
      selectedPrefix = pngDataPrefix;
      selectedPrefixLength = sizeof(pngDataPrefix);
    }

    size_t b64Size = SkBase64::Encode(imageData->data(), imageData->size(), nullptr);
    sk_sp<SkData> dataUri = SkData::MakeUninitialized(selectedPrefixLength + b64Size);
    char* dest = (char*)dataUri->writable_data();
    memcpy(dest, selectedPrefix, selectedPrefixLength);
    SkBase64::Encode(imageData->data(), imageData->size(), dest + selectedPrefixLength - 1);
    dest[dataUri->size() - 1] = 0;
    return dataUri;
}

void SkSVGDevice::AutoElement::addImageShaderResources(const SkShader* shader, const SkPaint& paint,
                                                       Resources* resources) {
    SkMatrix outMatrix;

    SkTileMode xy[2];
    SkImage* image = shader->isAImage(&outMatrix, xy);
    SkASSERT(image);

    SkString patternDims[2];  // width, height

    sk_sp<SkData> dataUri = AsDataUri(image);
    if (!dataUri) {
        return;
    }
    SkIRect imageSize = image->bounds();
    for (int i = 0; i < 2; i++) {
        int imageDimension = i == 0 ? imageSize.width() : imageSize.height();
        switch (xy[i]) {
            case SkTileMode::kRepeat:
                patternDims[i].appendScalar(imageDimension);
            break;
            default:
                // TODO: other tile modes?
                patternDims[i] = "100%";
        }
    }

    SkString patternID = fResourceBucket->addPattern();
    {
        AutoElement pattern("pattern", fWriter);
        pattern.addAttribute("id", patternID);
        pattern.addAttribute("patternUnits", "userSpaceOnUse");
        pattern.addAttribute("patternContentUnits", "userSpaceOnUse");
        pattern.addAttribute("width", patternDims[0]);
        pattern.addAttribute("height", patternDims[1]);
        pattern.addAttribute("x", 0);
        pattern.addAttribute("y", 0);

        {
            SkString imageID = fResourceBucket->addImage();
            AutoElement imageTag("image", fWriter);
            imageTag.addAttribute("id", imageID);
            imageTag.addAttribute("x", 0);
            imageTag.addAttribute("y", 0);
            imageTag.addAttribute("width", image->width());
            imageTag.addAttribute("height", image->height());
            imageTag.addAttribute("xlink:href", static_cast<const char*>(dataUri->data()));
        }
    }
    resources->fPaintServer.printf("url(#%s)", patternID.c_str());
}

void SkSVGDevice::AutoElement::addShaderResources(const SkPaint& paint, Resources* resources) {
    const SkShader* shader = paint.getShader();
    SkASSERT(shader);

    if (shader->asAGradient(nullptr) != SkShader::kNone_GradientType) {
        this->addGradientShaderResources(shader, paint, resources);
    } else if (shader->isAImage()) {
        this->addImageShaderResources(shader, paint, resources);
    }
    // TODO: other shader types?
}

void SkSVGDevice::AutoElement::addClipResources(const MxCp& mc, Resources* resources) {
    SkASSERT(!mc.fClipStack->isWideOpen());

    SkPath clipPath;
    (void) mc.fClipStack->asPath(&clipPath);

    SkString clipID = fResourceBucket->addClip();
    {
        // clipPath is in device space, but since we're only pushing transform attributes
        // to the leaf nodes, so are all our elements => SVG userSpaceOnUse == device space.
        AutoElement clipPathElement("clipPath", fWriter);
        clipPathElement.addAttribute("id", clipID);

        SkRect clipRect = SkRect::MakeEmpty();
        if (clipPath.isEmpty() || clipPath.isRect(&clipRect)) {
            AutoElement rectElement("rect", fWriter);
            rectElement.addRectAttributes(clipRect);
        } else {
            AutoElement pathElement("path", fWriter);
            pathElement.addPathAttributes(clipPath);

            if (clipPath.getFillType() == SkPath::kEvenOdd_FillType) {
                pathElement.addAttribute("clip-rule", "evenodd"); // Default: nonzero
            }
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

        if (!as_SB(shader)->getLocalMatrix().isIdentity()) {
            this->addAttribute("gradientTransform", svg_transform(as_SB(shader)->getLocalMatrix()));
        }

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

void SkSVGDevice::AutoElement::addPathAttributes(const SkPath& path) {
    SkString pathData;
    SkParsePath::ToSVGString(path, &pathData);
    this->addAttribute("d", pathData);
}

void SkSVGDevice::AutoElement::addTextAttributes(const SkFont& font) {
    this->addAttribute("font-size", font.getSize());

    SkString familyName;
    SkTHashSet<SkString> familySet;
    sk_sp<SkTypeface> tface = font.refTypefaceOrDefault();

    SkASSERT(tface);
    SkFontStyle style = tface->fontStyle();
    if (style.slant() == SkFontStyle::kItalic_Slant) {
        this->addAttribute("font-style", "italic");
    } else if (style.slant() == SkFontStyle::kOblique_Slant) {
        this->addAttribute("font-style", "oblique");
    }
    int weightIndex = (SkTPin(style.weight(), 100, 900) - 50) / 100;
    if (weightIndex != 3) {
        static constexpr const char* weights[] = {
            "100", "200", "300", "normal", "400", "500", "600", "bold", "800", "900"
        };
        this->addAttribute("font-weight", weights[weightIndex]);
    }
    int stretchIndex = style.width() - 1;
    if (stretchIndex != 4) {
        static constexpr const char* stretches[] = {
            "ultra-condensed", "extra-condensed", "condensed", "semi-condensed",
            "normal",
            "semi-expanded", "expanded", "extra-expanded", "ultra-expanded"
        };
        this->addAttribute("font-stretch", stretches[stretchIndex]);
    }

    sk_sp<SkTypeface::LocalizedStrings> familyNameIter(tface->createFamilyNameIterator());
    SkTypeface::LocalizedString familyString;
    if (familyNameIter) {
        while (familyNameIter->next(&familyString)) {
            if (familySet.contains(familyString.fString)) {
                continue;
            }
            familySet.add(familyString.fString);
            familyName.appendf((familyName.isEmpty() ? "%s" : ", %s"), familyString.fString.c_str());
        }
    }
    if (!familyName.isEmpty()) {
        this->addAttribute("font-family", familyName);
    }
}

sk_sp<SkBaseDevice> SkSVGDevice::Make(const SkISize& size, std::unique_ptr<SkXMLWriter> writer,
                                      uint32_t flags) {
    return writer ? sk_sp<SkBaseDevice>(new SkSVGDevice(size, std::move(writer), flags))
                  : nullptr;
}

SkSVGDevice::SkSVGDevice(const SkISize& size, std::unique_ptr<SkXMLWriter> writer, uint32_t flags)
    : INHERITED(SkImageInfo::MakeUnknown(size.fWidth, size.fHeight),
                SkSurfaceProps(0, kUnknown_SkPixelGeometry))
    , fWriter(std::move(writer))
    , fResourceBucket(new ResourceBucket)
    , fFlags(flags)
{
    SkASSERT(fWriter);

    fWriter->writeHeader();

    // The root <svg> tag gets closed by the destructor.
    fRootElement.reset(new AutoElement("svg", fWriter));

    fRootElement->addAttribute("xmlns", "http://www.w3.org/2000/svg");
    fRootElement->addAttribute("xmlns:xlink", "http://www.w3.org/1999/xlink");
    fRootElement->addAttribute("width", size.width());
    fRootElement->addAttribute("height", size.height());
}

SkSVGDevice::~SkSVGDevice() = default;

void SkSVGDevice::drawPaint(const SkPaint& paint) {
    AutoElement rect("rect", fWriter, fResourceBucket.get(), MxCp(this), paint);
    rect.addRectAttributes(SkRect::MakeWH(SkIntToScalar(this->width()),
                                          SkIntToScalar(this->height())));
}

void SkSVGDevice::drawAnnotation(const SkRect& rect, const char key[], SkData* value) {
    if (!value) {
        return;
    }

    if (!strcmp(SkAnnotationKeys::URL_Key(), key) ||
        !strcmp(SkAnnotationKeys::Link_Named_Dest_Key(), key)) {
        this->cs().save();
        this->cs().clipRect(rect, this->ctm(), kIntersect_SkClipOp, true);
        SkRect transformedRect = this->cs().bounds(this->getGlobalBounds());
        this->cs().restore();
        if (transformedRect.isEmpty()) {
            return;
        }

        SkString url(static_cast<const char*>(value->data()), value->size() - 1);
        AutoElement a("a", fWriter);
        a.addAttribute("xlink:href", url.c_str());
        {
            AutoElement r("rect", fWriter);
            r.addAttribute("fill-opacity", "0.0");
            r.addRectAttributes(transformedRect);
        }
    }
}

void SkSVGDevice::drawPoints(SkCanvas::PointMode mode, size_t count,
                             const SkPoint pts[], const SkPaint& paint) {
    SkPath path;

    switch (mode) {
            // todo
        case SkCanvas::kPoints_PointMode:
            // TODO?
            break;
        case SkCanvas::kLines_PointMode:
            count -= 1;
            for (size_t i = 0; i < count; i += 2) {
                path.rewind();
                path.moveTo(pts[i]);
                path.lineTo(pts[i+1]);
                AutoElement elem("path", fWriter, fResourceBucket.get(), MxCp(this), paint);
                elem.addPathAttributes(path);
            }
            break;
        case SkCanvas::kPolygon_PointMode:
            if (count > 1) {
                path.addPoly(pts, SkToInt(count), false);
                path.moveTo(pts[0]);
                AutoElement elem("path", fWriter, fResourceBucket.get(), MxCp(this), paint);
                elem.addPathAttributes(path);
            }
            break;
    }
}

void SkSVGDevice::drawRect(const SkRect& r, const SkPaint& paint) {
    std::unique_ptr<AutoElement> svg;
    if (RequiresViewportReset(paint)) {
      svg.reset(new AutoElement("svg", fWriter, fResourceBucket.get(), MxCp(this), paint));
      svg->addRectAttributes(r);
    }

    AutoElement rect("rect", fWriter, fResourceBucket.get(), MxCp(this), paint);

    if (svg) {
      rect.addAttribute("x", 0);
      rect.addAttribute("y", 0);
      rect.addAttribute("width", "100%");
      rect.addAttribute("height", "100%");
    } else {
      rect.addRectAttributes(r);
    }
}

void SkSVGDevice::drawOval(const SkRect& oval, const SkPaint& paint) {
    AutoElement ellipse("ellipse", fWriter, fResourceBucket.get(), MxCp(this), paint);
    ellipse.addAttribute("cx", oval.centerX());
    ellipse.addAttribute("cy", oval.centerY());
    ellipse.addAttribute("rx", oval.width() / 2);
    ellipse.addAttribute("ry", oval.height() / 2);
}

void SkSVGDevice::drawRRect(const SkRRect& rr, const SkPaint& paint) {
    SkPath path;
    path.addRRect(rr);

    AutoElement elem("path", fWriter, fResourceBucket.get(), MxCp(this), paint);
    elem.addPathAttributes(path);
}

void SkSVGDevice::drawPath(const SkPath& path, const SkPaint& paint, bool pathIsMutable) {
    AutoElement elem("path", fWriter, fResourceBucket.get(), MxCp(this), paint);
    elem.addPathAttributes(path);

    // TODO: inverse fill types?
    if (path.getFillType() == SkPath::kEvenOdd_FillType) {
        elem.addAttribute("fill-rule", "evenodd");
    }
}

static sk_sp<SkData> encode(const SkBitmap& src) {
    SkDynamicMemoryWStream buf;
    return SkEncodeImage(&buf, src, SkEncodedImageFormat::kPNG, 80) ? buf.detachAsData() : nullptr;
}

void SkSVGDevice::drawBitmapCommon(const MxCp& mc, const SkBitmap& bm, const SkPaint& paint) {
    sk_sp<SkData> pngData = encode(bm);
    if (!pngData) {
        return;
    }

    size_t b64Size = SkBase64::Encode(pngData->data(), pngData->size(), nullptr);
    SkAutoTMalloc<char> b64Data(b64Size);
    SkBase64::Encode(pngData->data(), pngData->size(), b64Data.get());

    SkString svgImageData("data:image/png;base64,");
    svgImageData.append(b64Data.get(), b64Size);

    SkString imageID = fResourceBucket->addImage();
    {
        AutoElement defs("defs", fWriter);
        {
            AutoElement image("image", fWriter);
            image.addAttribute("id", imageID);
            image.addAttribute("width", bm.width());
            image.addAttribute("height", bm.height());
            image.addAttribute("xlink:href", svgImageData);
        }
    }

    {
        AutoElement imageUse("use", fWriter, fResourceBucket.get(), mc, paint);
        imageUse.addAttribute("xlink:href", SkStringPrintf("#%s", imageID.c_str()));
    }
}

void SkSVGDevice::drawSprite(const SkBitmap& bitmap,
                             int x, int y, const SkPaint& paint) {
    MxCp mc(this);
    SkMatrix adjustedMatrix = *mc.fMatrix;
    adjustedMatrix.preTranslate(SkIntToScalar(x), SkIntToScalar(y));
    mc.fMatrix = &adjustedMatrix;

    drawBitmapCommon(mc, bitmap, paint);
}

void SkSVGDevice::drawBitmapRect(const SkBitmap& bm, const SkRect* srcOrNull,
                                 const SkRect& dst, const SkPaint& paint,
                                 SkCanvas::SrcRectConstraint) {
    SkClipStack* cs = &this->cs();
    SkClipStack::AutoRestore ar(cs, false);
    if (srcOrNull && *srcOrNull != SkRect::Make(bm.bounds())) {
        cs->save();
        cs->clipRect(dst, this->ctm(), kIntersect_SkClipOp, paint.isAntiAlias());
    }

    SkMatrix adjustedMatrix;
    adjustedMatrix.setRectToRect(srcOrNull ? *srcOrNull : SkRect::Make(bm.bounds()),
                                 dst,
                                 SkMatrix::kFill_ScaleToFit);
    adjustedMatrix.postConcat(this->ctm());

    drawBitmapCommon(MxCp(&adjustedMatrix, cs), bm, paint);
}

class SVGTextBuilder : SkNoncopyable {
public:
    SVGTextBuilder(SkPoint origin, const SkGlyphRun& glyphRun)
            : fOrigin(origin) {
        auto runSize = glyphRun.runSize();
        SkAutoSTArray<64, SkUnichar> unichars(runSize);
        SkFontPriv::GlyphsToUnichars(glyphRun.font(), glyphRun.glyphsIDs().data(),
                                     runSize, unichars.get());
        auto positions = glyphRun.positions();
        for (size_t i = 0; i < runSize; ++i) {
            this->appendUnichar(unichars[i], positions[i]);
        }
    }

    const SkString& text() const { return fText; }
    const SkString& posX() const { return fPosXStr; }
    const SkString& posY() const { return fHasConstY ? fConstYStr : fPosYStr; }

private:
    void appendUnichar(SkUnichar c, SkPoint position) {
        bool discardPos = false;
        bool isWhitespace = false;

        switch(c) {
            case ' ':
            case '\t':
                // consolidate whitespace to match SVG's xml:space=default munging
                // (http://www.w3.org/TR/SVG/text.html#WhiteSpace)
                if (fLastCharWasWhitespace) {
                    discardPos = true;
                } else {
                    fText.appendUnichar(c);
                }
                isWhitespace = true;
                break;
            case '\0':
                // SkPaint::glyphsToUnichars() returns \0 for inconvertible glyphs, but these
                // are not legal XML characters (http://www.w3.org/TR/REC-xml/#charsets)
                discardPos = true;
                isWhitespace = fLastCharWasWhitespace; // preserve whitespace consolidation
                break;
            case '&':
                fText.append("&amp;");
                break;
            case '"':
                fText.append("&quot;");
                break;
            case '\'':
                fText.append("&apos;");
                break;
            case '<':
                fText.append("&lt;");
                break;
            case '>':
                fText.append("&gt;");
                break;
            default:
                fText.appendUnichar(c);
                break;
        }

        fLastCharWasWhitespace = isWhitespace;

        if (discardPos) {
            return;
        }

        position += fOrigin;
        fPosXStr.appendf("%.8g, ", position.fX);
        fPosYStr.appendf("%.8g, ", position.fY);

        if (fConstYStr.isEmpty()) {
            fConstYStr = fPosYStr;
            fConstY    = position.fY;
        } else {
            fHasConstY &= SkScalarNearlyEqual(fConstY, position.fY);
        }
    }

    const SkPoint   fOrigin;

    SkString fText,
             fPosXStr, fPosYStr,
             fConstYStr;
    SkScalar fConstY;
    bool     fLastCharWasWhitespace = true, // start off in whitespace mode to strip leading space
             fHasConstY             = true;
};

void SkSVGDevice::drawGlyphRunAsPath(const SkGlyphRun& glyphRun, const SkPoint& origin,
                                     const SkPaint& runPaint) {
    this->drawPath(GetPath(glyphRun, origin), runPaint);
}

void SkSVGDevice::drawGlyphRunAsText(const SkGlyphRun& glyphRun, const SkPoint& origin,
                                     const SkPaint& runPaint) {
    AutoElement elem("text", fWriter, fResourceBucket.get(), MxCp(this), runPaint);
    elem.addTextAttributes(glyphRun.font());

    SVGTextBuilder builder(origin, glyphRun);
    elem.addAttribute("x", builder.posX());
    elem.addAttribute("y", builder.posY());
    elem.addText(builder.text());
}

void SkSVGDevice::drawGlyphRunList(const SkGlyphRunList& glyphRunList)  {
    const auto processGlyphRun = (fFlags & SkSVGCanvas::kConvertTextToPaths_Flag)
            ? &SkSVGDevice::drawGlyphRunAsPath
            : &SkSVGDevice::drawGlyphRunAsText;

    for (auto& glyphRun : glyphRunList) {
        (this->*processGlyphRun)(glyphRun, glyphRunList.origin(), glyphRunList.paint());
    }
}

void SkSVGDevice::drawVertices(const SkVertices*, const SkVertices::Bone[], int, SkBlendMode,
                               const SkPaint&) {
    // todo
}

void SkSVGDevice::drawDevice(SkBaseDevice*, int x, int y,
                             const SkPaint&) {
    // todo
}
