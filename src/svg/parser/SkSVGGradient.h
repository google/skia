
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkSVGGradient_DEFINED
#define SkSVGGradient_DEFINED

#include "SkSVGElements.h"

class SkSVGGradient : public SkSVGElement {
public:
    SkSVGGradient();
    virtual SkSVGElement* getGradient();
    virtual bool isDef();
    virtual bool isNotDef();
    virtual void write(SkSVGParser& , SkString& color);
protected:
    void translate(SkSVGParser& , bool defState);
    void translateGradientUnits(SkString& units);
private:
    typedef SkSVGElement INHERITED;
};

#endif // SkSVGGradient_DEFINED
