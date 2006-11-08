/* libs/corecg/SkDebug.cpp
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

#include "SkTypes.h"

#ifdef SK_DEBUG

int8_t SkToS8(long x)
{
    SkASSERT((int8_t)x == x);
    return (int8_t)x;
}

uint8_t SkToU8(size_t x)
{
    SkASSERT((uint8_t)x == x);
    return (uint8_t)x;
}

int16_t SkToS16(long x)
{
    SkASSERT((int16_t)x == x);
    return (int16_t)x;
}

uint16_t SkToU16(size_t x)
{
    SkASSERT((uint16_t)x == x);
    return (uint16_t)x;
}

int32_t SkToS32(long x)
{
    SkASSERT((int32_t)x == x);
    return (int32_t)x;
}

uint32_t SkToU32(size_t x)
{
    SkASSERT((uint32_t)x == x);
    return (uint32_t)x;
}

#endif

