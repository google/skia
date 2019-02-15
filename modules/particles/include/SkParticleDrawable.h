/*
* Copyright 2019 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef SkParticleDrawable_DEFINED
#define SkParticleDrawable_DEFINED

#include "SkReflected.h"

#include "SkColor.h"

class SkCanvas;
class SkPaint;
struct SkRSXform;
class SkString;

class SkParticleDrawable : public SkReflected {
public:
    REFLECTED_ABSTRACT(SkParticleDrawable, SkReflected)

    virtual void draw(SkCanvas* canvas, const SkRSXform xform[], const float tex[],
                      const SkColor colors[], int count, const SkPaint* paint) = 0;
    virtual SkPoint center() const = 0;

    static void RegisterDrawableTypes();

    static sk_sp<SkParticleDrawable> MakeCircle(int radius);
    static sk_sp<SkParticleDrawable> MakeImage(const SkString& path, int cols, int rows);
};

#endif // SkParticleEffect_DEFINED
