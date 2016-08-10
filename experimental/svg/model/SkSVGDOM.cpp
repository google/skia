/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkDOM.h"
#include "SkParsePath.h"
#include "SkString.h"
#include "SkSVGAttributeParser.h"
#include "SkSVGDOM.h"
#include "SkSVGG.h"
#include "SkSVGNode.h"
#include "SkSVGPath.h"
#include "SkSVGRect.h"
#include "SkSVGRenderContext.h"
#include "SkSVGSVG.h"
#include "SkSVGTypes.h"
#include "SkSVGValue.h"
#include "SkTSearch.h"

namespace {

bool SetPaintAttribute(const sk_sp<SkSVGNode>& node, SkSVGAttribute attr,
                       const char* stringValue) {
    SkSVGColorType color;
    SkSVGAttributeParser parser(stringValue);
    if (!parser.parseColor(&color)) {
        return false;
    }

    node->setAttribute(attr, SkSVGColorValue(color));
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
    { "d"        , { SkSVGAttribute::kD        , SetPathDataAttribute  }},
    { "fill"     , { SkSVGAttribute::kFill     , SetPaintAttribute     }},
    { "height"   , { SkSVGAttribute::kHeight   , SetLengthAttribute    }},
    { "stroke"   , { SkSVGAttribute::kStroke   , SetPaintAttribute     }},
    { "style"    , { SkSVGAttribute::kUnknown  , SetStyleAttributes    }},
    { "transform", { SkSVGAttribute::kTransform, SetTransformAttribute }},
    { "viewBox"  , { SkSVGAttribute::kViewBox  , SetViewBoxAttribute   }},
    { "width"    , { SkSVGAttribute::kWidth    , SetLengthAttribute    }},
    { "x"        , { SkSVGAttribute::kX        , SetLengthAttribute    }},
    { "y"        , { SkSVGAttribute::kY        , SetLengthAttribute    }},
};

SortedDictionaryEntry<sk_sp<SkSVGNode>(*)()> gTagFactories[] = {
    { "g"   , []() -> sk_sp<SkSVGNode> { return SkSVGG::Make();    }},
    { "path", []() -> sk_sp<SkSVGNode> { return SkSVGPath::Make(); }},
    { "rect", []() -> sk_sp<SkSVGNode> { return SkSVGRect::Make(); }},
    { "svg" , []() -> sk_sp<SkSVGNode> { return SkSVGSVG::Make();  }},
};

struct ConstructionContext {
    ConstructionContext() : fParent(nullptr) { }
    ConstructionContext(const ConstructionContext& other, const sk_sp<SkSVGNode>& newParent)
        : fParent(newParent.get()) { }

    const SkSVGNode* fParent;
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
                           const sk_sp<SkSVGNode>& svgNode) {
    const char* name, *value;
    SkDOM::AttrIter attrIter(xmlDom, xmlNode);
    while ((name = attrIter.next(&value))) {
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
    parse_node_attributes(dom, xmlNode, node);

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

SkSVGDOM::SkSVGDOM(const SkSize& containerSize)
    : fContainerSize(containerSize) {
}

sk_sp<SkSVGDOM> SkSVGDOM::MakeFromDOM(const SkDOM& xmlDom, const SkSize& containerSize) {
    sk_sp<SkSVGDOM> dom = sk_make_sp<SkSVGDOM>(containerSize);

    ConstructionContext ctx;
    dom->fRoot = construct_svg_node(xmlDom, ctx, xmlDom.getRootNode());

    return dom;
}

sk_sp<SkSVGDOM> SkSVGDOM::MakeFromStream(SkStream& svgStream, const SkSize& containerSize) {
    SkDOM xmlDom;
    if (!xmlDom.build(svgStream)) {
        return nullptr;
    }

    return MakeFromDOM(xmlDom, containerSize);
}

void SkSVGDOM::render(SkCanvas* canvas) const {
    if (fRoot) {
        SkSVGRenderContext ctx(canvas,
                               SkSVGLengthContext(fContainerSize),
                               SkSVGPresentationContext());
        fRoot->render(ctx);
    }
}

void SkSVGDOM::setContainerSize(const SkSize& containerSize) {
    // TODO: inval
    fContainerSize = containerSize;
}
