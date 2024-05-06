/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#include "include/core/SkScalar.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/utils/SkParse.h"
#include "include/utils/SkParsePath.h"
#include "src/core/SkGeometry.h"

#include <cstdio>

enum class SkPathDirection;

static inline bool is_between(int c, int min, int max) {
    return (unsigned)(c - min) <= (unsigned)(max - min);
}

static inline bool is_ws(int c) {
    return is_between(c, 1, 32);
}

static inline bool is_digit(int c) {
    return is_between(c, '0', '9');
}

static inline bool is_sep(int c) {
    return is_ws(c) || c == ',';
}

static inline bool is_lower(int c) {
    return is_between(c, 'a', 'z');
}

static inline int to_upper(int c) {
    return c - 'a' + 'A';
}

static const char* skip_ws(const char str[]) {
    SkASSERT(str);
    while (is_ws(*str))
        str++;
    return str;
}

static const char* skip_sep(const char str[]) {
    if (!str) {
        return nullptr;
    }
    while (is_sep(*str))
        str++;
    return str;
}

// If unable to read count points from str into value, this will return nullptr
// to signal the failure. Otherwise, it will return the next offset to read from.
static const char* find_points(const char str[], SkPoint value[], int count,
                               bool isRelative, SkPoint* relative) {
    str = SkParse::FindScalars(str, &value[0].fX, count * 2);
    if (isRelative) {
        for (int index = 0; index < count; index++) {
            value[index].fX += relative->fX;
            value[index].fY += relative->fY;
        }
    }
    return str;
}

// If unable to read a scalar from str into value, this will return nullptr
// to signal the failure. Otherwise, it will return the next offset to read from.
static const char* find_scalar(const char str[], SkScalar* value,
                               bool isRelative, SkScalar relative) {
    str = SkParse::FindScalar(str, value);
    if (!str) {
        return nullptr;
    }
    if (isRelative) {
        *value += relative;
    }
    str = skip_sep(str);
    return str;
}

// https://www.w3.org/TR/SVG11/paths.html#PathDataBNF
//
// flag:
//    "0" | "1"
static const char* find_flag(const char str[], bool* value) {
    if (!str) {
        return nullptr;
    }
    if (str[0] != '1' && str[0] != '0') {
        return nullptr;
    }
    *value = str[0] != '0';
    str = skip_sep(str + 1);
    return str;
}

