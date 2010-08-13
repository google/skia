/*
 **
 ** Copyright 2006-2010, The Android Open Source Project
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

#include "SkUtils.h"

extern "C" {
    void arm_memset16(uint16_t* dst, uint16_t value, int count);
    void arm_memset32(uint32_t* dst, uint32_t value, int count);
}

SkMemset16Proc SkMemset16GetPlatformProc() {
    return arm_memset16;
}

SkMemset32Proc SkMemset32GetPlatformProc() {
    return arm_memset32;
}
