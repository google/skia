/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "X11/Xlib.h"
#include "X11/keysym.h"

#include "SkKey.h"

#ifndef XKEYS_TOSKKEYS_H
#define XKEYS_TOSKKEYS_H

SkKey XKeyToSkKey(KeySym keysym) {
    switch (keysym) {
        case XK_BackSpace:
            return kBack_SkKey;
        case XK_Return:
            return kOK_SkKey;
        case XK_Home:
            return kHome_SkKey;
        case XK_End:
            return kEnd_SkKey;
        case XK_Right:
            return kRight_SkKey;
        case XK_Left:
            return kLeft_SkKey;
        case XK_Down:
            return kDown_SkKey;
        case XK_Up:
            return kUp_SkKey;
        case XK_KP_0:
        case XK_KP_Insert:
            return k0_SkKey;
        case XK_KP_1:
        case XK_KP_End:
            return k1_SkKey;
        case XK_KP_2:
        case XK_KP_Down:
            return k2_SkKey;
        case XK_KP_3:
        case XK_KP_Page_Down:
            return k3_SkKey;
        case XK_KP_4:
        case XK_KP_Left:
            return k4_SkKey;
        case XK_KP_5:
            return k5_SkKey;
        case XK_KP_6:
        case XK_KP_Right:
            return k6_SkKey;
        case XK_KP_7:
        case XK_KP_Home:
            return k7_SkKey;
        case XK_KP_8:
        case XK_KP_Up:
            return k8_SkKey;
        case XK_KP_9:
        case XK_KP_Page_Up:
            return k9_SkKey;
        default:
            return kNONE_SkKey;
    }
}
#endif
