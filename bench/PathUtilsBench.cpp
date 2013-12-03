/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkBenchmark.h"
#include "SkCanvas.h"
#include "SkPathUtils.h"
#include "SkRandom.h"
#include "SkTime.h"
#include "SkString.h"

#define H 16
#define W 16
#define STRIDE 2

//this function is redefined for sample, test, and bench. is there anywhere
// I can put it to avoid code duplcation?
static void fillRandomBits( int chars, char* bits ){
    SkRandom rand(SkTime::GetMSecs());

    for (int i = 0; i < chars; ++i){
        bits[i] = rand.nextU();
    }
}

static void path_proc(char* bits, SkPath* path) {
    SkPathUtils::BitsToPath_Path(path, bits, H, W, STRIDE);
}

static void region_proc(char* bits, SkPath* path) {
    SkPathUtils::BitsToPath_Region(path, bits, H, W, STRIDE);
}

/// Emulates the mix of rects blitted by gmail during scrolling
class PathUtilsBench : public SkBenchmark {
    typedef void (*Proc)(char*, SkPath*);

    Proc fProc;
    SkString fName;
    char* bits[H * STRIDE];

public:
    PathUtilsBench(Proc proc, const char name[])  {
        fProc = proc;
        fName.printf("pathUtils_%s", name);


    }

protected:
    virtual const char* onGetName() { return fName.c_str(); }

    virtual void onDraw(const int loops, SkCanvas* canvas) {

        for (int i = 0; i < loops; ++i){
            //create a random 16x16 bitmap
            fillRandomBits(H * STRIDE, (char*) &bits);

            //use passed function pointer to handle it
            SkPath path;
            fProc( (char*) &bits, &path);
        }
    }

private:
    typedef SkBenchmark INHERITED;
};

DEF_BENCH( return SkNEW_ARGS(PathUtilsBench, (path_proc, "path")); )
DEF_BENCH( return SkNEW_ARGS(PathUtilsBench, (region_proc, "region")); )
