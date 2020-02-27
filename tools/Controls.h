/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef Controls_DEFINED
#define Controls_DEFINED

#include "include/core/SkScalar.h"

class ControlBool;
class ControlRange;

class ControlVisitor {
public:
    virtual void visit(ControlBool& control) {}
    virtual void visit(ControlRange& control) {}
    virtual ~ControlVisitor();
};

class Control {
public:
    constexpr Control(const char* name) : label{name} {}
    virtual void accept(ControlVisitor& visitor) = 0;
    virtual ~Control();
    const char* label;
};

class ControlBool : public Control {
    virtual void accept(ControlVisitor& visitor) { visitor.visit(*this); }
public:
    constexpr ControlBool(const char* name, bool initialValue) : Control{name}, value{initialValue} {}
    bool value;
};

class ControlRange : public Control {
    virtual void accept(ControlVisitor& visitor) { visitor.visit(*this); }
public:
    constexpr ControlRange(const char* name, SkScalar initialValue, SkScalar min, SkScalar max)
        : Control{name}, value{initialValue}, min{min}, max{max} {}
    SkScalar value;
    SkScalar min;
    SkScalar max;
};

#endif
