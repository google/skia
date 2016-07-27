/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkDOM.h"
#include "SkParse.h"
#include "SkParsePath.h"
#include "SkSVGDOM.h"
#include "SkSVGG.h"
#include "SkSVGNode.h"
#include "SkSVGPath.h"
#include "SkSVGSVG.h"
#include "SkSVGValue.h"
#include "SkTSearch.h"

namespace {

SkColor ParseColor(const char* str) {
    // FIXME: real parser
    if (*str++ != '#') {
        return SK_ColorBLACK;
    }

    uint32_t v;
    const char* consumed = SkParse::FindHex(str, &v);

    switch(consumed - str) {
    case 6:
        // matched '#xxxxxx'
        break;
    case 3:
        // matched '#xxx;
        v = ((v << 12) & 0x00f00000) |
            ((v <<  8) & 0x000ff000) |
            ((v <<  4) & 0x00000ff0) |
            ((v <<  0) & 0x0000000f);
        break;
    default:
        // failed
        v = 0;
        break;
    }

    return v | 0xff000000;
}

const char* ParseScalarPair(const char* str, SkScalar v[2]) {
    str = SkParse::FindScalar(str, v);
    if (str) {
        const char* second = SkParse::FindScalar(str, v + 1);
        if (!second) {
            v[1] = v[0];
        } else {
            str = second;
        }
    }

    return str;
}

SkMatrix ParseTransform(const char* str) {
    SkMatrix m = SkMatrix::I();

    // FIXME: real parser
    if (!strncmp(str, "matrix(", 7)) {
        SkScalar values[6];
        str = SkParse::FindScalars(str + 7, values, 6);
        if (str) {
            m.setAffine(values);
        }
    } else if (!strncmp(str, "scale(", 6)) {
        SkScalar values[2];
        str = ParseScalarPair(str + 6, values);
        if (str) {
            m.setScale(values[0], values[1]);
        }
    } else if (!strncmp(str, "translate(", 10)) {
        SkScalar values[2];
        str = ParseScalarPair(str + 10, values);
        if (str) {
            m.setTranslate(values[0], values[1]);
        }
    } else if (!strncmp(str, "rotate(", 7)) {
        SkScalar value;
        str = SkParse::FindScalar(str + 7, &value);
        if (str) {
            m.setRotate(value);
        }
    }

    return m;
}

bool SetPaintAttribute(const sk_sp<SkSVGNode>& node, SkSVGAttribute attr,
                       const char* stringValue) {
    node->setAttribute(attr, SkSVGColorValue(ParseColor(stringValue)));
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
    node->setAttribute(attr, SkSVGTransformValue(ParseTransform(stringValue)));
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
    { "d",         { SkSVGAttribute::d,         SetPathDataAttribute  }},
    { "fill",      { SkSVGAttribute::fill,      SetPaintAttribute     }},
    { "stroke",    { SkSVGAttribute::stroke,    SetPaintAttribute     }},
    { "transform", { SkSVGAttribute::transform, SetTransformAttribute }},
};

SortedDictionaryEntry<sk_sp<SkSVGNode>(*)()> gTagFactories[] = {
    { "g"   , []() -> sk_sp<SkSVGNode> { return SkSVGG::Make();    }},
    { "path", []() -> sk_sp<SkSVGNode> { return SkSVGPath::Make(); }},
    { "svg" , []() -> sk_sp<SkSVGNode> { return SkSVGSVG::Make();  }},
};

struct ConstructionContext {
    ConstructionContext() : fParent(nullptr) { }
    ConstructionContext(const ConstructionContext& other, const sk_sp<SkSVGNode>& newParent)
        : fParent(newParent.get()) { }

    const SkSVGNode* fParent;
};

void parse_node_attributes(const SkDOM& xmlDom, const SkDOM::Node* xmlNode,
                           const sk_sp<SkSVGNode>& svgNode) {
    const char* name, *value;
    SkDOM::AttrIter attrIter(xmlDom, xmlNode);
    while ((name = attrIter.next(&value))) {
        const int attrIndex = SkStrSearch(&gAttributeParseInfo[0].fKey,
                                          SkTo<int>(SK_ARRAY_COUNT(gAttributeParseInfo)),
                                          name, sizeof(gAttributeParseInfo[0]));
        if (attrIndex < 0) {
            SkDebugf("unhandled attribute: %s\n", name);
            continue;
        }

        SkASSERT(SkTo<size_t>(attrIndex) < SK_ARRAY_COUNT(gAttributeParseInfo));
        const auto& attrInfo = gAttributeParseInfo[attrIndex].fValue;
        if (!attrInfo.fSetter(svgNode, attrInfo.fAttr, value)) {
            SkDebugf("could not parse attribute: '%s=\"%s\"'\n", name, value);
        }
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
        SkDebugf("unhandled element: <%s>\n", elem);
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
        fRoot->render(canvas);
    }
}

void SkSVGDOM::setContainerSize(const SkSize& containerSize) {
    // TODO: inval
    fContainerSize = containerSize;
}
