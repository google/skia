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

    // start with ex equal to the last computed value
    SkFixed ex = fx + (count - 1) * dx;

    if ((unsigned)(fx | ex) <= 0xFFFF) {
        fCount0 = fCount2 = 0;
        fCount1 = count;
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
    }
}

////////////////////////////
#include "SkRandom.h"

static bool in_range(int x, int target, int slop) {
    SkASSERT(slop >= 0);
    return SkAbs32(x - target) <= slop;
}

static int classify_value(SkFixed fx, int v0, int v1) {
    if (fx <= 0) {
        return v0;
    }
    if (fx >= 0xFFFF) {
        return v1;
    }
    SkASSERT(!"bad fx");
    return 0;
}

#define V0  -42
#define V1  1024

static void slow_check(const SkClampRange& range,
                       SkFixed fx, SkFixed dx, int count) {
    SkASSERT(range.fCount0 + range.fCount1 + range.fCount2 == count);

    int i;
    for (i = 0; i < range.fCount0; i++) {
        int v = classify_value(fx, V0, V1);
        SkASSERT(v == range.fV0);
        fx += dx;
    }
    for (i = 0; i < range.fCount1; i++) {
        SkASSERT(fx >= 0 && fx <= 0xFFFF);
        fx += dx;
    }
    for (i = 0; i < range.fCount2; i++) {
        int v = classify_value(fx, V0, V1);
        SkASSERT(v == range.fV1);
        fx += dx;
    }
}

static void test_range(SkFixed fx, SkFixed dx, int count) {
    SkClampRange range;
    range.init(fx, dx, count, V0, V1);
    slow_check(range, fx, dx, count);
}

#define ff(x)   SkIntToFixed(x)

void SkClampRange::UnitTest() {
    test_range(0, 0, 20);
    test_range(0xFFFF, 0, 20);
    test_range(-ff(2), 0, 20);
    test_range( ff(2), 0, 20);

    test_range(-10, 1, 20);
    test_range(10, -1, 20);
    test_range(-10, 3, 20);
    test_range(10, -3, 20);

    SkRandom rand;

    for (int i = 0; i < 1000000; i++) {
        SkFixed fx = rand.nextS() >> 1;
        SkFixed sx = rand.nextS() >> 1;
        int count = rand.nextU() % 1000 + 1;
        SkFixed dx = (sx - fx) / count;
        test_range(fx, dx, count);
    }
}

