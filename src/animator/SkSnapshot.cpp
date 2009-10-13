/* libs/graphics/animator/SkSnapshot.cpp
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

#include "SkTypes.h"

#include "SkSnapshot.h"
#include "SkAnimateMaker.h"
#include "SkCanvas.h"
#include "SkImageEncoder.h"

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkSnapshot::fInfo[] = {
    SK_MEMBER(filename, String),
    SK_MEMBER(quality, Float),
    SK_MEMBER(sequence, Boolean),
    SK_MEMBER(type, BitmapEncoding)
};

#endif

DEFINE_GET_MEMBER(SkSnapshot);

SkSnapshot::SkSnapshot()
{
    quality     = 100 * SK_Scalar1;
    type        = (SkImageEncoder::Type) -1;
    sequence    = false;
    fSeqVal     = 0;
}

#include "SkDevice.h"

bool SkSnapshot::draw(SkAnimateMaker& maker) {
    SkASSERT(type >= 0);
    SkASSERT(filename.size() > 0);
    SkImageEncoder* encoder = SkImageEncoder::Create((SkImageEncoder::Type) type);

    SkString name(filename);
    if (sequence) {
        char num[4] = "000";
        num[0] = (char) (num[0] + fSeqVal / 100);
        num[1] = (char) (num[1] + fSeqVal / 10 % 10);
        num[2] = (char) (num[2] + fSeqVal % 10);
        name.append(num);
        if (++fSeqVal > 999)
            sequence = false;
    }
    if (type == SkImageEncoder::kJPEG_Type)
        name.append(".jpg");
    else if (type == SkImageEncoder::kPNG_Type)
        name.append(".png");
    encoder->encodeFile(name.c_str(),
                        maker.fCanvas->getDevice()->accessBitmap(false),
                        SkScalarFloor(quality));
    return false;
}

