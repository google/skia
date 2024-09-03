/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/svg/include/SkSVGAttributeParser.h"

#include "include/core/SkMatrix.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkString.h"
#include "include/private/base/SkTPin.h"
#include "include/utils/SkParse.h"
#include "modules/svg/include/SkSVGTypes.h"
#include "src/base/SkUTF.h"

#include <math.h>
#include <utility>

namespace {

// TODO: these should be shared with SkParse.cpp

inline bool is_between(char c, char min, char max) {
    SkASSERT(min <= max);
    return (unsigned)(c - min) <= (unsigned)(max - min);
}

inline bool is_ws(char c) {
    return is_between(c, 1, 32);
}

inline bool is_sep(char c) {
    return is_ws(c) || c == ',' || c == ';';
}

inline bool is_nl(char c) {
    return c == '\n' || c == '\r' || c == '\f';
}

inline bool is_hex(char c) {
    return is_between(c, 'a', 'f') ||
           is_between(c, 'A', 'F') ||
           is_between(c, '0', '9');
}

}  // namespace

SkSVGAttributeParser::SkSVGAttributeParser(const char attributeString[])
    // TODO: need actual UTF-8 with length.
    : fCurPos(attributeString), fEndPos(fCurPos + strlen(attributeString)) {}

template <typename F>
inline bool SkSVGAttributeParser::advanceWhile(F f) {
    auto initial = fCurPos;
    while (fCurPos < fEndPos && f(*fCurPos)) {
        fCurPos++;
    }
    return fCurPos != initial;
}

bool SkSVGAttributeParser::matchStringToken(const char* token, const char** newPos) const {
    const char* c = fCurPos;

    while (c < fEndPos && *token && *c == *token) {
        c++;
        token++;
    }

    if (*token) {
        return false;
    }

    if (newPos) {
        *newPos = c;
    }

    return true;
}

bool SkSVGAttributeParser::parseEOSToken() {
    return fCurPos == fEndPos;
}

bool SkSVGAttributeParser::parseSepToken() {
    return this->advanceWhile(is_sep);
}

bool SkSVGAttributeParser::parseWSToken() {
    return this->advanceWhile(is_ws);
}

bool SkSVGAttributeParser::parseCommaWspToken() {
    // comma-wsp:
    //     (wsp+ comma? wsp*) | (comma wsp*)
    return this->parseWSToken() || this->parseExpectedStringToken(",");
}

bool SkSVGAttributeParser::parseExpectedStringToken(const char* expected) {
    const char* newPos;
    if (!matchStringToken(expected, &newPos)) {
        return false;
    }

    fCurPos = newPos;
    return true;
}

bool SkSVGAttributeParser::parseScalarToken(SkScalar* res) {
    if (const char* next = SkParse::FindScalar(fCurPos, res)) {
        fCurPos = next;
        return true;
    }
    return false;
}

bool SkSVGAttributeParser::parseInt32Token(int32_t* res) {
    if (const char* next = SkParse::FindS32(fCurPos, res)) {
        fCurPos = next;
        return true;
    }
    return false;
}

bool SkSVGAttributeParser::matchHexToken(const char** newPos) const {
    *newPos = fCurPos;
    while (*newPos < fEndPos && is_hex(**newPos)) { ++*newPos; }
    return *newPos != fCurPos;
}

bool SkSVGAttributeParser::parseEscape(SkUnichar* c) {
    // \(hexDigit{1,6}whitespace?|[^newline|hexDigit])
    RestoreCurPos restoreCurPos(this);

    if (!this->parseExpectedStringToken("\\")) {
        return false;
    }
    const char* hexEnd;
    if (this->matchHexToken(&hexEnd)) {
        if (hexEnd - fCurPos > 6) {
            hexEnd = fCurPos + 6;
        }
        char hexString[7];
        size_t hexSize = hexEnd - fCurPos;
        memcpy(hexString, fCurPos, hexSize);
        hexString[hexSize] = '\0';
        uint32_t cp;
        const char* hexFound = SkParse::FindHex(hexString, &cp);
        if (!hexFound || cp < 1 || (0xD800 <= cp && cp <= 0xDFFF) || 0x10FFFF < cp) {
            cp = 0xFFFD;
        }
        *c = cp;
        fCurPos = hexEnd;
        this->parseWSToken();
    } else if (this->parseEOSToken() || is_nl(*fCurPos)) {
        *c = 0xFFFD;
        return false;
    } else {
        if ((*c = SkUTF::NextUTF8(&fCurPos, fEndPos)) < 0) {
            return false;
        }
    }

    restoreCurPos.clear();
    return true;
}

