/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Fuzz.h"
#include "SkCommandLineFlags.h"
#include <signal.h>
#include <stdlib.h>

DEFINE_string2(bytes, b, "", "A path to a file containing fuzzed bytes.");
DEFINE_string2(match, m, "", "The usual --match, applied to DEF_FUZZ names.");

int main(int argc, char** argv) {
    SkCommandLineFlags::Parse(argc, argv);

    if (FLAGS_bytes.isEmpty()) {
        SkDebugf("Usage: %s -b <path/to/fuzzed.data> [-m pattern]\n", argv[0]);
        return 1;
    }
    SkAutoTUnref<SkData> bytes(SkData::NewFromFileName(FLAGS_bytes[0]));

    for (auto r = SkTRegistry<Fuzzable>::Head(); r; r = r->next()) {
        auto fuzzable = r->factory();
        if (!SkCommandLineFlags::ShouldSkip(FLAGS_match, fuzzable.name)) {
            SkDebugf("Fuzzing %s...\n", fuzzable.name);
            Fuzz fuzz(bytes);
            fuzzable.fn(&fuzz);
        }
    }
    return 0;
}


Fuzz::Fuzz(SkData* bytes) : fBytes(SkSafeRef(bytes)), fNextByte(0) {}

void Fuzz::signalBug   () { raise(SIGSEGV); }
void Fuzz::signalBoring() { exit(0); }

template <typename T>
T Fuzz::nextT() {
    if (fNextByte + sizeof(T) > fBytes->size()) {
        this->signalBoring();
    }

    T val;
    memcpy(&val, fBytes->bytes() + fNextByte, sizeof(T));
    fNextByte += sizeof(T);
    return val;
}

uint8_t  Fuzz::nextB() { return this->nextT<uint8_t >(); }
uint32_t Fuzz::nextU() { return this->nextT<uint32_t>(); }
float    Fuzz::nextF() { return this->nextT<float   >(); }

