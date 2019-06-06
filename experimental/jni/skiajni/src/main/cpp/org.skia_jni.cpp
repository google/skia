/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
// This is an automatically generated file. Please do not edit.

#include <jni.h>
#include <memory>
#include "Binder.h"
#include "EnumHelper.h"

#include "include/c/sk_canvas.h"
#include "include/c/sk_colorspace.h"
#include "include/c/sk_data.h"
#include "include/c/sk_image.h"
#include "include/c/sk_imageinfo.h"
#include "include/c/sk_maskfilter.h"
#include "include/c/sk_paint.h"
#include "include/c/sk_path.h"
#include "include/c/sk_picture.h"
#include "include/c/sk_shader.h"
#include "include/c/sk_surface.h"
#include "include/c/sk_types.h"
#include "sk_facade.h"

static Binder<sk_paint_t> sSk_PaintBinder;
static Binder<sk_colorspace_t> sSk_ColorSpaceBinder;
static Binder<sk_path_t> sSk_PathBinder;
static Binder<sk_surfaceprops_t> sSk_SurfacePropsBinder;
static Binder<sk_surface_t> sSk_SurfaceBinder;
static Binder<sk_imageinfo_t> sSk_ImageInfoBinder;
static Binder<sk_rect_t> sSk_RectBinder;
static Binder<sk_canvas_t> sSk_CanvasBinder;
static Binder<sk_data_t> sSk_DataBinder;
static Binder<sk_image_t> sSk_ImageBinder;
static Binder<sk_matrix_t> sSk_MatrixBinder;
static Binder<sk_shader_t> sSk_ShaderBinder;
static Binder<sk_maskfilter_t> sSk_MaskFilterBinder;
static Binder<sk_picture_t> sSk_PictureBinder;
static Binder<sk_irect_t> sSk_IRectBinder;
static Binder<sk_point_t> sSk_PointBinder;
static Binder<sk_picture_recorder_t> sSk_Picture_RecorderBinder;
static EnumHelper<sk_alphatype_t> sSk_AlphaTypeEnum;
static EnumHelper<sk_colortype_t> sSk_ColorTypeEnum;
static EnumHelper<sk_stroke_join_t> sSk_Stroke_JoinEnum;
static EnumHelper<sk_stroke_cap_t> sSk_Stroke_CapEnum;
static EnumHelper<sk_shader_tilemode_t> sSk_Shader_TileModeEnum;
static EnumHelper<sk_blurstyle_t> sSk_BlurStyleEnum;
static EnumHelper<sk_xfermode_mode_t> sSk_Xfermode_modeEnum;
static EnumHelper<sk_path_direction_t> sSk_Path_DirectionEnum;
static EnumHelper<sk_pixelgeometry_t> sSk_PixelGeometryEnum;

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkAlphaType_staticInit(JNIEnv* env, jclass clazz) {
    sSk_AlphaTypeEnum.init(env, clazz);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkColorType_staticInit(JNIEnv* env, jclass clazz) {
    sSk_ColorTypeEnum.init(env, clazz);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkStrokeJoin_staticInit(JNIEnv* env, jclass clazz) {
    sSk_Stroke_JoinEnum.init(env, clazz);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkStrokeCap_staticInit(JNIEnv* env, jclass clazz) {
    sSk_Stroke_CapEnum.init(env, clazz);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkShaderTileMode_staticInit(JNIEnv* env,
                                                                            jclass clazz) {
    sSk_Shader_TileModeEnum.init(env, clazz);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkBlurStyle_staticInit(JNIEnv* env, jclass clazz) {
    sSk_BlurStyleEnum.init(env, clazz);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkXfermodemode_staticInit(JNIEnv* env,
                                                                          jclass clazz) {
    sSk_Xfermode_modeEnum.init(env, clazz);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkPathDirection_staticInit(JNIEnv* env,
                                                                           jclass clazz) {
    sSk_Path_DirectionEnum.init(env, clazz);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkPixelGeometry_staticInit(JNIEnv* env,
                                                                           jclass clazz) {
    sSk_PixelGeometryEnum.init(env, clazz);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkPaint_staticInit(JNIEnv* env, jclass clazz) {
    sSk_PaintBinder.init(env, clazz);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkPaint_nativeFinalize(JNIEnv* env, jobject clazz) {
    sSk_PaintBinder.unbind(env, clazz);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkPaint_setColor(JNIEnv* env, jobject thiz,
                                                                 jint color) {
    auto cppThiz = sSk_PaintBinder.getCppObject(env, thiz);
    sk_paint_set_color(cppThiz.get(), (int)color);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkPaint_setAntiAlias(JNIEnv* env, jobject thiz,
                                                                     jboolean a) {
    auto cppThiz = sSk_PaintBinder.getCppObject(env, thiz);
    sk_paint_set_antialias(cppThiz.get(), JNI_TRUE == a);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkPaint_setStroke(JNIEnv* env, jobject thiz,
                                                                  jboolean doStroke) {
    auto cppThiz = sSk_PaintBinder.getCppObject(env, thiz);
    sk_paint_set_stroke(cppThiz.get(), JNI_TRUE == doStroke);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkPaint_setStrokeWidth(JNIEnv* env, jobject thiz,
                                                                       jfloat width) {
    auto cppThiz = sSk_PaintBinder.getCppObject(env, thiz);
    sk_paint_set_stroke_width(cppThiz.get(), (float)width);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkPaint_nativeInit(JNIEnv* env, jobject thiz) {
    auto rawResult = sk_paint_new();
    std::shared_ptr<sk_paint_t> result(rawResult, [](sk_paint_t* p) { sk_paint_delete(p); });
    sSk_PaintBinder.bind(env, result, thiz);
}

extern "C" JNIEXPORT jboolean JNICALL Java_org_skia_SkPaint_isAntiAlias(JNIEnv* env, jobject thiz) {
    auto cppThiz = sSk_PaintBinder.getCppObject(env, thiz);
    auto rawResult = sk_paint_is_antialias(cppThiz.get());
    return rawResult ? JNI_TRUE : JNI_FALSE;
}

extern "C" JNIEXPORT jint JNICALL Java_org_skia_SkPaint_getColor(JNIEnv* env, jobject thiz) {
    auto cppThiz = sSk_PaintBinder.getCppObject(env, thiz);
    auto rawResult = sk_paint_get_color(cppThiz.get());
    return rawResult;
}

extern "C" JNIEXPORT jboolean JNICALL Java_org_skia_SkPaint_isStroke(JNIEnv* env, jobject thiz) {
    auto cppThiz = sSk_PaintBinder.getCppObject(env, thiz);
    auto rawResult = sk_paint_is_stroke(cppThiz.get());
    return rawResult ? JNI_TRUE : JNI_FALSE;
}

extern "C" JNIEXPORT jfloat JNICALL Java_org_skia_SkPaint_getStrokeWidth(JNIEnv* env,
                                                                         jobject thiz) {
    auto cppThiz = sSk_PaintBinder.getCppObject(env, thiz);
    auto rawResult = sk_paint_get_stroke_width(cppThiz.get());
    return rawResult;
}

extern "C" JNIEXPORT jfloat JNICALL Java_org_skia_SkPaint_getStrokeMiter(JNIEnv* env,
                                                                         jobject thiz) {
    auto cppThiz = sSk_PaintBinder.getCppObject(env, thiz);
    auto rawResult = sk_paint_get_stroke_miter(cppThiz.get());
    return rawResult;
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkPaint_setStrokeMiter(JNIEnv* env, jobject thiz,
                                                                       jfloat miter) {
    auto cppThiz = sSk_PaintBinder.getCppObject(env, thiz);
    sk_paint_set_stroke_miter(cppThiz.get(), (float)miter);
}

extern "C" JNIEXPORT jobject JNICALL Java_org_skia_SkPaint_getStrokeCap(JNIEnv* env, jobject thiz) {
    auto cppThiz = sSk_PaintBinder.getCppObject(env, thiz);
    auto rawResult = sk_paint_get_stroke_cap(cppThiz.get());
    return sSk_Stroke_CapEnum.getJavaValue(env, rawResult);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkPaint_setStrokeCap(JNIEnv* env, jobject thiz,
                                                                     jobject cap) {
    auto cppThiz = sSk_PaintBinder.getCppObject(env, thiz);
    sk_stroke_cap_t bp1 = sSk_Stroke_CapEnum.getCppValue(env, cap);
    sk_paint_set_stroke_cap(cppThiz.get(), bp1);
}

extern "C" JNIEXPORT jobject JNICALL Java_org_skia_SkPaint_getStrokeJoin(JNIEnv* env,
                                                                         jobject thiz) {
    auto cppThiz = sSk_PaintBinder.getCppObject(env, thiz);
    auto rawResult = sk_paint_get_stroke_join(cppThiz.get());
    return sSk_Stroke_JoinEnum.getJavaValue(env, rawResult);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkPaint_setStrokeJoin(JNIEnv* env, jobject thiz,
                                                                      jobject cap) {
    auto cppThiz = sSk_PaintBinder.getCppObject(env, thiz);
    sk_stroke_join_t bp1 = sSk_Stroke_JoinEnum.getCppValue(env, cap);
    sk_paint_set_stroke_join(cppThiz.get(), bp1);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkPaint_setShader(JNIEnv* env, jobject thiz,
                                                                  jobject shader) {
    auto cppThiz = sSk_PaintBinder.getCppObject(env, thiz);
    std::shared_ptr<sk_shader_t> bp1 = sSk_ShaderBinder.getCppObject(env, shader);
    sk_paint_set_shader(cppThiz.get(), bp1.get());
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkPaint_setMaskFilter(JNIEnv* env, jobject thiz,
                                                                      jobject filter) {
    auto cppThiz = sSk_PaintBinder.getCppObject(env, thiz);
    std::shared_ptr<sk_maskfilter_t> bp1 = sSk_MaskFilterBinder.getCppObject(env, filter);
    sk_paint_set_maskfilter(cppThiz.get(), bp1.get());
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkPaint_setXfermodeMode(JNIEnv* env, jobject thiz,
                                                                        jobject mode) {
    auto cppThiz = sSk_PaintBinder.getCppObject(env, thiz);
    sk_xfermode_mode_t bp1 = sSk_Xfermode_modeEnum.getCppValue(env, mode);
    sk_paint_set_xfermode_mode(cppThiz.get(), bp1);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkColorSpace_staticInit(JNIEnv* env, jclass clazz) {
    sSk_ColorSpaceBinder.init(env, clazz);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkColorSpace_nativeFinalize(JNIEnv* env,
                                                                            jobject clazz) {
    sSk_ColorSpaceBinder.unbind(env, clazz);
}

extern "C" JNIEXPORT jobject JNICALL Java_org_skia_SkColorSpace_newSRGB(JNIEnv* env, jclass clazz) {
    auto rawResult = sk_colorspace_new_srgb();
    std::shared_ptr<sk_colorspace_t> result(rawResult,
                                            [](sk_colorspace_t* p) { sk_colorspace_unref(p); });
    return sSk_ColorSpaceBinder.getJavaObject(env, result);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkPath_staticInit(JNIEnv* env, jclass clazz) {
    sSk_PathBinder.init(env, clazz);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkPath_nativeFinalize(JNIEnv* env, jobject clazz) {
    sSk_PathBinder.unbind(env, clazz);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkPath_moveTo(JNIEnv* env, jobject thiz, jfloat x,
                                                              jfloat y) {
    auto cppThiz = sSk_PathBinder.getCppObject(env, thiz);
    sk_path_move_to(cppThiz.get(), (float)x, (float)y);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkPath_lineTo(JNIEnv* env, jobject thiz, jfloat x,
                                                              jfloat y) {
    auto cppThiz = sSk_PathBinder.getCppObject(env, thiz);
    sk_path_line_to(cppThiz.get(), (float)x, (float)y);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkPath_cubicTo(JNIEnv* env, jobject thiz, jfloat x0,
                                                               jfloat y0, jfloat x1, jfloat y1,
                                                               jfloat x2, jfloat y2) {
    auto cppThiz = sSk_PathBinder.getCppObject(env, thiz);
    sk_path_cubic_to(cppThiz.get(), (float)x0, (float)y0, (float)x1, (float)y1, (float)x2,
                     (float)y2);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkPath_nativeInit(JNIEnv* env, jobject thiz) {
    auto rawResult = sk_path_new();
    std::shared_ptr<sk_path_t> result(rawResult, [](sk_path_t* p) { sk_path_delete(p); });
    sSk_PathBinder.bind(env, result, thiz);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkPath_quadTo(JNIEnv* env, jobject thiz, jfloat x0,
                                                              jfloat y0, jfloat x1, jfloat y1) {
    auto cppThiz = sSk_PathBinder.getCppObject(env, thiz);
    sk_path_quad_to(cppThiz.get(), (float)x0, (float)y0, (float)x1, (float)y1);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkPath_conicTo(JNIEnv* env, jobject thiz, jfloat x0,
                                                               jfloat y0, jfloat x1, jfloat y1,
                                                               jfloat w) {
    auto cppThiz = sSk_PathBinder.getCppObject(env, thiz);
    sk_path_conic_to(cppThiz.get(), (float)x0, (float)y0, (float)x1, (float)y1, (float)w);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkPath_close(JNIEnv* env, jobject thiz) {
    auto cppThiz = sSk_PathBinder.getCppObject(env, thiz);
    sk_path_close(cppThiz.get());
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkPath_addRect(JNIEnv* env, jobject thiz,
                                                               jobject rect, jobject dir) {
    auto cppThiz = sSk_PathBinder.getCppObject(env, thiz);
    std::shared_ptr<sk_rect_t> bp1 = sSk_RectBinder.getCppObject(env, rect);
    if (bp1.get()) {
        auto& props1 = sSk_RectBinder.properties();
        if (props1.empty()) {
            props1.push_back(env->GetFieldID(sSk_RectBinder.getJavaClass(), "left", "F"));
            props1.push_back(env->GetFieldID(sSk_RectBinder.getJavaClass(), "top", "F"));
            props1.push_back(env->GetFieldID(sSk_RectBinder.getJavaClass(), "right", "F"));
            props1.push_back(env->GetFieldID(sSk_RectBinder.getJavaClass(), "bottom", "F"));
        }
        bp1->left = env->GetFloatField(rect, props1[0]);
        bp1->top = env->GetFloatField(rect, props1[1]);
        bp1->right = env->GetFloatField(rect, props1[2]);
        bp1->bottom = env->GetFloatField(rect, props1[3]);
    }
    sk_path_direction_t bp2 = sSk_Path_DirectionEnum.getCppValue(env, dir);
    sk_path_add_rect(cppThiz.get(), bp1.get(), bp2);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkPath_addOval(JNIEnv* env, jobject thiz,
                                                               jobject rect, jobject dir) {
    auto cppThiz = sSk_PathBinder.getCppObject(env, thiz);
    std::shared_ptr<sk_rect_t> bp1 = sSk_RectBinder.getCppObject(env, rect);
    if (bp1.get()) {
        auto& props1 = sSk_RectBinder.properties();
        if (props1.empty()) {
            props1.push_back(env->GetFieldID(sSk_RectBinder.getJavaClass(), "left", "F"));
            props1.push_back(env->GetFieldID(sSk_RectBinder.getJavaClass(), "top", "F"));
            props1.push_back(env->GetFieldID(sSk_RectBinder.getJavaClass(), "right", "F"));
            props1.push_back(env->GetFieldID(sSk_RectBinder.getJavaClass(), "bottom", "F"));
        }
        bp1->left = env->GetFloatField(rect, props1[0]);
        bp1->top = env->GetFloatField(rect, props1[1]);
        bp1->right = env->GetFloatField(rect, props1[2]);
        bp1->bottom = env->GetFloatField(rect, props1[3]);
    }
    sk_path_direction_t bp2 = sSk_Path_DirectionEnum.getCppValue(env, dir);
    sk_path_add_oval(cppThiz.get(), bp1.get(), bp2);
}

extern "C" JNIEXPORT jobject JNICALL Java_org_skia_SkPath_getBounds(JNIEnv* env, jobject thiz) {
    auto cppThiz = sSk_PathBinder.getCppObject(env, thiz);
    sk_rect_t* rawResult = new sk_rect_t();
    *rawResult = sk_path_get_bounds(cppThiz.get());
    std::shared_ptr<sk_rect_t> result(rawResult);
    return sSk_RectBinder.getJavaObject(env, result);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkSurfaceProps_staticInit(JNIEnv* env,
                                                                          jclass clazz) {
    sSk_SurfacePropsBinder.init(env, clazz);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkSurfaceProps_nativeFinalize(JNIEnv* env,
                                                                              jobject clazz) {
    sSk_SurfacePropsBinder.unbind(env, clazz);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkSurfaceProps_nativeInit(JNIEnv* env,
                                                                          jobject thiz) {
    auto rawResult = new sk_surfaceprops_t();
    std::shared_ptr<sk_surfaceprops_t> result(rawResult);
    sSk_SurfacePropsBinder.bind(env, result, thiz);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkSurface_staticInit(JNIEnv* env, jclass clazz) {
    sSk_SurfaceBinder.init(env, clazz);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkSurface_nativeFinalize(JNIEnv* env,
                                                                         jobject clazz) {
    sSk_SurfaceBinder.unbind(env, clazz);
}

extern "C" JNIEXPORT jobject JNICALL Java_org_skia_SkSurface_newRaster(JNIEnv* env, jclass clazz,
                                                                       jobject info,
                                                                       jobject props) {
    std::shared_ptr<sk_imageinfo_t> bp1 = sSk_ImageInfoBinder.getCppObject(env, info);
    std::shared_ptr<sk_surfaceprops_t> bp2 = sSk_SurfacePropsBinder.getCppObject(env, props);
    if (bp2.get()) {
        auto& props2 = sSk_SurfacePropsBinder.properties();
        if (props2.empty()) {
            props2.push_back(env->GetFieldID(sSk_SurfacePropsBinder.getJavaClass(), "pixelGeometry",
                                             "Lorg/skia/SkPixelGeometry;"));
        }
        bp2->pixelGeometry =
                sSk_PixelGeometryEnum.getCppValue(env, env->GetObjectField(props, props2[0]));
    }
    auto rawResult = sk_surface_new_raster(bp1.get(), bp2.get());
    std::shared_ptr<sk_surface_t> result(rawResult, [](sk_surface_t* p) { sk_surface_unref(p); });
    return sSk_SurfaceBinder.getJavaObject(env, result);
}

extern "C" JNIEXPORT jobject JNICALL Java_org_skia_SkSurface_getCanvas(JNIEnv* env, jobject thiz) {
    auto cppThiz = sSk_SurfaceBinder.getCppObject(env, thiz);
    auto rawResult = sk_surface_get_canvas(cppThiz.get());
    std::shared_ptr<sk_canvas_t> result(rawResult, [](sk_canvas_t* p) {});
    return sSk_CanvasBinder.getJavaObject(env, result);
}

extern "C" JNIEXPORT jobject JNICALL Java_org_skia_SkSurface_newImageSnapshot(JNIEnv* env,
                                                                              jobject thiz) {
    auto cppThiz = sSk_SurfaceBinder.getCppObject(env, thiz);
    auto rawResult = sk_surface_new_image_snapshot(cppThiz.get());
    std::shared_ptr<sk_image_t> result(rawResult, [](sk_image_t* p) { sk_image_unref(p); });
    return sSk_ImageBinder.getJavaObject(env, result);
}

extern "C" JNIEXPORT jobject JNICALL Java_org_skia_SkSurface_newRasterDirect(
        JNIEnv* env, jclass clazz, jobject info, jbyteArray pixels, jobject props) {
    std::shared_ptr<sk_imageinfo_t> bp1 = sSk_ImageInfoBinder.getCppObject(env, info);
    int rowBytes;
    auto binar2 = JNIHelper::as_unsigned_char_array2(env, pixels, rowBytes);
    std::shared_ptr<sk_surfaceprops_t> bp4 = sSk_SurfacePropsBinder.getCppObject(env, props);
    if (bp4.get()) {
        auto& props4 = sSk_SurfacePropsBinder.properties();
        if (props4.empty()) {
            props4.push_back(env->GetFieldID(sSk_SurfacePropsBinder.getJavaClass(), "pixelGeometry",
                                             "Lorg/skia/SkPixelGeometry;"));
        }
        bp4->pixelGeometry =
                sSk_PixelGeometryEnum.getCppValue(env, env->GetObjectField(props, props4[0]));
    }
    auto rawResult =
            sk_surface_new_raster_direct(bp1.get(), binar2.get(), (int)rowBytes, bp4.get());
    std::shared_ptr<sk_surface_t> result(rawResult, [](sk_surface_t* p) { sk_surface_unref(p); });
    return sSk_SurfaceBinder.getJavaObject(env, result);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkImageInfo_staticInit(JNIEnv* env, jclass clazz) {
    sSk_ImageInfoBinder.init(env, clazz);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkImageInfo_nativeFinalize(JNIEnv* env,
                                                                           jobject clazz) {
    sSk_ImageInfoBinder.unbind(env, clazz);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkImageInfo_nativeInit(JNIEnv* env, jobject thiz,
                                                                       jint width, jint height,
                                                                       jobject ct, jobject at,
                                                                       jobject cs) {
    sk_colortype_t bp3 = sSk_ColorTypeEnum.getCppValue(env, ct);
    sk_alphatype_t bp4 = sSk_AlphaTypeEnum.getCppValue(env, at);
    std::shared_ptr<sk_colorspace_t> bp5 = sSk_ColorSpaceBinder.getCppObject(env, cs);
    auto rawResult = sk_imageinfo_new((int)width, (int)height, bp3, bp4, bp5.get());
    std::shared_ptr<sk_imageinfo_t> result(rawResult,
                                           [](sk_imageinfo_t* p) { sk_imageinfo_delete(p); });
    sSk_ImageInfoBinder.bind(env, result, thiz);
}

extern "C" JNIEXPORT jint JNICALL Java_org_skia_SkImageInfo_getHeight(JNIEnv* env, jobject thiz) {
    auto cppThiz = sSk_ImageInfoBinder.getCppObject(env, thiz);
    auto rawResult = sk_imageinfo_get_height(cppThiz.get());
    return rawResult;
}

extern "C" JNIEXPORT jint JNICALL Java_org_skia_SkImageInfo_getWidth(JNIEnv* env, jobject thiz) {
    auto cppThiz = sSk_ImageInfoBinder.getCppObject(env, thiz);
    auto rawResult = sk_imageinfo_get_width(cppThiz.get());
    return rawResult;
}

extern "C" JNIEXPORT jobject JNICALL Java_org_skia_SkImageInfo_getColorSpace(JNIEnv* env,
                                                                             jobject thiz) {
    auto cppThiz = sSk_ImageInfoBinder.getCppObject(env, thiz);
    auto rawResult = sk_imageinfo_get_colorspace(cppThiz.get());
    std::shared_ptr<sk_colorspace_t> result(rawResult, [](sk_colorspace_t* p) {});
    return sSk_ColorSpaceBinder.getJavaObject(env, result);
}

extern "C" JNIEXPORT jobject JNICALL Java_org_skia_SkImageInfo_getColorType(JNIEnv* env,
                                                                            jobject thiz) {
    auto cppThiz = sSk_ImageInfoBinder.getCppObject(env, thiz);
    auto rawResult = sk_imageinfo_get_colortype(cppThiz.get());
    return sSk_ColorTypeEnum.getJavaValue(env, rawResult);
}

extern "C" JNIEXPORT jobject JNICALL Java_org_skia_SkImageInfo_getAlphaType(JNIEnv* env,
                                                                            jobject thiz) {
    auto cppThiz = sSk_ImageInfoBinder.getCppObject(env, thiz);
    auto rawResult = sk_imageinfo_get_alphatype(cppThiz.get());
    return sSk_AlphaTypeEnum.getJavaValue(env, rawResult);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkRect_staticInit(JNIEnv* env, jclass clazz) {
    sSk_RectBinder.init(env, clazz);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkRect_nativeFinalize(JNIEnv* env, jobject clazz) {
    sSk_RectBinder.unbind(env, clazz);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkRect_nativeInit(JNIEnv* env, jobject thiz) {
    auto rawResult = new sk_rect_t();
    std::shared_ptr<sk_rect_t> result(rawResult);
    sSk_RectBinder.bind(env, result, thiz);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkCanvas_staticInit(JNIEnv* env, jclass clazz) {
    sSk_CanvasBinder.init(env, clazz);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkCanvas_nativeFinalize(JNIEnv* env,
                                                                        jobject clazz) {
    sSk_CanvasBinder.unbind(env, clazz);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkCanvas_drawPaint(JNIEnv* env, jobject thiz,
                                                                   jobject paint) {
    auto cppThiz = sSk_CanvasBinder.getCppObject(env, thiz);
    std::shared_ptr<sk_paint_t> bp1 = sSk_PaintBinder.getCppObject(env, paint);
    sk_canvas_draw_paint(cppThiz.get(), bp1.get());
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkCanvas_drawRect(JNIEnv* env, jobject thiz,
                                                                  jobject rect, jobject paint) {
    auto cppThiz = sSk_CanvasBinder.getCppObject(env, thiz);
    std::shared_ptr<sk_rect_t> bp1 = sSk_RectBinder.getCppObject(env, rect);
    if (bp1.get()) {
        auto& props1 = sSk_RectBinder.properties();
        if (props1.empty()) {
            props1.push_back(env->GetFieldID(sSk_RectBinder.getJavaClass(), "left", "F"));
            props1.push_back(env->GetFieldID(sSk_RectBinder.getJavaClass(), "top", "F"));
            props1.push_back(env->GetFieldID(sSk_RectBinder.getJavaClass(), "right", "F"));
            props1.push_back(env->GetFieldID(sSk_RectBinder.getJavaClass(), "bottom", "F"));
        }
        bp1->left = env->GetFloatField(rect, props1[0]);
        bp1->top = env->GetFloatField(rect, props1[1]);
        bp1->right = env->GetFloatField(rect, props1[2]);
        bp1->bottom = env->GetFloatField(rect, props1[3]);
    }
    std::shared_ptr<sk_paint_t> bp2 = sSk_PaintBinder.getCppObject(env, paint);
    sk_canvas_draw_rect(cppThiz.get(), bp1.get(), bp2.get());
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkCanvas_drawPath(JNIEnv* env, jobject thiz,
                                                                  jobject path, jobject paint) {
    auto cppThiz = sSk_CanvasBinder.getCppObject(env, thiz);
    std::shared_ptr<sk_path_t> bp1 = sSk_PathBinder.getCppObject(env, path);
    std::shared_ptr<sk_paint_t> bp2 = sSk_PaintBinder.getCppObject(env, paint);
    sk_canvas_draw_path(cppThiz.get(), bp1.get(), bp2.get());
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkCanvas_drawOval(JNIEnv* env, jobject thiz,
                                                                  jobject rect, jobject paint) {
    auto cppThiz = sSk_CanvasBinder.getCppObject(env, thiz);
    std::shared_ptr<sk_rect_t> bp1 = sSk_RectBinder.getCppObject(env, rect);
    if (bp1.get()) {
        auto& props1 = sSk_RectBinder.properties();
        if (props1.empty()) {
            props1.push_back(env->GetFieldID(sSk_RectBinder.getJavaClass(), "left", "F"));
            props1.push_back(env->GetFieldID(sSk_RectBinder.getJavaClass(), "top", "F"));
            props1.push_back(env->GetFieldID(sSk_RectBinder.getJavaClass(), "right", "F"));
            props1.push_back(env->GetFieldID(sSk_RectBinder.getJavaClass(), "bottom", "F"));
        }
        bp1->left = env->GetFloatField(rect, props1[0]);
        bp1->top = env->GetFloatField(rect, props1[1]);
        bp1->right = env->GetFloatField(rect, props1[2]);
        bp1->bottom = env->GetFloatField(rect, props1[3]);
    }
    std::shared_ptr<sk_paint_t> bp2 = sSk_PaintBinder.getCppObject(env, paint);
    sk_canvas_draw_oval(cppThiz.get(), bp1.get(), bp2.get());
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkCanvas_save(JNIEnv* env, jobject thiz) {
    auto cppThiz = sSk_CanvasBinder.getCppObject(env, thiz);
    sk_canvas_save(cppThiz.get());
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkCanvas_saveLayer(JNIEnv* env, jobject thiz,
                                                                   jobject rect, jobject paint) {
    auto cppThiz = sSk_CanvasBinder.getCppObject(env, thiz);
    std::shared_ptr<sk_rect_t> bp1 = sSk_RectBinder.getCppObject(env, rect);
    if (bp1.get()) {
        auto& props1 = sSk_RectBinder.properties();
        if (props1.empty()) {
            props1.push_back(env->GetFieldID(sSk_RectBinder.getJavaClass(), "left", "F"));
            props1.push_back(env->GetFieldID(sSk_RectBinder.getJavaClass(), "top", "F"));
            props1.push_back(env->GetFieldID(sSk_RectBinder.getJavaClass(), "right", "F"));
            props1.push_back(env->GetFieldID(sSk_RectBinder.getJavaClass(), "bottom", "F"));
        }
        bp1->left = env->GetFloatField(rect, props1[0]);
        bp1->top = env->GetFloatField(rect, props1[1]);
        bp1->right = env->GetFloatField(rect, props1[2]);
        bp1->bottom = env->GetFloatField(rect, props1[3]);
    }
    std::shared_ptr<sk_paint_t> bp2 = sSk_PaintBinder.getCppObject(env, paint);
    sk_canvas_save_layer(cppThiz.get(), bp1.get(), bp2.get());
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkCanvas_restore(JNIEnv* env, jobject thiz) {
    auto cppThiz = sSk_CanvasBinder.getCppObject(env, thiz);
    sk_canvas_restore(cppThiz.get());
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkCanvas_translate(JNIEnv* env, jobject thiz,
                                                                   jfloat dx, jfloat dy) {
    auto cppThiz = sSk_CanvasBinder.getCppObject(env, thiz);
    sk_canvas_translate(cppThiz.get(), (float)dx, (float)dy);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkCanvas_scale(JNIEnv* env, jobject thiz, jfloat sx,
                                                               jfloat sy) {
    auto cppThiz = sSk_CanvasBinder.getCppObject(env, thiz);
    sk_canvas_scale(cppThiz.get(), (float)sx, (float)sy);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkCanvas_rotateRadians(JNIEnv* env, jobject thiz,
                                                                       jfloat radians) {
    auto cppThiz = sSk_CanvasBinder.getCppObject(env, thiz);
    sk_canvas_rotate_radians(cppThiz.get(), (float)radians);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkCanvas_skew(JNIEnv* env, jobject thiz, jfloat sx,
                                                              jfloat sy) {
    auto cppThiz = sSk_CanvasBinder.getCppObject(env, thiz);
    sk_canvas_skew(cppThiz.get(), (float)sx, (float)sy);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkCanvas_concat(JNIEnv* env, jobject thiz,
                                                                jobject matrix) {
    auto cppThiz = sSk_CanvasBinder.getCppObject(env, thiz);
    std::shared_ptr<sk_matrix_t> bp1 = sSk_MatrixBinder.getCppObject(env, matrix);
    if (bp1.get()) {
        auto& props1 = sSk_MatrixBinder.properties();
        if (props1.empty()) {
            props1.push_back(env->GetFieldID(sSk_MatrixBinder.getJavaClass(), "mat", "[F"));
        }
        {
            int proplen0;
            auto proparr0 = JNIHelper::as_float_array2(
                    env, (jfloatArray)env->GetObjectField(matrix, props1[0]), proplen0);
            memcpy(bp1->mat, proparr0.get(), proplen0 * sizeof(float));
        }
    }
    sk_canvas_concat(cppThiz.get(), bp1.get());
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkCanvas_clipRect(JNIEnv* env, jobject thiz,
                                                                  jobject rect) {
    auto cppThiz = sSk_CanvasBinder.getCppObject(env, thiz);
    std::shared_ptr<sk_rect_t> bp1 = sSk_RectBinder.getCppObject(env, rect);
    if (bp1.get()) {
        auto& props1 = sSk_RectBinder.properties();
        if (props1.empty()) {
            props1.push_back(env->GetFieldID(sSk_RectBinder.getJavaClass(), "left", "F"));
            props1.push_back(env->GetFieldID(sSk_RectBinder.getJavaClass(), "top", "F"));
            props1.push_back(env->GetFieldID(sSk_RectBinder.getJavaClass(), "right", "F"));
            props1.push_back(env->GetFieldID(sSk_RectBinder.getJavaClass(), "bottom", "F"));
        }
        bp1->left = env->GetFloatField(rect, props1[0]);
        bp1->top = env->GetFloatField(rect, props1[1]);
        bp1->right = env->GetFloatField(rect, props1[2]);
        bp1->bottom = env->GetFloatField(rect, props1[3]);
    }
    sk_canvas_clip_rect(cppThiz.get(), bp1.get());
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkCanvas_clipPath(JNIEnv* env, jobject thiz,
                                                                  jobject path) {
    auto cppThiz = sSk_CanvasBinder.getCppObject(env, thiz);
    std::shared_ptr<sk_path_t> bp1 = sSk_PathBinder.getCppObject(env, path);
    sk_canvas_clip_path(cppThiz.get(), bp1.get());
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkCanvas_drawCircle(JNIEnv* env, jobject thiz,
                                                                    jfloat cx, jfloat cy,
                                                                    jfloat rad, jobject paint) {
    auto cppThiz = sSk_CanvasBinder.getCppObject(env, thiz);
    std::shared_ptr<sk_paint_t> bp4 = sSk_PaintBinder.getCppObject(env, paint);
    sk_canvas_draw_circle(cppThiz.get(), (float)cx, (float)cy, (float)rad, bp4.get());
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkCanvas_drawImage(JNIEnv* env, jobject thiz,
                                                                   jobject image, jfloat x,
                                                                   jfloat y, jobject paint) {
    auto cppThiz = sSk_CanvasBinder.getCppObject(env, thiz);
    std::shared_ptr<sk_image_t> bp1 = sSk_ImageBinder.getCppObject(env, image);
    std::shared_ptr<sk_paint_t> bp4 = sSk_PaintBinder.getCppObject(env, paint);
    sk_canvas_draw_image(cppThiz.get(), bp1.get(), (float)x, (float)y, bp4.get());
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkCanvas_drawImageRect(JNIEnv* env, jobject thiz,
                                                                       jobject image, jobject src,
                                                                       jobject dst, jobject paint) {
    auto cppThiz = sSk_CanvasBinder.getCppObject(env, thiz);
    std::shared_ptr<sk_image_t> bp1 = sSk_ImageBinder.getCppObject(env, image);
    std::shared_ptr<sk_rect_t> bp2 = sSk_RectBinder.getCppObject(env, src);
    if (bp2.get()) {
        auto& props2 = sSk_RectBinder.properties();
        if (props2.empty()) {
            props2.push_back(env->GetFieldID(sSk_RectBinder.getJavaClass(), "left", "F"));
            props2.push_back(env->GetFieldID(sSk_RectBinder.getJavaClass(), "top", "F"));
            props2.push_back(env->GetFieldID(sSk_RectBinder.getJavaClass(), "right", "F"));
            props2.push_back(env->GetFieldID(sSk_RectBinder.getJavaClass(), "bottom", "F"));
        }
        bp2->left = env->GetFloatField(src, props2[0]);
        bp2->top = env->GetFloatField(src, props2[1]);
        bp2->right = env->GetFloatField(src, props2[2]);
        bp2->bottom = env->GetFloatField(src, props2[3]);
    }
    std::shared_ptr<sk_rect_t> bp3 = sSk_RectBinder.getCppObject(env, dst);
    if (bp3.get()) {
        auto& props3 = sSk_RectBinder.properties();
        if (props3.empty()) {
            props3.push_back(env->GetFieldID(sSk_RectBinder.getJavaClass(), "left", "F"));
            props3.push_back(env->GetFieldID(sSk_RectBinder.getJavaClass(), "top", "F"));
            props3.push_back(env->GetFieldID(sSk_RectBinder.getJavaClass(), "right", "F"));
            props3.push_back(env->GetFieldID(sSk_RectBinder.getJavaClass(), "bottom", "F"));
        }
        bp3->left = env->GetFloatField(dst, props3[0]);
        bp3->top = env->GetFloatField(dst, props3[1]);
        bp3->right = env->GetFloatField(dst, props3[2]);
        bp3->bottom = env->GetFloatField(dst, props3[3]);
    }
    std::shared_ptr<sk_paint_t> bp4 = sSk_PaintBinder.getCppObject(env, paint);
    sk_canvas_draw_image_rect(cppThiz.get(), bp1.get(), bp2.get(), bp3.get(), bp4.get());
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkCanvas_drawPicture(JNIEnv* env, jobject thiz,
                                                                     jobject picture,
                                                                     jobject matrix,
                                                                     jobject paint) {
    auto cppThiz = sSk_CanvasBinder.getCppObject(env, thiz);
    std::shared_ptr<sk_picture_t> bp1 = sSk_PictureBinder.getCppObject(env, picture);
    std::shared_ptr<sk_matrix_t> bp2 = sSk_MatrixBinder.getCppObject(env, matrix);
    if (bp2.get()) {
        auto& props2 = sSk_MatrixBinder.properties();
        if (props2.empty()) {
            props2.push_back(env->GetFieldID(sSk_MatrixBinder.getJavaClass(), "mat", "[F"));
        }
        {
            int proplen0;
            auto proparr0 = JNIHelper::as_float_array2(
                    env, (jfloatArray)env->GetObjectField(matrix, props2[0]), proplen0);
            memcpy(bp2->mat, proparr0.get(), proplen0 * sizeof(float));
        }
    }
    std::shared_ptr<sk_paint_t> bp3 = sSk_PaintBinder.getCppObject(env, paint);
    sk_canvas_draw_picture(cppThiz.get(), bp1.get(), bp2.get(), bp3.get());
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkData_staticInit(JNIEnv* env, jclass clazz) {
    sSk_DataBinder.init(env, clazz);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkData_nativeFinalize(JNIEnv* env, jobject clazz) {
    sSk_DataBinder.unbind(env, clazz);
}

extern "C" JNIEXPORT jint JNICALL Java_org_skia_SkData_getSize(JNIEnv* env, jobject thiz) {
    auto cppThiz = sSk_DataBinder.getCppObject(env, thiz);
    auto rawResult = sk_data_get_size(cppThiz.get());
    return rawResult;
}

extern "C" JNIEXPORT jbyteArray JNICALL Java_org_skia_SkData_getData(JNIEnv* env, jobject thiz) {
    auto cppThiz = sSk_DataBinder.getCppObject(env, thiz);
    auto rawResult = sk_data_get_data(cppThiz.get());
    auto dataSize = sk_data_get_size(cppThiz.get());
    jbyteArray result = env->NewByteArray(dataSize);
    env->SetByteArrayRegion(result, 0, dataSize, (const jbyte*)rawResult);
    return result;
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkData_nativeInit(JNIEnv* env, jobject thiz,
                                                                  jbyteArray src) {
    int length;
    auto binar1 = JNIHelper::as_unsigned_char_array2(env, src, length);
    auto rawResult = sk_data_new_with_copy(binar1.get(), (int)length);
    std::shared_ptr<sk_data_t> result(rawResult, [](sk_data_t* p) { sk_data_unref(p); });
    sSk_DataBinder.bind(env, result, thiz);
}

extern "C" JNIEXPORT jobject JNICALL Java_org_skia_SkData_newSubset(JNIEnv* env, jclass clazz,
                                                                    jobject src, jint offset,
                                                                    jint length) {
    std::shared_ptr<sk_data_t> bp1 = sSk_DataBinder.getCppObject(env, src);
    auto rawResult = sk_data_new_subset(bp1.get(), (int)offset, (int)length);
    std::shared_ptr<sk_data_t> result(rawResult, [](sk_data_t* p) { sk_data_unref(p); });
    return sSk_DataBinder.getJavaObject(env, result);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkImage_staticInit(JNIEnv* env, jclass clazz) {
    sSk_ImageBinder.init(env, clazz);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkImage_nativeFinalize(JNIEnv* env, jobject clazz) {
    sSk_ImageBinder.unbind(env, clazz);
}

extern "C" JNIEXPORT jobject JNICALL Java_org_skia_SkImage_encode(JNIEnv* env, jobject thiz) {
    auto cppThiz = sSk_ImageBinder.getCppObject(env, thiz);
    auto rawResult = sk_image_encode(cppThiz.get());
    std::shared_ptr<sk_data_t> result(rawResult, [](sk_data_t* p) { sk_data_unref(p); });
    return sSk_DataBinder.getJavaObject(env, result);
}

extern "C" JNIEXPORT jobject JNICALL Java_org_skia_SkImage_newRasterCopy(JNIEnv* env, jclass clazz,
                                                                         jobject info,
                                                                         jbyteArray pixels) {
    std::shared_ptr<sk_imageinfo_t> bp1 = sSk_ImageInfoBinder.getCppObject(env, info);
    int rowBytes;
    auto binar2 = JNIHelper::as_unsigned_char_array2(env, pixels, rowBytes);
    auto rawResult = sk_image_new_raster_copy(bp1.get(), binar2.get(), (int)rowBytes);
    std::shared_ptr<sk_image_t> result(rawResult, [](sk_image_t* p) { sk_image_unref(p); });
    return sSk_ImageBinder.getJavaObject(env, result);
}

extern "C" JNIEXPORT jobject JNICALL Java_org_skia_SkImage_newFromEncoded(JNIEnv* env, jclass clazz,
                                                                          jobject encoded,
                                                                          jobject subset) {
    std::shared_ptr<sk_data_t> bp1 = sSk_DataBinder.getCppObject(env, encoded);
    std::shared_ptr<sk_irect_t> bp2 = sSk_IRectBinder.getCppObject(env, subset);
    if (bp2.get()) {
        auto& props2 = sSk_IRectBinder.properties();
        if (props2.empty()) {
            props2.push_back(env->GetFieldID(sSk_IRectBinder.getJavaClass(), "left", "I"));
            props2.push_back(env->GetFieldID(sSk_IRectBinder.getJavaClass(), "top", "I"));
            props2.push_back(env->GetFieldID(sSk_IRectBinder.getJavaClass(), "right", "I"));
            props2.push_back(env->GetFieldID(sSk_IRectBinder.getJavaClass(), "bottom", "I"));
        }
        bp2->left = env->GetIntField(subset, props2[0]);
        bp2->top = env->GetIntField(subset, props2[1]);
        bp2->right = env->GetIntField(subset, props2[2]);
        bp2->bottom = env->GetIntField(subset, props2[3]);
    }
    auto rawResult = sk_image_new_from_encoded(bp1.get(), bp2.get());
    std::shared_ptr<sk_image_t> result(rawResult, [](sk_image_t* p) { sk_image_unref(p); });
    return sSk_ImageBinder.getJavaObject(env, result);
}

extern "C" JNIEXPORT jint JNICALL Java_org_skia_SkImage_getWidth(JNIEnv* env, jobject thiz) {
    auto cppThiz = sSk_ImageBinder.getCppObject(env, thiz);
    auto rawResult = sk_image_get_width(cppThiz.get());
    return rawResult;
}

extern "C" JNIEXPORT jint JNICALL Java_org_skia_SkImage_getHeight(JNIEnv* env, jobject thiz) {
    auto cppThiz = sSk_ImageBinder.getCppObject(env, thiz);
    auto rawResult = sk_image_get_height(cppThiz.get());
    return rawResult;
}

extern "C" JNIEXPORT jint JNICALL Java_org_skia_SkImage_getUniqueId(JNIEnv* env, jobject thiz) {
    auto cppThiz = sSk_ImageBinder.getCppObject(env, thiz);
    auto rawResult = sk_image_get_unique_id(cppThiz.get());
    return rawResult;
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkMatrix_staticInit(JNIEnv* env, jclass clazz) {
    sSk_MatrixBinder.init(env, clazz);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkMatrix_nativeFinalize(JNIEnv* env,
                                                                        jobject clazz) {
    sSk_MatrixBinder.unbind(env, clazz);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkMatrix_nativeInit(JNIEnv* env, jobject thiz) {
    auto rawResult = new sk_matrix_t();
    std::shared_ptr<sk_matrix_t> result(rawResult);
    sSk_MatrixBinder.bind(env, result, thiz);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkShader_staticInit(JNIEnv* env, jclass clazz) {
    sSk_ShaderBinder.init(env, clazz);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkShader_nativeFinalize(JNIEnv* env,
                                                                        jobject clazz) {
    sSk_ShaderBinder.unbind(env, clazz);
}

extern "C" JNIEXPORT jobject JNICALL Java_org_skia_SkShader_newLinearGradient(
        JNIEnv* env, jclass clazz, jobject startPoint, jobject endPoint, jintArray colors,
        jfloatArray colorPos, jint colorCount, jobject tileMode, jobject localMatrix) {
    std::shared_ptr<sk_point_t> bp1 = sSk_PointBinder.getCppObject(env, startPoint);
    if (bp1.get()) {
        auto& props1 = sSk_PointBinder.properties();
        if (props1.empty()) {
            props1.push_back(env->GetFieldID(sSk_PointBinder.getJavaClass(), "x", "F"));
            props1.push_back(env->GetFieldID(sSk_PointBinder.getJavaClass(), "y", "F"));
        }
        bp1->x = env->GetFloatField(startPoint, props1[0]);
        bp1->y = env->GetFloatField(startPoint, props1[1]);
    }
    std::shared_ptr<sk_point_t> bp2 = sSk_PointBinder.getCppObject(env, endPoint);
    if (bp2.get()) {
        auto& props2 = sSk_PointBinder.properties();
        if (props2.empty()) {
            props2.push_back(env->GetFieldID(sSk_PointBinder.getJavaClass(), "x", "F"));
            props2.push_back(env->GetFieldID(sSk_PointBinder.getJavaClass(), "y", "F"));
        }
        bp2->x = env->GetFloatField(endPoint, props2[0]);
        bp2->y = env->GetFloatField(endPoint, props2[1]);
    }
    int binarlen3;
    auto binar3 = JNIHelper::as_unsigned_int_array2(env, colors, binarlen3);
    int binarlen4;
    auto binar4 = JNIHelper::as_float_array2(env, colorPos, binarlen4);
    sk_shader_tilemode_t bp6 = sSk_Shader_TileModeEnum.getCppValue(env, tileMode);
    std::shared_ptr<sk_matrix_t> bp7 = sSk_MatrixBinder.getCppObject(env, localMatrix);
    if (bp7.get()) {
        auto& props7 = sSk_MatrixBinder.properties();
        if (props7.empty()) {
            props7.push_back(env->GetFieldID(sSk_MatrixBinder.getJavaClass(), "mat", "[F"));
        }
        {
            int proplen0;
            auto proparr0 = JNIHelper::as_float_array2(
                    env, (jfloatArray)env->GetObjectField(localMatrix, props7[0]), proplen0);
            memcpy(bp7->mat, proparr0.get(), proplen0 * sizeof(float));
        }
    }
    auto rawResult = sk_shader_new_linear_gradient(bp1.get(), bp2.get(), binar3.get(), binar4.get(),
                                                   (int)colorCount, bp6, bp7.get());
    std::shared_ptr<sk_shader_t> result(rawResult, [](sk_shader_t* p) { sk_shader_unref(p); });
    return sSk_ShaderBinder.getJavaObject(env, result);
}

extern "C" JNIEXPORT jobject JNICALL Java_org_skia_SkShader_newRadialGradient(
        JNIEnv* env, jclass clazz, jobject center, jfloat radius, jintArray colors,
        jfloatArray colorPos, jint colorCount, jobject tileMode, jobject localMatrix) {
    std::shared_ptr<sk_point_t> bp1 = sSk_PointBinder.getCppObject(env, center);
    if (bp1.get()) {
        auto& props1 = sSk_PointBinder.properties();
        if (props1.empty()) {
            props1.push_back(env->GetFieldID(sSk_PointBinder.getJavaClass(), "x", "F"));
            props1.push_back(env->GetFieldID(sSk_PointBinder.getJavaClass(), "y", "F"));
        }
        bp1->x = env->GetFloatField(center, props1[0]);
        bp1->y = env->GetFloatField(center, props1[1]);
    }
    int binarlen3;
    auto binar3 = JNIHelper::as_unsigned_int_array2(env, colors, binarlen3);
    int binarlen4;
    auto binar4 = JNIHelper::as_float_array2(env, colorPos, binarlen4);
    sk_shader_tilemode_t bp6 = sSk_Shader_TileModeEnum.getCppValue(env, tileMode);
    std::shared_ptr<sk_matrix_t> bp7 = sSk_MatrixBinder.getCppObject(env, localMatrix);
    if (bp7.get()) {
        auto& props7 = sSk_MatrixBinder.properties();
        if (props7.empty()) {
            props7.push_back(env->GetFieldID(sSk_MatrixBinder.getJavaClass(), "mat", "[F"));
        }
        {
            int proplen0;
            auto proparr0 = JNIHelper::as_float_array2(
                    env, (jfloatArray)env->GetObjectField(localMatrix, props7[0]), proplen0);
            memcpy(bp7->mat, proparr0.get(), proplen0 * sizeof(float));
        }
    }
    auto rawResult = sk_shader_new_radial_gradient(bp1.get(), (float)radius, binar3.get(),
                                                   binar4.get(), (int)colorCount, bp6, bp7.get());
    std::shared_ptr<sk_shader_t> result(rawResult, [](sk_shader_t* p) { sk_shader_unref(p); });
    return sSk_ShaderBinder.getJavaObject(env, result);
}

extern "C" JNIEXPORT jobject JNICALL Java_org_skia_SkShader_newSweepGradient(
        JNIEnv* env, jclass clazz, jobject center, jintArray colors, jfloatArray colorPos,
        jint colorCount, jobject localMatrix) {
    std::shared_ptr<sk_point_t> bp1 = sSk_PointBinder.getCppObject(env, center);
    if (bp1.get()) {
        auto& props1 = sSk_PointBinder.properties();
        if (props1.empty()) {
            props1.push_back(env->GetFieldID(sSk_PointBinder.getJavaClass(), "x", "F"));
            props1.push_back(env->GetFieldID(sSk_PointBinder.getJavaClass(), "y", "F"));
        }
        bp1->x = env->GetFloatField(center, props1[0]);
        bp1->y = env->GetFloatField(center, props1[1]);
    }
    int binarlen2;
    auto binar2 = JNIHelper::as_unsigned_int_array2(env, colors, binarlen2);
    int binarlen3;
    auto binar3 = JNIHelper::as_float_array2(env, colorPos, binarlen3);
    std::shared_ptr<sk_matrix_t> bp5 = sSk_MatrixBinder.getCppObject(env, localMatrix);
    if (bp5.get()) {
        auto& props5 = sSk_MatrixBinder.properties();
        if (props5.empty()) {
            props5.push_back(env->GetFieldID(sSk_MatrixBinder.getJavaClass(), "mat", "[F"));
        }
        {
            int proplen0;
            auto proparr0 = JNIHelper::as_float_array2(
                    env, (jfloatArray)env->GetObjectField(localMatrix, props5[0]), proplen0);
            memcpy(bp5->mat, proparr0.get(), proplen0 * sizeof(float));
        }
    }
    auto rawResult = sk_shader_new_sweep_gradient(bp1.get(), binar2.get(), binar3.get(),
                                                  (int)colorCount, bp5.get());
    std::shared_ptr<sk_shader_t> result(rawResult, [](sk_shader_t* p) { sk_shader_unref(p); });
    return sSk_ShaderBinder.getJavaObject(env, result);
}

extern "C" JNIEXPORT jobject JNICALL Java_org_skia_SkShader_newTwoPointConicalGradient(
        JNIEnv* env, jclass clazz, jobject startPoint, jfloat startRadius, jobject endPoint,
        jfloat endRadius, jintArray colors, jfloatArray colorPos, jint colorCount, jobject tileMode,
        jobject localMatrix) {
    std::shared_ptr<sk_point_t> bp1 = sSk_PointBinder.getCppObject(env, startPoint);
    if (bp1.get()) {
        auto& props1 = sSk_PointBinder.properties();
        if (props1.empty()) {
            props1.push_back(env->GetFieldID(sSk_PointBinder.getJavaClass(), "x", "F"));
            props1.push_back(env->GetFieldID(sSk_PointBinder.getJavaClass(), "y", "F"));
        }
        bp1->x = env->GetFloatField(startPoint, props1[0]);
        bp1->y = env->GetFloatField(startPoint, props1[1]);
    }
    std::shared_ptr<sk_point_t> bp3 = sSk_PointBinder.getCppObject(env, endPoint);
    if (bp3.get()) {
        auto& props3 = sSk_PointBinder.properties();
        if (props3.empty()) {
            props3.push_back(env->GetFieldID(sSk_PointBinder.getJavaClass(), "x", "F"));
            props3.push_back(env->GetFieldID(sSk_PointBinder.getJavaClass(), "y", "F"));
        }
        bp3->x = env->GetFloatField(endPoint, props3[0]);
        bp3->y = env->GetFloatField(endPoint, props3[1]);
    }
    int binarlen5;
    auto binar5 = JNIHelper::as_unsigned_int_array2(env, colors, binarlen5);
    int binarlen6;
    auto binar6 = JNIHelper::as_float_array2(env, colorPos, binarlen6);
    sk_shader_tilemode_t bp8 = sSk_Shader_TileModeEnum.getCppValue(env, tileMode);
    std::shared_ptr<sk_matrix_t> bp9 = sSk_MatrixBinder.getCppObject(env, localMatrix);
    if (bp9.get()) {
        auto& props9 = sSk_MatrixBinder.properties();
        if (props9.empty()) {
            props9.push_back(env->GetFieldID(sSk_MatrixBinder.getJavaClass(), "mat", "[F"));
        }
        {
            int proplen0;
            auto proparr0 = JNIHelper::as_float_array2(
                    env, (jfloatArray)env->GetObjectField(localMatrix, props9[0]), proplen0);
            memcpy(bp9->mat, proparr0.get(), proplen0 * sizeof(float));
        }
    }
    auto rawResult = sk_shader_new_two_point_conical_gradient(
            bp1.get(), (float)startRadius, bp3.get(), (float)endRadius, binar5.get(), binar6.get(),
            (int)colorCount, bp8, bp9.get());
    std::shared_ptr<sk_shader_t> result(rawResult, [](sk_shader_t* p) { sk_shader_unref(p); });
    return sSk_ShaderBinder.getJavaObject(env, result);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkMaskFilter_staticInit(JNIEnv* env, jclass clazz) {
    sSk_MaskFilterBinder.init(env, clazz);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkMaskFilter_nativeFinalize(JNIEnv* env,
                                                                            jobject clazz) {
    sSk_MaskFilterBinder.unbind(env, clazz);
}

extern "C" JNIEXPORT jobject JNICALL Java_org_skia_SkMaskFilter_newBlur(JNIEnv* env, jclass clazz,
                                                                        jobject blur,
                                                                        jfloat sigma) {
    sk_blurstyle_t bp1 = sSk_BlurStyleEnum.getCppValue(env, blur);
    auto rawResult = sk_maskfilter_new_blur(bp1, (float)sigma);
    std::shared_ptr<sk_maskfilter_t> result(rawResult,
                                            [](sk_maskfilter_t* p) { sk_maskfilter_unref(p); });
    return sSk_MaskFilterBinder.getJavaObject(env, result);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkPicture_staticInit(JNIEnv* env, jclass clazz) {
    sSk_PictureBinder.init(env, clazz);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkPicture_nativeFinalize(JNIEnv* env,
                                                                         jobject clazz) {
    sSk_PictureBinder.unbind(env, clazz);
}

extern "C" JNIEXPORT jint JNICALL Java_org_skia_SkPicture_getUniqueId(JNIEnv* env, jobject thiz) {
    auto cppThiz = sSk_PictureBinder.getCppObject(env, thiz);
    auto rawResult = sk_picture_get_unique_id(cppThiz.get());
    return rawResult;
}

extern "C" JNIEXPORT jobject JNICALL Java_org_skia_SkPicture_getBounds(JNIEnv* env, jobject thiz) {
    auto cppThiz = sSk_PictureBinder.getCppObject(env, thiz);
    sk_rect_t* rawResult = new sk_rect_t();
    *rawResult = sk_picture_get_bounds(cppThiz.get());
    std::shared_ptr<sk_rect_t> result(rawResult);
    return sSk_RectBinder.getJavaObject(env, result);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkIRect_staticInit(JNIEnv* env, jclass clazz) {
    sSk_IRectBinder.init(env, clazz);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkIRect_nativeFinalize(JNIEnv* env, jobject clazz) {
    sSk_IRectBinder.unbind(env, clazz);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkIRect_nativeInit(JNIEnv* env, jobject thiz) {
    auto rawResult = new sk_irect_t();
    std::shared_ptr<sk_irect_t> result(rawResult);
    sSk_IRectBinder.bind(env, result, thiz);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkPoint_staticInit(JNIEnv* env, jclass clazz) {
    sSk_PointBinder.init(env, clazz);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkPoint_nativeFinalize(JNIEnv* env, jobject clazz) {
    sSk_PointBinder.unbind(env, clazz);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkPoint_nativeInit(JNIEnv* env, jobject thiz) {
    auto rawResult = new sk_point_t();
    std::shared_ptr<sk_point_t> result(rawResult);
    sSk_PointBinder.bind(env, result, thiz);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkPictureRecorder_staticInit(JNIEnv* env,
                                                                             jclass clazz) {
    sSk_Picture_RecorderBinder.init(env, clazz);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkPictureRecorder_nativeFinalize(JNIEnv* env,
                                                                                 jobject clazz) {
    sSk_Picture_RecorderBinder.unbind(env, clazz);
}

extern "C" JNIEXPORT void JNICALL Java_org_skia_SkPictureRecorder_nativeInit(JNIEnv* env,
                                                                             jobject thiz) {
    auto rawResult = sk_picture_recorder_new();
    std::shared_ptr<sk_picture_recorder_t> result(
            rawResult, [](sk_picture_recorder_t* p) { sk_picture_recorder_delete(p); });
    sSk_Picture_RecorderBinder.bind(env, result, thiz);
}

extern "C" JNIEXPORT jobject JNICALL Java_org_skia_SkPictureRecorder_beginRecording(JNIEnv* env,
                                                                                    jobject thiz,
                                                                                    jobject rect) {
    auto cppThiz = sSk_Picture_RecorderBinder.getCppObject(env, thiz);
    std::shared_ptr<sk_rect_t> bp1 = sSk_RectBinder.getCppObject(env, rect);
    if (bp1.get()) {
        auto& props1 = sSk_RectBinder.properties();
        if (props1.empty()) {
            props1.push_back(env->GetFieldID(sSk_RectBinder.getJavaClass(), "left", "F"));
            props1.push_back(env->GetFieldID(sSk_RectBinder.getJavaClass(), "top", "F"));
            props1.push_back(env->GetFieldID(sSk_RectBinder.getJavaClass(), "right", "F"));
            props1.push_back(env->GetFieldID(sSk_RectBinder.getJavaClass(), "bottom", "F"));
        }
        bp1->left = env->GetFloatField(rect, props1[0]);
        bp1->top = env->GetFloatField(rect, props1[1]);
        bp1->right = env->GetFloatField(rect, props1[2]);
        bp1->bottom = env->GetFloatField(rect, props1[3]);
    }
    auto rawResult = sk_picture_recorder_begin_recording(cppThiz.get(), bp1.get());
    std::shared_ptr<sk_canvas_t> result(rawResult, [](sk_canvas_t* p) {});
    return sSk_CanvasBinder.getJavaObject(env, result);
}

extern "C" JNIEXPORT jobject JNICALL Java_org_skia_SkPictureRecorder_endRecording(JNIEnv* env,
                                                                                  jobject thiz) {
    auto cppThiz = sSk_Picture_RecorderBinder.getCppObject(env, thiz);
    auto rawResult = sk_picture_recorder_end_recording(cppThiz.get());
    std::shared_ptr<sk_picture_t> result(rawResult, [](sk_picture_t* p) { sk_picture_unref(p); });
    return sSk_PictureBinder.getJavaObject(env, result);
}