bool SkSVGAttributeParser::parseIdentToken(SkString* ident) {
    // <ident-token>
    // (--|-?([a-z|A-Z|_|non-ASCII]|escape))([a-z|A-Z|0-9|_|-|non-ASCII]|escape)?
    RestoreCurPos restoreCurPos(this);

    SkUnichar c;
    if (this->parseExpectedStringToken("--")) {
        ident->append("--");
    } else {
        if (this->parseExpectedStringToken("-")) {
            ident->append("-");
        }
        if (this->parseEscape(&c)) {
            ident->appendUnichar(c);
        } else {
            if ((c = SkUTF::NextUTF8(&fCurPos, fEndPos)) < 0) {
                return false;
            }
            if ((c < 'a' || 'z' < c) &&
                (c < 'A' || 'Z' < c) &&
                (c != '_') &&
                (c < 0x80 || 0x10FFFF < c))
            {
                return false;
            }
            ident->appendUnichar(c);
        }
    }
    while (fCurPos < fEndPos) {
        if (this->parseEscape(&c)) {
            ident->appendUnichar(c);
            continue;
        }
        const char* next = fCurPos;
        if ((c = SkUTF::NextUTF8(&next, fEndPos)) < 0) {
            break;
        }
        if ((c < 'a' || 'z' < c) &&
            (c < 'A' || 'Z' < c) &&
            (c < '0' || '9' < c) &&
            (c != '_') &&
            (c != '-') &&
            (c < 0x80 || 0x10FFFF < c))
        {
            break;
        }
        ident->appendUnichar(c);
        fCurPos = next;
    }

    restoreCurPos.clear();
    return true;
}

bool SkSVGAttributeParser::parseLengthUnitToken(SkSVGLength::Unit* unit) {
    static const struct {
        const char*       fUnitName;
        SkSVGLength::Unit fUnit;
    } gUnitInfo[] = {
        { "%" , SkSVGLength::Unit::kPercentage },
        { "em", SkSVGLength::Unit::kEMS        },
        { "ex", SkSVGLength::Unit::kEXS        },
        { "px", SkSVGLength::Unit::kPX         },
        { "cm", SkSVGLength::Unit::kCM         },
        { "mm", SkSVGLength::Unit::kMM         },
        { "in", SkSVGLength::Unit::kIN         },
        { "pt", SkSVGLength::Unit::kPT         },
        { "pc", SkSVGLength::Unit::kPC         },
    };

    for (size_t i = 0; i < std::size(gUnitInfo); ++i) {
        if (this->parseExpectedStringToken(gUnitInfo[i].fUnitName)) {
            *unit = gUnitInfo[i].fUnit;
            return true;
        }
    }
    return false;
}

// https://www.w3.org/TR/SVG11/types.html#DataTypeColor
bool SkSVGAttributeParser::parseNamedColorToken(SkColor* c) {
    RestoreCurPos restoreCurPos(this);

    SkString ident;
    if (!this->parseIdentToken(&ident)) {
        return false;
    }
    if (!SkParse::FindNamedColor(ident.c_str(), ident.size(), c)) {
        return false;
    }

    restoreCurPos.clear();
    return true;
}

bool SkSVGAttributeParser::parseHexColorToken(SkColor* c) {
    RestoreCurPos restoreCurPos(this);

    const char* hexEnd;
    if (!this->parseExpectedStringToken("#") || !this->matchHexToken(&hexEnd)) {
        return false;
    }

    uint32_t v;
    SkString hexString(fCurPos, hexEnd - fCurPos);
    SkParse::FindHex(hexString.c_str(), &v);

    switch (hexString.size()) {
    case 6:
        // matched #xxxxxxx
        break;
    case 3:
        // matched '#xxx;
        v = ((v << 12) & 0x00f00000) |
            ((v <<  8) & 0x000ff000) |
            ((v <<  4) & 0x00000ff0) |
            ((v <<  0) & 0x0000000f);
        break;
    default:
        return false;
    }

    *c = v | 0xff000000;
    fCurPos = hexEnd;

    restoreCurPos.clear();
    return true;
}

bool SkSVGAttributeParser::parseColorComponentIntegralToken(int32_t* c) {
    const char* p = SkParse::FindS32(fCurPos, c);
    if (!p || *p == '.') {
        // No value parsed, or fractional value.
        return false;
    }

    if (*p == '%') {
        *c = SkScalarRoundToInt(*c * 255.0f / 100);
        *c = SkTPin<int32_t>(*c, 0, 255);
        p++;
    }

    fCurPos = p;
    return true;
}

bool SkSVGAttributeParser::parseColorComponentFractionalToken(int32_t* c) {
    SkScalar s;
    const char* p = SkParse::FindScalar(fCurPos, &s);
    if (!p || *p != '%') {
        // Floating point must be a percentage (CSS2 rgb-percent syntax).
        return false;
    }
    p++;  // Skip '%'

    *c = SkScalarRoundToInt(s * 255.0f / 100);
    *c = SkTPin<int32_t>(*c, 0, 255);
    fCurPos = p;
    return true;
}

bool SkSVGAttributeParser::parseColorComponentScalarToken(int32_t* c) {
    SkScalar s;
    if (const char* p = SkParse::FindScalar(fCurPos, &s)) {
        *c = SkScalarRoundToInt(s * 255.0f);
        *c = SkTPin<int32_t>(*c, 0, 255);
        fCurPos = p;
        return true;
    }
    return false;
}

