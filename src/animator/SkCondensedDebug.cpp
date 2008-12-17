/* libs/graphics/animator/SkCondensedDebug.cpp
**
** Copyright 2006, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
*/

#include "SkTypes.h"
#ifndef SK_BUILD_FOR_UNIX
#ifdef SK_DEBUG
// This file was automatically generated.
// To change it, edit the file with the matching debug info.
// Then execute SkDisplayType::BuildCondensedInfo() to regenerate this file.

static const char gMathStrings[] = 
    "E\0"
    "LN10\0"
    "LN2\0"
    "LOG10E\0"
    "LOG2E\0"
    "PI\0"
    "SQRT1_2\0"
    "SQRT2\0"
    "abs\0"
    "acos\0"
    "asin\0"
    "atan\0"
    "atan2\0"
    "ceil\0"
    "cos\0"
    "exp\0"
    "floor\0"
    "log\0"
    "max\0"
    "min\0"
    "pow\0"
    "random\0"
    "round\0"
    "sin\0"
    "sqrt\0"
    "tan"
;

static const SkMemberInfo gMathInfo[] = {
    {0, -1, 67, 98},
    {2, -2, 67, 98},
    {7, -3, 67, 98},
    {11, -4, 67, 98},
    {18, -5, 67, 98},
    {24, -6, 67, 98},
    {27, -7, 67, 98},
    {35, -8, 67, 98},
    {41, -1, 66, 98},
    {45, -2, 66, 98},
    {50, -3, 66, 98},
    {55, -4, 66, 98},
    {60, -5, 66, 98},
    {66, -6, 66, 98},
    {71, -7, 66, 98},
    {75, -8, 66, 98},
    {79, -9, 66, 98},
    {85, -10, 66, 98},
    {89, -11, 66, 98},
    {93, -12, 66, 98},
    {97, -13, 66, 98},
    {101, -14, 66, 98},
    {108, -15, 66, 98},
    {114, -16, 66, 98},
    {118, -17, 66, 98},
    {123, -18, 66, 98}
};

static const char gAddStrings[] = 
    "inPlace\0"
    "offset\0"
    "use\0"
    "where"
;

static const SkMemberInfo gAddInfo[] = {
    {0, 16, 26, 1},
    {8, 20, 96, 1},
    {15, 24, 37, 1},
    {19, 28, 37, 1}
};

static const char gAddCircleStrings[] = 
    "\0"
    "radius\0"
    "x\0"
    "y"
;

static const SkMemberInfo gAddCircleInfo[] = {
    {0, 3, 18, 1},
    {1, 24, 98, 1},
    {8, 28, 98, 1},
    {10, 32, 98, 1}
};

static const char gUnknown1Strings[] = 
    "direction"
;

static const SkMemberInfo gUnknown1Info[] = {
    {0, 20, 75, 1}
};

static const char gAddOvalStrings[] = 
    ""
;

static const SkMemberInfo gAddOvalInfo[] = {
    {0, 6, 18, 5}
};

static const char gAddPathStrings[] = 
    "matrix\0"
    "path"
;

static const SkMemberInfo gAddPathInfo[] = {
    {0, 20, 65, 1},
    {7, 24, 74, 1}
};

static const char gAddRectangleStrings[] = 
    "\0"
    "bottom\0"
    "left\0"
    "right\0"
    "top"
;

static const SkMemberInfo gAddRectangleInfo[] = {
    {0, 3, 18, 1},
    {1, 36, 98, 1},
    {8, 24, 98, 1},
    {13, 32, 98, 1},
    {19, 28, 98, 1}
};

static const char gAddRoundRectStrings[] = 
    "\0"
    "rx\0"
    "ry"
;

static const SkMemberInfo gAddRoundRectInfo[] = {
    {0, 6, 18, 5},
    {1, 40, 98, 1},
    {4, 44, 98, 1}
};

static const char gUnknown2Strings[] = 
    "begin\0"
    "blend\0"
    "dur\0"
    "dynamic\0"
    "field\0"
    "formula\0"
    "from\0"
    "mirror\0"
    "repeat\0"
    "reset\0"
    "target\0"
    "to\0"
    "values"
