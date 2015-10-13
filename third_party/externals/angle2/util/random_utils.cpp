//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "random_utils.h"
#include <time.h>
#include <cstdlib>

float RandomBetween(float min, float max)
{
    static bool randInitialized = false;
    if (!randInitialized)
    {
        srand(static_cast<unsigned int>(time(NULL)));
        randInitialized = true;
    }

    const size_t divisor = 10000;
    return min + ((rand() % divisor) / static_cast<float>(divisor)) * (max - min);
}
