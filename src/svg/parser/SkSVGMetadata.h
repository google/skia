/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkSVGMetadata_DEFINED
#define SkSVGMetadata_DEFINED

#include "SkSVGElements.h"

class SkSVGMetadata : public SkSVGElement {
    DECLARE_SVG_INFO(Metadata);
    virtual bool isDef();
    virtual bool isNotDef();
private:
    typedef SkSVGElement INHERITED;
};

#endif // SkSVGMetadata_DEFINED