bool SkSVGAttributeParser::parseColorComponentToken(int32_t* c) {
    return parseColorComponentIntegralToken(c) ||
           parseColorComponentFractionalToken(c);
}

bool SkSVGAttributeParser::parseRGBColorToken(SkColor* c) {
    return this->parseParenthesized("rgb", [this](SkColor* c) -> bool {
        int32_t r, g, b;
        if (this->parseColorComponentToken(&r) &&
            this->parseSepToken() &&
            this->parseColorComponentToken(&g) &&
            this->parseSepToken() &&
            this->parseColorComponentToken(&b)) {

            *c = SkColorSetRGB(static_cast<uint8_t>(r),
                               static_cast<uint8_t>(g),
                               static_cast<uint8_t>(b));
            return true;
        }
        return false;
    }, c);
}

bool SkSVGAttributeParser::parseRGBAColorToken(SkColor* c) {
    return this->parseParenthesized("rgba", [this](SkColor* c) -> bool {
        int32_t r, g, b, a;
        if (this->parseColorComponentToken(&r) &&
            this->parseSepToken() &&
            this->parseColorComponentToken(&g) &&
            this->parseSepToken() &&
            this->parseColorComponentToken(&b) &&
            this->parseSepToken() &&
            this->parseColorComponentScalarToken(&a)) {

            *c = SkColorSetARGB(static_cast<uint8_t>(a),
                                static_cast<uint8_t>(r),
                                static_cast<uint8_t>(g),
                                static_cast<uint8_t>(b));
            return true;
        }
        return false;
    }, c);
}

bool SkSVGAttributeParser::parseColorToken(SkColor* c) {
    return this->parseHexColorToken(c) ||
           this->parseNamedColorToken(c) ||
           this->parseRGBAColorToken(c) ||
           this->parseRGBColorToken(c);
}

bool SkSVGAttributeParser::parseSVGColorType(SkSVGColorType* color) {
    SkColor c;
    if (!this->parseColorToken(&c)) {
        return false;
    }
    *color = SkSVGColorType(c);
    return true;
}

// https://www.w3.org/TR/SVG11/types.html#DataTypeColor
// And https://www.w3.org/TR/CSS2/syndata.html#color-units for the alternative
// forms supported by SVG (e.g. RGB percentages).
template <>
bool SkSVGAttributeParser::parse(SkSVGColorType* color) {
    this->parseWSToken();
    if (!this->parseSVGColorType(color)) {
        return false;
    }
    this->parseWSToken();
    return this->parseEOSToken();
}

bool SkSVGAttributeParser::parseSVGColor(SkSVGColor* color, SkSVGColor::Vars&& vars) {
    static const constexpr int kVarsLimit = 32;

    if (SkSVGColorType c; this->parseSVGColorType(&c)) {
        *color = SkSVGColor(c, std::move(vars));
        return true;
    }
    if (this->parseExpectedStringToken("currentColor")) {
        *color = SkSVGColor(SkSVGColor::Type::kCurrentColor, std::move(vars));
        return true;
    }
    // https://drafts.csswg.org/css-variables/#using-variables
    if (this->parseParenthesized("var", [this, &vars](SkSVGColor* colorResult) -> bool {
            SkString ident;
            if (!this->parseIdentToken(&ident) || ident.size() < 2 || !ident.startsWith("--")) {
                return false;
            }
            ident.remove(0, 2);
            vars.push_back(std::move(ident));
            this->parseWSToken();
            if (!this->parseExpectedStringToken(",")) {
                *colorResult = SkSVGColor(SK_ColorBLACK, std::move(vars));
                return true;
            }
            this->parseWSToken();
            if (this->matchStringToken(")")) {
                *colorResult = SkSVGColor(SK_ColorBLACK, std::move(vars));
                return true;
            }
            return vars.size() < kVarsLimit && this->parseSVGColor(colorResult, std::move(vars));
        }, color))
    {
        return true;
    }
    return false;
}

// https://www.w3.org/TR/SVG11/types.html#InterfaceSVGColor
template <>
bool SkSVGAttributeParser::parse(SkSVGColor* color) {
    this->parseWSToken();
    if (!this->parseSVGColor(color, SkSVGColor::Vars())) {
        return false;
    }
    this->parseWSToken();
    return this->parseEOSToken();
}

// https://www.w3.org/TR/SVG11/linking.html#IRIReference
template <>
bool SkSVGAttributeParser::parse(SkSVGIRI* iri) {
    // consume preceding whitespace
    this->parseWSToken();

    SkSVGIRI::Type iriType;
    if (this->parseExpectedStringToken("#")) {
        iriType = SkSVGIRI::Type::kLocal;
    } else if (this->matchStringToken("data:")) {
        iriType = SkSVGIRI::Type::kDataURI;
    } else {
        iriType = SkSVGIRI::Type::kNonlocal;
    }

    const auto* start = fCurPos;
    if (!this->advanceWhile([](char c) -> bool { return c != ')'; })) {
        return false;
    }
    *iri = SkSVGIRI(iriType, SkString(start, fCurPos - start));
    return true;
}

