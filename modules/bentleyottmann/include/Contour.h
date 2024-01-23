// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#ifndef Contour_DEFINED
#define Contour_DEFINED

#include "include/core/SkRect.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkSpan_impl.h"

#include <limits.h>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <vector>

class SkPath;
namespace myers { class Segment; }
struct SkPoint;

namespace contour {
struct Point {
    int32_t x;
    int32_t y;
};

class Contour {
public:
    SkSpan<const Point> points;
    SkIRect bounds;
};

class Contours {
    class Iterator {
    public:
        using value_type = Contour;
        using difference_type = ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type;
        using iterator_category = std::input_iterator_tag;
        Iterator(const Contours& contours, size_t index)
                : fContours{contours}
                , fIndex{index} { }
        Iterator(const Iterator& that) : Iterator{ that.fContours, that.fIndex } { }
        Iterator& operator++() { ++fIndex; return *this; }
        Iterator operator++(int) { Iterator tmp(*this); operator++(); return tmp; }
        bool operator==(const Iterator& rhs) const { return fIndex == rhs.fIndex; }
        bool operator!=(const Iterator& rhs) const { return fIndex != rhs.fIndex; }
        value_type operator*() { return fContours[fIndex]; }
        friend difference_type operator-(Iterator lhs, Iterator rhs) {
            return lhs.fIndex - rhs.fIndex;
        }

    private:
        const Contours& fContours;
        size_t fIndex = 0;
    };
public:
    static constexpr double kScaleFactor = 1024;
    static Contours Make(SkPath path);

    Contour operator[](size_t i) const {
        SkASSERT(i < fContours.size());
        auto& [bounds, end] = fContours[i];
        int32_t start = i == 0 ? 0 : fContours[i-1].end;
        SkSpan<const Point> points{&fPoints[start], end - start};
        return {points, bounds};
    }

    Iterator begin() const {
        return Iterator{*this, 0};
    }

    Iterator end() const {
        return Iterator{*this, fContours.size()};
    }

    size_t size() const {
        return fContours.size();
    }

    bool empty() const {
        return fContours.empty();
    }

    std::vector<myers::Segment> segments() const;

private:
    static constexpr SkIRect kEmptyRect = SkIRect::MakeLTRB(INT_MAX, INT_MAX, INT_MIN, INT_MIN);
    struct CompactContour {
        SkIRect bounds;
        int32_t end;
    };

    static Point RoundSkPoint(SkPoint p);
    bool currentContourIsEmpty() const;
    void addPointToCurrentContour(SkPoint p);
    void moveToStartOfContour(SkPoint p);
    void closeContourIfNeeded();

    Point fContourStart;
    SkIRect fContourBounds = kEmptyRect;

    std::vector<Point> fPoints;
    std::vector<CompactContour> fContours;
};
}  // namespace contour

#endif  // Contour_DEFINED
