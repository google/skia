/* libs/graphics/animator/SkDisplayRandom.cpp
**
** Copyright 2006, Google Inc.
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
*/

#include "SkDisplayRandom.h"
#include "SkInterpolator.h"

enum SkDisplayRandom_Properties {
    SK_PROPERTY(random),
    SK_PROPERTY(seed)
};

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkDisplayRandom::fInfo[] = {
    SK_MEMBER(blend, Float),
    SK_MEMBER(max, Float),
    SK_MEMBER(min, Float),
    SK_MEMBER_DYNAMIC_PROPERTY(random, Float),
    SK_MEMBER_PROPERTY(seed, Int)
};

#endif

DEFINE_GET_MEMBER(SkDisplayRandom);

SkDisplayRandom::SkDisplayRandom() : blend(0), min(0), max(SK_Scalar1) {
}

#ifdef SK_DUMP_ENABLED
void SkDisplayRandom::dump(SkAnimateMaker* maker) {
    dumpBase(maker);
#ifdef SK_CAN_USE_FLOAT
    SkDebugf("min=\"%g\" ", SkScalarToFloat(min));
    SkDebugf("max=\"%g\" ", SkScalarToFloat(max));
    SkDebugf("blend=\"%g\" ", SkScalarToFloat(blend));    
#else
    SkDebugf("min=\"%x\" ", min);
    SkDebugf("max=\"%x\" ", max);
    SkDebugf("blend=\"%x\" ", blend);    
#endif
    SkDebugf("/>\n");
}
#endif

bool SkDisplayRandom::getProperty(int index, SkScriptValue* value) const {
    switch(index) {
        case SK_PROPERTY(random): {
            SkScalar random = fRandom.nextUScalar1();
            SkScalar relativeT = SkUnitCubicInterp(random, SK_Scalar1 - blend, 0, 0, SK_Scalar1 - blend);
            value->fOperand.fScalar = min + SkScalarMul(max - min, relativeT);
            value->fType = SkType_Float;
            return true;
        }
        default:
            SkASSERT(0);
    }
    return false;
}

bool SkDisplayRandom::setProperty(int index, SkScriptValue& value) {
    SkASSERT(index == SK_PROPERTY(seed));
    SkASSERT(value.fType == SkType_Int);
    fRandom.setSeed(value.fOperand.fS32);
    return true;
}