// https://www.w3.org/TR/SVG11/types.html#DataTypeFuncIRI
bool SkSVGAttributeParser::parseFuncIRI(SkSVGFuncIRI* iri) {
    return this->parseParenthesized("url", [this](SkSVGFuncIRI* iriResult) -> bool {
        SkSVGIRI iri;
        if (this->parse(&iri)) {
            *iriResult = SkSVGFuncIRI(std::move(iri));
            return true;
        }
        return false;
    }, iri);
}

template <>
bool SkSVGAttributeParser::parse(SkSVGStringType* result) {
    if (this->parseEOSToken()) {
        return false;
    }
    *result = SkSVGStringType(fCurPos);
    fCurPos += result->size();
    return this->parseEOSToken();
}

// https://www.w3.org/TR/SVG11/types.html#DataTypeNumber
template <>
bool SkSVGAttributeParser::parse(SkSVGNumberType* number) {
    // consume WS
    this->parseWSToken();

    SkScalar s;
    if (this->parseScalarToken(&s)) {
        *number = SkSVGNumberType(s);
        // consume trailing separators
        this->parseSepToken();
        return true;
    }

    return false;
}

// https://www.w3.org/TR/SVG11/types.html#DataTypeInteger
bool SkSVGAttributeParser::parseInteger(SkSVGIntegerType* number) {
    // consume WS
    this->parseWSToken();

    // consume optional '+'
    this->parseExpectedStringToken("+");

    SkSVGIntegerType i;
    if (this->parseInt32Token(&i)) {
        *number = SkSVGNumberType(i);
        // consume trailing separators
        this->parseSepToken();
        return true;
    }

    return false;
}

// https://www.w3.org/TR/SVG11/types.html#DataTypeLength
template <>
bool SkSVGAttributeParser::parse(SkSVGLength* length) {
    SkScalar s;
    SkSVGLength::Unit u = SkSVGLength::Unit::kNumber;

    if (this->parseScalarToken(&s) &&
        (this->parseLengthUnitToken(&u) || this->parseSepToken() || this->parseEOSToken())) {
        *length = SkSVGLength(s, u);
        // consume trailing separators
        this->parseSepToken();
        return true;
    }

    return false;
}

// https://www.w3.org/TR/SVG11/coords.html#ViewBoxAttribute
bool SkSVGAttributeParser::parseViewBox(SkSVGViewBoxType* vb) {
    SkScalar x, y, w, h;
    this->parseWSToken();

    bool parsedValue = false;
    if (this->parseScalarToken(&x) && this->parseSepToken() &&
        this->parseScalarToken(&y) && this->parseSepToken() &&
        this->parseScalarToken(&w) && this->parseSepToken() &&
        this->parseScalarToken(&h)) {

        *vb = SkSVGViewBoxType(SkRect::MakeXYWH(x, y, w, h));
        parsedValue = true;
        // consume trailing whitespace
        this->parseWSToken();
    }
    return parsedValue && this->parseEOSToken();
}

template <typename Func, typename T>
bool SkSVGAttributeParser::parseParenthesized(const char* prefix, Func f, T* result) {
    RestoreCurPos restoreCurPos(this);

    this->parseWSToken();
    if (prefix && !this->parseExpectedStringToken(prefix)) {
        return false;
    }
    this->parseWSToken();
    if (!this->parseExpectedStringToken("(")) {
        return false;
    }
    this->parseWSToken();

    if (!f(result)) {
        return false;
    }

    this->parseWSToken();
    if (!this->parseExpectedStringToken(")")) {
        return false;
    }

    restoreCurPos.clear();
    return true;
}

bool SkSVGAttributeParser::parseMatrixToken(SkMatrix* matrix) {
    return this->parseParenthesized("matrix", [this](SkMatrix* m) -> bool {
        SkScalar scalars[6];
        for (int i = 0; i < 6; ++i) {
            if (!(this->parseScalarToken(scalars + i) &&
                  (i > 4 || this->parseSepToken()))) {
                return false;
            }
        }

        m->setAll(scalars[0], scalars[2], scalars[4], scalars[1], scalars[3], scalars[5], 0, 0, 1);
        return true;
    }, matrix);
}

bool SkSVGAttributeParser::parseTranslateToken(SkMatrix* matrix) {
    return this->parseParenthesized("translate", [this](SkMatrix* m) -> bool {
        SkScalar tx = 0.0, ty = 0.0;
        this->parseWSToken();
        if (!this->parseScalarToken(&tx)) {
            return false;
        }

        if (!this->parseSepToken() || !this->parseScalarToken(&ty)) {
            ty = 0.0;
        }

        m->setTranslate(tx, ty);
        return true;
    }, matrix);
}

