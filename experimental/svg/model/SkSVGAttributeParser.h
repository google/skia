/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGAttributeParser_DEFINED
#define SkSVGAttributeParser_DEFINED

#include "experimental/svg/model/SkSVGTypes.h"
#include "include/private/SkNoncopyable.h"

class SkSVGAttributeParser : public SkNoncopyable {
public:
    SkSVGAttributeParser(const char[]);

    bool parseColor(SkSVGColorType*);
    bool parseClipPath(SkSVGClip*);
    bool parseFillRule(SkSVGFillRule*);
    bool parseNumber(SkSVGNumberType*);
    bool parseLength(SkSVGLength*);
    bool parseViewBox(SkSVGViewBoxType*);
    bool parseTransform(SkSVGTransformType*);
    bool parsePaint(SkSVGPaint*);
    bool parseLineCap(SkSVGLineCap*);
    bool parseLineJoin(SkSVGLineJoin*);
    bool parsePoints(SkSVGPointsType*);
    bool parseIRI(SkSVGStringType*);
    bool parseSpreadMethod(SkSVGSpreadMethod*);
    bool parseVisibility(SkSVGVisibility*);
    bool parseDashArray(SkSVGDashArray*);

private:
    // Stack-only
    void* operator new(size_t) = delete;
    void* operator new(size_t, void*) = delete;

    template <typename F>
    bool advanceWhile(F func);

    bool parseWSToken();
    bool parseEOSToken();
    bool parseSepToken();
    bool parseExpectedStringToken(const char*);
    bool parseScalarToken(SkScalar*);
    bool parseHexToken(uint32_t*);
    bool parseLengthUnitToken(SkSVGLength::Unit*);
    bool parseNamedColorToken(SkColor*);
    bool parseHexColorToken(SkColor*);
    bool parseColorComponentToken(int32_t*);
    bool parseRGBColorToken(SkColor*);
    bool parseFuncIRI(SkSVGStringType*);

    // Transform helpers
    bool parseMatrixToken(SkMatrix*);
    bool parseTranslateToken(SkMatrix*);
    bool parseScaleToken(SkMatrix*);
    bool parseRotateToken(SkMatrix*);
    bool parseSkewXToken(SkMatrix*);
    bool parseSkewYToken(SkMatrix*);

    // Parses a sequence of 'WS* <prefix> WS* (<nested>)', where the nested sequence
    // is handled by the passed functor.
    template <typename Func, typename T>
    bool parseParenthesized(const char* prefix, Func, T* result);

    // The current position in the input string.
    const char* fCurPos;

    typedef SkNoncopyable INHERITED;
};

#endif // SkSVGAttributeParser_DEFINED
