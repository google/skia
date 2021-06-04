// Copyright 2021 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#ifndef Fake_DEFINED
#define Fake_DEFINED

#include "experimental/ngatoy/SortKey.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkColor.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"

#include <vector>

class Cmd;
class ClipCmd;
class FakeCanvas;
class SkBitmap;
class SkCanvas;

constexpr SkColor SK_ColorUNUSED       = SkColorSetARGB(0x00, 0xFF, 0xFF, 0xFF);

// This is roughly equivalent to a moment in time of an SkClipStack. It is snapped off of a
// FakeStateTracker.
class FakeMCBlob : public SkRefCnt {
public:
    class MCState {
    public:
        MCState() {}

        void addRect(SkIRect r, ClipCmd* clipCmd) {
            fRects1.push_back(r.makeOffset(fTrans.fX, fTrans.fY));
            fCmds.push_back(clipCmd);
            fCached = nullptr;
        }

        void translate(SkIPoint trans) {
            fTrans += trans;
            fCached = nullptr;
        }

        SkIPoint getTrans() const { return fTrans; }

        bool operator==(const MCState& other) const {
            return fTrans == other.fTrans &&
                   fRects1 == other.fRects1;
        }

        void apply(SkCanvas*) const;
        void apply(FakeCanvas*) const;
#if 0
        bool clipped(int x, int y) const {
            for (auto r : fRects1) {
                if (!r.contains(x, y)) {
                    return true;
                }
            }
            return false;
        }
#endif
        const std::vector<SkIRect>& rects() const { return fRects1; }
        const std::vector<ClipCmd*>& cmds() const { return fCmds; }

        sk_sp<FakeMCBlob> getCached() const {
            return fCached;
        }
        void setCached(sk_sp<FakeMCBlob> cached) {
            fCached = cached;
        }

        void popit(uint32_t zWhenPopped);

    protected:
        friend class FakeMCBlob;

        SkIPoint             fTrans { 0, 0 };
        // These clip rects are in the 'parent' space of this MCState (i.e., in the coordinate
        // frame of the MCState prior to this one in 'fStack'). Alternatively, the 'fTrans' in
        // effect when they were added has already been applied.
        std::vector<SkIRect>  fRects1;
        std::vector<ClipCmd*> fCmds;
        sk_sp<FakeMCBlob>     fCached;
    };

    FakeMCBlob(const std::vector<MCState>& stack) : fID(NextID()), fStack(stack) {
        fScissor = SkIRect::MakeLTRB(-1000, -1000, 1000, 1000);

        for (auto s : fStack) {
            // xform the clip rects into device space
            for (auto& r : s.fRects1) {
                r.offset(fCTM);
                SkAssertResult(fScissor.intersect(r));
            }
            fCTM += s.getTrans();
        }
    }

    // Find the common prefix between the two states
    int determineSharedPrefix(const FakeMCBlob* other) const {
        if (!other) {
            return 0;
        }

        int i = 0;
        for ( ; i < this->count() && i < other->count() && (*this)[i] == (*other)[i]; ++i) {
            /**/
        }

        return i;
    }

    int count() const { return fStack.size(); }
    int id() const { return fID; }
    SkIPoint ctm() const { return fCTM; }
    const std::vector<MCState>& mcStates() const { return fStack; }
    const MCState& operator[](int index) const { return fStack[index]; }
    SkIRect scissor() const { return fScissor; }

#if 0
    bool clipped(int x, int y) const {
        for (auto& s : fStack) {
            if (s.clipped(x, y)) {
                return true;
            }
        }

        return false;
    }
#endif

private:
    static int NextID() {
        static int sID = 1;
        return sID++;
    }

    const int            fID;
    SkIPoint             fCTM { 0, 0 };
    SkIRect              fScissor;
    std::vector<MCState> fStack;
};

class FakeStateTracker {
public:
    FakeStateTracker() {
        fStack.push_back(FakeMCBlob::MCState());
    }

    sk_sp<FakeMCBlob> snapState() {
        sk_sp<FakeMCBlob> tmp = fStack.back().getCached();
        if (tmp) {
            return tmp;
        }

        tmp = sk_make_sp<FakeMCBlob>(fStack);
        fStack.back().setCached(tmp);
        return tmp;
    }

    void push() {
        fStack.push_back(FakeMCBlob::MCState());
    }

    void clipRect(SkIRect clipRect, ClipCmd* clipCmd) {
        fStack.back().addRect(clipRect, clipCmd);
    }

    // For now we only store translates - in the full Skia this would be the whole 4x4 matrix
    void translate(SkIPoint trans) {
        fStack.back().translate(trans);
    }

    void pop(uint32_t z) {
        SkASSERT(fStack.size() > 0);
        fStack.back().popit(z);
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
    FakePaint(SkColor c)
        : fType(Type::kNormal)
        , fColor0(c)
        , fColor1(SK_ColorUNUSED) {
    }

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

    // Get a material id for this paint that should be jammed into the sort key
    int toID() const {
        switch (fType) {
            case Type::kNormal: return kSolidMat;
            case Type::kLinear: return kLinearMat;
            case Type::kRadial: return kRadialMat;
        }
        SkUNREACHABLE;
    }

    SkColor evalColor(int x, int y) const;

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

    void save();
    void drawRect(int id, uint32_t z, SkIRect, FakePaint);
    void clipRect(int id, uint32_t paintersOrder, SkIRect r);
    void translate(SkIPoint trans) {
        fTracker.translate(trans);
    }

    void restore(uint32_t z);

    void finalize();

    void getOrder(std::vector<int>*) const;
    sk_sp<FakeMCBlob> snapState() { return fTracker.snapState(); }

protected:

private:
    void sort();

    bool              fFinalized = false;
    std::vector<Cmd*> fSortedCmds;

    FakeStateTracker  fTracker;
    SkBitmap          fBM;
    uint32_t          fZBuffer[256][256];
};

class FakeCanvas {
public:
    FakeCanvas(SkBitmap& bm) {
        fDeviceStack.push_back(std::make_unique<FakeDevice>(bm));
    }

    void saveLayer() {
        SkASSERT(!fFinalized);

        // TODO: implement
    }

    void save() {
        SkASSERT(!fFinalized);
        fDeviceStack.back()->save();
    }

    void drawRect(int id, SkIRect, FakePaint);

    void clipRect(int id, SkIRect);

    void translate(SkIPoint trans) {
        SkASSERT(!fFinalized);

        fDeviceStack.back()->translate(trans);
    }

    void restore() {
        SkASSERT(!fFinalized);
        fDeviceStack.back()->restore(this->peekZ());
    }

    void finalize();

    std::vector<int> getOrder() const;
    sk_sp<FakeMCBlob> snapState() {
        return fDeviceStack.back()->snapState();
    }

protected:

private:
    uint32_t nextZ() {
        return fNextZ++;
    }
    uint32_t peekZ() const {
        return fNextZ;
    }

    int                                      fNextZ = 1;
    bool                                     fFinalized = false;
    std::vector<std::unique_ptr<FakeDevice>> fDeviceStack;
};


#endif // Fake_DEFINED