;

static const SkMemberInfo gUnknown2Info[] = {
    {0, 16, 71, 1},
    {6, 20, 119, 98},
    {12, 36, 71, 1},
    {16, -1, 67, 26},
    {24, 40, 108, 2},
    {30, 48, 40, 2},
    {38, 56, 40, 2},
    {43, -2, 67, 26},
    {50, 64, 98, 1},
    {57, -3, 67, 26},
    {63, 68, 40, 2},
    {70, 76, 40, 2},
    {73, -4, 67, 40}
};

static const char gAnimateFieldStrings[] = 
    ""
;

static const SkMemberInfo gAnimateFieldInfo[] = {
    {0, 8, 18, 13}
};

static const char gApplyStrings[] = 
    "animator\0"
    "begin\0"
    "dontDraw\0"
    "dynamicScope\0"
    "interval\0"
    "mode\0"
    "pickup\0"
    "restore\0"
    "scope\0"
    "step\0"
    "steps\0"
    "time\0"
    "transition"
;

static const SkMemberInfo gApplyInfo[] = {
    {0, -1, 67, 10},
    {9, 16, 71, 1},
    {15, 20, 26, 1},
    {24, 24, 108, 2},
    {37, 32, 71, 1},
    {46, 36, 13, 1},
    {51, 40, 26, 1},
    {58, 44, 26, 1},
    {66, 48, 37, 1},
    {72, -2, 67, 96},
    {77, 52, 96, 1},
    {83, -3, 67, 71},
    {88, 56, 14, 1}
};

static const char gUnknown3Strings[] = 
    "x\0"
    "y"
;

static const SkMemberInfo gUnknown3Info[] = {
    {0, 48, 98, 1},
    {2, 52, 98, 1}
};

static const char gBitmapStrings[] = 
    "\0"
    "erase\0"
    "format\0"
    "height\0"
    "rowBytes\0"
    "width"
;

static const SkMemberInfo gDrawBitmapInfo[] = {
    {0, 11, 18, 2},
    {1, -1, 67, 15},
    {7, 56, 21, 1},
    {14, 60, 96, 1},
    {21, 64, 96, 1},
    {30, 68, 96, 1}
};

static const char gBitmapShaderStrings[] = 
    "\0"
    "filterType\0"
    "image"
;

static const SkMemberInfo gDrawBitmapShaderInfo[] = {
    {0, 67, 18, 2},
    {1, 28, 47, 1},
    {12, 32, 17, 1}
};

static const char gBlurStrings[] = 
    "blurStyle\0"
    "radius"
;

static const SkMemberInfo gDrawBlurInfo[] = {
    {0, 24, 63, 1},
    {10, 20, 98, 1}
};

static const char gBoundsStrings[] = 
    "\0"
    "inval"
;

static const SkMemberInfo gDisplayBoundsInfo[] = {
    {0, 58, 18, 7},
    {1, 44, 26, 1}
};

static const char gClipStrings[] = 
    "path\0"
    "rectangle"
;

static const SkMemberInfo gDrawClipInfo[] = {
    {0, 20, 74, 1},
    {5, 16, 91, 1}
};

static const char gColorStrings[] = 
    "alpha\0"
    "blue\0"
    "color\0"
    "green\0"
    "hue\0"
    "red\0"
    "saturation\0"
    "value"
;

static const SkMemberInfo gDrawColorInfo[] = {
    {0, -1, 67, 98},
    {6, -2, 67, 98},
    {11, 20, 15, 1},
    {17, -3, 67, 98},
    {23, -4, 67, 98},
    {27, -5, 67, 98},
    {31, -6, 67, 98},
    {42, -7, 67, 98}
};

static const char gCubicToStrings[] = 
    "x1\0"
    "x2\0"
    "x3\0"
    "y1\0"
    "y2\0"
    "y3"
;

static const SkMemberInfo gCubicToInfo[] = {
    {0, 20, 98, 1},
    {3, 28, 98, 1},
    {6, 36, 98, 1},
    {9, 24, 98, 1},
    {12, 32, 98, 1},
    {15, 40, 98, 1}
};

