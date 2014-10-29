/*
 * Copyright 2014 Google Inc.
 *
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#include "Path2D.h"
#include "Global.h"

Global* Path2D::gGlobal = NULL;
v8::Persistent<v8::ObjectTemplate> Path2D::gPath2DTemplate;

void weakPath2DCallback(const v8::WeakCallbackData<v8::Object, Path2D>& args) {
    delete args.GetParameter();
}

// Wraps an SkPath* in a Path2D object.
Path2D::Path2D(SkPath* path) : path_(path) {
    // Handle scope for temporary handles.
    v8::HandleScope handleScope(gGlobal->getIsolate());

    // Just once create the ObjectTemplate for what Path2D looks like in JS.
    if (gPath2DTemplate.IsEmpty()) {
        v8::Local<v8::ObjectTemplate> localTemplate = v8::ObjectTemplate::New();

        // Add a field to store the pointer to a SkPath pointer.
        localTemplate->SetInternalFieldCount(1);

        gPath2DTemplate.Reset(gGlobal->getIsolate(), localTemplate);
    }
    v8::Handle<v8::ObjectTemplate> templ =
            v8::Local<v8::ObjectTemplate>::New(gGlobal->getIsolate(), gPath2DTemplate);

    // Create an empty Path2D wrapper.
    v8::Local<v8::Object> result = templ->NewInstance();

    // Store the SkPath pointer in the JavaScript wrapper.
    result->SetInternalField(0, v8::External::New(gGlobal->getIsolate(), this));
    gGlobal->getIsolate()->AdjustAmountOfExternalAllocatedMemory(sizeof(SkPath));

    // Make a weak persistent and set up the callback so we can delete the path pointer.
    // TODO(jcgregorio) Figure out why weakPath2DCallback never gets called and we leak.
    v8::Persistent<v8::Object> weak(gGlobal->getIsolate(), result);
    weak.SetWeak(this, weakPath2DCallback);
    this->handle_.Reset(gGlobal->getIsolate(), weak);
}

Path2D::~Path2D() {
    delete path_;
    handle_.Reset();
    gGlobal->getIsolate()->AdjustAmountOfExternalAllocatedMemory(-sizeof(SkPath));
}
