/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrContext_Base.h"

static int32_t next_id() {
    static std::atomic<int32_t> nextID{1};
    int32_t id;
    do {
        id = nextID++;
    } while (id == SK_InvalidGenID);
    return id;
}

GrContext_Base::GrContext_Base(GrBackendApi backend,
                               uint32_t contextID)
        : fBackend(backend)
        , fContextID(SK_InvalidGenID == contextID ? next_id() : contextID) {
}

GrContext_Base::~GrContext_Base() {
}


