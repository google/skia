
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
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