bool SkSVGAttributeParser::parseScaleToken(SkMatrix* matrix) {
    return this->parseParenthesized("scale", [this](SkMatrix* m) -> bool {
        SkScalar sx = 0.0, sy = 0.0;
        if (!this->parseScalarToken(&sx)) {
            return false;
        }

        if (!(this->parseSepToken() && this->parseScalarToken(&sy))) {
            sy = sx;
        }

        m->setScale(sx, sy);
        return true;
    }, matrix);
}

bool SkSVGAttributeParser::parseRotateToken(SkMatrix* matrix) {
    return this->parseParenthesized("rotate", [this](SkMatrix* m) -> bool {
        SkScalar angle;
        if (!this->parseScalarToken(&angle)) {
            return false;
        }

        SkScalar cx = 0;
        SkScalar cy = 0;
        // optional [<cx> <cy>]
        if (this->parseSepToken() && this->parseScalarToken(&cx)) {
            if (!(this->parseSepToken() && this->parseScalarToken(&cy))) {
                return false;
            }
        }

        m->setRotate(angle, cx, cy);
        return true;
    }, matrix);
}

bool SkSVGAttributeParser::parseSkewXToken(SkMatrix* matrix) {
    return this->parseParenthesized("skewX", [this](SkMatrix* m) -> bool {
        SkScalar angle;
        if (!this->parseScalarToken(&angle)) {
            return false;
        }
        m->setSkewX(tanf(SkDegreesToRadians(angle)));
        return true;
    }, matrix);
}

bool SkSVGAttributeParser::parseSkewYToken(SkMatrix* matrix) {
    return this->parseParenthesized("skewY", [this](SkMatrix* m) -> bool {
        SkScalar angle;
        if (!this->parseScalarToken(&angle)) {
            return false;
        }
        m->setSkewY(tanf(SkDegreesToRadians(angle)));
        return true;
    }, matrix);
}

// https://www.w3.org/TR/SVG11/coords.html#TransformAttribute
template <>
bool SkSVGAttributeParser::parse(SkSVGTransformType* t) {
    SkMatrix matrix = SkMatrix::I();

    bool parsed = false;
    while (true) {
        SkMatrix m;

        if (!( this->parseMatrixToken(&m)
            || this->parseTranslateToken(&m)
            || this->parseScaleToken(&m)
            || this->parseRotateToken(&m)
            || this->parseSkewXToken(&m)
            || this->parseSkewYToken(&m))) {
            break;
        }

        matrix.preConcat(m);
        parsed = true;

        this->parseCommaWspToken();
    }

    this->parseWSToken();
    if (!parsed || !this->parseEOSToken()) {
        return false;
    }

    *t = SkSVGTransformType(matrix);
    return true;
}

// https://www.w3.org/TR/SVG11/painting.html#SpecifyingPaint
template <>
bool SkSVGAttributeParser::parse(SkSVGPaint* paint) {
    SkSVGColor c;
    SkSVGFuncIRI iri;
    bool parsedValue = false;

    this->parseWSToken();
    if (this->parseSVGColor(&c, SkSVGColor::Vars())) {
        *paint = SkSVGPaint(std::move(c));
        parsedValue = true;
    } else if (this->parseExpectedStringToken("none")) {
        *paint = SkSVGPaint(SkSVGPaint::Type::kNone);
        parsedValue = true;
    } else if (this->parseFuncIRI(&iri)) {
        // optional fallback color
        this->parseWSToken();
        this->parseSVGColor(&c, SkSVGColor::Vars());
        *paint = SkSVGPaint(iri.iri(), std::move(c));
        parsedValue = true;
    }
    this->parseWSToken();
    return parsedValue && this->parseEOSToken();
}

// https://www.w3.org/TR/SVG11/masking.html#ClipPathProperty
// https://www.w3.org/TR/SVG11/masking.html#MaskProperty
// https://www.w3.org/TR/SVG11/filters.html#FilterProperty
template <>
bool SkSVGAttributeParser::parse(SkSVGFuncIRI* firi) {
    SkSVGStringType iri;
    bool parsedValue = false;

    if (this->parseExpectedStringToken("none")) {
        *firi = SkSVGFuncIRI();
        parsedValue = true;
    } else if (this->parseFuncIRI(firi)) {
        parsedValue = true;
    }

    return parsedValue && this->parseEOSToken();
}

// https://www.w3.org/TR/SVG11/painting.html#StrokeLinecapProperty
template <>
bool SkSVGAttributeParser::parse(SkSVGLineCap* cap) {
    static const struct {
        SkSVGLineCap fType;
        const char*        fName;
    } gCapInfo[] = {
        { SkSVGLineCap::kButt   , "butt"    },
        { SkSVGLineCap::kRound  , "round"   },
        { SkSVGLineCap::kSquare , "square"  },
    };

    bool parsedValue = false;
    for (size_t i = 0; i < std::size(gCapInfo); ++i) {
        if (this->parseExpectedStringToken(gCapInfo[i].fName)) {
            *cap = SkSVGLineCap(gCapInfo[i].fType);
            parsedValue = true;
            break;
        }
    }

    return parsedValue && this->parseEOSToken();
}

