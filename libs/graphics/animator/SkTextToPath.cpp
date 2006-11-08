/* libs/graphics/animator/SkTextToPath.cpp
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

#include "SkTextToPath.h"
#include "SkAnimateMaker.h"
#include "SkDrawPaint.h"
#include "SkDrawPath.h"
#include "SkDrawText.h"
#include "SkPaint.h"

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkTextToPath::fInfo[] = {
    SK_MEMBER(paint, Paint),
    SK_MEMBER(path, Path),
    SK_MEMBER(text, Text)
};

#endif

DEFINE_GET_MEMBER(SkTextToPath);

SkTextToPath::SkTextToPath() : paint(nil), path(nil), text(nil) {
}

bool SkTextToPath::draw(SkAnimateMaker& maker) {
    path->draw(maker);
    return false;
}

void SkTextToPath::onEndElement(SkAnimateMaker& maker) {
    if (paint == nil || path == nil || text == nil) {
        // !!! add error message here
        maker.setErrorCode(SkDisplayXMLParserError::kErrorInAttributeValue);
        return;
    }
    SkPaint realPaint;
    paint->setupPaint(&realPaint);
    realPaint.getTextPath(text->getText(), text->getSize(), text->x, 
        text->y, &path->getPath());
}

