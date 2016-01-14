/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Fuzz.h"

int main(int argc, char** argv) {
    ASSERT(argc > 2);
    const char* name = argv[1];
    const char* path = argv[2];

    SkAutoTUnref<SkData> bytes(SkData::NewFromFileName(path));
    Fuzz fuzz(bytes);

    for (auto r = SkTRegistry<Fuzzable>::Head(); r; r = r->next()) {
        auto fuzzable = r->factory();
        if (0 == strcmp(name, fuzzable.name)) {
            fuzzable.fn(&fuzz);
            return 0;
        }
    }
    return 1;
}


Fuzz::Fuzz(SkData* bytes) : fBytes(SkSafeRef(bytes)), fNextByte(0) {}

template <typename T>
static T read(const SkData* data, int* next) {
    ASSERT(sizeof(T) <= data->size());
    if (*next + sizeof(T) > data->size()) {
        *next = 0;
    }
    T val;
    memcpy(&val, data->bytes() + *next, sizeof(T));
    *next += sizeof(T);
    return val;
}

uint8_t  Fuzz::nextB() { return read<uint8_t >(fBytes, &fNextByte); }
uint32_t Fuzz::nextU() { return read<uint32_t>(fBytes, &fNextByte); }
float    Fuzz::nextF() { return read<float   >(fBytes, &fNextByte); }

