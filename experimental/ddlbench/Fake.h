// Copyright 2021 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#ifndef Fake_DEFINED
#define Fake_DEFINED

#include "include/core/SkBitmap.h"
#include "include/core/SkColor.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkTypes.h"
#include "Key.h"

#include <vector>

class Cmd;
class SkBitmap;

class FakePaint {
public:
    FakePaint() {}

    void setColor(SkColor c) { fColor = c; }
    SkColor getColor() const { return fColor; }

    bool isTransparent() const {
        return 0xFF != SkColorGetA(fColor);
    }

protected:

private:
    SkColor fColor = SK_ColorBLACK;
};

class FakeDevice {
public:
    FakeDevice(SkBitmap bm) : fBM(bm) {
        SkASSERT(bm.width() == 256 && bm.height() == 256);

        memset(fZBuffer, 0, sizeof(fZBuffer));
    }

    ~FakeDevice() {}

    void drawRect(int id, uint32_t z, SkIRect r, FakePaint p);

    void finalize();

    void getOrder(std::vector<int>*) const;

protected:

private:
    class KeyAndCmd {
    public:
        Key  fKey;
        Cmd* fCmd;
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

    void drawRect(int id, SkIRect r, FakePaint p);

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
