//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// WindowsTimer.h: Definition of a high precision timer class on Windows

#ifndef UTIL_WINDOWS_TIMER_H
#define UTIL_WINDOWS_TIMER_H

#include <windows.h>

#include "Timer.h"

class WindowsTimer : public Timer
{
  public:
    WindowsTimer();

    void start() override;
    void stop() override;
    double getElapsedTime() const override;

  private:
    bool mRunning;
    LONGLONG mStartTime;
    LONGLONG mStopTime;

    LONGLONG mFrequency;
};

#endif  // UTIL_WINDOWS_TIMER_H
