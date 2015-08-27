
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkRefDict.h"
#include "SkString.h"

struct SkRefDict::Impl {
    Impl*       fNext;
    SkString    fName;
    SkRefCnt*   fData;
};

SkRefDict::SkRefDict() : fImpl(nullptr) {}

SkRefDict::~SkRefDict() {
    this->removeAll();
}

SkRefCnt* SkRefDict::find(const char name[]) const {
    if (nullptr == name) {
        return nullptr;
    }

    Impl* rec = fImpl;
    while (rec) {
        if (rec->fName.equals(name)) {
            return rec->fData;
        }
        rec = rec->fNext;
    }
    return nullptr;
}

void SkRefDict::set(const char name[], SkRefCnt* data) {
    if (nullptr == name) {
        return;
    }

    Impl* rec = fImpl;
    Impl* prev = nullptr;
    while (rec) {
        if (rec->fName.equals(name)) {
            if (data) {
                // replace
                data->ref();
                rec->fData->unref();
                rec->fData = data;
            } else {
                // remove
                rec->fData->unref();
                if (prev) {
                    prev->fNext = rec->fNext;
                } else {
                    fImpl = rec->fNext;
                }
                delete rec;
            }
            return;
        }
        prev = rec;
        rec = rec->fNext;
    }

    // if get here, name was not found, so add it
    data->ref();
    rec = new Impl;
    rec->fName.set(name);
    rec->fData = data;
    // prepend to the head of our list
    rec->fNext = fImpl;
    fImpl = rec;
}

void SkRefDict::removeAll() {
    Impl* rec = fImpl;
    while (rec) {
        Impl* next = rec->fNext;
        rec->fData->unref();
        delete rec;
        rec = next;
    }
    fImpl = nullptr;
}
