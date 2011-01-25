/*
    Copyright 2011 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */

#include "SkRefDict.h"
#include "SkString.h"

struct SkRefDict::Impl {
    Impl*       fNext;
    SkString    fName;
    SkRefCnt*   fData;
};

SkRefDict::SkRefDict() : fImpl(NULL) {}

SkRefDict::~SkRefDict() {
    this->removeAll();
}

SkRefCnt* SkRefDict::find(const char name[]) const {
    if (NULL == name) {
        return NULL;
    }

    Impl* rec = fImpl;
    while (rec) {
        if (rec->fName.equals(name)) {
            return rec->fData;
        }
        rec = rec->fNext;
    }
    return NULL;
}

void SkRefDict::set(const char name[], SkRefCnt* data) {
    if (NULL == name) {
        return;
    }

    Impl* rec = fImpl;
    Impl* prev = NULL;
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
    fImpl = NULL;
}

