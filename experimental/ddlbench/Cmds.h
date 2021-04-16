// Copyright 2021 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#ifndef Cmds_DEFINED
#define Cmds_DEFINED

class SkCanvas;
class FakeCanvas;

#include "include/core/SkRect.h"
#include "include/core/SkColor.h"

class Cmd {
public:
    Cmd(int id) : fID(id) {}

    int id() const { return fID; }

    virtual void execute(FakeCanvas*) const = 0;
    virtual void execute(SkCanvas*) const = 0;
    virtual void dump() const = 0;

protected:
    const int fID;

private:
};

class RectCmd : public Cmd {
public:
    RectCmd(int id, SkRect r, SkColor c) : Cmd(id), fRect(r), fColor(c) {}

    void execute(FakeCanvas* c) const override;

    void execute(SkCanvas* c) const override;

    void dump() const override {
        SkDebugf("%d: drawRect", fID);
    }

protected:

private:
    SkRect  fRect;
    SkColor fColor;
};

#endif // Cmds_DEFINED
