//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// LinuxTimer.cpp: Implementation of a high precision timer class on Linux

#include "linux/LinuxTimer.h"
#include <iostream>

LinuxTimer::LinuxTimer()
    : mRunning(false)
{
}

void LinuxTimer::start()
{
    clock_gettime(CLOCK_MONOTONIC, &mStartTime);
    mRunning = true;
}

void LinuxTimer::stop()
{
    clock_gettime(CLOCK_MONOTONIC, &mStopTime);
    mRunning = false;
}

double LinuxTimer::getElapsedTime() const
{
    struct timespec endTime;
    if (mRunning)
    {
        clock_gettime(CLOCK_MONOTONIC, &endTime);
    }
    else
    {
        endTime = mStopTime;
    }

    double startSeconds = mStartTime.tv_sec + (1.0 / 1000000000) * mStartTime.tv_nsec;
    double endSeconds = endTime.tv_sec + (1.0 / 1000000000) * endTime.tv_nsec;
    return endSeconds - startSeconds;
}

Timer *CreateTimer()
{
    return new LinuxTimer();
}
