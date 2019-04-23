/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/svg/model/SkSVGAttributeParser.h"
#include "experimental/svg/model/SkSVGCircle.h"
#include "experimental/svg/model/SkSVGClipPath.h"
#include "experimental/svg/model/SkSVGDOM.h"
#include "experimental/svg/model/SkSVGDefs.h"
#include "experimental/svg/model/SkSVGEllipse.h"
#include "experimental/svg/model/SkSVGG.h"
#include "experimental/svg/model/SkSVGLine.h"
#include "experimental/svg/model/SkSVGLinearGradient.h"
#include "experimental/svg/model/SkSVGNode.h"
#include "experimental/svg/model/SkSVGPath.h"
#include "experimental/svg/model/SkSVGPattern.h"
#include "experimental/svg/model/SkSVGPoly.h"
#include "experimental/svg/model/SkSVGRadialGradient.h"
#include "experimental/svg/model/SkSVGRect.h"
#include "experimental/svg/model/SkSVGRenderContext.h"
#include "experimental/svg/model/SkSVGSVG.h"
#include "experimental/svg/model/SkSVGStop.h"
#include "experimental/svg/model/SkSVGTypes.h"
#include "experimental/svg/model/SkSVGUse.h"
#include "experimental/svg/model/SkSVGValue.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkString.h"
#include "include/private/SkTSearch.h"
#include "include/private/SkTo.h"
#include "include/utils/SkParsePath.h"
#include "src/xml/SkDOM.h"

