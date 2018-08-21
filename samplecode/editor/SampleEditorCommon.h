/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SampleEditorCommon_DEFINED
#define SampleEditorCommon_DEFINED

#include "Sample.h"

#include <iostream>

class EditorClickable;
class EditorClick;

enum EditorMode {
    kSetup_EditorMode   = 0,
    kAnimate_EditorMode = 1,
};

class EditorClickable {
public:
    virtual ~EditorClickable() {}
    virtual void onMouseDown(EditorClick*) {}
    virtual void onMouseMove(EditorClick*) {}
    virtual void onMouseUp(EditorClick*) {}
};

class EditorClick : public Sample::Click {
public:
    EditorClickable* fTarget;
    EditorMode fMode;

    EditorClick(Sample* view)
            : Sample::Click(view)
            , fTarget(nullptr)
            , fMode(kSetup_EditorMode)
    {}
};

#endif
