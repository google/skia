
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkSVGClipPath_DEFINED
#define SkSVGClipPath_DEFINED

#include "SkSVGElements.h"

class SkSVGClipPath : public SkSVGElement {
    DECLARE_SVG_INFO(ClipPath);
    virtual bool isDef();
    virtual bool isNotDef();
private:
    typedef SkSVGElement INHERITED;
};

#endif // SkSVGClipPath_DEFINED
