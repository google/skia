/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Fuzz.h"
#include "SkCommandLineFlags.h"

DEFINE_string2(match, m, "", "The usual match patterns, applied to name.");
DEFINE_string2(bytes, b, "", "Path to file containing fuzzed bytes.");

int main(int argc, char** argv) {
    SkCommandLineFlags::Parse(argc, argv);
    SkAutoTUnref<SkData> bytes;
    if (!FLAGS_bytes.isEmpty()) {
        bytes.reset(SkData::NewFromFileName(FLAGS_bytes[0]));
    }

    for (auto r = SkTRegistry<Fuzzable>::Head(); r; r = r->next()) {
        auto fuzzable = r->factory();
        if (!SkCommandLineFlags::ShouldSkip(FLAGS_match, fuzzable.name)) {
            SkDebugf("Running %s...\n", fuzzable.name);
            Fuzz fuzz(bytes);
            fuzzable.fn(&fuzz);
        }
    }
    return 0;
}


Fuzz::Fuzz(SkData* bytes) : fBytes(SkSafeRef(bytes)) {}

// These methods are all TODO(kjlubick).
uint32_t Fuzz::nextU() { return    0; }
float    Fuzz::nextF() { return 0.0f; }
uint32_t Fuzz::nextURange(uint32_t min, uint32_t max) { return min; }
float    Fuzz::nextFRange(float    min, float    max) { return min; }