bool SkParsePath::FromSVGString(const char data[], SkPath* result) {
    // We will write all data to this local path and only write it
    // to result if the whole parsing succeeds.
    SkPath path;
    SkPoint first = {0, 0};
    SkPoint c = {0, 0};
    SkPoint lastc = {0, 0};
    // We will use find_points and find_scalar to read into these.
    // There might not be enough data to fill them, so to avoid
    // MSAN warnings about using uninitialized bytes, we initialize
    // them there.
    SkPoint points[3] = {};
    SkScalar scratch = 0;
    char op = '\0';
    char previousOp = '\0';
    bool relative = false;
    for (;;) {
        if (!data) {
            // Truncated data
            return false;
        }
        data = skip_ws(data);
        if (data[0] == '\0') {
            break;
        }
        char ch = data[0];
        if (is_digit(ch) || ch == '-' || ch == '+' || ch == '.') {
            if (op == '\0' || op == 'Z') {
                return false;
            }
        } else if (is_sep(ch)) {
            data = skip_sep(data);
        } else {
            op = ch;
            relative = false;
            if (is_lower(op)) {
                op = (char) to_upper(op);
                relative = true;
            }
            data++;
            data = skip_sep(data);
        }
        switch (op) {
            case 'M':  // Move
                data = find_points(data, points, 1, relative, &c);
                // find_points might have failed, so this might be the
                // previous point. However, data will be set to nullptr
                // if it failed, so we will check this at the top of the loop.
                path.moveTo(points[0]);
                previousOp = '\0';
                op = 'L';
                c = points[0];
                break;
            case 'L':  // Line
                data = find_points(data, points, 1, relative, &c);
                path.lineTo(points[0]);
                c = points[0];
                break;
            case 'H':  // Horizontal Line
                data = find_scalar(data, &scratch, relative, c.fX);
                // Similarly, if there wasn't a scalar to read, data will
                // be set to nullptr and this lineTo is bogus but will
                // be ultimately ignored when the next time through the loop
                // detects that and bails out.
                path.lineTo(scratch, c.fY);
                c.fX = scratch;
                break;
            case 'V':  // Vertical Line
                data = find_scalar(data, &scratch, relative, c.fY);
                path.lineTo(c.fX, scratch);
                c.fY = scratch;
                break;
            case 'C':  // Cubic Bezier Curve
                data = find_points(data, points, 3, relative, &c);
                goto cubicCommon;
            case 'S':  // Continued "Smooth" Cubic Bezier Curve
                data = find_points(data, &points[1], 2, relative, &c);
                points[0] = c;
                if (previousOp == 'C' || previousOp == 'S') {
                    points[0].fX -= lastc.fX - c.fX;
                    points[0].fY -= lastc.fY - c.fY;
                }
            cubicCommon:
                path.cubicTo(points[0], points[1], points[2]);
                lastc = points[1];
                c = points[2];
                break;
            case 'Q':  // Quadratic Bezier Curve
                data = find_points(data, points, 2, relative, &c);
                goto quadraticCommon;
            case 'T':  // Continued Quadratic Bezier Curve
                data = find_points(data, &points[1], 1, relative, &c);
                points[0] = c;
                if (previousOp == 'Q' || previousOp == 'T') {
                    points[0].fX -= lastc.fX - c.fX;
                    points[0].fY -= lastc.fY - c.fY;
                }
            quadraticCommon:
                path.quadTo(points[0], points[1]);
                lastc = points[0];
                c = points[1];
                break;
            case 'A': {  // Arc (Elliptical)
                SkPoint radii;
                SkScalar angle;
                bool largeArc, sweep;
                if ((data = find_points(data, &radii, 1, false, nullptr))
                        && (data = skip_sep(data))
                        && (data = find_scalar(data, &angle, false, 0))
                        && (data = skip_sep(data))
                        && (data = find_flag(data, &largeArc))
                        && (data = skip_sep(data))
                        && (data = find_flag(data, &sweep))
                        && (data = skip_sep(data))
                        && (data = find_points(data, &points[0], 1, relative, &c))) {
                    path.arcTo(radii, angle, (SkPath::ArcSize) largeArc,
                            (SkPathDirection) !sweep, points[0]);
                    path.getLastPt(&c);
                }
                } break;
            case 'Z':  // Close Path
                path.close();
                c = first;
                break;
            default:
                return false;
        }
        if (previousOp == 0) {
            first = c;
        }
        previousOp = op;
    }
    // we're good, go ahead and swap in the result
    result->swap(path);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

SkString SkParsePath::ToSVGString(const SkPath& path, PathEncoding encoding) {
    SkDynamicMemoryWStream  stream;

    SkPoint current_point{0,0};
    const auto rel_selector = encoding == PathEncoding::Relative;

    const auto append_command = [&](char cmd, const SkPoint pts[], size_t count) {
        // Use lower case cmds for relative encoding.
        cmd += 32 * rel_selector;
        stream.write(&cmd, 1);

        for (size_t i = 0; i < count; ++i) {
            const auto pt = pts[i] - current_point;
            if (i > 0) {
                stream.write(" ", 1);
            }
            stream.writeScalarAsText(pt.fX);
            stream.write(" ", 1);
            stream.writeScalarAsText(pt.fY);
        }

        SkASSERT(count > 0);
        // For relative encoding, track the current point (otherwise == origin).
        current_point = pts[count - 1] * rel_selector;
    };

    SkPath::Iter    iter(path, false);
    SkPoint         pts[4];

    for (;;) {
        switch (iter.next(pts)) {
            case SkPath::kConic_Verb: {
                const SkScalar tol = SK_Scalar1 / 1024; // how close to a quad
                SkAutoConicToQuads quadder;
                const SkPoint* quadPts = quadder.computeQuads(pts, iter.conicWeight(), tol);
                for (int i = 0; i < quadder.countQuads(); ++i) {
                    append_command('Q', &quadPts[i*2 + 1], 2);
                }
            } break;
           case SkPath::kMove_Verb:
                append_command('M', &pts[0], 1);
                break;
            case SkPath::kLine_Verb:
                append_command('L', &pts[1], 1);
                break;
            case SkPath::kQuad_Verb:
                append_command('Q', &pts[1], 2);
                break;
            case SkPath::kCubic_Verb:
                append_command('C', &pts[1], 3);
                break;
            case SkPath::kClose_Verb:
                stream.write("Z", 1);
                break;
            case SkPath::kDone_Verb: {
                SkString str;
                str.resize(stream.bytesWritten());
                stream.copyTo(str.data());
                return str;
            }
        }
    }
}
