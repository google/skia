/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Fuzz.h"
#include <stdlib.h>
#include <signal.h>

int main(int argc, char** argv) {
    if (argc < 3) {
        SkDebugf("Usage: %s <fuzz name> <path/to/fuzzed.data>\n", argv[0]);
        return 1;
    }
    const char* name = argv[1];
    const char* path = argv[2];

    SkAutoTUnref<SkData> bytes(SkData::NewFromFileName(path));
    Fuzz fuzz(bytes);

    for (auto r = SkTRegistry<Fuzzable>::Head(); r; r = r->next()) {
        auto fuzzable = r->factory();
        if (0 == strcmp(name, fuzzable.name)) {
            SkDebugf("Running %s\n", fuzzable.name);
            fuzzable.fn(&fuzz);
            return 0;
        }
    }
    return 1;
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