static const char gDashStrings[] = 
    "intervals\0"
    "phase"
;

static const SkMemberInfo gDashInfo[] = {
    {0, 20, 119, 98},
    {10, 36, 98, 1}
};

static const char gDataStrings[] = 
    "\0"
    "name"
;

static const SkMemberInfo gDataInfo[] = {
    {0, 33, 18, 3},
    {1, 32, 108, 2}
};

static const char gDiscreteStrings[] = 
    "deviation\0"
    "segLength"
;

static const SkMemberInfo gDiscreteInfo[] = {
    {0, 20, 98, 1},
    {10, 24, 98, 1}
};

static const char gDrawToStrings[] = 
    "drawOnce\0"
    "use"
;

static const SkMemberInfo gDrawToInfo[] = {
    {0, 72, 26, 1},
    {9, 76, 19, 1}
};

static const char gDumpStrings[] = 
    "displayList\0"
    "eventList\0"
    "events\0"
    "groups\0"
    "name\0"
    "posts"
;

static const SkMemberInfo gDumpInfo[] = {
    {0, 16, 26, 1},
    {12, 20, 26, 1},
    {22, 24, 26, 1},
    {29, 36, 26, 1},
    {36, 28, 108, 2},
    {41, 40, 26, 1}
};

static const char gEmbossStrings[] = 
    "ambient\0"
    "direction\0"
    "radius\0"
    "specular"
;

static const SkMemberInfo gDrawEmbossInfo[] = {
    {0, -1, 67, 98},
    {8, 20, 119, 98},
    {18, 36, 98, 1},
    {25, -2, 67, 98}
};

static const char gEventStrings[] = 
    "code\0"
    "disable\0"
    "key\0"
    "keys\0"
    "kind\0"
    "target\0"
    "x\0"
    "y"
;

static const SkMemberInfo gDisplayEventInfo[] = {
    {0, 16, 43, 1},
    {5, 20, 26, 1},
    {13, -1, 67, 108},
    {17, -2, 67, 108},
    {22, 24, 44, 1},
    {27, 28, 108, 2},
    {34, 36, 98, 1},
    {36, 40, 98, 1}
};

static const char gFromPathStrings[] = 
    "mode\0"
    "offset\0"
    "path"
;

static const SkMemberInfo gFromPathInfo[] = {
    {0, 20, 49, 1},
    {5, 24, 98, 1},
    {12, 28, 74, 1}
};

static const char gUnknown4Strings[] = 
    "\0"
    "offsets\0"
    "unitMapper"
;

static const SkMemberInfo gUnknown4Info[] = {
    {0, 67, 18, 2},
    {1, 28, 119, 98},
    {9, 44, 108, 2}
};

static const char gGStrings[] = 
    "condition\0"
    "enableCondition"
;

static const SkMemberInfo gGInfo[] = {
    {0, 16, 40, 2},
    {10, 24, 40, 2}
};

static const char gHitClearStrings[] = 
    "targets"
;

static const SkMemberInfo gHitClearInfo[] = {
    {0, 16, 119, 36}
};

static const char gHitTestStrings[] = 
    "bullets\0"
    "hits\0"
    "targets\0"
    "value"
;

static const SkMemberInfo gHitTestInfo[] = {
    {0, 16, 119, 36},
    {8, 32, 119, 96},
    {13, 48, 119, 36},
    {21, 64, 26, 1}
};

static const char gImageStrings[] = 
    "\0"
    "base64\0"
    "src"
;

static const SkMemberInfo gImageInfo[] = {
    {0, 11, 18, 2},
    {1, 56, 16, 2},
    {8, 64, 108, 2}
};

static const char gIncludeStrings[] = 
    "src"
;

static const SkMemberInfo gIncludeInfo[] = {
    {0, 16, 108, 2}
};

static const char gInputStrings[] = 
    "s32\0"
    "scalar\0"
    "string"
;

static const SkMemberInfo gInputInfo[] = {
    {0, 16, 96, 1},
    {4, 20, 98, 1},
    {11, 24, 108, 2}
};

static const char gLineStrings[] = 
    "x1\0"
    "x2\0"
    "y1\0"
    "y2"
;

