
/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBenchmark.h"
#include "SkCanvas.h"
#include "SkWriter32.h"

class WriterBench : public SkBenchmark {
public:
    virtual bool isSuitableFor(Backend backend) SK_OVERRIDE {
        return backend == kNonRendering_Backend;
    }

protected:
    virtual const char* onGetName() SK_OVERRIDE {
        return "writer";
    }

    virtual void onDraw(const int loops, SkCanvas*) SK_OVERRIDE {
        static const char gStr[] = "abcdefghimjklmnopqrstuvwxyz";
        static const size_t gLen = strlen(gStr);
        SkWriter32 writer(256 * 4);
        for (int i = 0; i < loops; i++) {
            for (size_t j = 0; j <= gLen; j++) {
                writer.writeString(gStr, j);
            }
        }
    }

private:
    typedef SkBenchmark INHERITED;
};

////////////////////////////////////////////////////////////////////////////////

DEF_BENCH( return new WriterBench(); )
