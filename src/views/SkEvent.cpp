/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkEvent.h"
#include "SkMalloc.h"

void SkEvent::initialize(const char* type) {
    fType = nullptr;
    setType(type);
    f32 = 0;
}

SkEvent::SkEvent() {
    initialize("");
}

SkEvent::SkEvent(const SkEvent& src) {
    *this = src;
    setType(src.fType);
}

SkEvent::SkEvent(const char type[]) {
    SkASSERT(type);
    initialize(type);
}

SkEvent::~SkEvent() {
    sk_free(fType);
}

bool SkEvent::isType(const char type[]) const {
    size_t typeLen = strlen(type);
    return strncmp(fType, type, typeLen) == 0 && fType[typeLen] == 0;
}

void SkEvent::setType(const char type[]) {
    size_t typeLen = strlen(type);
    fType = (char*) sk_malloc_throw(typeLen + 1);
    memcpy(fType, type, typeLen);
    fType[typeLen] = 0;
}
