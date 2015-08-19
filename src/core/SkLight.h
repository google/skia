
/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkLight_DEFINED
#define SkLight_DEFINED

#include "SkPoint3.h"

class SK_API SkLight {
public:
    enum LightType {
        kAmbient_LightType,       // only 'fColor' is used
        kDirectional_LightType
    };

    SkLight() : fType(kAmbient_LightType) {
        fColor.set(0.0f, 0.0f, 0.0f);
        fDirection.set(0.0f, 0.0f, 1.0f);
    }

    SkLight(const SkColor3f& color)
        : fType(kAmbient_LightType)
        , fColor(color) {
        fDirection.set(0.0f, 0.0f, 1.0f);
    }

    SkLight(const SkColor3f& color, const SkVector3& dir)
        : fType(kDirectional_LightType)
        , fColor(color)
        , fDirection(dir) {
        if (!fDirection.normalize()) {
            fDirection.set(0.0f, 0.0f, 1.0f);
        }
    }

    LightType type() const { return fType; }
    const SkColor3f& color() const { return fColor; }
    const SkVector3& dir() const { 
        SkASSERT(kAmbient_LightType != fType);
        return fDirection; 
    }

private:
    LightType   fType;
    SkColor3f   fColor;           // linear (unpremul) color. Range is 0..1 in each channel.
    SkVector3   fDirection;       // direction towards the light (+Z is out of the screen).
                                  // If degenerate, it will be replaced with (0, 0, 1).
};


#endif
