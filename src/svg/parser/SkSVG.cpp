
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkSVG.h"
#include "SkSVGParser.h"
#include "SkTemplates.h"

SkSVG::SkSVG() {
}

SkSVG::~SkSVG() {
}

bool SkSVG::decodeStream(SkStream* stream);
{
    size_t size = stream->read(nil, 0);
    SkAutoTMalloc<char> storage(size);
    char* data = storage.get();
    size_t actual = stream->read(data, size);
    SkASSERT(size == actual);
    SkSVGParser parser(*fMaker);
    return parser.parse(data, actual, &fErrorCode, &fErrorLineNumber);
}
