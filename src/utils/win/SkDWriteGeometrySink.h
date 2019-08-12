/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDWriteToPath_DEFINED
#define SkDWriteToPath_DEFINED

#include "include/core/SkTypes.h"
#include "src/utils/win/SkObjBase.h"

class SkPath;

#include <dwrite.h>
#include <d2d1.h>

class SkDWriteGeometrySink : public IDWriteGeometrySink {
private:
    LONG fRefCount;
    SkPath* fPath;

protected:
    explicit SkDWriteGeometrySink(SkPath* path);
    virtual ~SkDWriteGeometrySink();

public:
    SK_STDMETHODIMP QueryInterface(REFIID iid, void **object) override;
    SK_STDMETHODIMP_(ULONG) AddRef() override;
    SK_STDMETHODIMP_(ULONG) Release() override;

    SK_STDMETHODIMP_(void) SetFillMode(D2D1_FILL_MODE fillMode) override;
    SK_STDMETHODIMP_(void) SetSegmentFlags(D2D1_PATH_SEGMENT vertexFlags) override;
    SK_STDMETHODIMP_(void) BeginFigure(D2D1_POINT_2F startPoint, D2D1_FIGURE_BEGIN figureBegin) override;
    SK_STDMETHODIMP_(void) AddLines(const D2D1_POINT_2F *points, UINT pointsCount) override;
    SK_STDMETHODIMP_(void) AddBeziers(const D2D1_BEZIER_SEGMENT *beziers, UINT beziersCount) override;
    SK_STDMETHODIMP_(void) EndFigure(D2D1_FIGURE_END figureEnd) override;
    SK_STDMETHODIMP Close() override;

    static HRESULT Create(SkPath* path, IDWriteGeometrySink** geometryToPath);
};

#endif
