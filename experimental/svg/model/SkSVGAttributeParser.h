/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGAttributeParser_DEFINED
#define SkSVGAttributeParser_DEFINED

#include "SkSVGTypes.h"

class SkSVGAttributeParser : public SkNoncopyable {
public:
    SkSVGAttributeParser(const char[]);

    bool parseColor(SkSVGColorType*);
    bool parseNumber(SkSVGNumberType*);
    bool parseLength(SkSVGLength*);
    bool parseViewBox(SkSVGViewBoxType*);

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

    // The current position in the input string.
    const char* fCurPos;

    typedef SkNoncopyable INHERITED;
};

#endif // SkSVGAttributeParser_DEFINED
