#include <ctype.h>
#include "SkPath.h"
#include "SkParse.h"
#include "SkPoint.h"
#include "SkUtils.h"
#define QUADRATIC_APPROXIMATION 0

const char logoStr[] =
    "<path fill=\"#0081C6\""
    "d=\"M440.51,289.479c1.623,1.342,5.01,4.164,5.01,9.531c0,5.223-2.965,7.697-5.93,10.024"
    "c-0.918,0.916-1.977,1.907-1.977,3.462c0,1.551,1.059,2.397,1.834,3.035l2.545,1.973c3.105,2.613,5.928,5.016,5.928,9.889"
    "c0,6.635-6.426,13.341-18.566,13.341c-10.238,0-15.178-4.87-15.178-10.097c0-2.543,1.268-6.139,5.438-8.613"
    "c4.373-2.682,10.307-3.033,13.482-3.249c-0.99-1.271-2.119-2.61-2.119-4.798c0-1.199,0.355-1.907,0.707-2.754"
    "c-0.779,0.07-1.553,0.141-2.26,0.141c-7.482,0-11.719-5.579-11.719-11.082c0-3.247,1.484-6.851,4.518-9.461"
    "c4.025-3.318,8.824-3.883,12.639-3.883h14.541l-4.518,2.541H440.51z"
    "M435.494,320.826c-0.562-0.072-0.916-0.072-1.619-0.072"
    "c-0.637,0-4.451,0.143-7.416,1.132c-1.553,0.564-6.07,2.257-6.07,7.271c0,5.013,4.873,8.615,12.426,8.615"
    "c6.775,0,10.379-3.253,10.379-7.624C443.193,326.54,440.863,324.64,435.494,320.826z"
    "M437.543,307.412"
    "c1.623-1.627,1.764-3.883,1.764-5.154c0-5.083-3.035-12.99-8.893-12.99c-1.838,0-3.812,0.918-4.945,2.331"
    "c-1.199,1.483-1.551,3.387-1.551,5.225c0,4.729,2.754,12.565,8.826,12.565C434.508,309.389,436.41,308.543,437.543,307.412z\"/>"
    "<path fill=\"#FFD200\""
    "d=\"M396.064,319.696c-11.206,0-17.198-8.739-17.198-16.636c0-9.233,7.542-17.126,18.258-17.126"
    "c10.357,0,16.844,8.104,16.844,16.635C413.969,310.884,407.557,319.696,396.064,319.696z"
    "M404.873,313.987"
    "c1.695-2.257,2.119-5.074,2.119-7.826c0-6.202-2.961-18.042-11.701-18.042c-2.326,0-4.652,0.918-6.342,2.399"
    "c-2.749,2.465-3.245,5.566-3.245,8.599c0,6.977,3.454,18.463,11.984,18.463C400.436,317.58,403.256,316.242,404.873,313.987z\"/>"
    "<path fill=\"#ED174F\""
    "d=\"M357.861,319.696c-11.207,0-17.199-8.739-17.199-16.636c0-9.233,7.544-17.126,18.258-17.126"
    "c10.359,0,16.845,8.104,16.845,16.635C375.764,310.884,369.351,319.696,357.861,319.696z"
    "M366.671,313.987"
    "c1.693-2.257,2.116-5.074,2.116-7.826c0-6.202-2.961-18.042-11.701-18.042c-2.325,0-4.652,0.918-6.344,2.399"
    "c-2.749,2.465-3.241,5.566-3.241,8.599c0,6.977,3.452,18.463,11.983,18.463C362.234,317.58,365.053,316.242,366.671,313.987z\"/>"
    "<path fill=\"#0081C6\""
    "d=\"M335.278,318.591l-10.135,2.339c-4.111,0.638-7.795,1.204-11.69,1.204"
    "c-19.56,0-26.998-14.386-26.998-25.654c0-13.746,10.558-26.498,28.629-26.498c3.827,0,7.51,0.564,10.839,1.486"
    "c5.316,1.488,7.796,3.331,9.355,4.394l-5.883,5.599l-2.479,0.565l1.771-2.837c-2.408-2.336-6.805-6.658-15.164-6.658"
    "c-11.196,0-19.63,8.507-19.63,20.906c0,13.319,9.638,25.861,25.084,25.861c4.539,0,6.874-0.918,9-1.771v-11.407l-10.698,0.566"
    "l5.667-3.047h15.023l-1.841,1.77c-0.5,0.424-0.567,0.57-0.71,1.133c-0.073,0.64-0.141,2.695-0.141,3.403V318.591z\"/>"
    "<path fill=\"#49A942\""
    "d=\"M462.908,316.552c-2.342-0.214-2.832-0.638-2.832-3.401v-0.782v-39.327c0.014-0.153,0.025-0.31,0.041-0.457"
    "c0.283-2.479,0.992-2.903,3.189-4.182h-10.135l-5.316,2.552h5.418v0.032l-0.004-0.024v41.406v2.341"
    "c0,1.416-0.281,1.629-1.912,3.753H463.9l2.623-1.557C465.318,316.763,464.113,316.692,462.908,316.552z\"/>"
    "<path fill=\"#ED174F\""
    "d=\"M491.742,317.203c-0.771,0.422-1.547,0.916-2.318,1.268c-2.326,1.055-4.719,1.336-6.83,1.336"
    "c-2.25,0-5.77-0.143-9.361-2.744c-4.992-3.521-7.176-9.572-7.176-14.851c0-10.906,8.869-16.255,16.115-16.255"
    "c2.533,0,5.141,0.633,7.252,1.972c3.516,2.318,4.43,5.344,4.922,6.963l-16.535,6.688l-5.422,0.422"
    "c1.758,8.938,7.812,14.145,14.498,14.145c3.59,0,6.193-1.266,8.586-2.461L491.742,317.203z"
    "M485.129,296.229"
    "c1.336-0.493,2.039-0.914,2.039-1.899c0-2.812-3.166-6.053-6.967-6.053c-2.818,0-8.094,2.183-8.094,9.783"
    "c0,1.197,0.141,2.464,0.213,3.73L485.129,296.229z\"/>"
    "<path fill=\"#77787B\""
    "d=\"M498.535,286.439v4.643h-0.564v-4.643h-1.537v-0.482h3.637v0.482H498.535z\"/>"
    "<path fill=\"#77787B\""
    "d=\"M504.863,291.082v-4.687h-0.023l-1.432,4.687h-0.439l-1.443-4.687h-0.02v4.687h-0.512v-5.125h0.877"
    "l1.307,4.143h0.018l1.285-4.143h0.891v5.125H504.863z\"/>"
