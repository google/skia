/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDrawShader_DEFINED
#define SkDrawShader_DEFINED

#include "SkPaintPart.h"
#include "SkShader.h"

class SkBaseBitmap;

class SkDrawBitmapShader : public SkDrawShader {
    DECLARE_DRAW_MEMBER_INFO(BitmapShader);
    SkDrawBitmapShader();
    bool add() override;
    SkShader* getShader() override;
protected:
    SkBool filterBitmap;
    SkBaseBitmap* image;
private:
    typedef SkDrawShader INHERITED;
};

#endif // SkDrawShader_DEFINED
