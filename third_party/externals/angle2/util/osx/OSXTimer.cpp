//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// OSXTimer.cpp: Implementation of a high precision timer class on OSX

#include "osx/OSXTimer.h"

#include <CoreServices/CoreServices.h>
#include <mach/mach.h>
#include <mach/mach_time.h>

OSXTimer::OSXTimer()
    : mRunning(false)
{
}

void OSXTimer::start()
{
    mStartTime = mach_absolute_time();
    mRunning = true;
}

void OSXTimer::stop()
{
    mStopTime = mach_absolute_time();
    mRunning = false;
}

double OSXTimer::getElapsedTime() const
{
    // If this is the first time we've run, get the timebase.
    // We can use denom == 0 to indicate that sTimebaseInfo is
    // uninitialised because it makes no sense to have a zero
    // denominator in a fraction.
    static mach_timebase_info_data_t timebaseInfo;
    static double secondCoeff;

    if ( timebaseInfo.denom == 0 )
    {
        mach_timebase_info(&timebaseInfo);
        secondCoeff = timebaseInfo.numer * (1.0 / 1000000000) / timebaseInfo.denom;
    }

    if (mRunning)
    {
        return secondCoeff * (mach_absolute_time() - mStartTime);
    }
    else
    {
        return secondCoeff * (mStopTime - mStartTime);
    }
}

Timer *CreateTimer()
{
    return new OSXTimer();
}
