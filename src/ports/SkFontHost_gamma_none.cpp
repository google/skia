/* Copyright 2008, Google Inc.
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

// -----------------------------------------------------------------------------
// This is a noop gamma implementation for systems where gamma is already
// corrected, or dealt with in a system wide fashion. For example, on X windows
// one uses the xgamma utility to set the server-wide gamma correction value.
// -----------------------------------------------------------------------------

#include "SkFontHost.h"

void SkFontHost::GetGammaTables(const uint8_t* tables[2])
{
    tables[0] = NULL;
    tables[1] = NULL;
}

int SkFontHost::ComputeGammaFlag(const SkPaint& paint)
{
    return 0;
}

