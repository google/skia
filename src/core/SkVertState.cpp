/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkVertState.h"

bool VertState::Triangles(VertState* state) {
    int index = state->fCurrIndex;
    if (index + 3 > state->fCount) {
        return false;
    }
    state->f0 = index + 0;
    state->f1 = index + 1;
    state->f2 = index + 2;
    state->fCurrIndex = index + 3;
    return true;
}

bool VertState::TrianglesX(VertState* state) {
    const uint16_t* indices = state->fIndices;
    int index = state->fCurrIndex;
    if (index + 3 > state->fCount) {
        return false;
    }
    state->f0 = indices[index + 0];
    state->f1 = indices[index + 1];
    state->f2 = indices[index + 2];
    state->fCurrIndex = index + 3;
    return true;
}

bool VertState::TriangleStrip(VertState* state) {
    int index = state->fCurrIndex;
    if (index + 3 > state->fCount) {
        return false;
    }
    state->f2 = index + 2;
    if (index & 1) {
        state->f0 = index + 1;
        state->f1 = index + 0;
    } else {
        state->f0 = index + 0;
        state->f1 = index + 1;
    }
    state->fCurrIndex = index + 1;
    return true;
}

bool VertState::TriangleStripX(VertState* state) {
    const uint16_t* indices = state->fIndices;
    int index = state->fCurrIndex;
    if (index + 3 > state->fCount) {
        return false;
    }
    state->f2 = indices[index + 2];
    if (index & 1) {
        state->f0 = indices[index + 1];
        state->f1 = indices[index + 0];
    } else {
        state->f0 = indices[index + 0];
        state->f1 = indices[index + 1];
    }
    state->fCurrIndex = index + 1;
    return true;
}

bool VertState::TriangleFan(VertState* state) {
    int index = state->fCurrIndex;
    if (index + 3 > state->fCount) {
        return false;
    }
    state->f0 = 0;
    state->f1 = index + 1;
    state->f2 = index + 2;
    state->fCurrIndex = index + 1;
    return true;
}

bool VertState::TriangleFanX(VertState* state) {
    const uint16_t* indices = state->fIndices;
    int index = state->fCurrIndex;
    if (index + 3 > state->fCount) {
        return false;
    }
    state->f0 = indices[0];
    state->f1 = indices[index + 1];
    state->f2 = indices[index + 2];
    state->fCurrIndex = index + 1;
    return true;
}

VertState::Proc VertState::chooseProc(SkCanvas::VertexMode mode) {
    switch (mode) {
        case SkCanvas::kTriangles_VertexMode:
            return fIndices ? TrianglesX : Triangles;
        case SkCanvas::kTriangleStrip_VertexMode:
            return fIndices ? TriangleStripX : TriangleStrip;
        case SkCanvas::kTriangleFan_VertexMode:
            return fIndices ? TriangleFanX : TriangleFan;
        default:
            return NULL;
    }
}
