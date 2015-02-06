
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkSVGStop_DEFINED
#define SkSVGStop_DEFINED

#include "SkSVGElements.h"

class SkSVGStop : public SkSVGElement {
    DECLARE_SVG_INFO(Stop);
private:
    SkString f_offset;
    friend class SkSVGGradient;
    typedef SkSVGElement INHERITED;
};

#endif // SkSVGStop_DEFINED