static const SkMemberInfo gLineInfo[] = {
    {0, 24, 98, 1},
    {3, 28, 98, 1},
    {6, 32, 98, 1},
    {9, 36, 98, 1}
};

static const char gLineToStrings[] = 
    "x\0"
    "y"
;

static const SkMemberInfo gLineToInfo[] = {
    {0, 20, 98, 1},
    {2, 24, 98, 1}
};

static const char gLinearGradientStrings[] = 
    "\0"
    "points"
;

static const SkMemberInfo gLinearGradientInfo[] = {
    {0, 27, 18, 3},
    {1, 88, 77, 4}
};

static const char gMatrixStrings[] = 
    "matrix\0"
    "perspectX\0"
    "perspectY\0"
    "rotate\0"
    "scale\0"
    "scaleX\0"
    "scaleY\0"
    "skewX\0"
    "skewY\0"
    "translate\0"
    "translateX\0"
    "translateY"
;

static const SkMemberInfo gDrawMatrixInfo[] = {
    {0, 16, 119, 98},
    {7, -1, 67, 98},
    {17, -2, 67, 98},
    {27, -3, 67, 98},
    {34, -4, 67, 98},
    {40, -5, 67, 98},
    {47, -6, 67, 98},
    {54, -7, 67, 98},
    {60, -8, 67, 98},
    {66, -9, 67, 77},
    {76, -10, 67, 98},
    {87, -11, 67, 98}
};

static const char gMoveStrings[] = 
    ""
;

static const SkMemberInfo gMoveInfo[] = {
    {0, 1, 18, 4}
};

static const char gMoveToStrings[] = 
    "x\0"
    "y"
;

static const SkMemberInfo gMoveToInfo[] = {
    {0, 20, 98, 1},
    {2, 24, 98, 1}
};

static const char gMovieStrings[] = 
    "src"
;

static const SkMemberInfo gMovieInfo[] = {
    {0, 16, 108, 2}
};

static const char gOvalStrings[] = 
    ""
;

static const SkMemberInfo gOvalInfo[] = {
    {0, 58, 18, 7}
};

static const char gPaintStrings[] = 
    "antiAlias\0"
    "ascent\0"
    "color\0"
    "descent\0"
    "filterType\0"
    "linearText\0"
    "maskFilter\0"
    "measureText\0"
    "pathEffect\0"
    "shader\0"
    "strikeThru\0"
    "stroke\0"
    "strokeCap\0"
    "strokeJoin\0"
    "strokeMiter\0"
    "strokeWidth\0"
    "style\0"
    "textAlign\0"
    "textScaleX\0"
    "textSize\0"
    "textSkewX\0"
    "textTracking\0"
    "typeface\0"
    "underline\0"
    "xfermode"
;

static const SkMemberInfo gDrawPaintInfo[] = {
    {0, 16, 26, 1},
    {10, -1, 67, 98},
    {17, 20, 31, 1},
    {23, -2, 67, 98},
    {31, 24, 47, 1},
    {42, 28, 26, 1},
    {53, 32, 62, 1},
    {64, -1, 66, 98},
    {76, 36, 76, 1},
    {87, 40, 102, 1},
    {94, 44, 26, 1},
    {105, 48, 26, 1},
    {112, 52, 27, 1},
    {122, 56, 58, 1},
    {133, 60, 98, 1},
    {145, 64, 98, 1},
    {157, 68, 109, 1},
    {163, 72, 9, 1},
    {173, 76, 98, 1},
    {184, 80, 98, 1},
    {193, 84, 98, 1},
    {203, 88, 98, 1},
    {216, 92, 120, 1},
    {225, 96, 26, 1},
    {235, 100, 121, 1}
};

static const char gPathStrings[] = 
    "d\0"
    "fillType\0"
    "length"
;

static const SkMemberInfo gDrawPathInfo[] = {
    {0, 52, 108, 2},
    {2, -1, 67, 46},
    {11, -2, 67, 98}
};

static const char gUnknown5Strings[] = 
    "x\0"
    "y\0"
    "z"
;

static const SkMemberInfo gUnknown5Info[] = {
    {0, 0, 98, 1},
    {2, 4, 98, 1},
    {4, 8, 98, 1}
};

