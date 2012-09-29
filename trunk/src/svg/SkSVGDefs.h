
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkSVGDefs_DEFINED
#define SkSVGDefs_DEFINED

#include "SkSVGGroup.h"

class SkSVGDefs : public SkSVGGroup {
    DECLARE_SVG_INFO(Defs);
    virtual bool isDef();
    virtual bool isNotDef();
private:
    typedef SkSVGGroup INHERITED;
};

#endif // SkSVGDefs_DEFINED
