/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDrawTransparentShader_DEFINED
#define SkDrawTransparentShader_DEFINED

#include "SkPaintPart.h"

class SkDrawTransparentShader : public SkDrawShader {
    DECLARE_EMPTY_MEMBER_INFO(TransparentShader);
    virtual SkShader* getShader();
};

#endif // SkDrawTransparentShader_DEFINED
