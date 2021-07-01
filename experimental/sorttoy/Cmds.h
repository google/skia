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
#include "include/core/SkRefCnt.h"

#include "experimental/sorttoy/Fake.h"
#include "experimental/sorttoy/sorttypes.h"

//------------------------------------------------------------------------------------------------
class Cmd : public SkRefCnt {
public:
    Cmd() : fID(ID::Invalid()) {}
    Cmd(ID id) : fID(id) {}
    ~Cmd() override {}

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

    SortKey getKey() override;

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

    SortKey getKey() override;

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
class DrawCmd : public Cmd {
public:
    DrawCmd(ID, Shape, SkIRect, const FakePaint&);  // for creating the test cases
    DrawCmd(ID, PaintersOrder, Shape, SkIRect, const FakePaint&, sk_sp<FakeMCBlob> state);

    bool contains(int x, int y) const;

    uint32_t getSortZ() const;
    uint32_t getDrawZ() const;

    SortKey getKey() override;
    const FakeMCBlob* state() const { return fMCState.get(); }

    void execute(FakeCanvas*) const override;
    void execute(SkCanvas*) const override;
    void rasterize(uint32_t zBuffer[256][256], SkBitmap* dstBM) const override;

    void dump() const override {
        SkDebugf("%d: draw%s %d %d %d %d -- %d",
                 fID.toInt(),
                 fShape == Shape::kRect ? "Rect" : "Oval",
                 fRect.fLeft, fRect.fTop, fRect.fRight, fRect.fBottom,
                 fPaintersOrder.toUInt());
    }

protected:

private:
    PaintersOrder     fPaintersOrder;
    Shape             fShape;
    SkIRect           fRect;
    FakePaint         fPaint;
    sk_sp<FakeMCBlob> fMCState;
};

//------------------------------------------------------------------------------------------------
class ClipCmd : public Cmd {
public:
    ClipCmd(ID, Shape, SkIRect);  // for creating the test cases
    ClipCmd(ID, PaintersOrder paintersOrderWhenAdded, Shape, SkIRect);
    ~ClipCmd() override;

    bool contains(int x, int y) const;

    uint32_t getSortZ() const;
    uint32_t getDrawZ() const;

    SortKey getKey() override;

    void onAboutToBePopped(PaintersOrder paintersOrderWhenPopped);

    void execute(FakeCanvas*) const override;
    void execute(SkCanvas*) const override;
    void rasterize(uint32_t zBuffer[256][256], SkBitmap* dstBM) const override;

    void dump() const override {
        SkDebugf("%d: clip%s %d %d %d %d",
                 fID.toInt(),
                 fShape == Shape::kRect ? "Rect" : "Oval",
                 fRect.fLeft, fRect.fTop, fRect.fRight, fRect.fBottom);
    }

    void mutate(SkIPoint trans) {
        SkASSERT(!fHasBeenMutated);

        fRect.offset(trans.fX, trans.fY);
        fHasBeenMutated = true;
    }

    bool hasBeenMutated() const { return fHasBeenMutated; }
    SkIRect rect() const { return fRect; }

protected:

private:
    bool          fHasBeenMutated = false;
    Shape         fShape;
    SkIRect       fRect;
    PaintersOrder fPaintersOrderWhenAdded;
    PaintersOrder fPaintersOrderWhenPopped;
};

//------------------------------------------------------------------------------------------------

#endif // Cmds_DEFINED