// https://www.w3.org/TR/SVG11/painting.html#StrokeLinejoinProperty
template <>
bool SkSVGAttributeParser::parse(SkSVGLineJoin* join) {
    static const struct {
        SkSVGLineJoin::Type fType;
        const char*         fName;
    } gJoinInfo[] = {
        { SkSVGLineJoin::Type::kMiter  , "miter"   },
        { SkSVGLineJoin::Type::kRound  , "round"   },
        { SkSVGLineJoin::Type::kBevel  , "bevel"   },
        { SkSVGLineJoin::Type::kInherit, "inherit" },
    };

    bool parsedValue = false;
    for (size_t i = 0; i < std::size(gJoinInfo); ++i) {
        if (this->parseExpectedStringToken(gJoinInfo[i].fName)) {
            *join = SkSVGLineJoin(gJoinInfo[i].fType);
            parsedValue = true;
            break;
        }
    }

    return parsedValue && this->parseEOSToken();
}

// https://www.w3.org/TR/SVG11/coords.html#ObjectBoundingBoxUnits
template <>
bool SkSVGAttributeParser::parse(SkSVGObjectBoundingBoxUnits* objectBoundingBoxUnits) {
    bool parsedValue = false;
    if (this->parseExpectedStringToken("userSpaceOnUse")) {
        *objectBoundingBoxUnits =
                SkSVGObjectBoundingBoxUnits(SkSVGObjectBoundingBoxUnits::Type::kUserSpaceOnUse);
        parsedValue = true;
    } else if (this->parseExpectedStringToken("objectBoundingBox")) {
        *objectBoundingBoxUnits =
                SkSVGObjectBoundingBoxUnits(SkSVGObjectBoundingBoxUnits::Type::kObjectBoundingBox);
        parsedValue = true;
    }
    return parsedValue && this->parseEOSToken();
}

// https://www.w3.org/TR/SVG11/shapes.html#PolygonElementPointsAttribute
template <>
bool SkSVGAttributeParser::parse(SkSVGPointsType* points) {
    SkSVGPointsType pts;

    // Skip initial wsp.
    // list-of-points:
    //     wsp* coordinate-pairs? wsp*
    this->advanceWhile(is_ws);

    bool parsedValue = false;
    for (;;) {
        // Adjacent coordinate-pairs separated by comma-wsp.
        // coordinate-pairs:
        //     coordinate-pair
        //     | coordinate-pair comma-wsp coordinate-pairs
        if (parsedValue && !this->parseCommaWspToken()) {
            break;
        }

        SkScalar x, y;
        if (!this->parseScalarToken(&x)) {
            break;
        }

        // Coordinate values separated by comma-wsp or '-'.
        // coordinate-pair:
        //     coordinate comma-wsp coordinate
        //     | coordinate negative-coordinate
        if (!this->parseCommaWspToken() && !this->parseEOSToken() && *fCurPos != '-') {
            break;
        }

        if (!this->parseScalarToken(&y)) {
            break;
        }

        pts.push_back(SkPoint::Make(x, y));
        parsedValue = true;
    }

    if (parsedValue && this->parseEOSToken()) {
        *points = std::move(pts);
        return true;
    }

    return false;
}

// https://www.w3.org/TR/SVG11/painting.html#FillRuleProperty
template <>
bool SkSVGAttributeParser::parse(SkSVGFillRule* fillRule) {
    static const struct {
        SkSVGFillRule::Type fType;
        const char*         fName;
    } gFillRuleInfo[] = {
        { SkSVGFillRule::Type::kNonZero, "nonzero" },
        { SkSVGFillRule::Type::kEvenOdd, "evenodd" },
        { SkSVGFillRule::Type::kInherit, "inherit" },
    };

    bool parsedValue = false;
    for (size_t i = 0; i < std::size(gFillRuleInfo); ++i) {
        if (this->parseExpectedStringToken(gFillRuleInfo[i].fName)) {
            *fillRule = SkSVGFillRule(gFillRuleInfo[i].fType);
            parsedValue = true;
            break;
        }
    }

    return parsedValue && this->parseEOSToken();
}

// https://www.w3.org/TR/SVG11/painting.html#VisibilityProperty
template <>
bool SkSVGAttributeParser::parse(SkSVGVisibility* visibility) {
    static const struct {
        SkSVGVisibility::Type fType;
        const char*           fName;
    } gVisibilityInfo[] = {
        { SkSVGVisibility::Type::kVisible , "visible"  },
        { SkSVGVisibility::Type::kHidden  , "hidden"   },
        { SkSVGVisibility::Type::kCollapse, "collapse" },
        { SkSVGVisibility::Type::kInherit , "inherit"  },
    };

    bool parsedValue = false;
    for (const auto& parseInfo : gVisibilityInfo) {
        if (this->parseExpectedStringToken(parseInfo.fName)) {
            *visibility = SkSVGVisibility(parseInfo.fType);
            parsedValue = true;
            break;
        }
    }

    return parsedValue && this->parseEOSToken();
}

