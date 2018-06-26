/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFontParameters_DEFINED
#define SkFontParameters_DEFINED

#include "SkScalar.h"
#include "SkTypes.h"

struct SkFontParameters {
    struct Variation {
        struct Axis {
            SkFourByteTag tag;
            SkScalar min;
            SkScalar def;
            SkScalar max;
            uint16_t flags;
            uint16_t nameID;
            static constexpr uint16_t HIDDEN = 0x0001;
            bool isHidden() { return flags & HIDDEN; }
        };
    };
};

#endif
