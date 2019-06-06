/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <algorithm>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "generator.h"
#include "skia_api_idl.h"

using namespace std;

namespace generator {

Interface c1 = {
        1,
        "Sk_Paint",
        {
                {1, "set_Color", eNormal, {{"", eVoid}, {"color", eInt}}},
                {2, "set_AntiAlias", eNormal, {{"", eVoid}, {"a", eBool}}},
                {3, "set_Stroke", eNormal, {{"", eVoid}, {"doStroke", eBool}}},
                {4, "set_Stroke_Width", eNormal, {{"", eVoid}, {"width", eFloat}}},
                {5, "new", eCtor, {{"", eInterfaceP, eOutputCustomDealloc, 1, 1, 6}}},
                {6, "delete", eNormal, {{"", eVoid}, {"", eInterfaceP, eInput, 1}}, false},
                {7, "is_AntiAlias", eNormal, {{"", eBool}}},
                {8, "get_Color", eNormal, {{"", eInt}}},
                {9, "is_Stroke", eNormal, {{"", eBool}}},
                {10, "get_Stroke_Width", eNormal, {{"", eFloat}}},
                {11, "get_Stroke_Miter", eNormal, {{"", eFloat}}},
                {12, "set_Stroke_Miter", eNormal, {{"", eVoid}, {"miter", eFloat}}},
                {13, "get_Stroke_Cap", eNormal, {{"", eEnum, eOutputNoDealloc, 4}}},
                {14, "set_Stroke_Cap", eNormal, {{"", eVoid}, {"cap", eEnum, eInput, 4}}},
                {15, "get_Stroke_Join", eNormal, {{"", eEnum, eOutputNoDealloc, 3}}},
                {16, "set_Stroke_Join", eNormal, {{"", eVoid}, {"cap", eEnum, eInput, 3}}},
                {17, "set_Shader", eNormal, {{"", eVoid}, {"shader", eInterfaceP, eInput, 12}}},
                {18, "set_MaskFilter", eNormal, {{"", eVoid}, {"filter", eInterfaceP, eInput, 13}}},
                {19, "set_Xfermode_Mode", eNormal, {{"", eVoid}, {"mode", eEnum, eInput, 7}}},
        }};

Interface c2 = {
        2,
        "Sk_ColorSpace",
        {
                {1, "new_SRGB", eStatic, {{"", eInterfaceP, eOutputCustomDealloc, 2, 2, 3}}},
                {2, "ref", eNormal, {{"", eVoid}, {"", eInterfaceP, eInput, 2}}, false},
                {3, "unref", eNormal, {{"", eVoid}, {"", eInterfaceP, eInput, 2}}, false},
        }};

Interface c3 = {
        3,
        "Sk_Path",
        {
                {1, "move_To", eNormal, {{"", eVoid}, {"x", eFloat}, {"y", eFloat}}},
                {2, "line_To", eNormal, {{"", eVoid}, {"x", eFloat}, {"y", eFloat}}},
                {3,
                 "cubic_To",
                 eNormal,
                 {{"", eVoid},
                  {"x0", eFloat},
                  {"y0", eFloat},
                  {"x1", eFloat},
                  {"y1", eFloat},
                  {"x2", eFloat},
                  {"y2", eFloat}}},
                {4, "new", eCtor, {{"", eInterfaceP, eOutputCustomDealloc, 3, 3, 5}}},
                {5, "delete", eNormal, {{"", eVoid}, {"", eInterfaceP, eInput, 3}}, false},
                {6,
                 "quad_To",
                 eNormal,
                 {{"", eVoid}, {"x0", eFloat}, {"y0", eFloat}, {"x1", eFloat}, {"y1", eFloat}}},
                {7,
                 "conic_To",
                 eNormal,
                 {{"", eVoid},
                  {"x0", eFloat},
                  {"y0", eFloat},
                  {"x1", eFloat},
                  {"y1", eFloat},
                  {"w", eFloat}}},
                {8, "close", eNormal, {{"", eVoid}}},
                {9,
                 "add_Rect",
                 eNormal,
                 {{"", eVoid}, {"rect", eInterfaceP, eInput, 7}, {"dir", eEnum, eInput, 8}}},
                {10,
                 "add_Oval",
                 eNormal,
                 {{"", eVoid}, {"rect", eInterfaceP, eInput, 7}, {"dir", eEnum, eInput, 8}}},
                {11, "get_Bounds", eNormal, {{"rect", eInterfaceP, eOutputByValue, 7}}},
        }};

Interface c4 = {4,
                "Sk_SurfaceProps",
                {
                        {{1, "", eCtor, {{"", eInterfaceP, eOutputDelete, 4}}}},

                },
                {{"pixelGeometry", eEnum, eInput, 9}}};

Interface c5 = {5,
                "Sk_Surface",
                {
                        {1,
                         "new_Raster",
                         eStatic,
                         {{"", eInterfaceP, eOutputCustomDealloc, 5, 5, 2},
                          {"info", eInterfaceP, eInput, 6},
                          {"props", eInterfaceP, eInput, 4}}},
                        {2, "unref", eNormal, {{"", eVoid}, {"", eInterfaceP, eInput, 2}}, false},
                        {3, "get_Canvas", eNormal, {{"", eInterfaceP, eOutputNoDealloc, 8}}},
                        {4,
                         "new_Image_Snapshot",
                         eNormal,
                         {{"", eInterfaceP, eOutputCustomDealloc, 10, 10, 2}}},
                        {5,
                         "new_Raster_Direct",
                         eStatic,
                         {{"", eInterfaceP, eOutputCustomDealloc, 5, 5, 2},
                          {"info", eInterfaceP, eInput, 6},
                          {"pixels", eByteArray, eInput, ._countVarName = "rowBytes"},
                          {"rowBytes", eInt, eInput, ._exposed = false},
                          {"props", eInterfaceP, eInput, 4}}},
                }};

Interface c6 = {6,
                "Sk_ImageInfo",
                {
                        {1,
                         "new",
                         eCtor,
                         {{"", eInterfaceP, eOutputCustomDealloc, 6, 6, 2},
                          {"width", eInt},
                          {"height", eInt},
                          {"ct", eEnum, eInput, 2},
                          {"at", eEnum, eInput, 1},
                          {"cs", eInterfaceP, eInput, 2}}},
                        {2, "delete", eNormal, {{"", eVoid}, {"", eInterfaceP, eInput, 6}}, false},
                        {3, "get_Height", eNormal, {{"", eInt}}},
                        {4, "get_Width", eNormal, {{"", eInt}}},
                        {5, "get_ColorSpace", eNormal, {{"", eInterfaceP, eOutputNoDealloc, 2}}},
                        {6, "get_ColorType", eNormal, {{"", eEnum, eOutputNoDealloc, 2}}},
                        {7, "get_AlphaType", eNormal, {{"", eEnum, eOutputNoDealloc, 1}}},
                }};

Interface c7 = {7,
                "Sk_Rect",
                {{1, "", eCtor, {{"", eInterfaceP, eOutputDelete, 7}}}},
                {{"left", eFloat}, {"top", eFloat}, {"right", eFloat}, {"bottom", eFloat}}};

Interface c8 = {
        8,
        "Sk_Canvas",
        {
                {1, "draw_Paint", eNormal, {{"", eVoid}, {"paint", eInterfaceP, eInput, 1}}},
                {2,
                 "draw_Rect",
                 eNormal,
                 {{"", eVoid},
                  {"rect", eInterfaceP, eInput, 7},
                  {"paint", eInterfaceP, eInput, 1}}},
                {3,
                 "draw_Path",
                 eNormal,
                 {{"", eVoid},
                  {"path", eInterfaceP, eInput, 3},
                  {"paint", eInterfaceP, eInput, 1}}},
                {4,
                 "draw_Oval",
                 eNormal,
                 {{"", eVoid},
                  {"rect", eInterfaceP, eInput, 7},
                  {"paint", eInterfaceP, eInput, 1}}},
                {5, "save", eNormal, {{"", eVoid}}},
                {6,
                 "save_Layer",
                 eNormal,
                 {{"", eVoid},
                  {"rect", eInterfaceP, eInput, 7},
                  {"paint", eInterfaceP, eInput, 1}}},
                {7, "restore", eNormal, {{"", eVoid}}},
                {8,
                 "translate",
                 eNormal,
                 {{"", eVoid}, {"dx", eFloat, eInput}, {"dy", eFloat, eInput}}},
                {9,
                 "scale",
                 eNormal,
                 {{"", eVoid}, {"sx", eFloat, eInput}, {"sy", eFloat, eInput}}},
                /*{10,    //TODO: fix C API, which exposes this function, but does not implement it
                 "rotate_Degrees",
                 eNormal,
                 {{"", eVoid},
                  {"degrees", eFloat, eInput}}},*/
                {11, "rotate_Radians", eNormal, {{"", eVoid}, {"radians", eFloat, eInput}}},
                {12,
                 "skew",
                 eNormal,
                 {{"", eVoid}, {"sx", eFloat, eInput}, {"sy", eFloat, eInput}}},
                {13, "concat", eNormal, {{"", eVoid}, {"matrix", eInterfaceP, eInput, 11}}},
                {14, "clip_Rect", eNormal, {{"", eVoid}, {"rect", eInterfaceP, eInput, 7}}},
                {15, "clip_Path", eNormal, {{"", eVoid}, {"path", eInterfaceP, eInput, 3}}},
                {16,
                 "draw_Circle",
                 eNormal,
                 {{"", eVoid},
                  {"cx", eFloat, eInput},
                  {"cy", eFloat, eInput},
                  {"rad", eFloat, eInput},
                  {"paint", eInterfaceP, eInput, 1}}},
                {17,
                 "draw_Image",
                 eNormal,
                 {{"", eVoid},
                  {"image", eInterfaceP, eInput, 10},
                  {"x", eFloat, eInput},
                  {"y", eFloat, eInput},
                  {"paint", eInterfaceP, eInput, 1}}},
                {18,
                 "draw_Image_Rect",
                 eNormal,
                 {{"", eVoid},
                  {"image", eInterfaceP, eInput, 10},
                  {"src", eInterfaceP, eInput, 7},
                  {"dst", eInterfaceP, eInput, 7},
                  {"paint", eInterfaceP, eInput, 1}}},
                {19,
                 "draw_Picture",
                 eNormal,
                 {{"", eVoid},
                  {"picture", eInterfaceP, eInput, 14},
                  {"matrix", eInterfaceP, eInput, 11},
                  {"paint", eInterfaceP, eInput, 1}}},
        }};

Interface c9 = {9,
                "Sk_Data",
                {
                        {1, "get_Size", eNormal, {{"", eInt}}},
                        {2, "get_Data", eNormal, {{"", eByteArray, ._getArraySizeFunc = 1}}},
                        {3, "unref", eNormal, {{"", eVoid}, {"", eInterfaceP, eInput, 9}}, false},
                        //{4, "new_From_Malloc", eCtor, {{"", eInterfaceP, eOutputCustomDealloc, 9,
                        // 9, 3}, {"memory", eByteArray, eInput}}},
                        {4,
                         "new_With_Copy",
                         eCtor,
                         {{"", eInterfaceP, eOutputCustomDealloc, 9, 9, 3},
                          {"src", eByteArray, eInput, ._countVarName = "length"},
                          {"length", eInt, eInput, ._exposed = false}}},
                        {5,
                         "new_Subset",
                         eStatic,
                         {{"", eInterfaceP, eOutputCustomDealloc, 9, 9, 3},
                          {"src", eInterfaceP, eInput, 9},
                          {"offset", eInt, eInput},
                          {"length", eInt, eInput}}},
                        {6, "ref", eNormal, {{"", eVoid}, {"", eInterfaceP, eInput, 9}}, false},
                }};

Interface c10 = {10,
                 "Sk_Image",
                 {
                         {1, "encode", eNormal, {{"", eInterfaceP, eOutputCustomDealloc, 9, 9, 3}}},
                         {2, "unref", eNormal, {{"", eVoid}, {"", eInterfaceP, eInput, 10}}, false},
                         {3,
                          "new_Raster_Copy",
                          eStatic,
                          {{"", eInterfaceP, eOutputCustomDealloc, 10, 10, 2},
                           {"info", eInterfaceP, eInput, 6},
                           {"pixels", eByteArray, eInput, ._countVarName = "rowBytes"},
                           {"rowBytes", eInt, eInput, ._exposed = false}}},
                         {4,
                          "new_From_Encoded",
                          eStatic,
                          {{"", eInterfaceP, eOutputCustomDealloc, 10, 10, 2},
                           {"encoded", eInterfaceP, eInput, 9},
                           {"subset", eInterfaceP, eInput, 15}}},
                         {5, "ref", eNormal, {{"", eVoid}, {"", eInterfaceP, eInput, 10}}, false},
                         {6, "get_Width", eNormal, {{"", eInt}}},
                         {7, "get_Height", eNormal, {{"", eInt}}},
                         {8, "get_Unique_Id", eNormal, {{"", eInt}}},
                 }};

Interface c11 = {
        11,
        "Sk_Matrix",
        {
                {1, "", eCtor, {{"", eInterfaceP, eOutputDelete, 11}}},
                // TODO: remove the following functions in C API, because they are not implemented
                /*{2, "set_Identity", eNormal, {{"", eVoid}}},
                {3,
                 "set_Translate",
                 eNormal,
                 {{"", eVoid}, {"tx", eFloat, eInput}, {"ty", eFloat, eInput}}},
                {4,
                 "pre_Translate",
                 eNormal,
                 {{"", eVoid}, {"tx", eFloat, eInput}, {"ty", eFloat, eInput}}},
                {5,
                 "post_Translate",
                 eNormal,
                 {{"", eVoid}, {"tx", eFloat, eInput}, {"ty", eFloat, eInput}}},
                {6,
                 "set_Scale",
                 eNormal,
                 {{"", eVoid}, {"sx", eFloat, eInput}, {"sy", eFloat, eInput}}},
                {7,
                 "pre_Scale",
                 eNormal,
                 {{"", eVoid}, {"sx", eFloat, eInput}, {"sy", eFloat, eInput}}},
                {8,
                 "post_Scale",
                 eNormal,
                 {{"", eVoid}, {"sx", eFloat, eInput}, {"sy", eFloat, eInput}}},*/
        },
        {{"mat", eFloatArray, ._fixedArraySize = 9}}};

Interface c12 = {12,
                 "Sk_Shader",
                 {
                         {1, "ref", eNormal, {{"", eVoid}, {"", eInterfaceP, eInput, 12}}, false},
                         {2, "unref", eNormal, {{"", eVoid}, {"", eInterfaceP, eInput, 12}}, false},
                         {3,
                          "new_Linear_Gradient",
                          eStatic,
                          {
                                  {"", eInterfaceP, eOutputCustomDealloc, 12, 12, 2},
                                  {"startPoint", eInterfaceP, eInput, 16},
                                  {"endPoint", eInterfaceP, eInput, 16},
                                  {"colors", eIntArray, eInput},
                                  {"colorPos", eFloatArray, eInput},
                                  {"colorCount", eInt, eInput},
                                  {"tileMode", eEnum, eInput, 5},
                                  {"localMatrix", eInterfaceP, eInput, 11},
                          }},
                         {4,
                          "new_Radial_Gradient",
                          eStatic,
                          {
                                  {"", eInterfaceP, eOutputCustomDealloc, 12, 12, 2},
                                  {"center", eInterfaceP, eInput, 16},
                                  {"radius", eFloat, eInput},
                                  {"colors", eIntArray, eInput},
                                  {"colorPos", eFloatArray, eInput},
                                  {"colorCount", eInt, eInput},
                                  {"tileMode", eEnum, eInput, 5},
                                  {"localMatrix", eInterfaceP, eInput, 11},
                          }},
                         {5,
                          "new_Sweep_Gradient",
                          eStatic,
                          {
                                  {"", eInterfaceP, eOutputCustomDealloc, 12, 12, 2},
                                  {"center", eInterfaceP, eInput, 16},
                                  {"colors", eIntArray, eInput},
                                  {"colorPos", eFloatArray, eInput},
                                  {"colorCount", eInt, eInput},
                                  {"localMatrix", eInterfaceP, eInput, 11},
                          }},
                         {6,
                          "new_Two_Point_Conical_Gradient",
                          eStatic,
                          {
                                  {"", eInterfaceP, eOutputCustomDealloc, 12, 12, 2},
                                  {"startPoint", eInterfaceP, eInput, 16},
                                  {"startRadius", eFloat, eInput},
                                  {"endPoint", eInterfaceP, eInput, 16},
                                  {"endRadius", eFloat, eInput},
                                  {"colors", eIntArray, eInput},
                                  {"colorPos", eFloatArray, eInput},
                                  {"colorCount", eInt, eInput},
                                  {"tileMode", eEnum, eInput, 5},
                                  {"localMatrix", eInterfaceP, eInput, 11},
                          }},
                 }};

Interface c13 = {13,
                 "Sk_MaskFilter",
                 {
                         {1,
                          "new_Blur",
                          eStatic,
                          {{"", eInterfaceP, eOutputCustomDealloc, 13, 13, 3},
                           {"blur", eEnum, eInput, 6},
                           {"sigma", eFloat, eInput}}},
                         {2, "ref", eNormal, {{"", eVoid}, {"", eInterfaceP, eInput, 13}}, false},
                         {3, "unref", eNormal, {{"", eVoid}, {"", eInterfaceP, eInput, 13}}, false},
                 }};

Interface c14 = {14,
                 "Sk_Picture",
                 {
                         {1, "ref", eNormal, {{"", eVoid}, {"", eInterfaceP, eInput, 14}}, false},
                         {2, "unref", eNormal, {{"", eVoid}, {"", eInterfaceP, eInput, 14}}, false},
                         {3, "get_Unique_Id", eNormal, {{"", eInt}}},
                         {4, "get_Bounds", eNormal, {{"", eInterfaceP, eOutputByValue, 7}}},
                 }};

Interface c15 = {15,
                 "Sk_IRect",
                 {{1, "", eCtor, {{"", eInterfaceP, eOutputDelete, 15}}}},
                 {{"left", eInt}, {"top", eInt}, {"right", eInt}, {"bottom", eInt}}};

Interface c16 = {16,
                 "Sk_Point",
                 {{1, "", eCtor, {{"", eInterfaceP, eOutputDelete, 16}}}},
                 {{"x", eFloat}, {"y", eFloat}}};

Interface c17 = {
        17,
        "Sk_Picture_Recorder",
        {
                {1, "new", eCtor, {{"", eInterfaceP, eOutputCustomDealloc, 17, 17, 2}}},
                {2, "delete", eNormal, {{"", eVoid}, {"", eInterfaceP, eInput, 17}}, false},
                {3,
                 "begin_Recording",
                 eNormal,
                 {{"", eInterfaceP, eOutputNoDealloc, 8}, {"rect", eInterfaceP, eInput, 7}}},
                {4, "end_Recording", eNormal, {{"", eInterfaceP, eOutputCustomDealloc, 14, 14, 2}}},
        }};

API skiaAPI = {
        {c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15, c16, c17},
        {
                {1,
                 "Sk_AlphaType",
                 {{"OPAQUE_SK_ALPHATYPE"}, {"PREMUL_SK_ALPHATYPE"}, {"UNPREMUL_SK_ALPHATYPE"}}},
                {2,
                 "Sk_ColorType",
                 {{"UNKNOWN_SK_COLORTYPE"},
                  {"RGBA_8888_SK_COLORTYPE"},
                  {"BGRA_8888_SK_COLORTYPE"},
                  {"ALPHA_8_SK_COLORTYPE"},
                  {"GRAY_8_SK_COLORTYPE"},
                  {"RGBA_F16_SK_COLORTYPE"},
                  {"RGBA_F32_SK_COLORTYPE"}}},
                {3,
                 "Sk_Stroke_Join",
                 {{"MITER_SK_STROKE_JOIN"}, {"ROUND_SK_STROKE_JOIN"}, {"BEVEL_SK_STROKE_JOIN"}}},
                {4,
                 "Sk_Stroke_Cap",
                 {{"BUTT_SK_STROKE_CAP"}, {"ROUND_SK_STROKE_CAP"}, {"SQUARE_SK_STROKE_CAP"}}},
                {5,
                 "Sk_Shader_TileMode",
                 {{"CLAMP_SK_SHADER_TILEMODE"},
                  {"REPEAT_SK_SHADER_TILEMODE"},
                  {"MIRROR_SK_SHADER_TILEMODE"}}},
                {6,
                 "Sk_BlurStyle",
                 {{"NORMAL_SK_BLUR_STYLE"},
                  {"SOLID_SK_BLUR_STYLE"},
                  {"OUTER_SK_BLUR_STYLE"},
                  {"INNER_SK_BLUR_STYLE"}}},
                {7,
                 "Sk_Xfermode_mode",
                 {{"CLEAR_SK_XFERMODE_MODE"},      {"SRC_SK_XFERMODE_MODE"},
                  {"DST_SK_XFERMODE_MODE"},        {"SRCOVER_SK_XFERMODE_MODE"},
                  {"DSTOVER_SK_XFERMODE_MODE"},    {"SRCIN_SK_XFERMODE_MODE"},
                  {"DSTIN_SK_XFERMODE_MODE"},      {"SRCOUT_SK_XFERMODE_MODE"},
                  {"DSTOUT_SK_XFERMODE_MODE"},     {"SRCATOP_SK_XFERMODE_MODE"},
                  {"DSTATOP_SK_XFERMODE_MODE"},    {"XOR_SK_XFERMODE_MODE"},
                  {"PLUS_SK_XFERMODE_MODE"},       {"MODULATE_SK_XFERMODE_MODE"},
                  {"SCREEN_SK_XFERMODE_MODE"},     {"OVERLAY_SK_XFERMODE_MODE"},
                  {"DARKEN_SK_XFERMODE_MODE"},     {"LIGHTEN_SK_XFERMODE_MODE"},
                  {"COLORDODGE_SK_XFERMODE_MODE"}, {"COLORBURN_SK_XFERMODE_MODE"},
                  {"HARDLIGHT_SK_XFERMODE_MODE"},  {"SOFTLIGHT_SK_XFERMODE_MODE"},
                  {"DIFFERENCE_SK_XFERMODE_MODE"}, {"EXCLUSION_SK_XFERMODE_MODE"},
                  {"MULTIPLY_SK_XFERMODE_MODE"},   {"HUE_SK_XFERMODE_MODE"},
                  {"SATURATION_SK_XFERMODE_MODE"}, {"COLOR_SK_XFERMODE_MODE"},
                  {"LUMINOSITY_SK_XFERMODE_MODE"}}},
                {8, "Sk_Path_Direction", {{"CW_SK_PATH_DIRECTION"}, {"CCW_SK_PATH_DIRECTION"}}},
                {9,
                 "Sk_PixelGeometry",
                 {{"UNKNOWN_SK_PIXELGEOMETRY"},
                  {"RGB_H_SK_PIXELGEOMETRY"},
                  {"BGR_H_SK_PIXELGEOMETRY"},
                  {"RGB_V_SK_PIXELGEOMETRY"},
                  {"BGR_V_SK_PIXELGEOMETRY"}

                 }},
        }};

// Skia specific logic for transforming interface name into C struct name
string ifaceNameC(const string& name) {
    string result;
    result.resize(name.size());
    transform(name.begin(), name.end(), result.begin(), ::tolower);
    if (result.substr(0, 2) == "sk") {
        return "sk" + result.substr(2) + "_t";
    } else {
        return result + "_t";
    }
}

// Skia specific logic for transforming function name into C function name
string funcNameC(const string& iface, const string& func) {
    string ifaceC;
    ifaceC.resize(iface.size());
    transform(iface.begin(), iface.end(), ifaceC.begin(), ::tolower);
    if (ifaceC.substr(0, 2) == "sk") {
        ifaceC = "sk" + ifaceC.substr(2);
    }

    string funcC;
    funcC.resize(func.size());
    transform(func.begin(), func.end(), funcC.begin(), ::tolower);

    return ifaceC + "_" + funcC;
}

// Skia specific logic for transforming interface name into Java class name
string ifaceNameJava(const string& name) {
    // return name;
    auto javaName = name;
    javaName.erase(remove(javaName.begin(), javaName.end(), '_'), javaName.end());
    return javaName;
}

// Skia specific logic for transforming function name into Java function name
string funcNameJava(const string& iface, const string& func) {
    auto javaName = func;
    javaName.erase(remove(javaName.begin(), javaName.end(), '_'), javaName.end());
    return javaName;
}

NameConverter skiaCNames{ifaceNameC, funcNameC};

NameConverter skiaJavaNames{ifaceNameJava, funcNameJava};

// JavaConverter skiaJavaConv = {"org.skia.skottie2", skiaCNames, skiaJavaNames, skiaAPI,
JavaConverter skiaJavaConv = {"org.skia", skiaCNames, skiaJavaNames, skiaAPI,
                              "#include \"include/c/sk_types.h\"\n"
                              "#include \"include/c/sk_canvas.h\"\n"
                              "#include \"include/c/sk_colorspace.h\"\n"
                              "#include \"include/c/sk_data.h\"\n"
                              "#include \"include/c/sk_image.h\"\n"
                              "#include \"include/c/sk_imageinfo.h\"\n"
                              "#include \"include/c/sk_paint.h\"\n"
                              "#include \"include/c/sk_path.h\"\n"
                              "#include \"include/c/sk_surface.h\"\n"
                              "#include \"include/c/sk_types.h\"\n"
                              "#include \"include/c/sk_shader.h\"\n"
                              "#include \"include/c/sk_picture.h\"\n"
                              "#include \"sk_facade.h\"\n"
                              "#include \"include/c/sk_maskfilter.h\"\n"};

}  // namespace generator
