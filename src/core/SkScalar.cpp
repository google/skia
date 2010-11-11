/*
 * Copyright 2010, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

#include "SkScalar.h"

SkScalar SkScalarInterpFunc(SkScalar searchKey, const SkScalar keys[],
                            const SkScalar values[], int length) {
    SkASSERT(length > 0);
    SkASSERT(keys != NULL);
    SkASSERT(values != NULL);
#ifdef SK_DEBUG
    for (int i = 1; i < length; i++)
        SkASSERT(keys[i] >= keys[i-1]);
#endif
    int right = 0;
    while (right < length && searchKey > keys[right])
        right++;
    // Could use sentinel values to eliminate conditionals, but since the
    // tables are taken as input, a simpler format is better.
    if (length == right)
        return values[length-1];
    if (0 == right)
        return values[0];
    // Otherwise, interpolate between right - 1 and right.
    SkScalar rightKey = keys[right];
    SkScalar leftKey = keys[right-1];
    SkScalar fract = SkScalarDiv(searchKey-leftKey,rightKey-leftKey);
    return SkScalarInterp(values[right-1], values[right], fract);
}
