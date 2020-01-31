/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkICUInterface_DEFINED
#define SkICUInterface_DEFINED

#include "include/core/SkTypes.h"

class SkICUInterface {
public:
    virtual ~SkICUInterface() {}

    ScriptID unicharToScriptID(SkUnichar);
    CombiningClass unicharToCombiningClass(SkUnichar);
    GeneralCategory unicharToGeneralCategory(SkUnichar);

    SkUnichar mirrorUnichar(SkUnichar);
    bool composeUnichars(SkUnichar a, SkUnichar b, SkUnichar* ab);
    bool decomposeUnichar(SkUnichar ab, SkUnichar* a, SkUnichar* b);

protected:
    virtual ScriptID onUnicharToScriptID(SkUnichar) = 0;
    virtual CombiningClass onUnicharToCombiningClass(SkUnichar) = 0;
    virtual GeneralCategory onUnicharToGeneralCategory(SkUnichar) = 0;
    virtual SkUnichar onMirrorUnichar(SkUnichar) = 0;
    virtual bool onComposeUnichars(SkUnichar a, SkUnichar b, SkUnichar* ab) = 0;
    virtual bool onDecomposeUnichar(SkUnichar ab, SkUnichar* a, SkUnichar* b) = 0;
};

#endif
