// Copyright 2021 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#ifndef Cmds_DEFINED
#define Cmds_DEFINED

class SkBitmap;
class SkCanvas;
class FakeCanvas;
class FakeMCBlob;
class SortKey;

#include "include/core/SkColor.h"
#include "include/core/SkRect.h"

#include "experimental/ngatoy/Fake.h"
#include "experimental/ngatoy/ngatypes.h"

class Cmd {
public:
    Cmd() : fID(ID::Invalid()) {}
    Cmd(ID id) : fID(id) {}
    virtual ~Cmd() {}

    ID id() const { return fID; }

    virtual SortKey getKey() = 0;

    virtual const FakeMCBlob* state() const { return nullptr; }

    // To generate the actual image
    virtual void execute(FakeCanvas*) const = 0;
    virtual void rasterize(uint32_t zBuffer[256][256], SkBitmap* dstBM) const = 0;

    // To generate the expected image
    virtual void execute(SkCanvas*, const FakeMCBlob* priorState) const = 0;
    virtual void dump() const = 0;

protected:
    const ID fID;

private:
};

class RectCmd : public Cmd {
public:
    RectCmd(ID, PaintersOrder, SkIRect, const FakePaint&, sk_sp<FakeMCBlob> state);

    uint32_t getSortZ() const;
    uint32_t getDrawZ() const;

    SortKey getKey() override;
    const FakeMCBlob* state() const override { return fMCState.get(); }

    void execute(FakeCanvas*) const override;
    void execute(SkCanvas* c, const FakeMCBlob* priorState) const override;
    void rasterize(uint32_t zBuffer[256][256], SkBitmap* dstBM) const override;

    void dump() const override {
        SkDebugf("%d: drawRect %d %d %d %d -- %d",
                 fID.toInt(),
                 fRect.fLeft, fRect.fTop, fRect.fRight, fRect.fBottom,
                 fPaintersOrder.toUInt());
    }

protected:

private:
    PaintersOrder     fPaintersOrder;
    SkIRect           fRect;
    FakePaint         fPaint;
    sk_sp<FakeMCBlob> fMCState;
};

#endif // Cmds_DEFINED
