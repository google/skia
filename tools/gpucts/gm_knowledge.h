/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef gm_knowledge_DEFINED
#define gm_knowledge_DEFINED

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    // FIXME: document kRGBA_8888_SkColorType or kBGRA_8888_SkColorType?
    const uint32_t* pix;
    int width;
    int height;
} GMK_ImageData;

float GMK_Check(GMK_ImageData data, const char* gm_name);

bool GMK_IsGoodGM(const char* gm_name);

#ifdef __cplusplus
}
#endif

#endif  // gm_knowledge_DEFINED