// https://www.w3.org/TR/SVG11/painting.html#StrokeDasharrayProperty
template <>
bool SkSVGAttributeParser::parse(SkSVGDashArray* dashArray) {
    bool parsedValue = false;
    if (this->parseExpectedStringToken("none")) {
        *dashArray = SkSVGDashArray(SkSVGDashArray::Type::kNone);
        parsedValue = true;
    } else if (this->parseExpectedStringToken("inherit")) {
        *dashArray = SkSVGDashArray(SkSVGDashArray::Type::kInherit);
        parsedValue = true;
    } else {
        std::vector<SkSVGLength> dashes;
        for (;;) {
            SkSVGLength dash;
            // parseLength() also consumes trailing separators.
            if (!this->parse(&dash)) {
                break;
            }

            dashes.push_back(dash);
            parsedValue = true;
        }

        if (parsedValue) {
            *dashArray = SkSVGDashArray(std::move(dashes));
        }
    }

    return parsedValue && this->parseEOSToken();
}

// https://www.w3.org/TR/SVG11/text.html#FontFamilyProperty
template <>
bool SkSVGAttributeParser::parse(SkSVGFontFamily* family) {
    bool parsedValue = false;
    if (this->parseExpectedStringToken("inherit")) {
        *family = SkSVGFontFamily();
        parsedValue = true;
    } else {
        // The spec allows specifying a comma-separated list for explicit fallback order.
        // For now, we only use the first entry and rely on the font manager to handle fallback.
        const auto* comma = strchr(fCurPos, ',');
        auto family_name = comma ? SkString(fCurPos, comma - fCurPos)
                                 : SkString(fCurPos);
        *family = SkSVGFontFamily(family_name.c_str());
        fCurPos += strlen(fCurPos);
        parsedValue = true;
    }

    return parsedValue && this->parseEOSToken();
}

// https://www.w3.org/TR/SVG11/text.html#FontSizeProperty
template <>
bool SkSVGAttributeParser::parse(SkSVGFontSize* size) {
    bool parsedValue = false;
    if (this->parseExpectedStringToken("inherit")) {
        *size = SkSVGFontSize();
        parsedValue = true;
    } else {
        SkSVGLength length;
        if (this->parse(&length)) {
            *size = SkSVGFontSize(length);
            parsedValue = true;
        }
    }

    return parsedValue && this->parseEOSToken();
}

// https://www.w3.org/TR/SVG11/text.html#FontStyleProperty
template <>
bool SkSVGAttributeParser::parse(SkSVGFontStyle* style) {
    static constexpr std::tuple<const char*, SkSVGFontStyle::Type> gStyleMap[] = {
        { "normal" , SkSVGFontStyle::Type::kNormal  },
        { "italic" , SkSVGFontStyle::Type::kItalic  },
        { "oblique", SkSVGFontStyle::Type::kOblique },
        { "inherit", SkSVGFontStyle::Type::kInherit },
    };

    bool parsedValue = false;
    SkSVGFontStyle::Type type;

    if (this->parseEnumMap(gStyleMap, &type)) {
        *style = SkSVGFontStyle(type);
        parsedValue = true;
    }

    return parsedValue && this->parseEOSToken();
}

// https://www.w3.org/TR/SVG11/text.html#FontWeightProperty
template <>
bool SkSVGAttributeParser::parse(SkSVGFontWeight* weight) {
    static constexpr std::tuple<const char*, SkSVGFontWeight::Type> gWeightMap[] = {
        { "normal" , SkSVGFontWeight::Type::kNormal  },
        { "bold"   , SkSVGFontWeight::Type::kBold    },
        { "bolder" , SkSVGFontWeight::Type::kBolder  },
        { "lighter", SkSVGFontWeight::Type::kLighter },
        { "100"    , SkSVGFontWeight::Type::k100     },
        { "200"    , SkSVGFontWeight::Type::k200     },
        { "300"    , SkSVGFontWeight::Type::k300     },
        { "400"    , SkSVGFontWeight::Type::k400     },
        { "500"    , SkSVGFontWeight::Type::k500     },
        { "600"    , SkSVGFontWeight::Type::k600     },
        { "700"    , SkSVGFontWeight::Type::k700     },
        { "800"    , SkSVGFontWeight::Type::k800     },
        { "900"    , SkSVGFontWeight::Type::k900     },
        { "inherit", SkSVGFontWeight::Type::kInherit },
    };

    bool parsedValue = false;
    SkSVGFontWeight::Type type;

    if (this->parseEnumMap(gWeightMap, &type)) {
        *weight = SkSVGFontWeight(type);
        parsedValue = true;
    }

    return parsedValue && this->parseEOSToken();
}

