/* libs/graphics/animator/SkDrawBitmap.cpp
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

#include "SkDrawBitmap.h"
#include "SkAnimateMaker.h"
#include "SkCanvas.h"
#include "SkImageDecoder.h"
#include "SkPaint.h"
#include "SkStream.h"

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkBaseBitmap::fInfo[] = {
    SK_MEMBER(x, Float),
    SK_MEMBER(y, Float)
};

#endif

DEFINE_GET_MEMBER(SkBaseBitmap);

SkBaseBitmap::SkBaseBitmap() : x(0), y(0) {
}

SkBaseBitmap::~SkBaseBitmap() {
}

bool SkBaseBitmap::draw(SkAnimateMaker& maker) {
    SkBoundableAuto boundable(this, maker);
    maker.fCanvas->drawBitmap(fBitmap, x, y, maker.fPaint);
    return false;
}

enum SkDrawBitmap_Properties {
    SK_PROPERTY(erase)
};

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkDrawBitmap::fInfo[] = {
    SK_MEMBER_INHERITED,
    SK_MEMBER_PROPERTY(erase, ARGB),
    SK_MEMBER(format, BitmapFormat),
    SK_MEMBER(height, Int),
    SK_MEMBER(rowBytes, Int),
    SK_MEMBER(width, Int),
};

#endif

DEFINE_GET_MEMBER(SkDrawBitmap);

SkDrawBitmap::SkDrawBitmap() : format((SkBitmap::Config) -1), height(-1), 
    rowBytes(0),    width(-1), fColor(0), fColorSet(false) {
}

SkDrawBitmap::~SkDrawBitmap() {
}

#ifdef SK_DUMP_ENABLED
void SkDrawBitmap::dump(SkAnimateMaker* maker) {
    dumpBase(maker);
    dumpAttrs(maker);
    if (fColorSet)
        SkDebugf("erase=\"argb(%d,%d,%d,%d)\" ", SkColorGetA(fColor)/255, SkColorGetR(fColor),
            SkColorGetG(fColor), SkColorGetB(fColor));
    if (rowBytes > 0)
        SkDebugf("rowBytes=\"%d\" ", rowBytes);
    const char* formatName;
    switch (format) {
        case 0: formatName = "none"; break;
        case 1: formatName = "A1"; break;
        case 2: formatName = "A8"; break;
        case 3: formatName = "Index8"; break;
        case 4: formatName = "RGB16"; break;
        case 5: formatName = "RGB32"; break;
    }
    SkDebugf("format=\"%s\" />\n", formatName);
}
#endif

void SkDrawBitmap::onEndElement(SkAnimateMaker& maker) {
    SkASSERT(format != (SkBitmap::Config) -1);
    SkASSERT(width != -1);
    SkASSERT(height != -1);
    SkASSERT(rowBytes >= 0);
    fBitmap.setConfig((SkBitmap::Config) format, width, height, rowBytes);
    fBitmap.allocPixels();
    if (fColorSet)
        fBitmap.eraseColor(fColor);
}

bool SkDrawBitmap::setProperty(int index, SkScriptValue& value)
{
    switch (index) {
        case SK_PROPERTY(erase):
            SkASSERT(value.fType == SkType_ARGB);
            fColor = value.fOperand.fS32;
            fColorSet = true;
            break;
        default:
            SkASSERT(0);
            return false;
    }
    return true;
}


enum SkImage_Properties {
    SK_PROPERTY(height),
    SK_PROPERTY(width)
};

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkImage::fInfo[] = {
    SK_MEMBER_INHERITED,
    SK_MEMBER(base64, Base64),
    SK_MEMBER_PROPERTY(height, Int),
    SK_MEMBER(src, String),
    SK_MEMBER_PROPERTY(width, Int)
};

#endif

DEFINE_GET_MEMBER(SkImage);

SkImage::SkImage() : fDirty(true), fUriBase(NULL) {
    base64.fData = NULL;
    base64.fLength = 0;
}

SkImage::~SkImage() {
    delete[] base64.fData; 
}

SkDisplayable* SkImage::deepCopy(SkAnimateMaker* maker) {
    SkDisplayable* copy = INHERITED::deepCopy(maker);
    ((SkImage*) copy)->fUriBase = ((SkImage*) this)->fUriBase;
    return copy;
}

void SkImage::dirty() {
    fDirty = true;
}

bool SkImage::draw(SkAnimateMaker& maker) {
    if (fDirty) 
        resolve();
    return INHERITED::draw(maker);
}

bool SkImage::getProperty(int index, SkScriptValue* value) const {
    if (fDirty) 
        resolve();
    switch (index) {
        case SK_PROPERTY(height):
            value->fOperand.fS32 = fBitmap.height();
            break;
        case SK_PROPERTY(width):
            value->fOperand.fS32 = fBitmap.width();
            break;
    default:
        SkASSERT(0);
        return false;
    }
    value->fType = SkType_Int;
    return true;
}

void SkImage::onEndElement(SkAnimateMaker& maker) {
    fUriBase = maker.fPrefix.c_str();
}

void SkImage::resolve() {
    fDirty = false;
    if (base64.fData) {
        fBitmap.reset();
        SkImageDecoder::DecodeMemory(base64.fData, base64.fLength, &fBitmap);
    } else if (src.size()) {
        if (fLast.equals(src))
            return;
        fLast.set(src);
        fBitmap.reset();
        
        //SkStream* stream = SkStream::GetURIStream(fUriBase, src.c_str());
        SkStream* stream = new SkFILEStream(src.c_str());

        SkAutoTDelete<SkStream> autoDel(stream);
        SkImageDecoder::DecodeStream(stream, &fBitmap);
    }   
}
