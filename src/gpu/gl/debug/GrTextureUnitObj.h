/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextureUnitObj_DEFINED
#define GrTextureUnitObj_DEFINED

#include "GrFakeRefObj.h"
class GrTextureObj;

////////////////////////////////////////////////////////////////////////////////
// Although texture unit objects are allocated & deallocated like the other
// GL emulation objects they are derived from GrFakeRefObj to provide some
// uniformity in how the GrDebugGL class manages resources
class GrTextureUnitObj : public GrFakeRefObj {
    GR_DEFINE_CREATOR(GrTextureUnitObj);

public:
    GrTextureUnitObj()
        : GrFakeRefObj()
        , fNumber(0)
        , fTexture(NULL) {
    }

    void setNumber(GrGLenum number) {
        fNumber = number;
    }
    GrGLenum getNumber() const { return fNumber; }

    void setTexture(GrTextureObj *texture);
    GrTextureObj *getTexture()                  { return fTexture; }

protected:
private:
    GrGLenum fNumber;
    GrTextureObj *fTexture;

    typedef GrFakeRefObj INHERITED;
};

#endif // GrTextureUnitObj_DEFINED
