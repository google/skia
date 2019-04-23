/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef Timer_DEFINED
#define Timer_DEFINED

#include "include/core/SkString.h"
#include "include/core/SkTime.h"
#include "include/core/SkTypes.h"

class WallTimer {
public:
    WallTimer() : fWall(-1) {}

    void start() { fWall = SkTime::GetNSecs(); }
    void end()   { fWall = (SkTime::GetNSecs() - fWall) * 1e-6; }

    double fWall;  // Milliseconds.
};

SkString HumanizeMs(double);

#endif
