/*
    Copyright (c) 2012 250bpm s.r.o.  All rights reserved.
    Copyright (c) 2012 Julien Ammous

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom
    the Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
    IN THE SOFTWARE.
*/

#if defined NN_HAVE_WINDOWS
#include "win.h"
#elif defined NN_HAVE_OSX
#include <mach/mach_time.h>
#elif defined NN_HAVE_CLOCK_MONOTONIC || defined NN_HAVE_GETHRTIME
#include <time.h>
#else
#include <sys/time.h>
#endif

#include "clock.h"
#include "fast.h"
#include "err.h"
#include "attr.h"

/* 1 millisecond expressed in CPU ticks. The value is chosen is such a way that
   it works pretty well for CPU frequencies above 500MHz. */
#define NN_CLOCK_PRECISION 1000000

#if defined NN_HAVE_OSX
static mach_timebase_info_data_t nn_clock_timebase_info = {0};
#endif

static uint64_t nn_clock_rdtsc ()
{
#if (defined _MSC_VER && (defined _M_IX86 || defined _M_X64))
    return __rdtsc ();
#elif (defined __GNUC__ && (defined __i386__ || defined __x86_64__))
    uint32_t low;
    uint32_t high;
    __asm__ volatile ("rdtsc" : "=a" (low), "=d" (high));
    return (uint64_t) high << 32 | low;
#elif (defined __SUNPRO_CC && (__SUNPRO_CC >= 0x5100) && (defined __i386 || \
    defined __amd64 || defined __x86_64))
    union {
        uint64_t u64val;
        uint32_t u32val [2];
    } tsc;
    asm("rdtsc" : "=a" (tsc.u32val [0]), "=d" (tsc.u32val [1]));
    return tsc.u64val;
#else

    /*  RDTSC is not available. */
    return 0;
#endif
}

static uint64_t nn_clock_time ()
{
#if defined NN_HAVE_WINDOWS

    LARGE_INTEGER tps;
    LARGE_INTEGER time;
    double tpms;

    QueryPerformanceFrequency (&tps);
    QueryPerformanceCounter (&time);
    tpms = (double) (tps.QuadPart / 1000);
    return (uint64_t) (time.QuadPart / tpms);

#elif defined NN_HAVE_OSX

    uint64_t ticks;

    /*  If the global timebase info is not initialised yet, init it. */
    if (nn_slow (!nn_clock_timebase_info.denom))
        mach_timebase_info (&nn_clock_timebase_info);

    ticks = mach_absolute_time ();
    return ticks * nn_clock_timebase_info.numer /
        nn_clock_timebase_info.denom / 1000000;

#elif defined NN_HAVE_CLOCK_MONOTONIC

    int rc;
    struct timespec tv;

    rc = clock_gettime (CLOCK_MONOTONIC, &tv);
    errno_assert (rc == 0);
    return tv.tv_sec * (uint64_t) 1000 + tv.tv_nsec / 1000000;

#elif defined NN_HAVE_GETHRTIME

    return gethrtime () / 1000000;

#else

    int rc;
    struct timeval tv;

    /*  Gettimeofday is slow on some systems. Moreover, it's not necessarily
        monotonic. Thus, it's used as a last resort mechanism. */
    rc = gettimeofday (&tv, NULL);
    errno_assert (rc == 0);
    return tv.tv_sec * (uint64_t) 1000 + tv.tv_usec / 1000;

#endif
}

void nn_clock_init (struct nn_clock *self)
{
    self->last_tsc = nn_clock_rdtsc ();
    self->last_time = nn_clock_time ();
}

void nn_clock_term (NN_UNUSED struct nn_clock *self)
{
}

uint64_t nn_clock_now (struct nn_clock *self)
{
    /*  If TSC is not supported, use the non-optimised time measurement. */
    uint64_t tsc = nn_clock_rdtsc ();
    if (!tsc)
        return nn_clock_time ();

    /*  If tsc haven't jumped back or run away too far, we can use the cached
        time value. */
    if (nn_fast (tsc - self->last_tsc <= (NN_CLOCK_PRECISION / 2) &&
          tsc >= self->last_tsc))
        return self->last_time;

    /*  It's a long time since we've last measured the time. We'll do a new
        measurement now. */
    self->last_tsc = tsc;
    self->last_time = nn_clock_time ();
    return self->last_time;
}

uint64_t nn_clock_timestamp ()
{
    return nn_clock_rdtsc ();
}

