/*
 * Copyright 2014 Google Inc.
 *
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#ifndef SkV8Example_Path2D_DEFINED
#define SkV8Example_Path2D_DEFINED

#include <v8.h>

#include "SkPath.h"
#include "SkTypes.h"

class Global;

// Path2D bridges between JS and SkPath.
class Path2D : SkNoncopyable {
public:
    Path2D(SkPath* path);
    virtual ~Path2D();

    static void AddToGlobal(Global* global) {
        gGlobal = global;
    }

    v8::Persistent<v8::Object>& persistent() {
        return handle_;
    }

    SkPath* path() {
        return path_;
    }

private:
    // The handle to this object in JS space.
    v8::Persistent<v8::Object> handle_;

    SkPath* path_;

    // The global context we are running in.
    static Global* gGlobal;

    // The template for what a JS Path2D object looks like.
    static v8::Persistent<v8::ObjectTemplate> gPath2DTemplate;
};

#endif
