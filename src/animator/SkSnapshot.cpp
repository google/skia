
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
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

bool SkSnapshot::draw(SkAnimateMaker& maker) {
    SkASSERT(type >= 0);
    SkASSERT(filename.size() > 0);
    SkImageEncoder* encoder = SkImageEncoder::Create((SkImageEncoder::Type) type);
    if (!encoder) {
        return false;
    }
    SkAutoTDelete<SkImageEncoder> ad(encoder);

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

    SkBitmap pixels;
    pixels.allocPixels(maker.fCanvas->imageInfo());
    maker.fCanvas->readPixels(&pixels, 0, 0);
    encoder->encodeFile(name.c_str(), pixels, SkScalarFloorToInt(quality));
    return false;
}
