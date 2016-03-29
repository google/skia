/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkSVGSymbol_DEFINED
#define SkSVGSymbol_DEFINED

#include "SkSVGElements.h"

class SkSVGSymbol : public SkSVGElement {
    DECLARE_SVG_INFO(Symbol);
private:
    SkString f_viewBox;
    typedef SkSVGElement INHERITED;
};

#endif // SkSVGSymbol_DEFINED
