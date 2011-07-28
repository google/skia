
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include <ctype.h>
#include "SkDrawPath.h"
#include "SkParse.h"
#include "SkPoint.h"
#include "SkUtils.h"
#define QUADRATIC_APPROXIMATION 1

#if QUADRATIC_APPROXIMATION
////////////////////////////////////////////////////////////////////////////////////
//functions to approximate a cubic using two quadratics

//      midPt sets the first argument to be the midpoint of the other two
//      it is used by quadApprox
static inline void midPt(SkPoint& dest,const SkPoint& a,const SkPoint& b)
{
    dest.set(SkScalarAve(a.fX, b.fX),SkScalarAve(a.fY, b.fY));
}
//      quadApprox - makes an approximation, which we hope is faster
static void quadApprox(SkPath &fPath, const SkPoint &p0, const SkPoint &p1, const SkPoint &p2)
{
    //divide the cubic up into two cubics, then convert them into quadratics
    //define our points
    SkPoint c,j,k,l,m,n,o,p,q, mid;
    fPath.getLastPt(&c);
    midPt(j, p0, c);
    midPt(k, p0, p1);
    midPt(l, p1, p2);
    midPt(o, j, k);
    midPt(p, k, l);
    midPt(q, o, p);
    //compute the first half
    m.set(SkScalarHalf(3*j.fX - c.fX), SkScalarHalf(3*j.fY - c.fY));
    n.set(SkScalarHalf(3*o.fX -q.fX), SkScalarHalf(3*o.fY - q.fY));
    midPt(mid,m,n);
    fPath.quadTo(mid,q);
    c = q;
    //compute the second half
    m.set(SkScalarHalf(3*p.fX - c.fX), SkScalarHalf(3*p.fY - c.fY));
    n.set(SkScalarHalf(3*l.fX -p2.fX),SkScalarHalf(3*l.fY -p2.fY));
    midPt(mid,m,n);
    fPath.quadTo(mid,p2);
}
#endif


static inline bool is_between(int c, int min, int max)
{
    return (unsigned)(c - min) <= (unsigned)(max - min);
}

static inline bool is_ws(int c)
{
    return is_between(c, 1, 32);
}

static inline bool is_digit(int c)
{
    return is_between(c, '0', '9');
}

static inline bool is_sep(int c)
{
    return is_ws(c) || c == ',';
}

static const char* skip_ws(const char str[])
{
    SkASSERT(str);
    while (is_ws(*str))
        str++;
    return str;
}

static const char* skip_sep(const char str[])
{
    SkASSERT(str);
    while (is_sep(*str))
        str++;
    return str;
}

static const char* find_points(const char str[], SkPoint value[], int count,
     bool isRelative, SkPoint* relative)
{
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
    bool isRelative, SkScalar relative)
{
    str = SkParse::FindScalar(str, value);
    if (isRelative)
        *value += relative;
    return str;
}

void SkDrawPath::parseSVG() {
    fPath.reset();
    const char* data = d.c_str();
    SkPoint f = {0, 0};
    SkPoint c = {0, 0};
    SkPoint lastc = {0, 0};
    SkPoint points[3];
    char op = '\0';
    char previousOp = '\0';
    bool relative = false;
    do {
        data = skip_ws(data);
        if (data[0] == '\0')
            break;
        char ch = data[0];
        if (is_digit(ch) || ch == '-' || ch == '+') {
            if (op == '\0')
                return;
        }
        else {
            op = ch;
            relative = false;
            if (islower(op)) {
                op = (char) toupper(op);
                relative = true;
            }
            data++;
            data = skip_sep(data);
        }
        switch (op) {
            case 'M':
                data = find_points(data, points, 1, relative, &c);
                fPath.moveTo(points[0]);
                op = 'L';
                c = points[0];
                break;
            case 'L': 
                data = find_points(data, points, 1, relative, &c);
                fPath.lineTo(points[0]);
                c = points[0];
                break;
            case 'H': {
                SkScalar x;
                data = find_scalar(data, &x, relative, c.fX);
                fPath.lineTo(x, c.fY);
                c.fX = x;
            }
                break;
            case 'V': {
                SkScalar y;
                data = find_scalar(data, &y, relative, c.fY);
                fPath.lineTo(c.fX, y);
                c.fY = y;
            }
                break;
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
    //          if (data[0] == '\0')
    //              return;
#if QUADRATIC_APPROXIMATION
                    quadApprox(fPath, points[0], points[1], points[2]);
#else   //this way just does a boring, slow old cubic
                    fPath.cubicTo(points[0], points[1], points[2]);
#endif
        //if we are using the quadApprox, lastc is what it would have been if we had used
        //cubicTo
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
                fPath.quadTo(points[0], points[1]);
                lastc = points[0];
                c = points[1];
                break;
            case 'Z':
                fPath.close();
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
                fPath.moveTo(args[0].fX, args[0].fY);
                fPath.lineTo(args[1].fX, args[1].fY);
            }
                break;
            default:
                SkASSERT(0);
                return;
        }
        if (previousOp == 0)
            f = c;
        previousOp = op;
    } while (data[0] > 0);
}