namespace {

bool SetPaintAttribute(const sk_sp<SkSVGNode>& node, SkSVGAttribute attr,
                       const char* stringValue) {
    SkSVGPaint paint;
    SkSVGAttributeParser parser(stringValue);
    if (!parser.parsePaint(&paint)) {
        return false;
    }

    node->setAttribute(attr, SkSVGPaintValue(paint));
    return true;
}

bool SetColorAttribute(const sk_sp<SkSVGNode>& node, SkSVGAttribute attr,
                       const char* stringValue) {
    SkSVGColorType color;
    SkSVGAttributeParser parser(stringValue);
    if (!parser.parseColor(&color)) {
        return false;
    }

    node->setAttribute(attr, SkSVGColorValue(color));
    return true;
}

bool SetIRIAttribute(const sk_sp<SkSVGNode>& node, SkSVGAttribute attr,
                      const char* stringValue) {
    SkSVGStringType iri;
    SkSVGAttributeParser parser(stringValue);
    if (!parser.parseIRI(&iri)) {
        return false;
    }

    node->setAttribute(attr, SkSVGStringValue(iri));
    return true;
}

bool SetClipPathAttribute(const sk_sp<SkSVGNode>& node, SkSVGAttribute attr,
                          const char* stringValue) {
    SkSVGClip clip;
    SkSVGAttributeParser parser(stringValue);
    if (!parser.parseClipPath(&clip)) {
        return false;
    }

    node->setAttribute(attr, SkSVGClipValue(clip));
    return true;
}


bool SetPathDataAttribute(const sk_sp<SkSVGNode>& node, SkSVGAttribute attr,
                          const char* stringValue) {
    SkPath path;
    if (!SkParsePath::FromSVGString(stringValue, &path)) {
        return false;
    }

    node->setAttribute(attr, SkSVGPathValue(path));
    return true;
}

bool SetTransformAttribute(const sk_sp<SkSVGNode>& node, SkSVGAttribute attr,
                           const char* stringValue) {
    SkSVGTransformType transform;
    SkSVGAttributeParser parser(stringValue);
    if (!parser.parseTransform(&transform)) {
        return false;
    }

    node->setAttribute(attr, SkSVGTransformValue(transform));
    return true;
}

bool SetLengthAttribute(const sk_sp<SkSVGNode>& node, SkSVGAttribute attr,
                        const char* stringValue) {
    SkSVGLength length;
    SkSVGAttributeParser parser(stringValue);
    if (!parser.parseLength(&length)) {
        return false;
    }

    node->setAttribute(attr, SkSVGLengthValue(length));
    return true;
}

bool SetNumberAttribute(const sk_sp<SkSVGNode>& node, SkSVGAttribute attr,
                        const char* stringValue) {
    SkSVGNumberType number;
    SkSVGAttributeParser parser(stringValue);
    if (!parser.parseNumber(&number)) {
        return false;
    }

    node->setAttribute(attr, SkSVGNumberValue(number));
    return true;
}

bool SetViewBoxAttribute(const sk_sp<SkSVGNode>& node, SkSVGAttribute attr,
                         const char* stringValue) {
    SkSVGViewBoxType viewBox;
    SkSVGAttributeParser parser(stringValue);
    if (!parser.parseViewBox(&viewBox)) {
        return false;
    }

    node->setAttribute(attr, SkSVGViewBoxValue(viewBox));
    return true;
}

bool SetLineCapAttribute(const sk_sp<SkSVGNode>& node, SkSVGAttribute attr,
                         const char* stringValue) {
    SkSVGLineCap lineCap;
    SkSVGAttributeParser parser(stringValue);
    if (!parser.parseLineCap(&lineCap)) {
        return false;
    }

    node->setAttribute(attr, SkSVGLineCapValue(lineCap));
    return true;
}

bool SetLineJoinAttribute(const sk_sp<SkSVGNode>& node, SkSVGAttribute attr,
                          const char* stringValue) {
    SkSVGLineJoin lineJoin;
    SkSVGAttributeParser parser(stringValue);
    if (!parser.parseLineJoin(&lineJoin)) {
        return false;
    }

    node->setAttribute(attr, SkSVGLineJoinValue(lineJoin));
    return true;
}

bool SetSpreadMethodAttribute(const sk_sp<SkSVGNode>& node, SkSVGAttribute attr,
                             const char* stringValue) {
    SkSVGSpreadMethod spread;
    SkSVGAttributeParser parser(stringValue);
    if (!parser.parseSpreadMethod(&spread)) {
        return false;
    }

    node->setAttribute(attr, SkSVGSpreadMethodValue(spread));
    return true;
}

bool SetPointsAttribute(const sk_sp<SkSVGNode>& node, SkSVGAttribute attr,
                        const char* stringValue) {
    SkSVGPointsType points;
    SkSVGAttributeParser parser(stringValue);
    if (!parser.parsePoints(&points)) {
        return false;
    }

    node->setAttribute(attr, SkSVGPointsValue(points));
    return true;
}

bool SetFillRuleAttribute(const sk_sp<SkSVGNode>& node, SkSVGAttribute attr,
                          const char* stringValue) {
    SkSVGFillRule fillRule;
    SkSVGAttributeParser parser(stringValue);
    if (!parser.parseFillRule(&fillRule)) {
        return false;
    }

    node->setAttribute(attr, SkSVGFillRuleValue(fillRule));
    return true;
}

bool SetVisibilityAttribute(const sk_sp<SkSVGNode>& node, SkSVGAttribute attr,
                            const char* stringValue) {
    SkSVGVisibility visibility;
    SkSVGAttributeParser parser(stringValue);
    if (!parser.parseVisibility(&visibility)) {
        return false;
    }

    node->setAttribute(attr, SkSVGVisibilityValue(visibility));
    return true;
}

bool SetDashArrayAttribute(const sk_sp<SkSVGNode>& node, SkSVGAttribute attr,
                           const char* stringValue) {
    SkSVGDashArray dashArray;
    SkSVGAttributeParser parser(stringValue);
    if (!parser.parseDashArray(&dashArray)) {
        return false;
    }

    node->setAttribute(attr, SkSVGDashArrayValue(dashArray));
    return true;
}

SkString TrimmedString(const char* first, const char* last) {
    SkASSERT(first);
    SkASSERT(last);
    SkASSERT(first <= last);

    while (first <= last && *first <= ' ') { first++; }
    while (first <= last && *last  <= ' ') { last--; }

    SkASSERT(last - first + 1 >= 0);
    return SkString(first, SkTo<size_t>(last - first + 1));
}

// Breaks a "foo: bar; baz: ..." string into key:value pairs.
class StyleIterator {
public:
    StyleIterator(const char* str) : fPos(str) { }

    std::tuple<SkString, SkString> next() {
        SkString name, value;

        if (fPos) {
            const char* sep = this->nextSeparator();
            SkASSERT(*sep == ';' || *sep == '\0');

            const char* valueSep = strchr(fPos, ':');
            if (valueSep && valueSep < sep) {
                name  = TrimmedString(fPos, valueSep - 1);
                value = TrimmedString(valueSep + 1, sep - 1);
            }

            fPos = *sep ? sep + 1 : nullptr;
        }

        return std::make_tuple(name, value);
    }

private:
    const char* nextSeparator() const {
        const char* sep = fPos;
        while (*sep != ';' && *sep != '\0') {
            sep++;
        }
        return sep;
    }

