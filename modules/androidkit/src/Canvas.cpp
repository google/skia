/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkShader.h"
#include "modules/androidkit/src/Utils.h"

#include <jni.h>

namespace {

jint Canvas_GetWidth(JNIEnv* env, jobject, jlong native_instance) {
    const auto* canvas = reinterpret_cast<const SkCanvas*>(native_instance);
    return canvas ? canvas->imageInfo().width() : 0;
}

jint Canvas_GetHeight(JNIEnv* env, jobject, jlong native_instance) {
    const auto* canvas = reinterpret_cast<const SkCanvas*>(native_instance);
    return canvas ? canvas->imageInfo().height() : 0;
}

jint Canvas_Save(JNIEnv* env, jobject, jlong native_instance) {
    if (auto* canvas = reinterpret_cast<SkCanvas*>(native_instance)) {
        return canvas->save();
    }
    return 0;
}

void Canvas_Restore(JNIEnv* env, jobject, jlong native_instance) {
    if (auto* canvas = reinterpret_cast<SkCanvas*>(native_instance)) {
        canvas->restore();
    }
}

void Canvas_RestoreToCount(JNIEnv* env, jobject, jlong native_instance, jint count) {
    if (auto* canvas = reinterpret_cast<SkCanvas*>(native_instance)) {
        canvas->restoreToCount(count);
    }
}

jint Canvas_SaveLayer(JNIEnv* env, jobject, jlong native_instance, jlong native_paint) {
    auto* paint = reinterpret_cast<SkPaint* >(native_paint);
    if (auto* canvas = reinterpret_cast<SkCanvas*>(native_instance)) {
        return canvas->saveLayer(nullptr, paint);
    }
    return 0;
}

jlong Canvas_LocalToDevice(JNIEnv* env, jobject, jlong native_instance) {
    if (auto* canvas = reinterpret_cast<SkCanvas*>(native_instance)) {
        SkM44* m = new SkM44(canvas->getLocalToDevice());
        return reinterpret_cast<jlong>(m);
    }
    return 0;
}

void Canvas_Concat(JNIEnv* env, jobject, jlong native_instance, jlong native_matrix) {
    auto* canvas = reinterpret_cast<SkCanvas*>(native_instance);
    auto* matrix = reinterpret_cast<SkM44*   >(native_matrix);

    if (canvas && matrix) {
        canvas->concat(*matrix);
    }
}

void Canvas_Concat16f(JNIEnv* env, jobject, jlong native_instance, jfloatArray jmatrix) {
    SkASSERT(env->GetArrayLength(jmatrix) == 16);

    if (auto* canvas = reinterpret_cast<SkCanvas*>(native_instance)) {
        auto* m = env->GetFloatArrayElements(jmatrix, nullptr);
        canvas->concat(SkM44::RowMajor(m));
        env->ReleaseFloatArrayElements(jmatrix, m, 0);
    }
}

void Canvas_Translate(JNIEnv* env, jobject, jlong native_instance,
                      jfloat tx, jfloat ty, jfloat tz) {
    if (auto* canvas = reinterpret_cast<SkCanvas*>(native_instance)) {
        canvas->concat(SkM44::Translate(tx, ty, tz));
    }
}

void Canvas_Scale(JNIEnv* env, jobject, jlong native_instance, jfloat sx, jfloat sy, jfloat sz) {
    if (auto* canvas = reinterpret_cast<SkCanvas*>(native_instance)) {
        canvas->concat(SkM44::Scale(sx, sy, sz));
    }
}

void Canvas_ClipPath(JNIEnv* env, jobject, jlong native_instance, jlong native_path,
                                           jint native_clipOp, jboolean doAA) {
    if (auto* canvas = reinterpret_cast<SkCanvas*>(native_instance)) {
        if (auto* path = reinterpret_cast<SkPath*>(native_path)) {
            canvas->clipPath(*path, static_cast<SkClipOp>(native_clipOp), doAA);
        }
    }
}

void Canvas_ClipRect(JNIEnv* env, jobject, jlong native_instance, jfloat l, jfloat t, jfloat r, jfloat b,
                                           jint native_clipOp, jboolean doAA) {
    if (auto* canvas = reinterpret_cast<SkCanvas*>(native_instance)) {
        canvas->clipRect(SkRect::MakeLTRB(l, t, r, b), static_cast<SkClipOp>(native_clipOp), doAA);
    }
}

void Canvas_ClipRRect(JNIEnv* env, jobject, jlong native_instance, jfloat l, jfloat t, jfloat r, jfloat b,
                                                                   jfloat xRad, jfloat yRad,
                                                                   jint native_clipOp, jboolean doAA) {
    if (auto* canvas = reinterpret_cast<SkCanvas*>(native_instance)) {
        canvas->clipRRect(SkRRect::MakeRectXY(SkRect::MakeLTRB(l, t, r, b), xRad, yRad),
                          static_cast<SkClipOp>(native_clipOp), doAA);
    }
}

void Canvas_ClipShader(JNIEnv* env, jobject, jlong native_instance, jlong native_shader, jint native_clipOp) {
    if (auto* canvas = reinterpret_cast<SkCanvas*>(native_instance)) {
        if (auto* shader = reinterpret_cast<SkShader*>(native_shader)) {
            canvas->clipShader(sk_ref_sp(shader), static_cast<SkClipOp>(native_clipOp));
        }
    }
}

void Canvas_DrawColor(JNIEnv* env, jobject, jlong native_instance,
                      float r, float g, float b, float a) {
    if (auto* canvas = reinterpret_cast<SkCanvas*>(native_instance)) {
        canvas->drawColor(SkColor4f{r, g, b, a});
    }
}

void Canvas_DrawRect(JNIEnv* env, jobject, jlong native_instance,
                     jfloat left, jfloat top, jfloat right, jfloat bottom,
                     jlong native_paint) {
    auto* canvas = reinterpret_cast<SkCanvas*>(native_instance);
    auto* paint  = reinterpret_cast<SkPaint* >(native_paint);
    if (canvas && paint) {
        canvas->drawRect(SkRect::MakeLTRB(left, top, right, bottom), *paint);
    }
}

void Canvas_DrawImage(JNIEnv* env, jobject, jlong native_instance, jlong native_image,
                      jfloat x, jfloat y,
                      jint sampling_desc, jfloat sampling_b, jfloat sampling_c) {
    auto* canvas = reinterpret_cast<SkCanvas*>(native_instance);
    auto*  image = reinterpret_cast<SkImage *>(native_image);

    if (canvas && image) {
        canvas->drawImage(image, x, y,
            androidkit::utils::SamplingOptions(sampling_desc, sampling_b, sampling_c));
    }
}

void Canvas_DrawPath(JNIEnv* env, jobject, jlong native_instance, jlong native_path,
                     jlong native_paint) {
    auto* canvas = reinterpret_cast<SkCanvas*>(native_instance);
    auto* path  = reinterpret_cast<SkPath* >(native_path);
    auto* paint  = reinterpret_cast<SkPaint* >(native_paint);
    if (canvas && paint && path) {
        canvas->drawPath(*path, *paint);
    }
}

// jPos: a composite array in the form of [x1, y1, x2, y2, ... ,xn, yn]
// callers of this function check should throw IllegalArgumentException in Java
void Canvas_DrawGlyphs(JNIEnv* env, jobject, jlong native_instance, jcharArray jglyphs,
                                             jfloatArray jPos, jfloat xOrigin, jfloat yOrigin,
                                             jlong native_font, jlong native_paint) {
    auto* canvas = reinterpret_cast<SkCanvas*>(native_instance);
    auto* font = reinterpret_cast<SkFont*>(native_font);
    auto* paint  = reinterpret_cast<SkPaint* >(native_paint);
    if (canvas && font && paint) {
        int count = env->GetArrayLength(jglyphs);
        auto* compositePositions = env->GetFloatArrayElements(jPos, nullptr);
        auto* positions = reinterpret_cast<SkPoint*>(compositePositions);
        auto* glyphs = env->GetCharArrayElements(jglyphs, nullptr);
        canvas->drawGlyphs(count, glyphs, positions, {xOrigin, yOrigin}, *font, *paint);

        env->ReleaseCharArrayElements(jglyphs, glyphs, 0);
        env->ReleaseFloatArrayElements(jPos, compositePositions, 0);
    }
}

}  // namespace

