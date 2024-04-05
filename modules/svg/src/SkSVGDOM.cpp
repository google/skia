/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/svg/include/SkSVGDOM.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkString.h"
#include "include/private/base/SkTo.h"
#include "modules/skshaper/include/SkShaper_factory.h"
#include "modules/svg/include/SkSVGAttributeParser.h"
#include "modules/svg/include/SkSVGCircle.h"
#include "modules/svg/include/SkSVGClipPath.h"
#include "modules/svg/include/SkSVGDefs.h"
#include "modules/svg/include/SkSVGEllipse.h"
#include "modules/svg/include/SkSVGFeBlend.h"
#include "modules/svg/include/SkSVGFeColorMatrix.h"
#include "modules/svg/include/SkSVGFeComposite.h"
#include "modules/svg/include/SkSVGFeDisplacementMap.h"
#include "modules/svg/include/SkSVGFeFlood.h"
#include "modules/svg/include/SkSVGFeGaussianBlur.h"
#include "modules/svg/include/SkSVGFeImage.h"
#include "modules/svg/include/SkSVGFeLightSource.h"
#include "modules/svg/include/SkSVGFeLighting.h"
#include "modules/svg/include/SkSVGFeMorphology.h"
#include "modules/svg/include/SkSVGFeOffset.h"
#include "modules/svg/include/SkSVGFeTurbulence.h"
#include "modules/svg/include/SkSVGFilter.h"
#include "modules/svg/include/SkSVGG.h"
#include "modules/svg/include/SkSVGImage.h"
#include "modules/svg/include/SkSVGLine.h"
#include "modules/svg/include/SkSVGLinearGradient.h"
#include "modules/svg/include/SkSVGMask.h"
#include "modules/svg/include/SkSVGNode.h"
#include "modules/svg/include/SkSVGPath.h"
#include "modules/svg/include/SkSVGPattern.h"
#include "modules/svg/include/SkSVGPoly.h"
#include "modules/svg/include/SkSVGRadialGradient.h"
#include "modules/svg/include/SkSVGRect.h"
#include "modules/svg/include/SkSVGRenderContext.h"
#include "modules/svg/include/SkSVGSVG.h"
#include "modules/svg/include/SkSVGStop.h"
#include "modules/svg/include/SkSVGText.h"
#include "modules/svg/include/SkSVGTypes.h"
#include "modules/svg/include/SkSVGUse.h"
#include "modules/svg/include/SkSVGValue.h"
#include "src/base/SkTSearch.h"
#include "src/core/SkTraceEvent.h"
#include "src/xml/SkDOM.h"