    const char* fPos;
};

void set_string_attribute(const sk_sp<SkSVGNode>& node, const char* name, const char* value);

bool SetStyleAttributes(const sk_sp<SkSVGNode>& node, SkSVGAttribute,
                        const char* stringValue) {

    SkString name, value;
    StyleIterator iter(stringValue);
    for (;;) {
        std::tie(name, value) = iter.next();
        if (name.isEmpty()) {
            break;
        }
        set_string_attribute(node, name.c_str(), value.c_str());
    }

    return true;
}

template<typename T>
struct SortedDictionaryEntry {
    const char* fKey;
    const T     fValue;
};

struct AttrParseInfo {
    SkSVGAttribute fAttr;
    bool (*fSetter)(const sk_sp<SkSVGNode>& node, SkSVGAttribute attr, const char* stringValue);
};

SortedDictionaryEntry<AttrParseInfo> gAttributeParseInfo[] = {
    { "clip-path"        , { SkSVGAttribute::kClipPath         , SetClipPathAttribute     }},
    { "clip-rule"        , { SkSVGAttribute::kClipRule         , SetFillRuleAttribute     }},
    { "cx"               , { SkSVGAttribute::kCx               , SetLengthAttribute       }},
    { "cy"               , { SkSVGAttribute::kCy               , SetLengthAttribute       }},
    { "d"                , { SkSVGAttribute::kD                , SetPathDataAttribute     }},
    { "fill"             , { SkSVGAttribute::kFill             , SetPaintAttribute        }},
    { "fill-opacity"     , { SkSVGAttribute::kFillOpacity      , SetNumberAttribute       }},
    { "fill-rule"        , { SkSVGAttribute::kFillRule         , SetFillRuleAttribute     }},
    // focal point x & y
    { "fx"               , { SkSVGAttribute::kFx               , SetLengthAttribute       }},
    { "fy"               , { SkSVGAttribute::kFy               , SetLengthAttribute       }},
    { "gradientTransform", { SkSVGAttribute::kGradientTransform, SetTransformAttribute    }},
    { "height"           , { SkSVGAttribute::kHeight           , SetLengthAttribute       }},
    { "offset"           , { SkSVGAttribute::kOffset           , SetLengthAttribute       }},
    { "opacity"          , { SkSVGAttribute::kOpacity          , SetNumberAttribute       }},
    { "patternTransform" , { SkSVGAttribute::kPatternTransform , SetTransformAttribute    }},
    { "points"           , { SkSVGAttribute::kPoints           , SetPointsAttribute       }},
    { "r"                , { SkSVGAttribute::kR                , SetLengthAttribute       }},
    { "rx"               , { SkSVGAttribute::kRx               , SetLengthAttribute       }},
    { "ry"               , { SkSVGAttribute::kRy               , SetLengthAttribute       }},
    { "spreadMethod"     , { SkSVGAttribute::kSpreadMethod     , SetSpreadMethodAttribute }},
    { "stop-color"       , { SkSVGAttribute::kStopColor        , SetColorAttribute        }},
    { "stop-opacity"     , { SkSVGAttribute::kStopOpacity      , SetNumberAttribute       }},
    { "stroke"           , { SkSVGAttribute::kStroke           , SetPaintAttribute        }},
    { "stroke-dasharray" , { SkSVGAttribute::kStrokeDashArray  , SetDashArrayAttribute    }},
    { "stroke-dashoffset", { SkSVGAttribute::kStrokeDashOffset , SetLengthAttribute       }},
    { "stroke-linecap"   , { SkSVGAttribute::kStrokeLineCap    , SetLineCapAttribute      }},
    { "stroke-linejoin"  , { SkSVGAttribute::kStrokeLineJoin   , SetLineJoinAttribute     }},
    { "stroke-miterlimit", { SkSVGAttribute::kStrokeMiterLimit , SetNumberAttribute       }},
    { "stroke-opacity"   , { SkSVGAttribute::kStrokeOpacity    , SetNumberAttribute       }},
    { "stroke-width"     , { SkSVGAttribute::kStrokeWidth      , SetLengthAttribute       }},
    { "style"            , { SkSVGAttribute::kUnknown          , SetStyleAttributes       }},
    { "transform"        , { SkSVGAttribute::kTransform        , SetTransformAttribute    }},
    { "viewBox"          , { SkSVGAttribute::kViewBox          , SetViewBoxAttribute      }},
    { "visibility"       , { SkSVGAttribute::kVisibility       , SetVisibilityAttribute   }},
    { "width"            , { SkSVGAttribute::kWidth            , SetLengthAttribute       }},
    { "x"                , { SkSVGAttribute::kX                , SetLengthAttribute       }},
    { "x1"               , { SkSVGAttribute::kX1               , SetLengthAttribute       }},
    { "x2"               , { SkSVGAttribute::kX2               , SetLengthAttribute       }},
    { "xlink:href"       , { SkSVGAttribute::kHref             , SetIRIAttribute          }},
    { "y"                , { SkSVGAttribute::kY                , SetLengthAttribute       }},
    { "y1"               , { SkSVGAttribute::kY1               , SetLengthAttribute       }},
    { "y2"               , { SkSVGAttribute::kY2               , SetLengthAttribute       }},
};

SortedDictionaryEntry<sk_sp<SkSVGNode>(*)()> gTagFactories[] = {
    { "a"             , []() -> sk_sp<SkSVGNode> { return SkSVGG::Make();              }},
    { "circle"        , []() -> sk_sp<SkSVGNode> { return SkSVGCircle::Make();         }},
    { "clipPath"      , []() -> sk_sp<SkSVGNode> { return SkSVGClipPath::Make();       }},
    { "defs"          , []() -> sk_sp<SkSVGNode> { return SkSVGDefs::Make();           }},
    { "ellipse"       , []() -> sk_sp<SkSVGNode> { return SkSVGEllipse::Make();        }},
    { "g"             , []() -> sk_sp<SkSVGNode> { return SkSVGG::Make();              }},
    { "line"          , []() -> sk_sp<SkSVGNode> { return SkSVGLine::Make();           }},
    { "linearGradient", []() -> sk_sp<SkSVGNode> { return SkSVGLinearGradient::Make(); }},
    { "path"          , []() -> sk_sp<SkSVGNode> { return SkSVGPath::Make();           }},
    { "pattern"       , []() -> sk_sp<SkSVGNode> { return SkSVGPattern::Make();        }},
    { "polygon"       , []() -> sk_sp<SkSVGNode> { return SkSVGPoly::MakePolygon();    }},
    { "polyline"      , []() -> sk_sp<SkSVGNode> { return SkSVGPoly::MakePolyline();   }},
    { "radialGradient", []() -> sk_sp<SkSVGNode> { return SkSVGRadialGradient::Make(); }},
    { "rect"          , []() -> sk_sp<SkSVGNode> { return SkSVGRect::Make();           }},
    { "stop"          , []() -> sk_sp<SkSVGNode> { return SkSVGStop::Make();           }},
    { "svg"           , []() -> sk_sp<SkSVGNode> { return SkSVGSVG::Make();            }},
    { "use"           , []() -> sk_sp<SkSVGNode> { return SkSVGUse::Make();            }},
};

struct ConstructionContext {
    ConstructionContext(SkSVGIDMapper* mapper) : fParent(nullptr), fIDMapper(mapper) {}
    ConstructionContext(const ConstructionContext& other, const sk_sp<SkSVGNode>& newParent)
        : fParent(newParent.get()), fIDMapper(other.fIDMapper) {}