int register_androidkit_Canvas(JNIEnv* env) {
    static const JNINativeMethod methods[] = {
        {"nGetWidth"        , "(J)I"           , reinterpret_cast<void*>(Canvas_GetWidth)      },
        {"nGetHeight"       , "(J)I"           , reinterpret_cast<void*>(Canvas_GetHeight)     },
        {"nSave"            , "(J)I"           , reinterpret_cast<void*>(Canvas_Save)          },
        {"nSaveLayer"       , "(JJ)I"          , reinterpret_cast<void*>(Canvas_SaveLayer)     },
        {"nRestore"         , "(J)V"           , reinterpret_cast<void*>(Canvas_Restore)       },
        {"nRestoreToCount"  , "(JI)V"          , reinterpret_cast<void*>(Canvas_RestoreToCount)},
        {"nGetLocalToDevice", "(J)J"           , reinterpret_cast<void*>(Canvas_LocalToDevice) },
        {"nConcat"          , "(JJ)V"          , reinterpret_cast<void*>(Canvas_Concat)        },
        {"nConcat16f"       , "(J[F)V"         , reinterpret_cast<void*>(Canvas_Concat16f)     },
        {"nTranslate"       , "(JFFF)V"        , reinterpret_cast<void*>(Canvas_Translate)     },
        {"nScale"           , "(JFFF)V"        , reinterpret_cast<void*>(Canvas_Scale)         },
        {"nClipPath"        , "(JJIZ)V"        , reinterpret_cast<void*>(Canvas_ClipPath)      },
        {"nClipRect"        , "(JFFFFIZ)V"     , reinterpret_cast<void*>(Canvas_ClipRect)      },
        {"nClipRRect"       , "(JFFFFFFIZ)V"   , reinterpret_cast<void*>(Canvas_ClipRRect)     },
        {"nClipShader"      , "(JJI)V"         , reinterpret_cast<void*>(Canvas_ClipShader)    },
        {"nDrawColor"       , "(JFFFF)V"       , reinterpret_cast<void*>(Canvas_DrawColor)     },
        {"nDrawRect"        , "(JFFFFJ)V"      , reinterpret_cast<void*>(Canvas_DrawRect)      },
        {"nDrawImage"       , "(JJFFIFF)V"     , reinterpret_cast<void*>(Canvas_DrawImage)     },
        {"nDrawPath"        , "(JJJ)V"         , reinterpret_cast<void*>(Canvas_DrawPath)      },
        {"nDrawGlyphs"      , "(J[C[FFFJJ)V", reinterpret_cast<void*>(Canvas_DrawGlyphs)    },

    };

    const auto clazz = env->FindClass("org/skia/androidkit/Canvas");
    return clazz
        ? env->RegisterNatives(clazz, methods, SK_ARRAY_COUNT(methods))
        : JNI_ERR;
}
