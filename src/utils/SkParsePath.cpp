
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkParse.h"
#include "SkParsePath.h"

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
    SkASSERT(str);
    while (is_sep(*str))
        str++;
    return str;
}

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

static const char* find_scalar(const char str[], SkScalar* value,
                               bool isRelative, SkScalar relative) {
    str = SkParse::FindScalar(str, value);
    if (isRelative) {
        *value += relative;
    }
    return str;
}

bool SkParsePath::FromSVGString(const char data[], SkPath* result) {
    SkPath path;
    SkPoint f = {0, 0};
    SkPoint c = {0, 0};
    SkPoint lastc = {0, 0};
    SkPoint points[3];
    char op = '\0';
    char previousOp = '\0';
    bool relative = false;
    for (;;) {
        data = skip_ws(data);
        if (data[0] == '\0') {
            break;
        }
        char ch = data[0];
        if (is_digit(ch) || ch == '-' || ch == '+') {
            if (op == '\0') {
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
            case 'M':
                data = find_points(data, points, 1, relative, &c);
                path.moveTo(points[0]);
                op = 'L';
                c = points[0];
                break;
            case 'L':
                data = find_points(data, points, 1, relative, &c);
                path.lineTo(points[0]);
                c = points[0];
                break;
            case 'H': {
                SkScalar x;
                data = find_scalar(data, &x, relative, c.fX);
                path.lineTo(x, c.fY);
                c.fX = x;
            } break;
            case 'V': {
                SkScalar y;
                data = find_scalar(data, &y, relative, c.fY);
                path.lineTo(c.fX, y);
                c.fY = y;
            } break;
            case 'C':
                data = find_points(data, points, 3, relative, &c);
                goto cubicCommon;
            case 'S':
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
            case 'T':
                data = find_points(data, &points[1], 1, relative, &c);
                points[0] = points[1];
                if (previousOp == 'Q' || previousOp == 'T') {
                    points[0].fX = c.fX * 2 - lastc.fX;
                    points[0].fY = c.fY * 2 - lastc.fY;
                }
            quadraticCommon:
                path.quadTo(points[0], points[1]);
                lastc = points[0];
                c = points[1];
                break;
            case 'Z':
                path.close();
#if 0   // !!! still a bug?
                if (fPath.isEmpty() && (f.fX != 0 || f.fY != 0)) {
                    c.fX -= SkScalar.Epsilon;   // !!! enough?
                    fPath.moveTo(c);
                    fPath.lineTo(f);
                    fPath.close();
                }
#endif
                c = f;
                op = '\0';
                break;
            case '~': {
                SkPoint args[2];
                data = find_points(data, args, 2, false, NULL);
                path.moveTo(args[0].fX, args[0].fY);
                path.lineTo(args[1].fX, args[1].fY);
            } break;
            default:
                return false;
        }
        if (previousOp == 0) {
            f = c;
        }
        previousOp = op;
    }
    // we're good, go ahead and swap in the result
    result->swap(path);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

#include "SkGeometry.h"
#include "SkString.h"
#include "SkStream.h"

static void write_scalar(SkWStream* stream, SkScalar value) {
    char buffer[64];
#ifdef SK_BUILD_FOR_WIN32
    int len = _snprintf(buffer, sizeof(buffer), "%g", value);
#else
    int len = snprintf(buffer, sizeof(buffer), "%g", value);
#endif
    char* stop = buffer + len;
    stream->write(buffer, stop - buffer);
}

static void append_scalars(SkWStream* stream, char verb, const SkScalar data[],
                           int count) {
    stream->write(&verb, 1);
    write_scalar(stream, data[0]);
    for (int i = 1; i < count; i++) {
        stream->write(" ", 1);
        write_scalar(stream, data[i]);
    }
}

void SkParsePath::ToSVGString(const SkPath& path, SkString* str) {
    SkDynamicMemoryWStream  stream;

    SkPath::Iter    iter(path, false);
    SkPoint         pts[4];

    for (;;) {
        switch (iter.next(pts, false)) {
            case SkPath::kConic_Verb: {
                const SkScalar tol = SK_Scalar1 / 1024; // how close to a quad
                SkAutoConicToQuads quadder;
                const SkPoint* quadPts = quadder.computeQuads(pts, iter.conicWeight(), tol);
                for (int i = 0; i < quadder.countQuads(); ++i) {
                    append_scalars(&stream, 'Q', &quadPts[i*2 + 1].fX, 4);
                }
            } break;
           case SkPath::kMove_Verb:
                append_scalars(&stream, 'M', &pts[0].fX, 2);
                break;
            case SkPath::kLine_Verb:
                append_scalars(&stream, 'L', &pts[1].fX, 2);
                break;
            case SkPath::kQuad_Verb:
                append_scalars(&stream, 'Q', &pts[1].fX, 4);
                break;
            case SkPath::kCubic_Verb:
                append_scalars(&stream, 'C', &pts[1].fX, 6);
                break;
            case SkPath::kClose_Verb:
                stream.write("Z", 1);
                break;
            case SkPath::kDone_Verb:
                str->resize(stream.getOffset());
                stream.copyTo(str->writable_str());
            return;
        }
    }
}