static const char gPointStrings[] = 
    "x\0"
    "y"
;

static const SkMemberInfo gDrawPointInfo[] = {
    {0, 16, 98, 1},
    {2, 20, 98, 1}
};

static const char gPolyToPolyStrings[] = 
    "destination\0"
    "source"
;

static const SkMemberInfo gPolyToPolyInfo[] = {
    {0, 24, 80, 1},
    {12, 20, 80, 1}
};

static const char gPolygonStrings[] = 
    ""
;

static const SkMemberInfo gPolygonInfo[] = {
    {0, 48, 18, 1}
};

static const char gPolylineStrings[] = 
    "points"
;

static const SkMemberInfo gPolylineInfo[] = {
    {0, 88, 119, 98}
};

static const char gPostStrings[] = 
    "delay\0"
    "initialized\0"
    "mode\0"
    "sink\0"
    "target\0"
    "type"
;

static const SkMemberInfo gPostInfo[] = {
    {0, 16, 71, 1},
    {6, 20, 26, 1},
    {18, 24, 45, 1},
    {23, -1, 67, 108},
    {28, -2, 67, 108},
    {35, -3, 67, 108}
};

static const char gQuadToStrings[] = 
    "x1\0"
    "x2\0"
    "y1\0"
    "y2"
;

static const SkMemberInfo gQuadToInfo[] = {
    {0, 20, 98, 1},
    {3, 28, 98, 1},
    {6, 24, 98, 1},
    {9, 32, 98, 1}
};

static const char gRCubicToStrings[] = 
    ""
;

static const SkMemberInfo gRCubicToInfo[] = {
    {0, 18, 18, 6}
};

static const char gRLineToStrings[] = 
    ""
;

static const SkMemberInfo gRLineToInfo[] = {
    {0, 35, 18, 2}
};

static const char gRMoveToStrings[] = 
    ""
;

static const SkMemberInfo gRMoveToInfo[] = {
    {0, 39, 18, 2}
};

static const char gRQuadToStrings[] = 
    ""
;

static const SkMemberInfo gRQuadToInfo[] = {
    {0, 50, 18, 4}
};

static const char gRadialGradientStrings[] = 
    "\0"
    "center\0"
    "radius"
;

static const SkMemberInfo gRadialGradientInfo[] = {
    {0, 27, 18, 3},
    {1, 88, 77, 2},
    {8, 96, 98, 1}
};

static const char gRandomStrings[] = 
    "blend\0"
    "max\0"
    "min\0"
    "random\0"
    "seed"
;

static const SkMemberInfo gDisplayRandomInfo[] = {
    {0, 16, 98, 1},
    {6, 24, 98, 1},
    {10, 20, 98, 1},
    {14, 1, 67, 98},
    {21, -2, 67, 96}
};

static const char gRectToRectStrings[] = 
    "destination\0"
    "source"
;

static const SkMemberInfo gRectToRectInfo[] = {
    {0, 24, 91, 1},
    {12, 20, 91, 1}
};

static const char gRectangleStrings[] = 
    "bottom\0"
    "height\0"
    "left\0"
    "needsRedraw\0"
    "right\0"
    "top\0"
    "width"
;

static const SkMemberInfo gRectangleInfo[] = {
    {0, 36, 98, 1},
    {7, -1, 67, 98},
    {14, 24, 98, 1},
    {19, -2, 67, 26},
    {31, 32, 98, 1},
    {37, 28, 98, 1},
    {41, -3, 67, 98}
};

static const char gRemoveStrings[] = 
    "offset\0"
    "where"
;

static const SkMemberInfo gRemoveInfo[] = {
    {0, 20, 96, 1},
    {7, 28, 37, 1}
};

static const char gReplaceStrings[] = 
    ""
;

static const SkMemberInfo gReplaceInfo[] = {
    {0, 1, 18, 4}
};

static const char gRotateStrings[] = 
    "center\0"
    "degrees"
;

static const SkMemberInfo gRotateInfo[] = {
    {0, 24, 77, 2},
    {7, 20, 98, 1}
};

