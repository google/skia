
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkDrawShader.h"
#include "SkDrawBitmap.h"
#include "SkDrawMatrix.h"
#include "SkDrawPaint.h"
#include "SkTemplates.h"

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkDrawShader::fInfo[] = {
    SK_MEMBER(matrix, Matrix),
    SK_MEMBER(tileMode, TileMode)
};

#endif

DEFINE_GET_MEMBER(SkDrawShader);

SkDrawShader::SkDrawShader() : matrix(NULL),
    tileMode(SkShader::kClamp_TileMode) {
}

bool SkDrawShader::add() {
    if (fPaint->shader != (SkDrawShader*) -1)
        return true;
    fPaint->shader = this;
    fPaint->fOwnsShader = true;
    return false;
}

void SkDrawShader::addPostlude(SkShader* shader) {
    if (matrix)
        shader->setLocalMatrix(matrix->getMatrix());
}

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkDrawBitmapShader::fInfo[] = {
    SK_MEMBER_INHERITED,
    SK_MEMBER(filterBitmap, Boolean),
    SK_MEMBER(image, BaseBitmap)
};

#endif

DEFINE_GET_MEMBER(SkDrawBitmapShader);

SkDrawBitmapShader::SkDrawBitmapShader() : filterBitmap(-1), image(NULL) {}

bool SkDrawBitmapShader::add() {
    if (fPaint->shader != (SkDrawShader*) -1)
        return true;
    fPaint->shader = this;
    fPaint->fOwnsShader = true;
    return false;
}

SkShader* SkDrawBitmapShader::getShader() {
    if (image == NULL)
        return NULL;

    // note: bitmap shader now supports independent tile modes for X and Y
    // we pass the same to both, but later we should extend this flexibility
    // to the xml (e.g. tileModeX="repeat" tileModeY="clmap")
    //
    // oops, bitmapshader no longer takes filterBitmap, but deduces it at
    // draw-time from the paint
    SkShader* shader  = SkShader::CreateBitmapShader(image->fBitmap,
                                                    (SkShader::TileMode) tileMode,
                                                    (SkShader::TileMode) tileMode);
    SkAutoTDelete<SkShader> autoDel(shader);
    addPostlude(shader);
    (void)autoDel.detach();
    return shader;
}
