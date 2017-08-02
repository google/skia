/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDeferredDisplayListMaker_DEFINED
#define SkDeferredDisplayListMaker_DEFINED

class SkCanvas;
class SkDeferredDisplayList;
class SkSurfaceCharacterization;

class SkDeferredDisplayListMaker {
public:
    SkDeferredDisplayListMaker(const SkSurfaceCharacterization&);

    SkCanvas* canvas();

    SkDeferredDisplayList* finish();

private:
};

#endif