// https://www.w3.org/TR/SVG11/text.html#TextAnchorProperty
template <>
bool SkSVGAttributeParser::parse(SkSVGTextAnchor* anchor) {
    static constexpr std::tuple<const char*, SkSVGTextAnchor::Type> gAnchorMap[] = {
        { "start"  , SkSVGTextAnchor::Type::kStart  },
        { "middle" , SkSVGTextAnchor::Type::kMiddle },
        { "end"    , SkSVGTextAnchor::Type::kEnd    },
        { "inherit", SkSVGTextAnchor::Type::kInherit},
    };

    bool parsedValue = false;
    SkSVGTextAnchor::Type type;

    if (this->parseEnumMap(gAnchorMap, &type)) {
        *anchor = SkSVGTextAnchor(type);
        parsedValue = true;
    }

    return parsedValue && this->parseEOSToken();
}

// https://www.w3.org/TR/SVG11/coords.html#PreserveAspectRatioAttribute
bool SkSVGAttributeParser::parsePreserveAspectRatio(SkSVGPreserveAspectRatio* par) {
    static constexpr std::tuple<const char*, SkSVGPreserveAspectRatio::Align> gAlignMap[] = {
        { "none"    , SkSVGPreserveAspectRatio::kNone     },
        { "xMinYMin", SkSVGPreserveAspectRatio::kXMinYMin },
        { "xMidYMin", SkSVGPreserveAspectRatio::kXMidYMin },
        { "xMaxYMin", SkSVGPreserveAspectRatio::kXMaxYMin },
        { "xMinYMid", SkSVGPreserveAspectRatio::kXMinYMid },
        { "xMidYMid", SkSVGPreserveAspectRatio::kXMidYMid },
        { "xMaxYMid", SkSVGPreserveAspectRatio::kXMaxYMid },
        { "xMinYMax", SkSVGPreserveAspectRatio::kXMinYMax },
        { "xMidYMax", SkSVGPreserveAspectRatio::kXMidYMax },
        { "xMaxYMax", SkSVGPreserveAspectRatio::kXMaxYMax },
    };

    static constexpr std::tuple<const char*, SkSVGPreserveAspectRatio::Scale> gScaleMap[] = {
        { "meet" , SkSVGPreserveAspectRatio::kMeet  },
        { "slice", SkSVGPreserveAspectRatio::kSlice },
    };

    bool parsedValue = false;

    // ignoring optional 'defer'
    this->parseExpectedStringToken("defer");
    this->parseWSToken();

    if (this->parseEnumMap(gAlignMap, &par->fAlign)) {
        parsedValue = true;

        // optional scaling selector
        this->parseWSToken();
        this->parseEnumMap(gScaleMap, &par->fScale);
    }

    return parsedValue && this->parseEOSToken();
}

template <>
bool SkSVGAttributeParser::parse(SkSVGPreserveAspectRatio* par) {
    return this->parsePreserveAspectRatio(par);
}

// https://www.w3.org/TR/SVG11/types.html#DataTypeCoordinates
template <typename T>
bool SkSVGAttributeParser::parseList(std::vector<T>* vals) {
    SkASSERT(vals->empty());

    T v;
    for (;;) {
        if (!this->parse(&v)) {
            break;
        }

        vals->push_back(v);

        this->parseCommaWspToken();
    }

    return !vals->empty() && this->parseEOSToken();
}

template <>
bool SkSVGAttributeParser::parse(std::vector<SkSVGLength>* lengths) {
    return this->parseList(lengths);
}

template <>
bool SkSVGAttributeParser::parse(std::vector<SkSVGNumberType>* numbers) {
    return this->parseList(numbers);
}

template <>
bool SkSVGAttributeParser::parse(SkSVGColorspace* colorspace) {
    static constexpr std::tuple<const char*, SkSVGColorspace> gColorspaceMap[] = {
        { "auto"     , SkSVGColorspace::kAuto      },
        { "sRGB"     , SkSVGColorspace::kSRGB      },
        { "linearRGB", SkSVGColorspace::kLinearRGB },
    };

    return this->parseEnumMap(gColorspaceMap, colorspace) && this->parseEOSToken();
}

// https://www.w3.org/TR/SVG11/painting.html#DisplayProperty
template <>
bool SkSVGAttributeParser::parse(SkSVGDisplay* display) {
    static const struct {
        SkSVGDisplay fType;
        const char*  fName;
    } gDisplayInfo[] = {
        { SkSVGDisplay::kInline, "inline" },
        { SkSVGDisplay::kNone  , "none"   },
    };

    bool parsedValue = false;
    for (const auto& parseInfo : gDisplayInfo) {
        if (this->parseExpectedStringToken(parseInfo.fName)) {
            *display = SkSVGDisplay(parseInfo.fType);
            parsedValue = true;
            break;
        }
    }

    return parsedValue && this->parseEOSToken();
}
