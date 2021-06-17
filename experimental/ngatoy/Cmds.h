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


//------------------------------------------------------------------------------------------------
class Cmd {
public:
    Cmd() : fID(ID::Invalid()) {}
    Cmd(ID id) : fID(id) {}
    virtual ~Cmd() {}

    ID id() const { return fID; }

    virtual SortKey getKey() = 0;

    // To generate the actual image
    virtual void execute(FakeCanvas*) const = 0;
    virtual void rasterize(uint32_t zBuffer[256][256], SkBitmap* dstBM) const = 0;

    // To generate the expected image
    virtual void execute(SkCanvas*) const = 0;
    virtual void dump() const = 0;

protected:
    const ID fID;

private:
};

//------------------------------------------------------------------------------------------------
// This Cmd only appears in the initial list defining a test case. It never makes it into the
// sorted Cmds.
class SaveCmd : public Cmd {
public:
    SaveCmd() : Cmd() {}

    SortKey getKey() override { SkASSERT(0); return {}; }

    void execute(FakeCanvas*) const override;
    void execute(SkCanvas*) const override;
    void rasterize(uint32_t zBuffer[256][256], SkBitmap* dstBM) const override {
        SkASSERT(0);
    }

    void dump() const override {
        SkDebugf("%d: save", fID.toInt());
    }

protected:
private:
};

//------------------------------------------------------------------------------------------------
// This Cmd only appears in the initial list defining a test case. It never makes it into the
// sorted Cmds.
class RestoreCmd : public Cmd {
public:
    RestoreCmd() : Cmd() {}

    SortKey getKey() override { SkASSERT(0); return {}; }

    void execute(FakeCanvas*) const override;
    void execute(SkCanvas*) const override;
    void rasterize(uint32_t zBuffer[256][256], SkBitmap* dstBM) const override {
        SkASSERT(0);
    }

    void dump() const override {
        SkDebugf("%d: restore", fID.toInt());
    }

protected:
private:
};

//------------------------------------------------------------------------------------------------
class RectCmd : public Cmd {
public:
    RectCmd(ID, PaintersOrder, SkIRect, const FakePaint&, sk_sp<FakeMCBlob> state);

    uint32_t getSortZ() const;
    uint32_t getDrawZ() const;

    SortKey getKey() override;
    const FakeMCBlob* state() const { return fMCState.get(); }

    void execute(FakeCanvas*) const override;
    void execute(SkCanvas*) const override;
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

//------------------------------------------------------------------------------------------------
class ClipCmd : public Cmd {
public:
    ClipCmd(ID, PaintersOrder paintersOrderWhenAdded, SkIRect r);

    uint32_t getSortZ() const;
    uint32_t getDrawZ() const;

    SortKey getKey() override;

    void execute(FakeCanvas*) const override;
    void execute(SkCanvas*) const override;
    void rasterize(uint32_t zBuffer[256][256], SkBitmap* dstBM) const override;

    void dump() const override {
        SkDebugf("%d: clipRect %d %d %d %d",
                 fID.toInt(),
                 fRect.fLeft, fRect.fTop, fRect.fRight, fRect.fBottom);
    }

protected:

private:
    SkIRect       fRect;
    PaintersOrder fPaintersOrderWhenAdded;
};

//------------------------------------------------------------------------------------------------

#endif // Cmds_DEFINED
