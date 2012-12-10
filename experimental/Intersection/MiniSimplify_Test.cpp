#include "EdgeWalker_Test.h"
#include "Intersection_Tests.h"
#include "ShapeOps.h"

bool gShowOriginal = true;

struct curve {
    SkPath::Verb verb;
    SkPoint pts[4];
};

struct curve test1[] = {
{SkPath::kQuad_Verb, {{366.608826f, 151.196014f}, {378.803101f, 136.674606f}, {398.164948f, 136.674606f}}},
{SkPath::kLine_Verb, {{354.009216f, 208.816208f}, {393.291473f, 102.232819f}}},
{SkPath::kQuad_Verb, {{359.978058f, 136.581512f}, {378.315979f, 136.581512f}, {388.322723f, 149.613556f}}},
{SkPath::kQuad_Verb, {{364.390686f, 157.898193f}, {375.281769f, 136.674606f}, {396.039917f, 136.674606f}}},
{SkPath::kLine_Verb, {{396.039917f, 136.674606f}, {350, 120}}},
{SkPath::kDone_Verb}
};

struct curve test2[] = {
{SkPath::kQuad_Verb, {{366.608826f, 151.196014f}, {378.803101f, 136.674606f}, {398.164948f, 136.674606f}}},
{SkPath::kQuad_Verb, {{359.978058f, 136.581512f}, {378.315979f, 136.581512f}, {388.322723f, 149.613556f}}},
{SkPath::kQuad_Verb, {{364.390686f, 157.898193f}, {375.281769f, 136.674606f}, {396.039917f, 136.674606f}}},
{SkPath::kDone_Verb}
};

struct curve* testSet[] = {
    test2,
    test1
};

size_t testSet_count = sizeof(testSet) / sizeof(testSet[0]);

static void construct() {
    for (size_t idx = 0; idx < testSet_count; ++idx) {
        const curve* test = testSet[idx];
        SkPath path;
        bool pathComplete = false;
        bool first = true;
        do {
            if (first) {
                path.moveTo(test->pts[0].fX, test->pts[0].fY);
                first = false;
            } else if (test->verb != SkPath::kDone_Verb) {
                path.lineTo(test->pts[0].fX, test->pts[0].fY);
            }
            switch (test->verb) {
                case SkPath::kDone_Verb:
                    pathComplete = true;
                    break;
                case SkPath::kLine_Verb:
                    path.lineTo(test->pts[1].fX, test->pts[1].fY);
                    break;
                case SkPath::kQuad_Verb:
                    path.quadTo(test->pts[1].fX, test->pts[1].fY, test->pts[2].fX, test->pts[2].fY);
                    break;
                case SkPath::kCubic_Verb:
                    path.cubicTo(test->pts[1].fX, test->pts[1].fY, test->pts[2].fX, test->pts[2].fY, test->pts[3].fX, test->pts[3].fY);
                    break;
                default:
                    SkASSERT(0);
            }
            test++;
        } while (!pathComplete);
        path.close();
        if (gShowOriginal) {
            showPath(path, NULL);
            SkDebugf("simplified:\n");
        }
        testSimplifyx(path);
    }
}

static void (*tests[])() = {
    construct,
};

static const size_t testCount = sizeof(tests) / sizeof(tests[0]);

static void (*firstTest)() = 0;
static bool skipAll = false;

void MiniSimplify_Test() {
    if (skipAll) {
        return;
    }
    size_t index = 0;
    if (firstTest) {
        while (index < testCount && tests[index] != firstTest) {
            ++index;
        }
    }
    bool firstTestComplete = false;
    for ( ; index < testCount; ++index) {
        (*tests[index])();
        firstTestComplete = true;
    }
}
