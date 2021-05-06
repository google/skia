// Copyright 2021 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#ifndef Fake_DEFINED
#define Fake_DEFINED

#include "experimental/ddlbench/SortKey.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkColor.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"

#include <vector>

class Cmd;
class SkBitmap;
class SkCanvas;

constexpr SkColor SK_ColorUNUSED       = SkColorSetARGB(0x00, 0xFF, 0xFF, 0xFF);

class FakeMCBlob : public SkRefCnt {
public:
    class MCState {
    public:
        MCState() {}

        void addRect(SkIRect r) {
            fRects.push_back(r.makeOffset(fTrans.fX, fTrans.fY));
        }

        void translate(int x, int y) {
            fTrans.fX += x;
            fTrans.fY += y;
        }

        SkIPoint getTrans() const { return fTrans; }

        bool operator==(const MCState& other) const {
            return fTrans == other.fTrans &&
                   fRects == other.fRects;
        }

        void apply(SkCanvas*) const;

    protected:
        friend class FakeMCBlob;

        SkIPoint             fTrans { 0, 0 };
        // These clip rects are in the 'parent' space of this MCState (i.e., in the coordinate
        // frame of the MCState prior to this one in 'fStack'). Alternatively, the 'fTrans' in
        // effect when they were added has already been applied.
        std::vector<SkIRect> fRects;
    };

    FakeMCBlob(const std::vector<MCState>& stack) : fID(NextID()), fStack(stack) {
        for (auto s : fStack) {
            // xform the clip rects into device space
            for (auto& r : s.fRects) {
                r.offset(fCTM);
            }
            fCTM += s.getTrans();
        }
    }

    int count() const { return fStack.size(); }
    SkIPoint ctm() const { return fCTM; }
    const std::vector<MCState>& clipRects() const { return fStack; }
    const MCState& operator[](int index) const { return fStack[index]; }

private:
    static int NextID() {
        static int sID = 1;
        return sID++;
    }

    const int            fID;
    SkIPoint             fCTM { 0, 0 };
    std::vector<MCState> fStack;
};

class FakeStateTracker {
public:
    FakeStateTracker() {
        fStack.push_back(FakeMCBlob::MCState());
    }

    sk_sp<FakeMCBlob> snapState() {
        return sk_make_sp<FakeMCBlob>(fStack);
    }

    void push() {
        fStack.push_back(FakeMCBlob::MCState());
    }

    void clipRect(SkIRect clipRect) {
        fStack.back().addRect(clipRect);
    }

    void translate(int x, int y) {
        fStack.back().translate(x, y);
    }

    void pop() {
        SkASSERT(fStack.size() > 1);
        fStack.pop_back();
    }

protected:

private:
    std::vector<FakeMCBlob::MCState> fStack;
};

// The FakePaint simulates two aspects of the SkPaint:
//
// Batching based on FP context changes:
//   There are three types of paint (solid color, linear gradient and radial gradient) and,
//   ideally, they would all be batched together
//
// Transparency:
//   The transparent objects need to be draw back to front.
class FakePaint {
public:
    FakePaint() {}

    void setColor(SkColor c) {
        fType = Type::kNormal;
        fColor0 = c;
        fColor1 = SK_ColorUNUSED;
    }
    SkColor getColor() const {
        SkASSERT(fType == Type::kNormal);
        return fColor0;
    }

    void setLinear(SkColor c0, SkColor c1) {
        fType = Type::kLinear;
        fColor0 = c0;
        fColor1 = c1;
    }

    void setRadial(SkColor c0, SkColor c1) {
        fType = Type::kRadial;
        fColor0 = c0;
        fColor1 = c1;
    }

    SkColor c0() const { return fColor0; }
    SkColor c1() const { return fColor1; }

    bool isTransparent() const {
        if (fType == Type::kNormal) {
            return 0xFF != SkColorGetA(fColor0);
        } else {
            return 0xFF != SkColorGetA(fColor0) && 0xFF != SkColorGetA(fColor1);
        }
    }

    // Get an id for this paint that should be jammed into the sort key
    int toID() const {
        switch (fType) {
            case Type::kNormal: return kSolidMat;
            case Type::kLinear: return kLinearMat;
            case Type::kRadial: return kRadialMat;
        }
        SkUNREACHABLE;
    }

protected:

private:
    enum class Type {
        kNormal,
        kLinear,
        kRadial
    };

    Type    fType   = Type::kNormal;
    SkColor fColor0 = SK_ColorBLACK;
    SkColor fColor1 = SK_ColorBLACK;
};

class FakeDevice {
public:
    FakeDevice(SkBitmap bm) : fBM(bm) {
        SkASSERT(bm.width() == 256 && bm.height() == 256);

        memset(fZBuffer, 0, sizeof(fZBuffer));
    }

    ~FakeDevice() {}

    void drawRect(int id, uint32_t z, SkIRect, FakePaint, sk_sp<FakeMCBlob>);

    void finalize();

    void getOrder(std::vector<int>*) const;

protected:

private:
    class KeyAndCmd {
    public:
        SortKey fKey;
        Cmd*    fCmd;
    };

    void sort() {
        // In general we want:
        //  opaque draws to occur front to back (i.e., in reverse painter's order)
        //  transparent draws to occur back to front (i.e., in painter's order)
        std::sort(fSortedCmds.begin(), fSortedCmds.end(),
                  [](const KeyAndCmd& a, const KeyAndCmd& b) {
                      return a.fKey < b.fKey;
                  });
    }

    bool                   fFinalized = false;
    std::vector<KeyAndCmd> fSortedCmds;

    SkBitmap               fBM;
    uint32_t               fZBuffer[256][256];
};

class FakeCanvas {
public:
    FakeCanvas(SkBitmap& bm) {
        fStateStack.push_back({bm, SkMatrix::I()});
    }

    void saveLayer() {
        SkASSERT(!fFinalized);

    }

    void drawRect(int id, SkIRect, FakePaint, sk_sp<FakeMCBlob>);

    void clipRect(SkIRect r) {
        SkASSERT(!fFinalized);

    }

    void translate(SkVector trans) {
        SkASSERT(!fFinalized);

    }

    void finalize();

    std::vector<int> getOrder() const;

protected:

private:
    uint32_t nextZ() {
        return fNextZ++;
    }

    class State {
    public:
        FakeDevice fDevice;
        SkMatrix   fCTM;
    };

    int                    fNextZ = 1;
    bool                   fFinalized = false;
    std::vector<State>     fStateStack;
};


#endif // Fake_DEFINED
