/*
 * Copyright (C) 2011 Skia
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _ANDROID_TO_SKIA_KEYCODES_H
#define _ANDROID_TO_SKIA_KEYCODES_H

#include "keycodes.h"
#include "SkKey.h"

// Convert an Android keycode to an SkKey.  This is an incomplete list, only
// including keys used by the sample app.
SkKey AndroidKeycodeToSkKey(int keycode) {
    switch (keycode) {
        case AKEYCODE_DPAD_LEFT:
            return kLeft_SkKey;
        case AKEYCODE_DPAD_RIGHT:
            return kRight_SkKey;
        case AKEYCODE_DPAD_UP:
            return kUp_SkKey;
        case AKEYCODE_DPAD_DOWN:
            return kDown_SkKey;
        default:
            return kNONE_SkKey;
    }
}

#endif
