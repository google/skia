
/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkShadowUtils_DEFINED
#define SkShadowUtils_DEFINED

#include "SkColor.h"
#include "SkScalar.h"
#include "../private/SkShadowFlags.h"

class SkCanvas;
class SkPath;

class SkShadowUtils {
public:
    // Draw an offset spot shadow and outlining ambient shadow for the given path.
    static void DrawShadow(SkCanvas*, const SkPath& path, SkScalar occluderHeight,
                           const SkPoint3& lightPos, SkScalar lightRadius,
                           SkScalar ambientAlpha, SkScalar spotAlpha, SkColor color,
                           uint32_t flags = SkShadowFlags::kNone_ShadowFlag);
    static void ClearCache();
};

#endif