    const SkSVGNode* fParent;
    SkSVGIDMapper*   fIDMapper;
};

void set_string_attribute(const sk_sp<SkSVGNode>& node, const char* name, const char* value) {
    const int attrIndex = SkStrSearch(&gAttributeParseInfo[0].fKey,
                                      SkTo<int>(SK_ARRAY_COUNT(gAttributeParseInfo)),
                                      name, sizeof(gAttributeParseInfo[0]));
    if (attrIndex < 0) {
#if defined(SK_VERBOSE_SVG_PARSING)
        SkDebugf("unhandled attribute: %s\n", name);
#endif
        return;
    }

    SkASSERT(SkTo<size_t>(attrIndex) < SK_ARRAY_COUNT(gAttributeParseInfo));
    const auto& attrInfo = gAttributeParseInfo[attrIndex].fValue;
    if (!attrInfo.fSetter(node, attrInfo.fAttr, value)) {
#if defined(SK_VERBOSE_SVG_PARSING)
        SkDebugf("could not parse attribute: '%s=\"%s\"'\n", name, value);
#endif
    }
}

void parse_node_attributes(const SkDOM& xmlDom, const SkDOM::Node* xmlNode,
                           const sk_sp<SkSVGNode>& svgNode, SkSVGIDMapper* mapper) {
    const char* name, *value;
    SkDOM::AttrIter attrIter(xmlDom, xmlNode);
    while ((name = attrIter.next(&value))) {
        // We're handling id attributes out of band for now.
        if (!strcmp(name, "id")) {
            mapper->set(SkString(value), svgNode);
            continue;
        }
        set_string_attribute(svgNode, name, value);
    }
}

sk_sp<SkSVGNode> construct_svg_node(const SkDOM& dom, const ConstructionContext& ctx,
                                    const SkDOM::Node* xmlNode) {
    const char* elem = dom.getName(xmlNode);
    const SkDOM::Type elemType = dom.getType(xmlNode);

    if (elemType == SkDOM::kText_Type) {
        SkASSERT(dom.countChildren(xmlNode) == 0);
        // TODO: text handling
        return nullptr;
    }

    SkASSERT(elemType == SkDOM::kElement_Type);

    const int tagIndex = SkStrSearch(&gTagFactories[0].fKey,
                                     SkTo<int>(SK_ARRAY_COUNT(gTagFactories)),
                                     elem, sizeof(gTagFactories[0]));
    if (tagIndex < 0) {
#if defined(SK_VERBOSE_SVG_PARSING)
        SkDebugf("unhandled element: <%s>\n", elem);
#endif
        return nullptr;
    }

    SkASSERT(SkTo<size_t>(tagIndex) < SK_ARRAY_COUNT(gTagFactories));
    sk_sp<SkSVGNode> node = gTagFactories[tagIndex].fValue();
    parse_node_attributes(dom, xmlNode, node, ctx.fIDMapper);

    ConstructionContext localCtx(ctx, node);
    for (auto* child = dom.getFirstChild(xmlNode, nullptr); child;
         child = dom.getNextSibling(child)) {
        sk_sp<SkSVGNode> childNode = construct_svg_node(dom, localCtx, child);
        if (childNode) {
            node->appendChild(std::move(childNode));
        }
    }

    return node;
}

} // anonymous namespace