static const char gRoundRectStrings[] = 
    "\0"
    "rx\0"
    "ry"
;

static const SkMemberInfo gRoundRectInfo[] = {
    {0, 58, 18, 7},
    {1, 44, 98, 1},
    {4, 48, 98, 1}
};

static const char gS32Strings[] = 
    "value"
;

static const SkMemberInfo gS32Info[] = {
    {0, 16, 96, 1}
};

static const char gScalarStrings[] = 
    "value"
;

static const SkMemberInfo gScalarInfo[] = {
    {0, 16, 98, 1}
};

static const char gScaleStrings[] = 
    "center\0"
    "x\0"
    "y"
;

static const SkMemberInfo gScaleInfo[] = {
    {0, 28, 77, 2},
    {7, 20, 98, 1},
    {9, 24, 98, 1}
};

static const char gSetStrings[] = 
    "begin\0"
    "dur\0"
    "dynamic\0"
    "field\0"
    "formula\0"
    "reset\0"
    "target\0"
    "to"
;

static const SkMemberInfo gSetInfo[] = {
    {0, 16, 71, 1},
    {6, 36, 71, 1},
    {10, -1, 67, 26},
    {18, 40, 108, 2},
    {24, 48, 40, 2},
    {32, -3, 67, 26},
    {38, 68, 40, 2},
    {45, 76, 40, 2}
};

static const char gShaderStrings[] = 
    "matrix\0"
    "tileMode"
;

static const SkMemberInfo gShaderInfo[] = {
    {0, 20, 65, 1},
    {7, 24, 116, 1}
};

static const char gSkewStrings[] = 
    "center\0"
    "x\0"
    "y"
;

static const SkMemberInfo gSkewInfo[] = {
    {0, 28, 77, 2},
    {7, 20, 98, 1},
    {9, 24, 98, 1}
};

static const char g3D_CameraStrings[] = 
    "axis\0"
    "hackHeight\0"
    "hackWidth\0"
    "location\0"
    "observer\0"
    "patch\0"
    "zenith"
;

static const SkMemberInfo g3D_CameraInfo[] = {
    {0, 36, 106, 3},
    {5, 20, 98, 1},
    {16, 16, 98, 1},
    {26, 24, 106, 3},
    {35, 60, 106, 3},
    {44, 108, 105, 1},
    {50, 48, 106, 3}
};

static const char g3D_PatchStrings[] = 
    "origin\0"
    "rotateDegrees\0"
    "u\0"
    "v"
;

static const SkMemberInfo g3D_PatchInfo[] = {
    {0, 40, 106, 3},
    {7, -1, 66, 98},
    {21, 16, 106, 3},
    {23, 28, 106, 3}
};

static const char gUnknown6Strings[] = 
    "x\0"
    "y\0"
    "z"
;

static const SkMemberInfo gUnknown6Info[] = {
    {0, 0, 98, 1},
    {2, 4, 98, 1},
    {4, 8, 98, 1}
};

static const char gSnapshotStrings[] = 
    "filename\0"
    "quality\0"
    "sequence\0"
    "type"
;

static const SkMemberInfo gSnapshotInfo[] = {
    {0, 16, 108, 2},
    {9, 24, 98, 1},
    {17, 28, 26, 1},
    {26, 32, 20, 1}
};

static const char gStringStrings[] = 
    "length\0"
    "slice\0"
    "value"
;

static const SkMemberInfo gStringInfo[] = {
    {0, -1, 67, 96},
    {7, -1, 66, 108},
    {13, 16, 108, 2}
};

static const char gTextStrings[] = 
    "length\0"
    "text\0"
    "x\0"
    "y"
;

static const SkMemberInfo gTextInfo[] = {
    {0, -1, 67, 96},
    {7, 24, 108, 2},
    {12, 32, 98, 1},
    {14, 36, 98, 1}
};

static const char gTextBoxStrings[] = 
    "\0"
    "mode\0"
    "spacingAdd\0"
    "spacingAlign\0"
    "spacingMul\0"
    "text"
;

static const SkMemberInfo gTextBoxInfo[] = {
    {0, 58, 18, 7},
    {1, 60, 113, 1},
    {6, 56, 98, 1},
    {17, 64, 112, 1},
    {30, 52, 98, 1},
    {41, 44, 108, 2}
};

