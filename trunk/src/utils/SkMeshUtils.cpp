
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkMeshUtils.h"
#include "SkCanvas.h"
#include "SkPaint.h"

SkMeshIndices::SkMeshIndices() {
    sk_bzero(this, sizeof(*this));
}

SkMeshIndices::~SkMeshIndices() {
    sk_free(fStorage);
}

bool SkMeshIndices::init(SkPoint tex[], uint16_t indices[],
                         int texW, int texH, int rows, int cols) {
    if (rows < 2 || cols < 2) {
        sk_free(fStorage);
        fStorage = NULL;
        fTex = NULL;
        fIndices = NULL;
        fTexCount = fIndexCount = 0;
        return false;
    }

    sk_free(fStorage);
    fStorage = NULL;

    fTexCount = rows * cols;
    rows -= 1;
    cols -= 1;
    fIndexCount = rows * cols * 6;

    if (tex) {
        fTex = tex;
        fIndices = indices;
    } else {
        fStorage = sk_malloc_throw(fTexCount * sizeof(SkPoint) +
                                   fIndexCount * sizeof(uint16_t));
        fTex = (SkPoint*)fStorage;
        fIndices = (uint16_t*)(fTex + fTexCount);
    }

    // compute the indices
    {
        uint16_t* idx = fIndices;
        int index = 0;
        for (int y = 0; y < cols; y++) {
            for (int x = 0; x < rows; x++) {
                *idx++ = index;
                *idx++ = index + rows + 1;
                *idx++ = index + 1;

                *idx++ = index + 1;
                *idx++ = index + rows + 1;
                *idx++ = index + rows + 2;

                index += 1;
            }
            index += 1;
        }
    }

    // compute texture coordinates
    {
        SkPoint* tex = fTex;
        const SkScalar dx = SkIntToScalar(texW) / rows;
        const SkScalar dy = SkIntToScalar(texH) / cols;
        for (int y = 0; y <= cols; y++) {
            for (int x = 0; x <= rows; x++) {
                tex->set(x*dx, y*dy);
                tex += 1;
            }
        }
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

#include "SkShader.h"

void SkMeshUtils::Draw(SkCanvas* canvas, const SkBitmap& bitmap,
                       int rows, int cols, const SkPoint verts[],
                       const SkColor colors[], const SkPaint& paint) {
    SkMeshIndices idx;

    if (idx.init(bitmap.width(), bitmap.height(), rows, cols)) {
        SkPaint p(paint);
        p.setShader(SkShader::CreateBitmapShader(bitmap,
                                         SkShader::kClamp_TileMode,
                                         SkShader::kClamp_TileMode))->unref();
        canvas->drawVertices(SkCanvas::kTriangles_VertexMode,
                             rows * cols, verts, idx.tex(), colors, NULL,
                             idx.indices(), idx.indexCount(), p);
    }
}

