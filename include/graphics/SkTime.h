/* include/graphics/SkTime.h
**
** Copyright 2006, Google Inc.
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
*/

#ifndef SkTime_DEFINED
#define SkTime_DEFINED

#include "SkTypes.h"

/** \class SkTime
    Platform-implemented utilities to return time of day, and millisecond counter.
*/
class SkTime {
public:
    struct DateTime {
        U16 fYear;          //!< e.g. 2005
        U8  fMonth;         //!< 1..12
        U8  fDayOfWeek;     //!< 0..6, 0==Sunday
        U8  fDay;           //!< 1..31
        U8  fHour;          //!< 0..23
        U8  fMinute;        //!< 0..59
        U8  fSecond;        //!< 0..59
    };
    static void GetDateTime(DateTime*);

    static SkMSec GetMSecs();
};

#if defined(SK_DEBUG) && defined(SK_BUILD_FOR_WIN32)
    extern SkMSec gForceTickCount;
#endif

#define SK_TIME_FACTOR      1

#endif

