
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkTypedArray_DEFINED
#define SkTypedArray_DEFINED

#include "SkScript.h"
#include "SkTDArray_Experimental.h"

class SkTypedArray : public SkTDOperandArray {
public:
    SkTypedArray();
    SkTypedArray(SkDisplayTypes type);
    bool getIndex(int index, SkOperand* operand);
    SkDisplayTypes getType() { return fType; }
    SkScriptEngine::SkOpType getOpType() { return SkScriptEngine::ToOpType(fType); }
    void setType(SkDisplayTypes type) {
    //  SkASSERT(count() == 0);
        fType = type;
    }
protected:
    SkDisplayTypes fType;
};

#endif // SkTypedArray_DEFINED