namespace {

bool SetIRIAttribute(const sk_sp<SkSVGNode>& node, SkSVGAttribute attr,
                      const char* stringValue) {
    auto parseResult = SkSVGAttributeParser::parse<SkSVGIRI>(stringValue);
    if (!parseResult.isValid()) {
        return false;
    }

    node->setAttribute(attr, SkSVGStringValue(parseResult->iri()));
    return true;
}

bool SetStringAttribute(const sk_sp<SkSVGNode>& node, SkSVGAttribute attr,
                           const char* stringValue) {
    SkString str(stringValue, strlen(stringValue));
    SkSVGStringType strType = SkSVGStringType(str);
    node->setAttribute(attr, SkSVGStringValue(strType));
    return true;
}

bool SetTransformAttribute(const sk_sp<SkSVGNode>& node, SkSVGAttribute attr,
                           const char* stringValue) {
    auto parseResult = SkSVGAttributeParser::parse<SkSVGTransformType>(stringValue);
    if (!parseResult.isValid()) {
        return false;
    }

    node->setAttribute(attr, SkSVGTransformValue(*parseResult));
    return true;
}

bool SetLengthAttribute(const sk_sp<SkSVGNode>& node, SkSVGAttribute attr,
                        const char* stringValue) {
    auto parseResult = SkSVGAttributeParser::parse<SkSVGLength>(stringValue);
    if (!parseResult.isValid()) {
        return false;
    }

    node->setAttribute(attr, SkSVGLengthValue(*parseResult));
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

bool SetObjectBoundingBoxUnitsAttribute(const sk_sp<SkSVGNode>& node,
                                        SkSVGAttribute attr,
                                        const char* stringValue) {
    auto parseResult = SkSVGAttributeParser::parse<SkSVGObjectBoundingBoxUnits>(stringValue);
    if (!parseResult.isValid()) {
        return false;
    }

    node->setAttribute(attr, SkSVGObjectBoundingBoxUnitsValue(*parseResult));
    return true;
}

bool SetPreserveAspectRatioAttribute(const sk_sp<SkSVGNode>& node, SkSVGAttribute attr,
                                     const char* stringValue) {
    SkSVGPreserveAspectRatio par;
    SkSVGAttributeParser parser(stringValue);
    if (!parser.parsePreserveAspectRatio(&par)) {
        return false;
    }

    node->setAttribute(attr, SkSVGPreserveAspectRatioValue(par));
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

bool set_string_attribute(const sk_sp<SkSVGNode>& node, const char* name, const char* value);

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
    { "cx"                 , { SkSVGAttribute::kCx               , SetLengthAttribute       }},
    { "cy"                 , { SkSVGAttribute::kCy               , SetLengthAttribute       }},
    { "filterUnits"        , { SkSVGAttribute::kFilterUnits      ,
                               SetObjectBoundingBoxUnitsAttribute }},
    // focal point x & y
    { "fx"                 , { SkSVGAttribute::kFx               , SetLengthAttribute       }},
    { "fy"                 , { SkSVGAttribute::kFy               , SetLengthAttribute       }},
    { "height"             , { SkSVGAttribute::kHeight           , SetLengthAttribute       }},
    { "preserveAspectRatio", { SkSVGAttribute::kPreserveAspectRatio,
                               SetPreserveAspectRatioAttribute }},
    { "r"                  , { SkSVGAttribute::kR                , SetLengthAttribute       }},
    { "rx"                 , { SkSVGAttribute::kRx               , SetLengthAttribute       }},
    { "ry"                 , { SkSVGAttribute::kRy               , SetLengthAttribute       }},
    { "style"              , { SkSVGAttribute::kUnknown          , SetStyleAttributes       }},
    { "text"               , { SkSVGAttribute::kText             , SetStringAttribute       }},
    { "transform"          , { SkSVGAttribute::kTransform        , SetTransformAttribute    }},
    { "viewBox"            , { SkSVGAttribute::kViewBox          , SetViewBoxAttribute      }},
    { "width"              , { SkSVGAttribute::kWidth            , SetLengthAttribute       }},
    { "x"                  , { SkSVGAttribute::kX                , SetLengthAttribute       }},
    { "x1"                 , { SkSVGAttribute::kX1               , SetLengthAttribute       }},
    { "x2"                 , { SkSVGAttribute::kX2               , SetLengthAttribute       }},
    { "xlink:href"         , { SkSVGAttribute::kHref             , SetIRIAttribute          }},
    { "y"                  , { SkSVGAttribute::kY                , SetLengthAttribute       }},
    { "y1"                 , { SkSVGAttribute::kY1               , SetLengthAttribute       }},
    { "y2"                 , { SkSVGAttribute::kY2               , SetLengthAttribute       }},
};

SortedDictionaryEntry<sk_sp<SkSVGNode>(*)()> gTagFactories[] = {
    { "a"                 , []() -> sk_sp<SkSVGNode> { return SkSVGG::Make();                  }},
    { "circle"            , []() -> sk_sp<SkSVGNode> { return SkSVGCircle::Make();             }},
    { "clipPath"          , []() -> sk_sp<SkSVGNode> { return SkSVGClipPath::Make();           }},
    { "defs"              , []() -> sk_sp<SkSVGNode> { return SkSVGDefs::Make();               }},
    { "ellipse"           , []() -> sk_sp<SkSVGNode> { return SkSVGEllipse::Make();            }},
    { "feBlend"           , []() -> sk_sp<SkSVGNode> { return SkSVGFeBlend::Make();            }},
    { "feColorMatrix"     , []() -> sk_sp<SkSVGNode> { return SkSVGFeColorMatrix::Make();      }},
    { "feComposite"       , []() -> sk_sp<SkSVGNode> { return SkSVGFeComposite::Make();        }},
    { "feDiffuseLighting" , []() -> sk_sp<SkSVGNode> { return SkSVGFeDiffuseLighting::Make();  }},
    { "feDisplacementMap" , []() -> sk_sp<SkSVGNode> { return SkSVGFeDisplacementMap::Make();  }},
    { "feDistantLight"    , []() -> sk_sp<SkSVGNode> { return SkSVGFeDistantLight::Make();     }},
    { "feFlood"           , []() -> sk_sp<SkSVGNode> { return SkSVGFeFlood::Make();            }},
    { "feGaussianBlur"    , []() -> sk_sp<SkSVGNode> { return SkSVGFeGaussianBlur::Make();     }},
    { "feImage"           , []() -> sk_sp<SkSVGNode> { return SkSVGFeImage::Make();            }},
    { "feMorphology"      , []() -> sk_sp<SkSVGNode> { return SkSVGFeMorphology::Make();       }},
    { "feOffset"          , []() -> sk_sp<SkSVGNode> { return SkSVGFeOffset::Make();           }},
    { "fePointLight"      , []() -> sk_sp<SkSVGNode> { return SkSVGFePointLight::Make();       }},
    { "feSpecularLighting", []() -> sk_sp<SkSVGNode> { return SkSVGFeSpecularLighting::Make(); }},
    { "feSpotLight"       , []() -> sk_sp<SkSVGNode> { return SkSVGFeSpotLight::Make();        }},
    { "feTurbulence"      , []() -> sk_sp<SkSVGNode> { return SkSVGFeTurbulence::Make();       }},
    { "filter"            , []() -> sk_sp<SkSVGNode> { return SkSVGFilter::Make();             }},
    { "g"                 , []() -> sk_sp<SkSVGNode> { return SkSVGG::Make();                  }},
    { "image"             , []() -> sk_sp<SkSVGNode> { return SkSVGImage::Make();              }},
    { "line"              , []() -> sk_sp<SkSVGNode> { return SkSVGLine::Make();               }},
    { "linearGradient"    , []() -> sk_sp<SkSVGNode> { return SkSVGLinearGradient::Make();     }},
    { "mask"              , []() -> sk_sp<SkSVGNode> { return SkSVGMask::Make();               }},
    { "path"              , []() -> sk_sp<SkSVGNode> { return SkSVGPath::Make();               }},
    { "pattern"           , []() -> sk_sp<SkSVGNode> { return SkSVGPattern::Make();            }},
    { "polygon"           , []() -> sk_sp<SkSVGNode> { return SkSVGPoly::MakePolygon();        }},
    { "polyline"          , []() -> sk_sp<SkSVGNode> { return SkSVGPoly::MakePolyline();       }},
    { "radialGradient"    , []() -> sk_sp<SkSVGNode> { return SkSVGRadialGradient::Make();     }},
    { "rect"              , []() -> sk_sp<SkSVGNode> { return SkSVGRect::Make();               }},
    { "stop"              , []() -> sk_sp<SkSVGNode> { return SkSVGStop::Make();               }},
//    "svg" handled explicitly
    { "text"              , []() -> sk_sp<SkSVGNode> { return SkSVGText::Make();               }},
    { "textPath"          , []() -> sk_sp<SkSVGNode> { return SkSVGTextPath::Make();           }},
    { "tspan"             , []() -> sk_sp<SkSVGNode> { return SkSVGTSpan::Make();              }},
    { "use"               , []() -> sk_sp<SkSVGNode> { return SkSVGUse::Make();                }},
};

struct ConstructionContext {
    ConstructionContext(SkSVGIDMapper* mapper) : fParent(nullptr), fIDMapper(mapper) {}
    ConstructionContext(const ConstructionContext& other, const sk_sp<SkSVGNode>& newParent)
        : fParent(newParent.get()), fIDMapper(other.fIDMapper) {}

    SkSVGNode*     fParent;
    SkSVGIDMapper* fIDMapper;
};

bool set_string_attribute(const sk_sp<SkSVGNode>& node, const char* name, const char* value) {
    if (node->parseAndSetAttribute(name, value)) {
        // Handled by new code path
        return true;
    }

    const int attrIndex = SkStrSearch(&gAttributeParseInfo[0].fKey,
                                      SkTo<int>(std::size(gAttributeParseInfo)),
                                      name, sizeof(gAttributeParseInfo[0]));
    if (attrIndex < 0) {
#if defined(SK_VERBOSE_SVG_PARSING)
        SkDebugf("unhandled attribute: %s\n", name);
#endif
        return false;
    }

    SkASSERT(SkTo<size_t>(attrIndex) < std::size(gAttributeParseInfo));
    const auto& attrInfo = gAttributeParseInfo[attrIndex].fValue;
    if (!attrInfo.fSetter(node, attrInfo.fAttr, value)) {
#if defined(SK_VERBOSE_SVG_PARSING)
        SkDebugf("could not parse attribute: '%s=\"%s\"'\n", name, value);
#endif
        return false;
    }

    return true;
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
        // Text literals require special handling.
        SkASSERT(dom.countChildren(xmlNode) == 0);
        auto txt = SkSVGTextLiteral::Make();
        txt->setText(SkString(dom.getName(xmlNode)));
        ctx.fParent->appendChild(std::move(txt));

        return nullptr;
    }

    SkASSERT(elemType == SkDOM::kElement_Type);

    auto make_node = [](const ConstructionContext& ctx, const char* elem) -> sk_sp<SkSVGNode> {
        if (strcmp(elem, "svg") == 0) {
            // Outermost SVG element must be tagged as such.
            return SkSVGSVG::Make(ctx.fParent ? SkSVGSVG::Type::kInner
                                              : SkSVGSVG::Type::kRoot);
        }

        const int tagIndex = SkStrSearch(&gTagFactories[0].fKey,
                                         SkTo<int>(std::size(gTagFactories)),
                                         elem, sizeof(gTagFactories[0]));
        if (tagIndex < 0) {
#if defined(SK_VERBOSE_SVG_PARSING)
            SkDebugf("unhandled element: <%s>\n", elem);
#endif
            return nullptr;
        }
        SkASSERT(SkTo<size_t>(tagIndex) < std::size(gTagFactories));

        return gTagFactories[tagIndex].fValue();
    };

    auto node = make_node(ctx, elem);
    if (!node) {
        return nullptr;
    }

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

SkSVGDOM::Builder& SkSVGDOM::Builder::setFontManager(sk_sp<SkFontMgr> fmgr) {
    fFontMgr = std::move(fmgr);
    return *this;
}

SkSVGDOM::Builder& SkSVGDOM::Builder::setResourceProvider(sk_sp<skresources::ResourceProvider> rp) {
    fResourceProvider = std::move(rp);
    return *this;
}

SkSVGDOM::Builder& SkSVGDOM::Builder::setTextShapingFactory(sk_sp<SkShapers::Factory> f) {
    fTextShapingFactory = f;
    return *this;
}

sk_sp<SkSVGDOM> SkSVGDOM::Builder::make(SkStream& str) const {
    TRACE_EVENT0("skia", TRACE_FUNC);
    SkDOM xmlDom;
    if (!xmlDom.build(str)) {
        return nullptr;
    }

    SkSVGIDMapper mapper;
    ConstructionContext ctx(&mapper);

    auto root = construct_svg_node(xmlDom, ctx, xmlDom.getRootNode());
    if (!root || root->tag() != SkSVGTag::kSvg) {
        return nullptr;
    }

    class NullResourceProvider final : public skresources::ResourceProvider {
        sk_sp<SkData> load(const char[], const char[]) const override { return nullptr; }
    };

    auto resource_provider = fResourceProvider ? fResourceProvider
                                               : sk_make_sp<NullResourceProvider>();

    auto factory = fTextShapingFactory ? fTextShapingFactory : SkShapers::Primitive::Factory();

    return sk_sp<SkSVGDOM>(new SkSVGDOM(sk_sp<SkSVGSVG>(static_cast<SkSVGSVG*>(root.release())),
                                        std::move(fFontMgr),
                                        std::move(resource_provider),
                                        std::move(mapper),
                                        std::move(factory)));
}

SkSVGDOM::SkSVGDOM(sk_sp<SkSVGSVG> root,
                   sk_sp<SkFontMgr> fmgr,
                   sk_sp<skresources::ResourceProvider> rp,
                   SkSVGIDMapper&& mapper,
                   sk_sp<SkShapers::Factory> fact)
        : fRoot(std::move(root))
        , fFontMgr(std::move(fmgr))
        , fTextShapingFactory(std::move(fact))
        , fResourceProvider(std::move(rp))
        , fIDMapper(std::move(mapper))
        , fContainerSize(fRoot->intrinsicSize(SkSVGLengthContext(SkSize::Make(0, 0)))) {
    SkASSERT(fResourceProvider);
    SkASSERT(fTextShapingFactory);
}

void SkSVGDOM::render(SkCanvas* canvas) const {
    TRACE_EVENT0("skia", TRACE_FUNC);
    if (fRoot) {
        SkSVGLengthContext       lctx(fContainerSize);
        SkSVGPresentationContext pctx;
        fRoot->render(SkSVGRenderContext(canvas,
                                         fFontMgr,
                                         fResourceProvider,
                                         fIDMapper,
                                         lctx,
                                         pctx,
                                         {nullptr, nullptr},
                                         fTextShapingFactory));
    }
}

void SkSVGDOM::renderNode(SkCanvas* canvas, SkSVGPresentationContext& pctx, const char* id) const {
    TRACE_EVENT0("skia", TRACE_FUNC);

    if (fRoot) {
        SkSVGLengthContext lctx(fContainerSize);
        fRoot->renderNode(SkSVGRenderContext(canvas,
                                             fFontMgr,
                                             fResourceProvider,
                                             fIDMapper,
                                             lctx,
                                             pctx,
                                             {nullptr, nullptr},
                                             fTextShapingFactory),
                          SkSVGIRI(SkSVGIRI::Type::kLocal, SkSVGStringType(id)));
    }
}

const SkSize& SkSVGDOM::containerSize() const {
    return fContainerSize;
}

void SkSVGDOM::setContainerSize(const SkSize& containerSize) {
    // TODO: inval
    fContainerSize = containerSize;
}

sk_sp<SkSVGNode>* SkSVGDOM::findNodeById(const char* id) {
    SkString idStr(id);
    return this->fIDMapper.find(idStr);
}

// TODO(fuego): move this to SkSVGNode or its own CU.
bool SkSVGNode::setAttribute(const char* attributeName, const char* attributeValue) {
    return set_string_attribute(sk_ref_sp(this), attributeName, attributeValue);
}
