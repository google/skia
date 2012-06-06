
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkDebugDumper_DEFINED
#define SkDebugDumper_DEFINED
#include "SkDumpCanvas.h"
#include "SkEvent.h"

/** Formats the draw commands, and send them to a function-pointer provided
 by the caller.
 */
class SkDebugDumper : public SkDumpCanvas::Dumper {
public:
    SkDebugDumper(SkEventSinkID cID, SkEventSinkID clID, SkEventSinkID ipID);
    // override from baseclass that does the formatting, and in turn calls
    // the function pointer that was passed to the constructor
    virtual void dump(SkDumpCanvas*, SkDumpCanvas::Verb, const char str[],
                      const SkPaint*);
    
    void load() { fInit = true; };
    void unload() { fInit = false; fCount = 0;};
    void disable() { fDisabled = true; };
    void enable() { fDisabled = false; };
private:
    int             fCount;
    bool            fInit;
    bool            fDisabled;
    SkEventSinkID   fContentID;
    SkEventSinkID   fCommandsID;
    SkEventSinkID   fStateID;
    
    typedef SkDumpCanvas::Dumper INHERITED;
};
#endif
