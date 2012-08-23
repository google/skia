
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkSystemEventTypes_DEFINED
#define SkSystemEventTypes_DEFINED

/*
    The goal of these strings is two-fold:
    1) make funny strings (containing at least one char < 32) to avoid colliding with "user" strings
    2) keep them <= 4 bytes, so we can avoid an allocation in SkEvent::setType()
*/
#define SK_EventType_Delay      "\xd" "lay"
#define SK_EventType_Inval      "nv" "\xa" "l"
#define SK_EventType_Key        "key" "\x1"
#define SK_EventType_OnEnd "on" "\xe" "n"
#define SK_EventType_Unichar    "\xc" "har"
#define SK_EventType_KeyUp      "key" "\xf"

#endif
