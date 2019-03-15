// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "SkTypesToString.h"

const char* SkColorTypeToString(SkColorType v) {
    #define M(X) case k ## X ## _SkColorType: return #X
    switch (v) {
        M(Unknown); M(Alpha_8); M(RGB_565); M(ARGB_4444); M(RGBA_8888);
        M(RGB_888x); M(BGRA_8888); M(RGBA_1010102); M(RGB_101010x); M(Gray_8);
        M(RGBA_F16Norm); M(RGBA_F16); M(RGBA_F32); default: return nullptr;
    }
    #undef M
}

const char* SkAlphaTypeToString(SkAlphaType v) {
    #define M(X) case k ## X ## _SkAlphaType: return #X
    switch (v) {
        M(Unknown); M(Opaque); M(Premul); M(Unpremul); default: return nullptr;
    }
    #undef M
}

const char* SkPaintCapToString(SkPaint::Cap v) {
    #define M(X) case SkPaint::k ## X ## _Cap: return #X
    switch (v) {
        M(Butt); M(Round); M(Square); default: return nullptr;
    }
    #undef M
}

const char* SkPaintJoinToString(SkPaint::Join v) {
    #define M(X) case SkPaint::k ## X ## _Join: return #X
    switch (v) {
        M(Miter); M(Round); M(Bevel); default: return nullptr;
    }
    #undef M
}

const char* SkPaintStyleToString(SkPaint::Style v) {
    #define M(X) case SkPaint::k ## X ## _Style: return #X
    switch (v) {
        M(Fill); M(Stroke); M(StrokeAndFill); default: return nullptr;
    }
    #undef M
}

const char* SkFilterQualityToString(SkFilterQuality v) {
    #define M(X) case k ## X ## _SkFilterQuality: return #X
    switch (v) {
        M(None); M(Low); M(Medium); M(High); default: return nullptr;
    }
    #undef M
}
