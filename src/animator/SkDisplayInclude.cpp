/* libs/graphics/animator/SkDisplayInclude.cpp
**
** Copyright 2006, The Android Open Source Project
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

#include "SkDisplayInclude.h"
#include "SkAnimateMaker.h"
#include "SkAnimator.h"

#if 0
#undef SK_MEMBER
#define SK_MEMBER(_member, _type) \
    { #_member, SK_OFFSETOF(BASE_CLASS::_A, _member), SkType_##_type, \
    sizeof(((BASE_CLASS::_A*) 0)->_member) / sizeof(SkScalar) }
#endif

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkInclude::fInfo[] = {
    SK_MEMBER(src, String)
};

#endif

DEFINE_GET_MEMBER(SkInclude);

//SkInclude::SkInclude() {
//  src.init();
//}

//SkInclude::~SkInclude() {
//  src.unref();
//}

bool SkInclude::enable(SkAnimateMaker & ) {
    return true;
}

bool SkInclude::hasEnable() const {
    return true;
}

void SkInclude::onEndElement(SkAnimateMaker& maker) {
    maker.fInInclude = true;
    if (src.size() == 0 || maker.decodeURI(src.c_str()) == false) {
        if (maker.getErrorCode() != SkXMLParserError::kNoError || maker.getNativeCode() != -1) {
            maker.setInnerError(&maker, src);
            maker.setErrorCode(SkDisplayXMLParserError::kInInclude);
        } else {
            maker.setErrorNoun(src);
            maker.setErrorCode(SkDisplayXMLParserError::kIncludeNameUnknownOrMissing);
        }
    }
    maker.fInInclude = false;
}
