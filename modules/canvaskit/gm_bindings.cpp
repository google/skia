/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <emscripten.h>
#include <emscripten/bind.h>
#include "gm/gm.h"

using namespace emscripten;

static void ListGMs() {
    SkDebugf("Hello world\n");
    for (skiagm::GMFactory fact : skiagm::GMRegistry::Range()) {
        std::unique_ptr<skiagm::GM> gm(fact());
        SkDebugf("gm %s\n", gm->getName());
    }

}

EMSCRIPTEN_BINDINGS(GMs) {
    function("ListGMs", &ListGMs);
}
