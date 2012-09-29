
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkSVGAttribute_DEFINED
#define SkSVGAttribute_DEFINED

#include "SkTypes.h"

struct SkSVGAttribute {
    const char* fName;
#ifdef SK_DEBUG
    size_t fOffset;
#endif
};

#ifndef SK_OFFSETOF
#define SK_OFFSETOF(a, b) (((size_t) (&(((a*) 1)->b)))-1)
#endif

#ifdef SK_DEBUG
#define SVG_ATTRIBUTE(attr) { #attr, SK_OFFSETOF(BASE_CLASS, f_##attr) }
#define SVG_LITERAL_ATTRIBUTE(svgAttr, cAttr) { #svgAttr, SK_OFFSETOF(BASE_CLASS, cAttr) }
#else
#define SVG_ATTRIBUTE(attr) { #attr }
#define SVG_LITERAL_ATTRIBUTE(svgAttr, cAttr) { #svgAttr }
#endif

#define SVG_ADD_ATTRIBUTE(attr) \
    if (f_##attr.size() > 0) \
        parser._addAttributeLen(#attr, f_##attr.c_str(), f_##attr.size())

#define SVG_ADD_ATTRIBUTE_ALIAS(attr, alias) \
    if (f_##alias.size() > 0) \
        parser._addAttributeLen(#attr, f_##alias.c_str(), f_##alias.size())

#endif // SkSVGAttribute_DEFINED
