/*
    Copyright 2011 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */

#include "SkClampRange.h"

/*
 *  returns [0..count] for the number of steps (<= count) for which x0 <= edge
 *  given each step is followed by x0 += dx
 */
static int chop(SkFixed x0, SkFixed edge, SkFixed x1, SkFixed dx, int count) {
    SkASSERT(dx > 0);
    SkASSERT(count >= 0);

    if (x0 >= edge) {
        return 0;
    }
    if (x1 <= edge) {
        return count;
    }
    int n = (edge - x0 + dx - 1) / dx;
    SkASSERT(n >= 0);
    SkASSERT(n <= count);
    return n;
}

void SkClampRange::init(SkFixed fx, SkFixed dx, int count, int v0, int v1) {
    SkASSERT(count > 0);

    fV0 = v0;
    fV1 = v1;

    // check for over/underflow
    {
        int64_t eex = (int64_t)fx + count * (int64_t)dx;
        if (eex > SK_FixedMax) {
            
        } else if (eex < -SK_FixedMax) {
        }
    }

    // remember our original fx
    const SkFixed fx0 = fx;
    // start with ex equal to the last computed value
    SkFixed ex = fx + (count - 1) * dx;

    if ((unsigned)(fx | ex) <= 0xFFFF) {
        fCount0 = fCount2 = 0;
        fCount1 = count;
        fFx1 = fx;
        return;
    }
    if (fx <= 0 && ex <= 0) {
        fCount1 = fCount2 = 0;
        fCount0 = count;
        return;
    }
    if (fx >= 0xFFFF && ex >= 0xFFFF) {
        fCount0 = fCount1 = 0;
        fCount2 = count;
        return;
    }

    // now make ex be 1 past the last computed value
    ex += dx;

    bool doSwap = dx < 0;

    if (doSwap) {
        ex -= dx;
        fx -= dx;
        SkTSwap(fx, ex);
        dx = -dx;
    }

    fCount0 = chop(fx, 0, ex, dx, count);
    count -= fCount0;
    fx += fCount0 * dx;
    SkASSERT(fx >= 0);
    SkASSERT(fCount0 == 0 || (fx - dx) < 0);
    fCount1 = chop(fx, 0xFFFF, ex, dx, count);
    count -= fCount1;
    fCount2 = count;

#ifdef SK_DEBUG
    fx += fCount1 * dx;
    SkASSERT(fx <= ex);
    if (fCount2 > 0) {
        SkASSERT(fx >= 0xFFFF);
        if (fCount1 > 0) {
            SkASSERT(fx - dx < 0xFFFF);
        }
    }
#endif

    if (doSwap) {
        SkTSwap(fCount0, fCount2);
        SkTSwap(fV0, fV1);
        dx = -dx;
    }

    if (fCount1 > 0) {
        fFx1 = fx0 + fCount0 * dx;
    }
}