static const char gTextOnPathStrings[] = 
    "offset\0"
    "path\0"
    "text"
;

static const SkMemberInfo gTextOnPathInfo[] = {
    {0, 24, 98, 1},
    {7, 28, 74, 1},
    {12, 32, 110, 1}
};

static const char gTextToPathStrings[] = 
    "path\0"
    "text"
;

static const SkMemberInfo gTextToPathInfo[] = {
    {0, 16, 74, 1},
    {5, 20, 110, 1}
};

static const char gTranslateStrings[] = 
    "x\0"
    "y"
;

static const SkMemberInfo gTranslateInfo[] = {
    {0, 20, 98, 1},
    {2, 24, 98, 1}
};

static const char gTypedArrayStrings[] = 
    "length\0"
    "values"
;

static const SkMemberInfo gTypedArrayInfo[] = {
    {0, -1, 67, 96},
    {7, 16, 119, 0}
};

static const char gTypefaceStrings[] = 
    "fontName"
;

static const SkMemberInfo gTypefaceInfo[] = {
    {0, 20, 108, 2}
};

static const SkMemberInfo* const gInfoTables[] = {
    gMathInfo,
    gAddInfo,
    gAddCircleInfo,
    gUnknown1Info,
    gAddOvalInfo,
    gAddPathInfo,
    gAddRectangleInfo,
    gAddRoundRectInfo,
    gUnknown2Info,
    gAnimateFieldInfo,
    gApplyInfo,
    gUnknown3Info,
    gDrawBitmapInfo,
    gDrawBitmapShaderInfo,
    gDrawBlurInfo,
    gDisplayBoundsInfo,
    gDrawClipInfo,
    gDrawColorInfo,
    gCubicToInfo,
    gDashInfo,
    gDataInfo,
    gDiscreteInfo,
    gDrawToInfo,
    gDumpInfo,
    gDrawEmbossInfo,
    gDisplayEventInfo,
    gFromPathInfo,
    gUnknown4Info,
    gGInfo,
    gHitClearInfo,
    gHitTestInfo,
    gImageInfo,
    gIncludeInfo,
    gInputInfo,
    gLineInfo,
    gLineToInfo,
    gLinearGradientInfo,
    gDrawMatrixInfo,
    gMoveInfo,
    gMoveToInfo,
    gMovieInfo,
    gOvalInfo,
    gDrawPaintInfo,
    gDrawPathInfo,
    gUnknown5Info,
    gDrawPointInfo,
    gPolyToPolyInfo,
    gPolygonInfo,
    gPolylineInfo,
    gPostInfo,
    gQuadToInfo,
    gRCubicToInfo,
    gRLineToInfo,
    gRMoveToInfo,
    gRQuadToInfo,
    gRadialGradientInfo,
    gDisplayRandomInfo,
    gRectToRectInfo,
    gRectangleInfo,
    gRemoveInfo,
    gReplaceInfo,
    gRotateInfo,
    gRoundRectInfo,
    gS32Info,
    gScalarInfo,
    gScaleInfo,
    gSetInfo,
    gShaderInfo,
    gSkewInfo,
    g3D_CameraInfo,
    g3D_PatchInfo,
    gUnknown6Info,
    gSnapshotInfo,
    gStringInfo,
    gTextInfo,
    gTextBoxInfo,
    gTextOnPathInfo,
    gTextToPathInfo,
    gTranslateInfo,
    gTypedArrayInfo,
    gTypefaceInfo,
};

static const unsigned char gInfoCounts[] = {
    26,4,4,1,1,2,5,3,13,1,13,2,6,3,2,2,2,8,6,
    2,2,2,2,6,4,8,3,3,2,1,4,3,1,3,4,2,2,12,1,
    2,1,1,25,3,3,2,2,1,1,6,4,1,1,1,1,3,5,2,7,
    2,1,2,3,1,1,3,8,2,3,7,4,3,4,3,4,6,3,2,2,
    2,1
};

