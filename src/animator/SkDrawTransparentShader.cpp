
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkDrawTransparentShader.h"
#include "SkTransparentShader.h"

SkShader* SkDrawTransparentShader::getShader() {
    return new SkTransparentShader();
}
