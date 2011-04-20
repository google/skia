/*
    Copyright 2011 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */

#include "SkPorterDuff.h"
#include "SkXfermode.h"

/*  This file just exists as a compatibility layer, gluing the PorterDuff API
    into the (extended) SkXfermode API
 */

#define MAKE_PAIR(mode) { SkPorterDuff::k##mode##_Mode, SkXfermode::k##mode##_Mode }

// this table must be in SkPorterDuff::Mode order, so it can be indexed directly
// with a porterduff mode.
static const struct Pair {
    SkPorterDuff::Mode  fPD;
    SkXfermode::Mode    fXF;
} gPairs[] = {
    MAKE_PAIR(Clear),
    MAKE_PAIR(Src),
    MAKE_PAIR(Dst),
    MAKE_PAIR(SrcOver),
    MAKE_PAIR(DstOver),
    MAKE_PAIR(SrcIn),
    MAKE_PAIR(DstIn),
    MAKE_PAIR(SrcOut),
    MAKE_PAIR(DstOut),
    MAKE_PAIR(SrcATop),
    MAKE_PAIR(DstATop),
    MAKE_PAIR(Xor),
    MAKE_PAIR(Darken),
    MAKE_PAIR(Lighten),
    MAKE_PAIR(Multiply),
    MAKE_PAIR(Screen),
    { SkPorterDuff::kAdd_Mode, SkXfermode::kPlus_Mode },
#ifdef ANDROID
    MAKE_PAIR(Overlay),
#endif
};

static bool find_pdmode(SkXfermode::Mode src, SkPorterDuff::Mode* dst) {
    const Pair* pairs = gPairs;
    for (size_t i = 0; i < SK_ARRAY_COUNT(gPairs); i++) {
        if (pairs[i].fXF == src) {
            if (dst) {
                *dst = pairs[i].fPD;
            }
            return true;
        }
    }
    return false;
}

SkXfermode::Mode SkPorterDuff::ToXfermodeMode(Mode mode) {
    SkASSERT((unsigned)mode < SkPorterDuff::kModeCount);
    const Pair& pair = gPairs[mode];
    SkASSERT(pair.fPD == mode);
    return pair.fXF;
}

SkXfermode* SkPorterDuff::CreateXfermode(SkPorterDuff::Mode mode) {
    const Pair& pair = gPairs[mode];
    SkASSERT(pair.fPD == mode);
    return SkXfermode::Create(pair.fXF);
}

bool SkPorterDuff::IsMode(SkXfermode* xfer, Mode* pdmode) {
    SkXfermode::Mode xfmode;
    if (!SkXfermode::IsMode(xfer, &xfmode)) {
        return false;
    }
    return find_pdmode(xfmode, pdmode);
}

SkXfermodeProc SkPorterDuff::GetXfermodeProc(Mode mode) {
    return SkXfermode::GetProc(gPairs[mode].fXF);
}

SkXfermodeProc16 SkPorterDuff::GetXfermodeProc16(Mode mode, SkColor srcColor) {
    return SkXfermode::GetProc16(gPairs[mode].fXF, srcColor);
}

