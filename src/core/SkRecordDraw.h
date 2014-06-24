/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRecordDraw_DEFINED
#define SkRecordDraw_DEFINED

#include "SkRecord.h"
#include "SkCanvas.h"
#include "SkDrawPictureCallback.h"

// Draw an SkRecord into an SkCanvas.  A convenience wrapper around SkRecords::Draw.
void SkRecordDraw(const SkRecord&, SkCanvas*, SkDrawPictureCallback* = NULL);

namespace SkRecords {

// This is an SkRecord visitor that will draw that SkRecord to an SkCanvas.
class Draw : SkNoncopyable {
public:
    explicit Draw(SkCanvas* canvas)
        : fInitialCTM(canvas->getTotalMatrix()), fCanvas(canvas), fIndex(0) {}

    unsigned index() const { return fIndex; }
    void next() { ++fIndex; }

    template <typename T> void operator()(const T& r) {
        if (!this->skip(r)) {
            this->draw(r);
        }
    }

private:
    // No base case, so we'll be compile-time checked that we implement all possibilities.
    template <typename T> void draw(const T&);

    // skip() should return true if we can skip this command, false if not.
    // It may update fIndex directly to skip more than just this one command.

    // Mostly we just blindly call fCanvas and let it handle quick rejects itself.
    template <typename T> bool skip(const T&) { return false; }

    // We add our own quick rejects for commands added by optimizations.
    bool skip(const PairedPushCull&);
    bool skip(const BoundedDrawPosTextH&);

    const SkMatrix fInitialCTM;
    SkCanvas* fCanvas;
    unsigned fIndex;
};

}  // namespace SkRecords

#endif//SkRecordDraw_DEFINED
