
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkSVGBase_DEFINED
#define SkSVGBase_DEFINED

#include "SkSVGAttribute.h"

class SkSVGParser;

class SkSVGBase {
public:
    virtual ~SkSVGBase();
    virtual void addAttribute(SkSVGParser& parser, int attrIndex,
        const char* attrValue, size_t attrLength);
    virtual int getAttributes(const SkSVGAttribute** attrPtr) = 0;
};

#endif // SkSVGBase_DEFINEDes(const SkSVGAttribute** attrPtr) = 0;
