/*
* Copyright 2019 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef SkParticleDrawable_DEFINED
#define SkParticleDrawable_DEFINED

#include "modules/particles/include/SkReflected.h"

class SkCanvas;
struct SkParticleState;
class SkPaint;
class SkString;

class SkParticleDrawable : public SkReflected {
public:
    REFLECTED_ABSTRACT(SkParticleDrawable, SkReflected)

    virtual void draw(SkCanvas* canvas, const SkParticleState particles[], int count,
                      const SkPaint* paint) = 0;

    static void RegisterDrawableTypes();

    static sk_sp<SkParticleDrawable> MakeCircle(int radius);
    static sk_sp<SkParticleDrawable> MakeImage(const SkString& path, int cols, int rows);
};

#endif // SkParticleEffect_DEFINED
