/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"

#include <jni.h>

namespace {

void Canvas_Save(JNIEnv* env, jobject, jlong native_instance) {
    if (auto* canvas = reinterpret_cast<SkCanvas*>(native_instance)) {
        canvas->save();
    }
}

void Canvas_Restore(JNIEnv* env, jobject, jlong native_instance) {
    if (auto* canvas = reinterpret_cast<SkCanvas*>(native_instance)) {
        canvas->restore();
    }
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

}  // namespace

int register_androidkit_Canvas(JNIEnv* env) {
    static const JNINativeMethod methods[] = {
        {"nSave"            , "(J)V"     , reinterpret_cast<void*>(Canvas_Save)          },
        {"nRestore"         , "(J)V"     , reinterpret_cast<void*>(Canvas_Restore)       },
        {"nGetLocalToDevice", "(J)J"     , reinterpret_cast<void*>(Canvas_LocalToDevice) },
        {"nConcat"          , "(JJ)V"    , reinterpret_cast<void*>(Canvas_Concat)        },
        {"nConcat16f"       , "(J[F)V"   , reinterpret_cast<void*>(Canvas_Concat16f)     },
        {"nDrawColor"       , "(JFFFF)V" , reinterpret_cast<void*>(Canvas_DrawColor)     },
        {"nDrawRect"        , "(JFFFFJ)V", reinterpret_cast<void*>(Canvas_DrawRect)      },
    };

    const auto clazz = env->FindClass("org/skia/androidkit/Canvas");
    return clazz
        ? env->RegisterNatives(clazz, methods, SK_ARRAY_COUNT(methods))
        : JNI_ERR;
}
