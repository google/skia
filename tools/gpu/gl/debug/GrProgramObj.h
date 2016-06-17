
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrProgramObj_DEFINED
#define GrProgramObj_DEFINED

#include "SkTArray.h"
#include "GrFakeRefObj.h"
class GrShaderObj;

////////////////////////////////////////////////////////////////////////////////
class GrProgramObj : public GrFakeRefObj {
    GR_DEFINE_CREATOR(GrProgramObj);

public:
    GrProgramObj()
        : GrFakeRefObj()
        , fInUse(false) {}

    void AttachShader(GrShaderObj *shader);

    void deleteAction() override;

    // TODO: this flag system won't work w/ multiple contexts!
    void setInUse()         { fInUse = true; }
    void resetInUse()       { fInUse = false; }
    bool getInUse() const   { return fInUse; }

protected:

private:
    SkTArray<GrShaderObj *> fShaders;
    bool fInUse;            // has this program been activated by a glUseProgram call?

    typedef GrFakeRefObj INHERITED;
};

#endif // GrProgramObj_DEFINED