;

size_t logoStrLen = sizeof(logoStr);

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

static void showPathContour(SkPath::Iter& iter) {
    uint8_t verb;
    SkPoint pts[4];
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kMove_Verb:
                SkDebugf("path.moveTo(%1.9gf,%1.9gf);\n", pts[0].fX, pts[0].fY);
                continue;
            case SkPath::kLine_Verb:
                SkDebugf("path.lineTo(%1.9gf,%1.9gf);\n", pts[1].fX, pts[1].fY);
                break;
            case SkPath::kQuad_Verb:
                SkDebugf("path.quadTo(%1.9gf,%1.9gf, %1.9gf,%1.9gf);\n",
                    pts[1].fX, pts[1].fY, pts[2].fX, pts[2].fY);
                break;
            case SkPath::kCubic_Verb:
                SkDebugf("path.cubicTo(%1.9gf,%1.9gf, %1.9gf,%1.9gf, %1.9gf,%1.9gf);\n",
                    pts[1].fX, pts[1].fY, pts[2].fX, pts[2].fY, pts[3].fX, pts[3].fY);
                break;
            case SkPath::kClose_Verb:
                SkDebugf("path.close();\n");
                break;
            default:
                SkDEBUGFAIL("bad verb");
                return;
        }
    }
}

static void showPath(const SkPath& path) {
    SkPath::Iter iter(path, true);
    int rectCount = path.isRectContours() ? path.rectContours(NULL, NULL) : 0;
    if (rectCount > 0) {
        SkTDArray<SkRect> rects;
        SkTDArray<SkPath::Direction> directions;
        rects.setCount(rectCount);
        directions.setCount(rectCount);
        path.rectContours(rects.begin(), directions.begin());
        for (int contour = 0; contour < rectCount; ++contour) {
            const SkRect& rect = rects[contour];
            SkDebugf("path.addRect(%1.9g, %1.9g, %1.9g, %1.9g, %s);\n", rect.fLeft, rect.fTop,
                    rect.fRight, rect.fBottom, directions[contour] == SkPath::kCCW_Direction
                    ? "SkPath::kCCW_Direction" : "SkPath::kCW_Direction");
        }
        return;
    }
    iter.setPath(path, true);
    showPathContour(iter);
}

static const char* parsePath(const char* data) {
    SkPath fPath;
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
            if (op == '\0') {
                SkASSERT(0);
                return 0;
            }
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
                return 0;
        }
        if (previousOp == 0)
            f = c;
        previousOp = op;
    } while (data[0] != '"');
    showPath(fPath);
    return data;
}

const char pathPrefix[] = "<path fill=\"";

void parseSVG();
void parseSVG() {
    const char* data = logoStr;
    const char* dataEnd = logoStr + logoStrLen - 1;
    while (data < dataEnd) {
        SkASSERT(strncmp(data, pathPrefix, sizeof(pathPrefix) - 1) == 0);
        data += sizeof(pathPrefix) - 1;
        SkDebugf("paint.setColor(0xFF%c%c%c%c%c%c);\n", data[1], data[2], data[3], data[4],
            data[5], data[6]);
        data += 8;
        SkASSERT(strncmp(data, "d=\"", 3) == 0);
        data += 3;
        SkDebugf("path.reset();\n");
        data = parsePath(data);
        SkDebugf("canvas->drawPath(path, paint);\n");
        SkASSERT(strncmp(data, "\"/>", 3) == 0);
        data += 3;
    }
}
