/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#if defined(SK_BUILD_FOR_WIN)

#include "include/core/SkPath.h"
#include "src/utils/SkFloatUtils.h"
#include "src/utils/win/SkDWriteGeometrySink.h"
#include "src/utils/win/SkObjBase.h"

#include <dwrite.h>
#include <d2d1.h>

SkDWriteGeometrySink::SkDWriteGeometrySink(SkPath* path)
    : fRefCount{1}, fPath{path}, fStarted{false}, fCurrent{0,0} {}

SkDWriteGeometrySink::~SkDWriteGeometrySink() { }

SK_STDMETHODIMP SkDWriteGeometrySink::QueryInterface(REFIID iid, void **object) {
    if (nullptr == object) {
        return E_INVALIDARG;
    }
    if (iid == __uuidof(IUnknown) || iid == __uuidof(IDWriteGeometrySink)) {
        *object = static_cast<IDWriteGeometrySink*>(this);
        this->AddRef();
        return S_OK;
    } else {
        *object = nullptr;
        return E_NOINTERFACE;
    }
}

SK_STDMETHODIMP_(ULONG) SkDWriteGeometrySink::AddRef(void) {
    return static_cast<ULONG>(InterlockedIncrement(&fRefCount));
}

SK_STDMETHODIMP_(ULONG) SkDWriteGeometrySink::Release(void) {
    ULONG res = static_cast<ULONG>(InterlockedDecrement(&fRefCount));
    if (0 == res) {
        delete this;
    }
    return res;
}

SK_STDMETHODIMP_(void) SkDWriteGeometrySink::SetFillMode(D2D1_FILL_MODE fillMode) {
    switch (fillMode) {
    case D2D1_FILL_MODE_ALTERNATE:
        fPath->setFillType(SkPathFillType::kEvenOdd);
        break;
    case D2D1_FILL_MODE_WINDING:
        fPath->setFillType(SkPathFillType::kWinding);
        break;
    default:
        SkDEBUGFAIL("Unknown D2D1_FILL_MODE.");
        break;
    }
}

SK_STDMETHODIMP_(void) SkDWriteGeometrySink::SetSegmentFlags(D2D1_PATH_SEGMENT vertexFlags) {
    if (vertexFlags == D2D1_PATH_SEGMENT_NONE || vertexFlags == D2D1_PATH_SEGMENT_FORCE_ROUND_LINE_JOIN) {
        SkDEBUGFAIL("Invalid D2D1_PATH_SEGMENT value.");
    }
}

SK_STDMETHODIMP_(void) SkDWriteGeometrySink::BeginFigure(D2D1_POINT_2F startPoint, D2D1_FIGURE_BEGIN figureBegin) {
    if (figureBegin == D2D1_FIGURE_BEGIN_HOLLOW) {
        SkDEBUGFAIL("Invalid D2D1_FIGURE_BEGIN value.");
    }
    fStarted = false;
    fCurrent = startPoint;
}

SK_STDMETHODIMP_(void) SkDWriteGeometrySink::AddLines(const D2D1_POINT_2F *points, UINT pointsCount) {
    for (const D2D1_POINT_2F *end = &points[pointsCount]; points < end; ++points) {
        if (this->currentIsNot(*points)) {
            this->goingTo(*points);
            fPath->lineTo(points->x, points->y);
        }
    }
}

static bool approximately_equal(float a, float b) {
    const SkFloatingPoint<float, 10> lhs(a), rhs(b);
    return lhs.AlmostEquals(rhs);
}

typedef struct {
    float x;
    float y;
} Cubic[4], Point;

static bool check_quadratic(const Cubic& cubic, Point& quadraticP1) {
    float dx10 = cubic[1].x - cubic[0].x;
    float dx23 = cubic[2].x - cubic[3].x;
    float midX = cubic[0].x + dx10 * 3 / 2;
    //NOTE: !approximately_equal(midX - cubic[3].x, dx23 * 3 / 2)
    //does not work as subnormals get in between the left side and 0.
    if (!approximately_equal(midX, (dx23 * 3 / 2) + cubic[3].x)) {
        return false;
    }
    float dy10 = cubic[1].y - cubic[0].y;
    float dy23 = cubic[2].y - cubic[3].y;
    float midY = cubic[0].y + dy10 * 3 / 2;
    if (!approximately_equal(midY, (dy23 * 3 / 2) + cubic[3].y)) {
        return false;
    }
    quadraticP1 = {midX, midY};
    return true;
}

SK_STDMETHODIMP_(void) SkDWriteGeometrySink::AddBeziers(const D2D1_BEZIER_SEGMENT *beziers, UINT beziersCount) {
    for (const D2D1_BEZIER_SEGMENT *end = &beziers[beziersCount]; beziers < end; ++beziers) {
        if (this->currentIsNot(beziers->point1) ||
            this->currentIsNot(beziers->point2) ||
            this->currentIsNot(beziers->point3))
        {
            Cubic cubic = { {        fCurrent.x,        fCurrent.y },
                            { beziers->point1.x, beziers->point1.y },
                            { beziers->point2.x, beziers->point2.y },
                            { beziers->point3.x, beziers->point3.y }, };
            this->goingTo(beziers->point3);
            Point quadraticP1;
            if (check_quadratic(cubic, quadraticP1)) {
                fPath->quadTo(    quadraticP1.x,     quadraticP1.y,
                              beziers->point3.x, beziers->point3.y);
            } else {
                fPath->cubicTo(beziers->point1.x, beziers->point1.y,
                               beziers->point2.x, beziers->point2.y,
                               beziers->point3.x, beziers->point3.y);
            }
        }
    }
}

SK_STDMETHODIMP_(void) SkDWriteGeometrySink::EndFigure(D2D1_FIGURE_END figureEnd) {
    if (fStarted) {
        fPath->close();
    }
}

SK_STDMETHODIMP SkDWriteGeometrySink::Close() {
    return S_OK;
}

HRESULT SkDWriteGeometrySink::Create(SkPath* path, IDWriteGeometrySink** geometryToPath) {
    *geometryToPath = new SkDWriteGeometrySink(path);
    return S_OK;
}

#endif//defined(SK_BUILD_FOR_WIN)
