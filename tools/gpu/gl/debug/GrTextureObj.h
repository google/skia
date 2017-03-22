
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextureObj_DEFINED
#define GrTextureObj_DEFINED

#include "GrFBBindableObj.h"

class GrTextureUnitObj;

////////////////////////////////////////////////////////////////////////////////
class GrTextureObj : public GrFBBindableObj {
    GR_DEFINE_CREATOR(GrTextureObj)

public:
    GrTextureObj()
        : GrFBBindableObj() {
    }

    ~GrTextureObj() override {
        GrAlwaysAssert(0 == fTextureUnitReferees.count());
    }

    void setBound(GrTextureUnitObj *referee) {
        fTextureUnitReferees.append(1, &referee);
    }

    void resetBound(GrTextureUnitObj *referee) {
        int index = fTextureUnitReferees.find(referee);
        GrAlwaysAssert(0 <= index);
        fTextureUnitReferees.removeShuffle(index);
    }
    bool getBound(GrTextureUnitObj *referee) const {
        int index = fTextureUnitReferees.find(referee);
        return 0 <= index;
    }
    bool getBound() const {
        return 0 != fTextureUnitReferees.count();
    }

    void deleteAction() override;

protected:

private:
    // texture units that bind this texture (via "glBindTexture")
    SkTDArray<GrTextureUnitObj *> fTextureUnitReferees;

    typedef GrFBBindableObj INHERITED;
};

#endif // GrTextureObj_DEFINED
