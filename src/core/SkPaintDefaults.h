/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPaintDefaults_DEFINED
#define SkPaintDefaults_DEFINED

#include "SkPaint.h"

/**
 *  Any of these can be specified by the build system (or SkUserConfig.h)
 *  to change the default values for a SkPaint. This file should not be
 *  edited directly.
 */

#ifndef SkPaintDefaults_Flags
    #define SkPaintDefaults_Flags           0
#endif

#ifndef SkPaintDefaults_TextSize
    #define SkPaintDefaults_TextSize        SkIntToScalar(12)
#endif

#ifndef SkPaintDefaults_Hinting
    #define SkPaintDefaults_Hinting         SkPaint::kNormal_Hinting
#endif

#ifndef SkPaintDefaults_MiterLimit
    #define SkPaintDefaults_MiterLimit      SkIntToScalar(4)
#endif

#endif