static const unsigned char gTypeIDs[] = {
    1, // Math
    2, // Add
    3, // AddCircle
    4, // Unknown1
    5, // AddOval
    6, // AddPath
    7, // AddRectangle
    8, // AddRoundRect
    10, // Unknown2
    11, // AnimateField
    12, // Apply
    17, // Unknown3
    19, // Bitmap
    22, // BitmapShader
    23, // Blur
    25, // Bounds
    29, // Clip
    31, // Color
    32, // CubicTo
    33, // Dash
    34, // Data
    35, // Discrete
    38, // DrawTo
    39, // Dump
    41, // Emboss
    42, // Event
    48, // FromPath
    51, // Unknown4
    52, // G
    53, // HitClear
    54, // HitTest
    55, // Image
    56, // Include
    57, // Input
    59, // Line
    60, // LineTo
    61, // LinearGradient
    65, // Matrix
    68, // Move
    69, // MoveTo
    70, // Movie
    72, // Oval
    73, // Paint
    74, // Path
    77, // Unknown5
    78, // Point
    79, // PolyToPoly
    80, // Polygon
    81, // Polyline
    82, // Post
    83, // QuadTo
    84, // RCubicTo
    85, // RLineTo
    86, // RMoveTo
    87, // RQuadTo
    88, // RadialGradient
    89, // Random
    90, // RectToRect
    91, // Rectangle
    92, // Remove
    93, // Replace
    94, // Rotate
    95, // RoundRect
    96, // S32
    98, // Scalar
    99, // Scale
    101, // Set
    102, // Shader
    103, // Skew
    104, // 3D_Camera
    105, // 3D_Patch
    106, // Unknown6
    107, // Snapshot
    108, // String
    110, // Text
    111, // TextBox
    114, // TextOnPath
    115, // TextToPath
    117, // Translate
    119, // TypedArray
    120, // Typeface
    
};

static const int kTypeIDs = 81;

static const char* const gInfoNames[] = {
    gMathStrings,
    gAddStrings,
    gAddCircleStrings,
    gUnknown1Strings,
    gAddOvalStrings,
    gAddPathStrings,
    gAddRectangleStrings,
    gAddRoundRectStrings,
    gUnknown2Strings,
    gAnimateFieldStrings,
    gApplyStrings,
    gUnknown3Strings,
    gBitmapStrings,
    gBitmapShaderStrings,
    gBlurStrings,
    gBoundsStrings,
    gClipStrings,
    gColorStrings,
    gCubicToStrings,
    gDashStrings,
    gDataStrings,
    gDiscreteStrings,
    gDrawToStrings,
    gDumpStrings,
    gEmbossStrings,
    gEventStrings,
    gFromPathStrings,
    gUnknown4Strings,
    gGStrings,
    gHitClearStrings,
    gHitTestStrings,
    gImageStrings,
    gIncludeStrings,
    gInputStrings,
    gLineStrings,
    gLineToStrings,
    gLinearGradientStrings,
    gMatrixStrings,
    gMoveStrings,
    gMoveToStrings,
    gMovieStrings,
    gOvalStrings,
    gPaintStrings,
    gPathStrings,
    gUnknown5Strings,
    gPointStrings,
    gPolyToPolyStrings,
    gPolygonStrings,
    gPolylineStrings,
    gPostStrings,
    gQuadToStrings,
    gRCubicToStrings,
    gRLineToStrings,
    gRMoveToStrings,
    gRQuadToStrings,
    gRadialGradientStrings,
    gRandomStrings,
    gRectToRectStrings,
    gRectangleStrings,
    gRemoveStrings,
    gReplaceStrings,
    gRotateStrings,
    gRoundRectStrings,
    gS32Strings,
    gScalarStrings,
    gScaleStrings,
    gSetStrings,
    gShaderStrings,
    gSkewStrings,
    g3D_CameraStrings,
    g3D_PatchStrings,
    gUnknown6Strings,
    gSnapshotStrings,
    gStringStrings,
    gTextStrings,
    gTextBoxStrings,
    gTextOnPathStrings,
    gTextToPathStrings,
    gTranslateStrings,
    gTypedArrayStrings,
    gTypefaceStrings
};

#endif
#endif


