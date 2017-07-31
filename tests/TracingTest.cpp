/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCommandLineFlags.h"
#include "SkLeanWindows.h"
#include "SkTraceEvent.h"
#include "Test.h"

DEFINE_bool(traceTestSleep, false, "Add sleeps to tracing test to produce nicer JSON");

namespace {

/**
 * Helper types for demonstrating usage of TRACE_EVENT_OBJECT_XXX macros.
 */
struct TracingShape {
    TracingShape() {
        TRACE_EVENT_OBJECT_CREATED_WITH_ID("skia.objects", "TracingShape", this);
    }
    virtual ~TracingShape() {
        TRACE_EVENT_OBJECT_DELETED_WITH_ID("skia.objects", "TracingShape", this);
    }
};

struct TracingCircle : public TracingShape {
    TracingCircle(SkPoint center, SkScalar radius) : fCenter(center), fRadius(radius) {}
    SkPoint fCenter;
    SkScalar fRadius;
};

struct TracingRect : public TracingShape {
    TracingRect(SkRect rect) : fRect(rect) {}
    SkRect fRect;
};

}

static void maybe_sleep() {
    if (FLAGS_traceTestSleep) {
#if defined(SK_BUILD_FOR_WIN)
        Sleep(20);
#else
        sleep(1);
#endif
    }
}

DEF_TEST(Tracing, reporter) {
    // Demonstrate nesting of events
    {
        // Simple event that lasts until the end of the current scope. TRACE_FUNC is an easy way
        // to insert the current function name.
        TRACE_EVENT0("skia", TRACE_FUNC);

        {
            // There are versions of the macro that take 1 or 2 named arguments. The arguments
            // can be any simple type. Strings need to be static/literal - we just copy pointers.
            // Argument names & values are shown when the event is selected in the viewer.
            TRACE_EVENT1("skia", "Nested work",
                         "isBGRA", kN32_SkColorType == kBGRA_8888_SkColorType);
            maybe_sleep();
        }
    }

    // Counters
    {
        TRACE_EVENT0("skia", "Counters");

        // Counter macros allow recording a named value (which must be a 32-bit integer).
        // The value will be graphed in the viewer.
        for (int i = 0; i < 360; ++i) {
            SkScalar rad = SkDegreesToRadians(SkIntToScalar(i));
            TRACE_COUNTER1("skia", "sin", SkScalarSin(rad) * 1000.0f + 1000.0f);
            maybe_sleep();
        }
    }
}