SkSVGDOM::SkSVGDOM()
    : fContainerSize(SkSize::Make(0, 0)) {
}

sk_sp<SkSVGDOM> SkSVGDOM::MakeFromDOM(const SkDOM& xmlDom) {
    sk_sp<SkSVGDOM> dom = sk_make_sp<SkSVGDOM>();

    ConstructionContext ctx(&dom->fIDMapper);
    dom->fRoot = construct_svg_node(xmlDom, ctx, xmlDom.getRootNode());

    // Reset the default container size to match the intrinsic SVG size.
    dom->setContainerSize(dom->intrinsicSize());

    return dom;
}

sk_sp<SkSVGDOM> SkSVGDOM::MakeFromStream(SkStream& svgStream) {
    SkDOM xmlDom;
    if (!xmlDom.build(svgStream)) {
        return nullptr;
    }

    return MakeFromDOM(xmlDom);
}

void SkSVGDOM::render(SkCanvas* canvas) const {
    if (fRoot) {
        SkSVGLengthContext       lctx(fContainerSize);
        SkSVGPresentationContext pctx;
        fRoot->render(SkSVGRenderContext(canvas, fIDMapper, lctx, pctx));
    }
}

SkSize SkSVGDOM::intrinsicSize() const {
    if (!fRoot || fRoot->tag() != SkSVGTag::kSvg) {
        return SkSize::Make(0, 0);
    }

    // Intrinsic sizes are never relative, so the viewport size is irrelevant.
    const SkSVGLengthContext lctx(SkSize::Make(0, 0));
    return static_cast<const SkSVGSVG*>(fRoot.get())->intrinsicSize(lctx);
}

const SkSize& SkSVGDOM::containerSize() const {
    return fContainerSize;
}

void SkSVGDOM::setContainerSize(const SkSize& containerSize) {
    // TODO: inval
    fContainerSize = containerSize;
}

void SkSVGDOM::setRoot(sk_sp<SkSVGNode> root) {
    fRoot = std::move(root);
}
