/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkImageInfo.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/private/SkLeanWindows.h"
#include "src/core/SkTraceEvent.h"
#include "tests/Test.h"
#include "tools/flags/CommandLineFlags.h"

static DEFINE_bool(slowTracingTest, false,
                   "Artificially slow down tracing test to produce nicer JSON");

namespace {

/**
 * Helper types for demonstrating usage of TRACE_EVENT_OBJECT_XXX macros.
 */
struct TracingShape {
    TracingShape() {
        TRACE_EVENT_OBJECT_CREATED_WITH_ID("skia.objects", this->typeName(), this);
    }
    virtual ~TracingShape() {
        TRACE_EVENT_OBJECT_DELETED_WITH_ID("skia.objects", this->typeName(), this);
    }
    void traceSnapshot() {
        // The state of an object can be specified at any point with the OBJECT_SNAPSHOT macro.
        // This takes the "name" (actually the type name), the ID of the object (typically a
        // pointer), and a single (unnnamed) argument, which is the "snapshot" of that object.
        //
        // Tracing viewer requires that all object macros use the same name and id for creation,
        // deletion, and snapshots. However: It's convenient to put creation and deletion in the
        // base-class constructor/destructor where the actual type name isn't known yet. That's
        // what we're doing here. The JSON for snapshots can therefore include the actual type
        // name, and a special tag that refers to the type name originally used at creation time.
        // Skia's JSON tracer handles this automatically, so SNAPSHOT macros can simply use the
        // derived type name, and the JSON will be formatted correctly to link the events.
        TRACE_EVENT_OBJECT_SNAPSHOT_WITH_ID("skia.objects", this->typeName(), this,
                                            TRACE_STR_COPY(this->toString().c_str()));
    }

    virtual const char* typeName() { return "TracingShape"; }
    virtual SkString toString() { return SkString("Shape()"); }
};

struct TracingCircle : public TracingShape {
    TracingCircle(SkPoint center, SkScalar radius) : fCenter(center), fRadius(radius) {}
    const char* typeName() override { return "TracingCircle"; }
    SkString toString() override {
        return SkStringPrintf("Circle(%f, %f, %f)", fCenter.fX, fCenter.fY, fRadius);
    }

    SkPoint fCenter;
    SkScalar fRadius;
};

struct TracingRect : public TracingShape {
    TracingRect(SkRect rect) : fRect(rect) {}
    const char* typeName() override { return "TracingRect"; }
    SkString toString() override {
        return SkStringPrintf("Rect(%f, %f, %f, %f)",
                              fRect.fLeft, fRect.fTop, fRect.fRight, fRect.fBottom);
    }

    SkRect fRect;
};

}

static SkScalar gTracingTestWorkSink = 1.0f;

static void do_work(int howMuchWork) {
    // Do busy work so the trace marker durations are large enough to be readable in trace viewer
    if (FLAGS_slowTracingTest) {
        for (int i = 0; i < howMuchWork * 100; ++i) {
            gTracingTestWorkSink += SkScalarSin(i);
        }
    }
}

static void test_trace_simple() {
    // Simple event that lasts until the end of the current scope. TRACE_FUNC is an easy way
    // to insert the current function name.
    TRACE_EVENT0("skia", TRACE_FUNC);

    {
        // There are versions of the macro that take 1 or 2 named arguments. The arguments
        // can be any simple type. Strings need to be static/literal - we just copy pointers.
        // Argument names & values are shown when the event is selected in the viewer.
        TRACE_EVENT1("skia", "Nested work",
                     "isBGRA", kN32_SkColorType == kBGRA_8888_SkColorType);
        do_work(500);
    }

    {
        // If you must copy a string as an argument value, use the TRACE_STR_COPY macro.
        // This will instruct the tracing system (if one is active) to make a copy.
        SkString message = SkStringPrintf("%s %s", "Hello", "World");
        TRACE_EVENT1("skia", "Dynamic String", "message", TRACE_STR_COPY(message.c_str()));
        do_work(500);
    }
}

static void test_trace_counters() {
    TRACE_EVENT0("skia", TRACE_FUNC);

    {
        TRACE_EVENT0("skia", "Single Counter");

        // Counter macros allow recording a named value (which must be a 32-bit integer).
        // The value will be graphed in the viewer.
        for (int i = 0; i < 180; ++i) {
            SkScalar rad = SkDegreesToRadians(SkIntToScalar(i));
            TRACE_COUNTER1("skia", "sin", SkScalarSin(rad) * 1000.0f + 1000.0f);
            do_work(10);
        }
    }

    {
        TRACE_EVENT0("skia", "Independent Counters");

        // Recording multiple counters with separate COUNTER1 macros will make separate graphs.
        for (int i = 0; i < 180; ++i) {
            SkScalar rad = SkDegreesToRadians(SkIntToScalar(i));
            TRACE_COUNTER1("skia", "sin", SkScalarSin(rad) * 1000.0f + 1000.0f);
            TRACE_COUNTER1("skia", "cos", SkScalarCos(rad) * 1000.0f + 1000.0f);
            do_work(10);
        }
    }

    {
        TRACE_EVENT0("skia", "Stacked Counters");

        // Two counters can be recorded together with COUNTER2. They will be graphed together,
        // as a stacked bar graph. The combined graph needs a name, as does each data series.
        for (int i = 0; i < 180; ++i) {
            SkScalar rad = SkDegreesToRadians(SkIntToScalar(i));
            TRACE_COUNTER2("skia", "trig",
                           "sin", SkScalarSin(rad) * 1000.0f + 1000.0f,
                           "cos", SkScalarCos(rad) * 1000.0f + 1000.0f);
            do_work(10);
        }
    }
}

static void test_trace_objects() {
    TRACE_EVENT0("skia", TRACE_FUNC);

    // Objects can be tracked through time with the TRACE_EVENT_OBJECT_ macros.
    // The macros in use (and their idiosyncracies) are commented in the TracingShape class above.

    TracingCircle* circle = new TracingCircle(SkPoint::Make(20, 20), 15);
    circle->traceSnapshot();
    do_work(100);

    // Make another object. Objects with the same base type are shown in the same row in the viewer.
    TracingRect* rect = new TracingRect(SkRect::MakeWH(100, 50));
    rect->traceSnapshot();
    do_work(100);

    // We can create multiple snapshots of objects to reflect their state over time.
    circle->fCenter.offset(10, 10);
    circle->traceSnapshot();

    {
        // Other events (duration or instant) can refer directly to objects. For Skia's JSON
        // tracer, having an argument whose name starts with '#' will trigger the creation of JSON
        // that links the event to the object (with a direct link to the most recent snapshot).
        TRACE_EVENT1("skia", "Processing Shape", "#shape", circle);
        do_work(100);
    }

    delete circle;
    delete rect;
}

DEF_TEST(Tracing, reporter) {
    test_trace_simple();
    test_trace_counters();
    test_trace_objects();
}
