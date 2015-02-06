
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkSVGPath_DEFINED
#define SkSVGPath_DEFINED

#include "SkSVGElements.h"

class SkSVGPath : public SkSVGElement {
    DECLARE_SVG_INFO(Path);
private:
    SkString f_d;
    typedef SkSVGElement INHERITED;
};

#endif // SkSVGPath_DEFINED
