//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// LinuxTimer.h: Definition of a high precision timer class on Linux

#ifndef UTIL_LINUX_TIMER_H
#define UTIL_LINUX_TIMER_H

#include <time.h>

#include "Timer.h"

class LinuxTimer : public Timer
{
  public:
    LinuxTimer();

    void start() override;
    void stop() override;
    double getElapsedTime() const override;

  private:
    bool mRunning;
    struct timespec mStartTime;
    struct timespec mStopTime;
};

#endif // UTIL_LINUX_TIMER_H
