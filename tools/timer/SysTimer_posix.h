/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SysTimer_DEFINED
#define SysTimer_DEFINED

#include <time.h>

class SysTimer {
public:
    void startWall();
    void startCpu();
    double endCpu();
    double endWall();
private:
    timespec fCpu;
    timespec fWall;
};

#endif
