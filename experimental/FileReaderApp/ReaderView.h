
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkView.h"
#include "SkColor.h"
#include "SkBitmap.h"

/*
 * Pipe Reader with File IO. This view reads from the data file produced by the
 * Pipe Writer.
 */

class ReaderView : public SkView {
public:
    ReaderView();
    virtual void draw(SkCanvas* canvas);

private:
    int     fFilePos;
    int     fFront;
    int     fBack;
    SkColor fBGColor;
    SkBitmap fBufferBitmaps[2];
    typedef SkView INHERITED;
};
